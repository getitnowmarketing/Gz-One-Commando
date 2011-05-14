
/*
 * Gadget Driver for Android MTP
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
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


/* #define DEBUG */
/* #define VERBOSE_DEBUG */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>

#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include <linux/usb/android_composite.h>
#include <linux/usb/f_mtp.h>
#include "comm_usb.h"

#define BULK_BUFFER_SIZE           16384
#define INTR_BUFFER_SIZE           64

/* number of rx and tx requests to allocate */
#define RX_REQ_MAX 2
#define TX_REQ_MAX 4

/* String IDs */
#define INTERFACE_STRING_INDEX	0

/* ID for Microsoft MTP OS String */
#define MTP_OS_STRING_ID   0xEE

/* values for mtp_dev.state */
#define STATE_OFFLINE               0   /* initial state, disconnected */
#define STATE_READY                 1   /* ready for userspace calls */
#define STATE_BUSY                  2   /* processing userspace calls */
#define STATE_CANCELED              3   /* transaction canceled by host */
#define STATE_ERROR                 4   /* error from completion routine */

/* MTP class reqeusts */
#define MTP_REQ_CANCEL              0x64
#define MTP_REQ_GET_EXT_EVENT_DATA  0x65
#define MTP_REQ_RESET               0x66
#define MTP_REQ_GET_DEVICE_STATUS   0x67

/* constants for device status */
#define MTP_RESPONSE_OK             0x2001
#define MTP_RESPONSE_DEVICE_BUSY    0x2019

/* android_mtp_control */
#define CTL_VBUS_OFFLINE            0xff
#define CTL_PWR_OFFLINE             0
#define CTL_PWR_ONLINE              1
#define CTL_CSR_CANCEL_REQUEST      2
#define CTL_CSR_GET_DEVICE_STATUS   3
#define CTL_CSR_DEVICE_RESET        4

extern int msm72k_udc_usb_state;
extern int android_get_product_id(void);

static int control_active = 1;
static int mtp_active = 0;

struct mtp_dev {
	struct usb_function function;
	struct usb_composite_dev *cdev;
	spinlock_t lock;

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;
	struct usb_ep *ep_intr;

	int state;
	int control_state;

	/* synchronize access to our device file */
	atomic_t read_excl;
	atomic_t write_excl;
	atomic_t open_excl;

	struct list_head tx_idle;

	wait_queue_head_t read_wq;
	wait_queue_head_t write_wq;
	wait_queue_head_t intr_wq;
	struct usb_request *rx_req[RX_REQ_MAX];
	struct usb_request *intr_req;
	int rx_done;

	/* synchronize access to interrupt endpoint */
	struct mutex intr_mutex;
	/* true if interrupt endpoint is busy */
	int intr_busy;
	
	wait_queue_head_t wait_wq; /* WAIT */

};

static struct usb_interface_descriptor mtp_interface_desc = {
	.bLength                = USB_DT_INTERFACE_SIZE,
	.bDescriptorType        = USB_DT_INTERFACE,
	.bInterfaceNumber       = 0,
	.bNumEndpoints          = 3,
	.bInterfaceClass        = 0xFF,
	.bInterfaceSubClass     = 0xFF,
	.bInterfaceProtocol     = 0xFF,
};

static struct usb_endpoint_descriptor mtp_highspeed_in_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor mtp_highspeed_out_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize         = __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor mtp_fullspeed_in_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor mtp_fullspeed_out_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_OUT,
	.bmAttributes           = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor mtp_intr_desc = {
	.bLength                = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType        = USB_DT_ENDPOINT,
	.bEndpointAddress       = USB_DIR_IN,
	.bmAttributes           = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize         = __constant_cpu_to_le16(INTR_BUFFER_SIZE),
    .bInterval              = 8,
};

static struct usb_descriptor_header *fs_mtp_descs[] = {
	(struct usb_descriptor_header *) &mtp_interface_desc,
	(struct usb_descriptor_header *) &mtp_fullspeed_in_desc,
	(struct usb_descriptor_header *) &mtp_fullspeed_out_desc,
	(struct usb_descriptor_header *) &mtp_intr_desc,
	NULL,
};

static struct usb_descriptor_header *hs_mtp_descs[] = {
	(struct usb_descriptor_header *) &mtp_interface_desc,
	(struct usb_descriptor_header *) &mtp_highspeed_in_desc,
	(struct usb_descriptor_header *) &mtp_highspeed_out_desc,
	(struct usb_descriptor_header *) &mtp_intr_desc,
	NULL,
};

