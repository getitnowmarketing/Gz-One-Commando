/*
 * Processing for UI of USB event
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/connector.h>
#include <linux/workqueue.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>

#include "comm_usb.h"

#define DF_TIMEOUT_CNT		120
#define DF_CONCAT_SIZE		(10 * 1024)


#define CONFIG_COMM_USB_USE_SPIN_LOCK

/**
 * @brief Status of USB mode
 */
enum {
	EN_STATE_IDLE = 0,		/**< Idle */
	EN_STATE_CONNECTING,		/**< Connecting */
	EN_STATE_CONNECTED,		/**< Connected */
	EN_STATE_OPENING,		/**< Opening */
	EN_STATE_OPENED,		/**< Opened */
	EN_STATE_CLOSING,		/**< Closing */
	EN_STATE_CLOSED,		/**< Closed */
	EN_STATE_DISCONNECTING,		/**< Disconnecting */
	EN_STATE_READING		/**< Reading */
};

/**< Request command */
static struct usb_comm_request  	grequest;


/**< State */
static u32	state = EN_STATE_IDLE;

/**< Prev state */
static u32	prev_state = EN_STATE_IDLE;

/**< User data */
static void *	gpuser = NULL;

/**< Callback */
static void 	(*pcb)(struct usb_comm_response *, void *);

/**< Callback ID */
static struct cb_id 	comm_usb_id = { CN_NETLINK_USERS, CN_VAL_PROC };

/**< Socket name */
static char comm_usb_name[] = "comm_usb";

/**< Socket discriptor for USB driver */
static struct sock *nls;

/**< Timer handle */
static struct timer_list comm_usb_timer;

/**< Timer handle */
static u32	retry = 0;

/**< Read buffer */
static u8 *	read_buffer = NULL;

/**< Read offset */
static u32	buff_offset = 0;

static spinlock_t	comm_usb_lock;

static u8	comm_usb_interrupt = 0;   

static int comm_usb_send(unsigned char * data, u32 len);


extern int cdrom_close(void);


int comm_usb_cdrom_interruptible(void)
{
	comm_usb_interrupt = 1;    
	return 0;
}

static void comm_usb_post_callback(struct usb_comm_response * response, void * user)
{

	if (!response) {
		return;
	}

	pcb(response, user);
}

static void comm_usb_read_callback(struct usb_comm_response_body_read_concat * read_concat, struct usb_comm_response * response)
{
	u8 *	pdata;


	if (comm_usb_interrupt) {
		printk(KERN_ERR "%s: read interrupted\n", __func__);
		cdrom_close();
		return;
	}


	pdata = (u8 *) read_concat + sizeof(struct usb_comm_response_body_read_concat);

	memcpy(&read_buffer[buff_offset], pdata, read_concat->size);
	buff_offset += read_concat->size;

	if (read_concat->concat == read_concat->total) {
		response->body.read.bodysize = read_concat->bodysize;
		response->body.read.size	 = read_concat->size;
		memcpy(response->body.read.data, read_buffer, read_concat->bodysize);
		if (read_buffer) {
			kfree(read_buffer);
			read_buffer = NULL;
			buff_offset = 0;
		}
        comm_usb_post_callback(response, gpuser);
		return;
	}
}

/**
 * @brief Connector callback.
 * @param msg cn_msg
 * @param nsp netlink_skb_parms
 *
 */
static void comm_usb_callback(struct cn_msg *msg, struct netlink_skb_parms *nsp)
{
	struct usb_comm_response * response = NULL;
	void * pdata ;
	struct usb_comm_response_body_read_concat * read_concat;


	retry = 0;


	response = (struct usb_comm_response *) msg->data;

	if (!response) {
		if (read_buffer) {
			kfree(read_buffer);
			read_buffer = NULL;
			buff_offset = 0;
		}
		state = prev_state;
		comm_usb_post_callback(response, gpuser);
		return;
	}

	if (response->status != STATUS_SUCCESS) {
		if (read_buffer) {
			kfree(read_buffer);
			read_buffer = NULL;
			buff_offset = 0;
		}
		state = prev_state;
		comm_usb_post_callback(response, gpuser);
		return;
	}

	if (response->head.type == TYPE_RESPONSE_READ) {
		pdata = (void *) response;
		pdata += sizeof(struct usb_comm_header) + sizeof(u32);
		read_concat = pdata;
		comm_usb_read_callback(read_concat, response);
		return;
	}

	if (response->head.type != TYPE_RESPONSE_READ) {
		printk(KERN_DEBUG "%s: type=%d\n", __func__, response->head.type);
	}

	prev_state = state;
	switch (response->head.type) {
	case TYPE_RESPONSE_OPEN:
		state = EN_STATE_OPENED;
		break;
	case TYPE_RESPONSE_CLOSE:
		state = EN_STATE_CLOSED;
		break;
	case TYPE_RESPONSE_CONNECT:
		state = EN_STATE_CONNECTED;
		break;
	case TYPE_RESPONSE_DISCONNECT:
		state = EN_STATE_IDLE;
		break;
	default:
		return;
	}

	if (response->head.type != TYPE_RESPONSE_READ) {
		printk(KERN_DEBUG "%s: call comm_usb_post_callback\n", __func__);
	}

	comm_usb_post_callback(response, gpuser);

	if (response->head.type != TYPE_RESPONSE_READ) {
		printk(KERN_DEBUG "%s: called comm_usb_post_callback\n", __func__);
	}
}