/* Microsoft MTP OS String */
static u8 mtp_os_string[] = {
	18, /* sizeof(mtp_os_string) */
	USB_DT_STRING,
	/* Signature field: "MSFT100" */
	'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
	/* vendor code */
	0xFE,
	/* padding */
	0
};

/* Microsoft Extended Configuration Descriptor Header Section */
struct mtp_ext_config_desc_header {
	__le32	dwLength;
	__u16	bcdVersion;
	__le16	wIndex;
	__u8	bCount;
	__u8	reserved[7];
};

/* Microsoft Extended Configuration Descriptor Function Section */
struct mtp_ext_config_desc_function {
	__u8	bFirstInterfaceNumber;
	__u8	bInterfaceCount;
	__u8	compatibleID[8];
	__u8	subCompatibleID[8];
	__u8	reserved[6];
};

/* MTP Extended Configuration Descriptor */
struct {
	struct mtp_ext_config_desc_header	header;
	struct mtp_ext_config_desc_function    function;
} mtp_ext_config_desc = {
	.header = {
		.dwLength = __constant_cpu_to_le32(sizeof(mtp_ext_config_desc)),
		.bcdVersion = __constant_cpu_to_le16(0x0100),
		.wIndex = __constant_cpu_to_le16(4),
		.bCount = __constant_cpu_to_le16(1),
	},
	.function = {
		.bFirstInterfaceNumber = 0,
		.bInterfaceCount = 1,
		.compatibleID = { 'M', 'T', 'P' },
	},
};

/* Microsoft Extended Properties Descriptor Header Section */
struct mtp_ext_property_desc_header {
	__le32  dwLength;
	__u16   bcdVersion;
	__le16  wIndex;
	__u16   wCount;
};

/* MTP Extended Properties Descriptor */
struct {
	struct mtp_ext_property_desc_header	header;
} mtp_ext_property_desc = {
	.header = {
	.dwLength = __constant_cpu_to_le32(sizeof(mtp_ext_property_desc)),
	.bcdVersion = __constant_cpu_to_le16(0x0100),
	.wIndex = __constant_cpu_to_le16(5),
	.wCount = __constant_cpu_to_le16(0),
	}
};

struct mtp_device_status {
	__le16	wLength;
	__le16	wCode;
};


/* temporary variable used between mtp_open() and mtp_gadget_bind() */
static struct mtp_dev *_mtp_dev;

static inline struct mtp_dev *func_to_dev(struct usb_function *f)
{
	return container_of(f, struct mtp_dev, function);
}


static struct usb_request *mtp_request_new(struct usb_ep *ep, int buffer_size)
{
	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);
	if (!req)
		return NULL;

	/* now allocate buffers for the requests */
	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (!req->buf) {
		usb_ep_free_request(ep, req);
		return NULL;
	}

	return req;
}

static void mtp_request_free(struct usb_request *req, struct usb_ep *ep)
{
	if (req) {
		kfree(req->buf);
		usb_ep_free_request(ep, req);
	}
}

static inline int _lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1) {
		return 0;
	} else {
		atomic_dec(excl);
		return -1;
	}
}

static inline void _unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

/* add a request to the tail of a list */
static void req_put(struct mtp_dev *dev, struct list_head *head,
		struct usb_request *req)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	list_add_tail(&req->list, head);
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* remove a request from the head of a list */
static struct usb_request *req_get(struct mtp_dev *dev, struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req;

	spin_lock_irqsave(&dev->lock, flags);
	if (list_empty(head)) {
		req = 0;
	} else {
		req = list_first_entry(head, struct usb_request, list);
		list_del(&req->list);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
	return req;
}

static void set_control_state(struct mtp_dev *dev, const int state)
{
	if(dev){
		if(state == CTL_VBUS_OFFLINE) {
			if(dev->control_state == CTL_PWR_OFFLINE)
				dev->control_state = CTL_VBUS_OFFLINE;
			else
				dev->control_state = CTL_PWR_OFFLINE;
		}else{
			dev->control_state = state;
		}
		control_active = 1;
		wake_up(&dev->wait_wq);
	}
}

static void mtp_complete_in(struct usb_ep *ep, struct usb_request *req)
{
	struct mtp_dev *dev = _mtp_dev;

	if (req->status != 0)
		dev->state = STATE_ERROR;

	req_put(dev, &dev->tx_idle, req);

	wake_up(&dev->write_wq);
}

static void mtp_complete_out(struct usb_ep *ep, struct usb_request *req)
{
	struct mtp_dev *dev = _mtp_dev;

	dev->rx_done = 1;
	if (req->status != 0)
		dev->state = STATE_ERROR;

	wake_up(&dev->read_wq);
}

static void mtp_complete_intr(struct usb_ep *ep, struct usb_request *req)
{
	struct mtp_dev *dev = _mtp_dev;

	DBG(dev->cdev, "mtp_complete_intr status: %d actual: %d\n", req->status, req->actual);
	dev->intr_busy = 0;
	if (req->status != 0)
		dev->state = STATE_ERROR;

	wake_up(&dev->intr_wq);
}

static int __init create_bulk_endpoints(struct mtp_dev *dev,
				struct usb_endpoint_descriptor *in_desc,
				struct usb_endpoint_descriptor *out_desc,
				struct usb_endpoint_descriptor *intr_desc)
{
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request *req;
	struct usb_ep *ep;
	int i;

	DBG(cdev, "create_bulk_endpoints dev: %p\n", dev);

	ep = usb_ep_autoconfig(cdev->gadget, in_desc);
	if (!ep) {
		DBG(cdev, "usb_ep_autoconfig for ep_in failed\n");
		return -ENODEV;
	}
	DBG(cdev, "usb_ep_autoconfig for ep_in got %s\n", ep->name);
	ep->driver_data = dev;		/* claim the endpoint */
	dev->ep_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, out_desc);
	if (!ep) {
		DBG(cdev, "usb_ep_autoconfig for ep_out failed\n");
		return -ENODEV;
	}
	DBG(cdev, "usb_ep_autoconfig for mtp ep_out got %s\n", ep->name);
	ep->driver_data = dev;		/* claim the endpoint */
	dev->ep_out = ep;

	ep = usb_ep_autoconfig(cdev->gadget, intr_desc);
	if (!ep) {
		DBG(cdev, "usb_ep_autoconfig for ep_intr failed\n");
		return -ENODEV;
	}
	DBG(cdev, "usb_ep_autoconfig for mtp ep_intr got %s\n", ep->name);
	ep->driver_data = dev;		/* claim the endpoint */
	dev->ep_intr = ep;

	/* now allocate requests for our endpoints */
	for (i = 0; i < TX_REQ_MAX; i++) {
		req = mtp_request_new(dev->ep_in, BULK_BUFFER_SIZE);
		if (!req)
			goto fail;
		req->complete = mtp_complete_in;
		req_put(dev, &dev->tx_idle, req);
	}
	for (i = 0; i < RX_REQ_MAX; i++) {
		req = mtp_request_new(dev->ep_out, BULK_BUFFER_SIZE);
		if (!req)
			goto fail;
		req->complete = mtp_complete_out;
		dev->rx_req[i] = req;
	}
	req = mtp_request_new(dev->ep_intr, INTR_BUFFER_SIZE);
	if (!req)
		goto fail;
	req->complete = mtp_complete_intr;
	dev->intr_req = req;

	return 0;

fail:
	printk(KERN_ERR "mtp_bind() could not allocate requests\n");
	return -1;
}

static ssize_t mtp_read(struct file *fp, char __user *buf,
	size_t count, loff_t *pos)
{
	struct mtp_dev *dev = fp->private_data;
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request *req;
	int r = count, xfer;
	int ret = 0;

	DBG(cdev, "mtp_read(%d)\n", count);

	if (_lock(&dev->read_excl))
		return -EBUSY;

	if (count > BULK_BUFFER_SIZE){
		_unlock(&dev->read_excl);
		printk(KERN_ERR "USB-MTP mtp_read EINVAL\n");
		return -EINVAL;
	}
	/* we will block until we're online */
	DBG(cdev, "mtp_read: waiting for online state\n");
	ret = wait_event_interruptible(dev->read_wq,
		dev->state != STATE_OFFLINE);
	if (ret < 0) {
		r = ret;
		goto done;
	}
	if (dev->state == STATE_CANCELED) {
		/* report cancelation to userspace */
		dev->state = STATE_READY;
		_unlock(&dev->read_excl);
		printk(KERN_ERR "USB-MTP mtp_read ECANCELED\n");
		return -ECANCELED;
	}

requeue_req:
	/* queue a request */
	req = dev->rx_req[0];
	req->length = count;
	dev->rx_done = 0;
	ret = usb_ep_queue(dev->ep_out, req, GFP_KERNEL);
	if (ret < 0) {
		r = -EIO;
		goto done;
	} else {
		DBG(cdev, "rx %p queue\n", req);
	}

	/* wait for a request to complete */
	ret = wait_event_interruptible(dev->read_wq, dev->rx_done);
	if (ret < 0) {
		r = ret;
		goto done;
	}
		/* If we got a 0-len packet, throw it back and try again. */
		if (req->actual == 0)
			goto requeue_req;

		DBG(cdev, "rx %p %d\n", req, req->actual);
		xfer = (req->actual < count) ? req->actual : count;
		r = xfer;


	{
		int i;
		char* buf_work;
		
		buf_work = req->buf;
		for(i = 0; i < r; i++){
			buf_work++;
		}
	}


		if (copy_to_user(buf, req->buf, xfer))
			r = -EFAULT;
done:
	if (dev->state == STATE_CANCELED)
		r = -ECANCELED;
	else if (dev->state != STATE_OFFLINE)
		dev->state = STATE_READY;

	_unlock(&dev->read_excl);
	DBG(cdev, "mtp_read returning %d\n", r);

	return r;
}