/**
 * @brief Timer processing
 * @param __data
 */
static void comm_usb_timeout(unsigned long __data)
{
	int	ret;


	retry++;


	if (retry == DF_TIMEOUT_CNT) {
		state = prev_state;
    	printk(KERN_ERR "%s: timeout !! retry=%d\n", __func__, retry);
		return;
	}


	ret = comm_usb_send((unsigned char *)&grequest, grequest.head.size);
		switch (ret) {
		case 0:
			break;
		case -ESRCH:
			mod_timer(&comm_usb_timer, jiffies + msecs_to_jiffies(1000));
			ret = 0;
			break;
		default:
			return;
	}
}

/**
 * @brief Data transmission processing to new vold.
 * @param data Transmission data
 * @param len Data length
 * @return Zero is a success. It is a failure excluding Zero.
 */
static int comm_usb_send(unsigned char * data, u32 len)
{
        struct cn_msg *m;
	int err = 0;

        m = kzalloc(sizeof(*m) + len, GFP_ATOMIC);
        if (m) {
                memcpy(&m->id, &comm_usb_id, sizeof(m->id));
				m->seq = 0;
                m->len = len;
                memcpy(m + 1, data, m->len);
                err = cn_netlink_send(m, 0, GFP_ATOMIC);
                kfree(m);
        }
	return err;
}

/**
 * @brief Data transmission processing to new vold.
 * @param work Workqueue
 */
static void comm_usb_exec(struct usb_comm_request * request)
{
	int	ret = 0;

	switch (request->head.type) {
	case TYPE_REQUEST_OPEN:
		if ((state != EN_STATE_CONNECTED) &&
			(state != EN_STATE_CLOSED)) {
			printk(KERN_ERR "%s: Invalid state (TYPE_REQUEST_OPEN) state=%d\n",
				__func__, state);
			goto proc_end;
		}
		break;
	case TYPE_REQUEST_CLOSE:
		if (state != EN_STATE_OPENED) {
			printk(KERN_ERR "%s: Invalid state (TYPE_REQUEST_CLOSE) state=%d\n",
				__func__, state);
			goto proc_end;
		}
		break;
	case TYPE_REQUEST_READ:
		if (state != EN_STATE_OPENED) {
			printk(KERN_ERR "%s: Invalid state (TYPE_REQUEST_READ) state=%d\n",
				__func__, state);
			goto proc_end;
		}
		break;
	case TYPE_REQUEST_CONNECT:
		if (state != EN_STATE_IDLE) {
			printk(KERN_ERR "%s: Invalid state (TYPE_REQUEST_CONNECT) bef_state=%d\n",
				__func__, state);

		    state = EN_STATE_IDLE;
			printk(KERN_ERR "%s: Invalid state (TYPE_REQUEST_CONNECT) aft_state=%d\n",
				__func__, state);

		}
		break;
	case TYPE_REQUEST_DISCONNECT:
		if ((state != EN_STATE_CONNECTED) &&
			(state != EN_STATE_OPENED) &&
			(state != EN_STATE_CLOSED)) {
			printk(KERN_ERR "%s: Invalid state (TYPE_REQUEST_DISCONNECT) state=%d\n",
				__func__, state);
			goto proc_end;
		}
		break;
	default:
		printk(KERN_ERR "%s: Invalid state (OTHER) state=%d\n",
			__func__, state);
		goto proc_end;
	}

	ret = comm_usb_send((unsigned char *) request,
			request->head.size);
	switch (ret) {
	case 0:
		break;
	case -ESRCH:
		memcpy(&grequest, request, sizeof(grequest));
		mod_timer(&comm_usb_timer, jiffies + msecs_to_jiffies(1000));
		ret = 0;
		retry = 0;
		printk(KERN_ERR "%s: comm_usb_send() ret=-ESRCH\n", 
			__func__);
		break;
	default:
		printk(KERN_ERR "%s: comm_usb_send() ret=%d\n", 
			__func__, ret);
		goto proc_end;
	}

	prev_state = state;

	switch (request->head.type) {
	case TYPE_REQUEST_OPEN:
		state = EN_STATE_OPENED;
		break;
	case TYPE_REQUEST_CLOSE:
		state = EN_STATE_CLOSED;
		break;
	case TYPE_REQUEST_READ:
		break;
	case TYPE_REQUEST_CONNECT:
		state = EN_STATE_CONNECTED;
		break;
	case TYPE_REQUEST_DISCONNECT:
		state = EN_STATE_IDLE;
		break;
	default:
		goto proc_end;
	}

	if (request->head.type != TYPE_REQUEST_READ) {
		printk(KERN_DEBUG "%s: Send request for vold is success. type=%d\n",
			__func__, request->head.type);
	}

proc_end:
	return;
}