static ssize_t mtp_write(struct file *fp, const char __user *buf,
	size_t count, loff_t *pos)
{
	struct mtp_dev *dev = fp->private_data;
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request *req = 0;
	int r = count, xfer;
	int ret;

	DBG(cdev, "mtp_write(%d)\n", count);

	if (_lock(&dev->write_excl))
		return -EBUSY;

	if (dev->state == STATE_CANCELED) {
		/* report cancelation to userspace */
		dev->state = STATE_READY;
		_unlock(&dev->write_excl);
		printk(KERN_ERR "USB-MTP mtp_write ECANCELED\n");
		return -ECANCELED;
	}
	if (dev->state == STATE_OFFLINE) {
		_unlock(&dev->write_excl);
		printk(KERN_ERR "USB-MTP mtp_write ENODEV\n");
		return -ENODEV;
	}

	while (count > 0) {
		/* get an idle tx request to use */
		req = 0;
		ret = wait_event_interruptible(dev->write_wq,
			((req = req_get(dev, &dev->tx_idle))
				));
		if (!req) {
			r = ret;
			break;
		}

		if (count > BULK_BUFFER_SIZE)
			xfer = BULK_BUFFER_SIZE;
		else
			xfer = count;
		if (copy_from_user(req->buf, buf, xfer)) {
			r = -EFAULT;
			break;
		}

		req->length = xfer;
		ret = usb_ep_queue(dev->ep_in, req, GFP_KERNEL);
		if (ret < 0) {
			DBG(cdev, "mtp_write: xfer error %d\n", ret);
			r = -EIO;
			break;
		}

		buf += xfer;
		count -= xfer;

		/* zero this so we don't try to free it on error exit */
		req = 0;
	}

	if (req)
		req_put(dev, &dev->tx_idle, req);

	if (dev->state == STATE_CANCELED)
		r = -ECANCELED;
	else if (dev->state != STATE_OFFLINE)
		dev->state = STATE_READY;

	_unlock(&dev->write_excl);
	DBG(cdev, "mtp_write returning %d\n", r);
	return r;
}


static int mtp_open(struct inode *ip, struct file *fp)
{
	printk(KERN_DEBUG "USB-MTP mtp_open\n");
	if (!_mtp_dev)
		return -1;

	if(!msm72k_udc_usb_state)
		return -1;

	if (_lock(&_mtp_dev->open_excl))
		return -1;
	
	/* clear any error condition */
	if (_mtp_dev->state != STATE_OFFLINE)
		_mtp_dev->state = STATE_READY;

	fp->private_data = _mtp_dev;
	return 0;
}

static int mtp_open_dummy(struct inode *ip, struct file *fp)
{
	printk(KERN_DEBUG "USB-MTP mtp_open\n");
	if (!_mtp_dev)
		return -1;
	if(!msm72k_udc_usb_state)
		return -1;
	/* clear any error condition */
	if (_mtp_dev->state != STATE_OFFLINE)
		_mtp_dev->state = STATE_READY;

	fp->private_data = _mtp_dev;

	return 0;
}

static int mtp_release(struct inode *ip, struct file *fp)
{
	printk(KERN_DEBUG "USB-MTP mtp_release\n");
	if (_mtp_dev) {
		_unlock(&_mtp_dev->open_excl);
		set_control_state(_mtp_dev, CTL_PWR_OFFLINE);
	}
	return 0;
}

static int mtp_release_dummy(struct inode *ip, struct file *fp)
{
	printk(KERN_DEBUG "USB-MTP mtp_release\n");
	if (_mtp_dev) {
		set_control_state(_mtp_dev, CTL_PWR_OFFLINE);
	}
	return 0;
}

static int mtp_send_event(struct mtp_dev *dev, struct mtp_event *event)
{
	struct usb_request *req;
	int ret;
	int length = event->length;

	DBG(dev->cdev, "mtp_send_event(%d)\n", event->length);
	printk(KERN_DEBUG "USB-MTP mtp_send_event(%d)\n", event->length);

	if (length < 0 || length > INTR_BUFFER_SIZE)
		return -EINVAL;

	mutex_lock(&dev->intr_mutex);

	/* wait for a request to complete */
	ret = wait_event_interruptible(dev->intr_wq, !dev->intr_busy || dev->state == STATE_OFFLINE);
	if (ret < 0)
		goto done;
	if (dev->state == STATE_OFFLINE) {
		ret = -ENODEV;
		goto done;
	}
	req = dev->intr_req;
	if (copy_from_user(req->buf, (void __user *)event->data, length)) {
		ret = -EFAULT;
		goto done;
	}
	req->length = length;
	dev->intr_busy = 1;
	ret = usb_ep_queue(dev->ep_intr, req, GFP_KERNEL);
	if (ret)
		dev->intr_busy = 0;

done:
	mutex_unlock(&dev->intr_mutex);
	return ret;
}

static int mtp_ioctl(struct inode *inode, struct file *fp, 
		unsigned int code, unsigned long value)
{
	struct mtp_dev *dev = fp->private_data;
	int ret = -EINVAL;

	switch (code) {
	case USB_MTP_FUNC_IOC_GET_DEVICE_STATUS_SET:
	{
		struct mtp_event	event;
		/* return here so we don't change dev->state below,
		 * which would interfere with bulk transfer state.
		 */
		if (copy_from_user(&event, (void __user *)value, sizeof(event)))
			return -EFAULT;
		else
			return mtp_send_event(dev, &event);
	}
	}

	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_CANCELED)
		ret = -ECANCELED;
	else if (dev->state != STATE_OFFLINE)
		dev->state = STATE_READY;
	spin_unlock_irq(&dev->lock);
	DBG(dev->cdev, "ioctl returning %d\n", ret);
	return ret;
}

static ssize_t mtp_control_read(struct file *fp, char __user *buf,
	size_t count, loff_t *pos)
{
	struct mtp_dev *dev = fp->private_data;
	int ret = 0;

	printk(KERN_DEBUG "USB-MTP %s -s-\n",__func__);
	if(!dev)
		return -EINVAL;

    if( mtp_active == 1 ){
		ret = wait_event_interruptible(dev->wait_wq, control_active != 0);
	}	
	control_active = 0;
	switch (dev->control_state) {
	case CTL_PWR_ONLINE:
		ret = sprintf(buf, "online");
		break;
	case CTL_CSR_CANCEL_REQUEST:
		ret = sprintf(buf, "Cancel_Request");
		break;
	case CTL_CSR_GET_DEVICE_STATUS:
		ret = sprintf(buf, "Get_Device_Status");
		break;
	case CTL_CSR_DEVICE_RESET:
		ret = sprintf(buf, "Device_Reset");
		break;
	case CTL_VBUS_OFFLINE:
		ret = sprintf(buf, "vbus_offline");
		set_control_state(dev, CTL_PWR_OFFLINE);
		break;
	default:
		ret = sprintf(buf, "offline");
	}
	printk(KERN_DEBUG "USB-MTP %s %s -e-\n",__func__, buf);

	return ret;
}

static ssize_t mtp_write_dummy(struct file *fp, const char __user *buf,
	size_t count, loff_t *pos)
{
	int ret = 0;

	printk(KERN_DEBUG "mtp_write \n");

	return ret;
}

static struct file_operations mtp_tunnel_fops = {
	.owner =   THIS_MODULE,
	.open =    mtp_open,
	.read =    mtp_read,
	.write =   mtp_write,
	.release = mtp_release,
};

static struct file_operations mtp_control_fops = {
	.owner =   THIS_MODULE,
	.open =    mtp_open_dummy,
	.read =    mtp_control_read,
	.write =   mtp_write_dummy,
	.release = mtp_release_dummy,
	.ioctl =   mtp_ioctl,
};