/**
 * @brief Request of comm_usb
 * @param request Request parameters
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_request(struct usb_comm_request * request)
{
	if (state == -1) {
		printk(KERN_ERR "%s: wait\n", __func__);
		return -EAGAIN;
	}

	if (!request) {
		return -EINVAL;
	}

	comm_usb_exec(request);
	return 0;
}

/**
 * @brief Connected request
 * @param boot Boot Status
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_request_connect(u32 boot)
{
	struct usb_comm_request	request;
	int			ret = 0;
	unsigned long 		flags;

	spin_lock_irqsave(&comm_usb_lock, flags);

	memset(&request, 0, sizeof(request));
	request.head.size 		= 12;
	request.head.type 		= TYPE_REQUEST_CONNECT;
	request.body.connect.boot 	= boot;

	ret = comm_usb_request(&request);
	if (ret) {
		printk(KERN_ERR "%s: comm_usb_request() ret=%d\n",
			__func__, ret);
		goto err_out;
	}

	printk(KERN_ERR "%s: accept connect request %d\n", __func__, boot);

err_out:

	spin_unlock_irqrestore(&comm_usb_lock, flags);

	return ret;
}

/**
 * @param mode USB mode
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_request_open(u32 mode)
{
	struct usb_comm_request	request;
	int			ret = 0;
	unsigned long 		flags;

	spin_lock_irqsave(&comm_usb_lock, flags);

	comm_usb_interrupt = 0;    

	memset(&request, 0, sizeof(request));
	request.head.size 	= 12;
	request.head.type 	= TYPE_REQUEST_OPEN;
	request.body.open.mode	= mode;
	ret = comm_usb_request(&request);
	if (ret) {
		printk(KERN_ERR "%s: comm_usb_request() ret=%d\n",
			__func__, ret);
		goto err_out;
	}
	printk(KERN_ERR "%s: accept open request\n", __func__);

err_out:
	spin_unlock_irqrestore(&comm_usb_lock, flags);
	return ret;
}

/**
 * @brief Closed request
 * @param mode USB mode
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_request_close(u32 mode)
{
	struct usb_comm_request	request;
	int			ret = 0;
	unsigned long 		flags;

	spin_lock_irqsave(&comm_usb_lock, flags);

	memset(&request, 0, sizeof(request));
	request.head.size 	= 12;
	request.head.type 	= TYPE_REQUEST_CLOSE;
	request.body.close.mode	= mode;
	ret = comm_usb_request(&request);
	if (ret) {
		printk(KERN_ERR "%s: comm_usb_request() ret=%d\n",
			__func__, ret);
		goto err_out;
	}
	printk(KERN_ERR "%s: accept close request\n", __func__);

err_out:
	spin_unlock_irqrestore(&comm_usb_lock, flags);
	return ret;
	}

/**
 * @brief Disconnect request
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_request_disconnect(void)
{
	struct usb_comm_request	request;
	int			ret = 0;
	unsigned long 		flags;

	spin_lock_irqsave(&comm_usb_lock, flags);


	cdrom_close();
	printk(KERN_DEBUG "%s: cdrom_close was called.\n", __func__);


	memset(&request, 0, sizeof(request));
	request.head.size 	= 8;
	request.head.type 	= TYPE_REQUEST_DISCONNECT;
	ret = comm_usb_request(&request);
	if (ret) {
		printk(KERN_ERR "%s: comm_usb_request() ret=%d\n",
			__func__, ret);
		goto err_out;
	}
	printk(KERN_ERR "%s: accept disconnect request\n", __func__);

err_out:
	spin_unlock_irqrestore(&comm_usb_lock, flags);
	return ret;
}

/**
 * @brief Read request
 * @param 
 * @param
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_request_read(u32 offset, u32 size)
{
	struct usb_comm_request	request;
	int			ret = 0;
	int			concat;
	int			i;
	unsigned long 		flags;

	spin_lock_irqsave(&comm_usb_lock, flags);

	if (read_buffer) {
		printk(KERN_ERR "%s: read_buffer=%p\n", __func__, read_buffer);
		kfree(read_buffer);
		read_buffer = NULL;
	}

	buff_offset = 0;

	read_buffer = (u8 *)kmalloc(size, GFP_KERNEL);
	if (!read_buffer) {
		ret = -ENOMEM;
		printk(KERN_ERR "%s: comm_usb_request() ret=%d\n",
			__func__, ret);
		goto err_out;
	}

	concat = (size / DF_CONCAT_SIZE) + 1;

	for (i = 0; i < concat; i++) {
		memset(&request, 0, sizeof(request));
		request.head.size		= 24;
		request.head.type		= TYPE_REQUEST_READ;
		request.body.read.offset	= offset + (DF_CONCAT_SIZE * i);
		if (concat == 1) {
		request.body.read.size		= size;
		}
		else {
			if (concat == i + 1) {
				request.body.read.size	= size % DF_CONCAT_SIZE;
			}
			else {
				request.body.read.size	= DF_CONCAT_SIZE;
			}
		}
		request.body.read.concat	= i + 1;
		request.body.read.total		= concat;

		ret = comm_usb_request(&request);
		if (ret) {
			printk(KERN_ERR "%s: comm_usb_request() ret=%d\n",
				__func__, ret);
			goto err_out;
	}
	}

	spin_unlock_irqrestore(&comm_usb_lock, flags);
	return 0;

err_out:
	if (read_buffer) {
		kfree(read_buffer);
		read_buffer = NULL;
		buff_offset = 0;
	}

	spin_unlock_irqrestore(&comm_usb_lock, flags);
	return ret;
}


/**
 * @brief Initialization of comm_usb
 * @return Zero is a success. It is a failure excluding Zero.
 */
int comm_usb_init(void (* pcb_func)(struct usb_comm_response *, void *), void *puser)
{
	int	ret = 0;
	
	if (!pcb_func) {
		return -EINVAL;
	}

	pcb = pcb_func;
	gpuser 	= puser;

	spin_lock_init(&comm_usb_lock);

	ret = cn_add_callback(&comm_usb_id, comm_usb_name, comm_usb_callback);
	if (ret) {
		goto err_out;
	}

	comm_usb_id.val++;
	
	ret = cn_add_callback(&comm_usb_id, comm_usb_name, comm_usb_callback);
		if (ret) {
		goto err_out;
	}

	setup_timer(&comm_usb_timer, comm_usb_timeout, 0);

	state = EN_STATE_IDLE;
	prev_state = EN_STATE_IDLE;

	return 0;

err_out:
	if (nls && nls->sk_socket) {
		sock_release(nls->sk_socket);
	}

	return ret;
}

/**
 * @brief End of comm_usb
 */
void comm_usb_end(void)
{

	del_timer_sync(&comm_usb_timer);

	cn_del_callback(&comm_usb_id);
	comm_usb_id.val--;
	cn_del_callback(&comm_usb_id);

	if (nls && nls->sk_socket)
		sock_release(nls->sk_socket);

	if (read_buffer) {
		kfree(read_buffer);
		read_buffer = NULL;
		buff_offset = 0;
	}

	pcb 	= NULL;
	gpuser 	= NULL;
}


int comm_usb_is_connect_polling(void)
{
	if (!retry) {
		printk(KERN_ERR "%s: Connect polling is not done.\n", __func__);
		return 0;
	}

	printk(KERN_ERR "%s: Connect polling is done. retry=%d\n", __func__, retry);
	del_timer_sync(&comm_usb_timer);
	state = prev_state;
	retry = 0;
	return 1;
}