static struct file_operations mtp_event_fops = {
	.owner =   THIS_MODULE,
	.open =    mtp_open_dummy,
	.release = mtp_release_dummy,
};

static struct miscdevice mtp_tunnel_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_mtp_tunnel",
	.fops = &mtp_tunnel_fops,
};
static struct miscdevice mtp_control_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_mtp_control",
	.fops = &mtp_control_fops,
};
static struct miscdevice mtp_event_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_mtp_event",
	.fops = &mtp_event_fops,
};

static int mtp_enable_open(struct inode *ip, struct file *fp)
{
	int ret = 0;

	printk(KERN_DEBUG "USB-MTP %s\n",__func__);
	pr_debug("enabling mtp\n");
	if(!msm72k_udc_usb_state)
		return -1;
	control_active = 1;
	mtp_active = 1;
	return ret;
}

static int mtp_enable_release(struct inode *ip, struct file *fp)
{
	int ret = 0;

	struct mtp_dev *dev = fp->private_data;
	printk(KERN_DEBUG "USB-MTP %s\n",__func__);
	pr_debug("disabling mtp\n");
	if(dev)
		set_control_state(dev, CTL_PWR_OFFLINE);
	mtp_active = 0;
	if(android_get_product_id() == 0x02ED){
		ret = comm_usb_request_close(USBMODE_MTP);
	}
	return ret;
}

void mtp_offline(void)
{
	if (_mtp_dev){
		set_control_state(_mtp_dev, CTL_VBUS_OFFLINE);
	}
}

static struct file_operations mtp_enable_fops = {
	.owner =   THIS_MODULE,
	.open =    mtp_enable_open,
	.release = mtp_enable_release,
};

static struct miscdevice mtp_enable_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_mtp_enable",
	.fops = &mtp_enable_fops,
};

static int
mtp_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct mtp_dev	*dev = func_to_dev(f);
	int			id;
	int			ret;

	dev->cdev = cdev;
	DBG(cdev, "mtp_function_bind dev: %p\n", dev);

	/* allocate interface ID(s) */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	mtp_interface_desc.bInterfaceNumber = id;

	/* allocate endpoints */
	ret = create_bulk_endpoints(dev, &mtp_fullspeed_in_desc,
			&mtp_fullspeed_out_desc, &mtp_intr_desc);
	if (ret)
		return ret;

	/* support high speed hardware */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		mtp_highspeed_in_desc.bEndpointAddress =
			mtp_fullspeed_in_desc.bEndpointAddress;
		mtp_highspeed_out_desc.bEndpointAddress =
			mtp_fullspeed_out_desc.bEndpointAddress;
	}

	DBG(cdev, "%s speed %s: IN/%s, OUT/%s\n",
			gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full",
			f->name, dev->ep_in->name, dev->ep_out->name);
	return 0;
}

static void
mtp_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
	struct mtp_dev	*dev = func_to_dev(f);
	struct usb_request *req;
	int i;

	spin_lock_irq(&dev->lock);
	while ((req = req_get(dev, &dev->tx_idle)))
		mtp_request_free(req, dev->ep_in);
	for (i = 0; i < RX_REQ_MAX; i++)
		mtp_request_free(dev->rx_req[i], dev->ep_out);
	mtp_request_free(dev->intr_req, dev->ep_intr);
	dev->state = STATE_OFFLINE;
	set_control_state(dev, CTL_PWR_OFFLINE);
	spin_unlock_irq(&dev->lock);
	wake_up(&dev->intr_wq);

	misc_deregister(&mtp_event_device);
	misc_deregister(&mtp_control_device);
	misc_deregister(&mtp_tunnel_device);
	misc_deregister(&mtp_enable_device);
	kfree(_mtp_dev);
	_mtp_dev = NULL;

}

static int mtp_function_setup(struct usb_function *f,
					const struct usb_ctrlrequest *ctrl)
{
	struct mtp_dev  *dev = func_to_dev(f);
	struct usb_composite_dev *cdev;
	int     value = -EOPNOTSUPP;
	u16     w_index = le16_to_cpu(ctrl->wIndex);
	u16     w_value = le16_to_cpu(ctrl->wValue);
	u16     w_length = le16_to_cpu(ctrl->wLength);
	unsigned long   flags;

	if(dev == NULL)
		return value;
         
    cdev = dev->cdev;
	if(cdev == NULL)
		return value;

	printk(KERN_DEBUG "USB-MTP mtp_function_setup bRequestType %x bRequest %x w_value %d\n"
			,ctrl->bRequestType,ctrl->bRequest,w_value);
	VDBG(cdev, "mtp_function_setup %02x.%02x v%04x i%04x l%u\n",
			ctrl->bRequestType, ctrl->bRequest, w_value, w_index, w_length);

	/* Handle MTP OS string */
	if (ctrl->bRequestType == (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE)
			&& ctrl->bRequest == USB_REQ_GET_DESCRIPTOR
			&& (w_value >> 8) == USB_DT_STRING
			&& (w_value & 0xFF) == MTP_OS_STRING_ID) {
		value = (w_length < sizeof(mtp_os_string)
				? w_length : sizeof(mtp_os_string));
		memcpy(cdev->req->buf, mtp_os_string, value);
		/* return here since composite.c will send for us */
		return value;
	}
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR) {
		/* Handle MTP OS descriptor */
		DBG(cdev, "vendor request: %d index: %d value: %d length: %d\n",
				ctrl->bRequest, w_index, w_value, w_length);

		if (ctrl->bRequest == 0xFE 
				&& (ctrl->bRequestType & USB_DIR_IN)
				&& (w_index == 4 || w_index == 5)) {
			if(w_index == 4){
				value = (w_length < sizeof(mtp_ext_config_desc) ?
						w_length : sizeof(mtp_ext_config_desc));
				memcpy(cdev->req->buf, &mtp_ext_config_desc, value);
			}else{
				value = (w_length < sizeof(mtp_ext_property_desc) ?
						w_length : sizeof(mtp_ext_property_desc));
				memcpy(cdev->req->buf, &mtp_ext_property_desc, value);
			}
		}
	}
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS) {
		DBG(cdev, "class request: %d index: %d value: %d length: %d\n",
				ctrl->bRequest, w_index, w_value, w_length);

		if (ctrl->bRequest == MTP_REQ_CANCEL && w_index == 0
				&& w_value == 0) {
			DBG(cdev, "MTP_REQ_CANCEL\n");

			spin_lock_irqsave(&dev->lock, flags);
			if (dev->state == STATE_BUSY) {
				dev->state = STATE_CANCELED;
				wake_up(&dev->read_wq);
				wake_up(&dev->write_wq);
			}
			spin_unlock_irqrestore(&dev->lock, flags);

			/* We need to queue a request to read the remaining
			 *  bytes, but we don't actually need to look at
			 * the contents.
			 */
			value = w_length;
			set_control_state(dev, CTL_CSR_CANCEL_REQUEST);

		} else if (ctrl->bRequest == MTP_REQ_GET_DEVICE_STATUS
				&& w_index == 0 && w_value == 0) {
			struct mtp_device_status *status = cdev->req->buf;
			status->wLength = __constant_cpu_to_le16(sizeof(*status));

			DBG(cdev, "MTP_REQ_GET_DEVICE_STATUS\n");
			spin_lock_irqsave(&dev->lock, flags);
			/* device status is "busy" until we report
			 * the cancelation to userspace
			 */
			if (dev->state == STATE_BUSY || dev->state == STATE_CANCELED)
				status->wCode = __cpu_to_le16(MTP_RESPONSE_DEVICE_BUSY);
			else
				status->wCode = __cpu_to_le16(MTP_RESPONSE_OK);
			spin_unlock_irqrestore(&dev->lock, flags);
			value = sizeof(*status);
			set_control_state(dev, CTL_CSR_GET_DEVICE_STATUS);

		} else if (ctrl->bRequest == MTP_REQ_RESET
				&& w_index == 0 && w_value == 0) {
			value = w_length;
			set_control_state(dev, CTL_CSR_DEVICE_RESET);
		}
	}
	printk(KERN_DEBUG "cdev->req %p\n", cdev->req);

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		int rc;
		cdev->req->zero = value < w_length;
		cdev->req->length = value;
		rc = usb_ep_queue(cdev->gadget->ep0, cdev->req, GFP_ATOMIC);
		if (rc < 0)
			ERROR(cdev, "%s setup response queue error\n", __func__);
	}

	if (value == -EOPNOTSUPP)
		VDBG(cdev, "unknown class-specific control req "
				"%02x.%02x v%04x i%04x l%u\n",
				ctrl->bRequestType, ctrl->bRequest,
				w_value, w_index, w_length);
	return value;
}

static int mtp_function_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct mtp_dev	*dev = func_to_dev(f);
	struct usb_composite_dev *cdev = f->config->cdev;
	int ret;

	DBG(cdev, "mtp_function_set_alt intf: %d alt: %d\n", intf, alt);
	ret = usb_ep_enable(dev->ep_in,
			ep_choose(cdev->gadget,
				&mtp_highspeed_in_desc,
				&mtp_fullspeed_in_desc));
	if (ret)
		return ret;
	ret = usb_ep_enable(dev->ep_out,
			ep_choose(cdev->gadget,
				&mtp_highspeed_out_desc,
				&mtp_fullspeed_out_desc));
	if (ret) {
		usb_ep_disable(dev->ep_in);
		return ret;
	}

	ret = usb_ep_enable(dev->ep_intr, &mtp_intr_desc);
	if (ret) {
		usb_ep_disable(dev->ep_out);
		usb_ep_disable(dev->ep_in);
		return ret;
	}
	dev->state = STATE_READY;
	set_control_state(dev, CTL_PWR_ONLINE);

	/* readers may be blocked waiting for us to go online */
	wake_up(&dev->read_wq);
	return 0;
}

static void mtp_function_disable(struct usb_function *f)
{
	struct mtp_dev	*dev = func_to_dev(f);
	struct usb_composite_dev	*cdev = dev->cdev;

	DBG(cdev, "mtp_function_disable\n");
	dev->state = STATE_OFFLINE;
	set_control_state(dev, CTL_PWR_OFFLINE);
	usb_ep_disable(dev->ep_in);
	usb_ep_disable(dev->ep_out);
	usb_ep_disable(dev->ep_intr);

	/* readers may be blocked waiting for us to go online */
	wake_up(&dev->read_wq);
	wake_up(&dev->write_wq);
	wake_up(&dev->intr_wq);
	wake_up(&dev->wait_wq);

	VDBG(cdev, "%s disabled\n", dev->function.name);
}

static int mtp_bind_config(struct usb_configuration *c)
{
	struct mtp_dev *dev;
	int ret;


	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	spin_lock_init(&dev->lock);

	init_waitqueue_head(&dev->read_wq);
	init_waitqueue_head(&dev->write_wq);
	init_waitqueue_head(&dev->intr_wq);
	init_waitqueue_head(&dev->wait_wq);
	atomic_set(&dev->open_excl, 0);
	atomic_set(&dev->read_excl, 0);
	atomic_set(&dev->write_excl, 0);
	INIT_LIST_HEAD(&dev->tx_idle);
	mutex_init(&dev->intr_mutex);

	dev->cdev = c->cdev;
	dev->function.name = "mtp";
	dev->function.descriptors = fs_mtp_descs;
	dev->function.hs_descriptors = hs_mtp_descs;
	dev->function.bind = mtp_function_bind;
	dev->function.unbind = mtp_function_unbind;
	dev->function.setup = mtp_function_setup;
	dev->function.set_alt = mtp_function_set_alt;
	dev->function.disable = mtp_function_disable;
	set_control_state(dev, CTL_PWR_OFFLINE);

	/* start disabled */
	dev->function.hidden = 1;

	/* _mtp_dev must be set before calling usb_gadget_register_driver */
	_mtp_dev = dev;

	ret = misc_register(&mtp_enable_device);
	if (ret)
		goto err1;
	ret = misc_register(&mtp_tunnel_device);
	if (ret)
		goto err2;
	ret = misc_register(&mtp_control_device);
	if (ret)
		goto err3;
	ret = misc_register(&mtp_event_device);
	if (ret)
		goto err4;
	ret = usb_add_function(c, &_mtp_dev->function);
	if (ret)
		goto err5;

	return ret;

err5:
	misc_deregister(&mtp_event_device);
err4:
	misc_deregister(&mtp_control_device);
err3:
	misc_deregister(&mtp_tunnel_device);
err2:
	misc_deregister(&mtp_enable_device);
err1:
	kfree(dev);
	_mtp_dev = NULL;

	printk(KERN_ERR "mtp gadget driver failed to initialize\n");
	return ret;
}

static struct android_usb_function mtp_function = {
	.name = "mtp",
	.bind_config = mtp_bind_config,
};

static int __init init(void)
{
	printk(KERN_DEBUG "f_mtp init\n");
	android_register_function(&mtp_function);
	return 0;
}
module_init(init);
