/*
 * Processing for UI of USB event header
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


#ifndef	__USB_GADGET_COMM_USB_H
#define	__USB_GADGET_COMM_USB_H

#define TYPE_REQUEST_OPEN		0x00000001	/**< Open request for new vold */
#define TYPE_REQUEST_CLOSE		0x00000002	/**< Close request for new vold */
#define TYPE_REQUEST_READ		0x00000003	/**< Read request for new vold */
#define TYPE_REQUEST_CONNECT		0x00000004	/**< Connect request for new vold */
#define TYPE_REQUEST_DISCONNECT		0x00000005	/**< Disconnect request for new vold */

#define TYPE_RESPONSE_OPEN		0x00000001	/**< Open response from new vold */
#define TYPE_RESPONSE_CLOSE		0x00000002	/**< Close response from new vold */
#define TYPE_RESPONSE_READ		0x00000003	/**< Read Response from new vold */
#define TYPE_RESPONSE_CONNECT		0x00000004	/**< Connect response from new vold */
#define TYPE_RESPONSE_DISCONNECT	0x00000005	/**< Disconnect response from vold */

#define USBMODE_IDLE			0x00000000	/**< IDLE mode */
#define USBMODE_MASS			0x00000001	/**< Mass Storage mode */
#define USBMODE_MTP			0x00000002	/**< MTP mode */
#define USBMODE_MODEM			0x00000003	/**< MODEM mode */
#define USBMODE_CDROM			0x00000004	/**< CDROM mode */

#define STATUS_SUCCESS			0		/**< Success */
#define STATUS_FAILURE			-1		/**< Failure */

#define USB_REPLY_MAXSIZE		(10*1024)	/**< Packet max size */

/**
 * @brief USB communication response commonness header
 */
struct usb_comm_header {
	u32	size;					/**< Packet size */
	u32	type;					/**< Request type */
};

/**
 * @brief USB communication response open request body
 */
struct usb_comm_request_body_connect {
	u32	boot;					/**< Open mode */
};

/**
 * @brief USB communication response open request body
 */
struct usb_comm_request_body_open {
	u32	mode;					/**< Open mode */
};

/**
 * @brief USB communication response close request body
 */
struct usb_comm_request_body_close {
	u32	mode;					/**< Open mode */
};

/**
 * @brief USB communication response read request body
 */
struct usb_comm_request_body_read {
	u32	offset;					/**< Read offset */
	u32	size;					/**< Read size */
	u32	concat;					/**< concat number */
	u32	total;					/**< total number */
};

/**
 * @brief USB communication request commonness packet data
 */
struct usb_comm_request {
	/**< USB communication response commonness header */
	struct usb_comm_header				head;	
	union {
		/**< USB communication response connect request body */
		struct usb_comm_request_body_connect	connect;	

		/**< USB communication response open request body */
		struct usb_comm_request_body_open	open;	

		/**< USB communication response close request body */
		struct usb_comm_request_body_close	close;	

		/**< USB communication response read request body */
		struct usb_comm_request_body_read	read;	
	} body;
};

/**
 * @brief USB communication response open response body
 */
struct usb_comm_response_body_open {
	u32	filesize;				/**< file size */
};

/**
 * @brief USB communication response read response body
 */
struct usb_comm_response_body_read_concat {
	u32	bodysize;				/**< body size */
	u32	concat;					/**< concat number */
	u32	total;					/**< total number */
	u32	size;					/**< data size */
};

/**
 * @brief USB communication response read response body
 */
struct usb_comm_response_body_read {
	u32	bodysize;				/**< body size */
	u32	size;					/**< data size */
	u8	data[64*1024];				/**< data buff */
};

/**
 * @brief USB communication request commonness packet data
 */
struct usb_comm_response {
	/**< USB communication response commonness header */
	struct usb_comm_header				head;	
	u32						status;
	union {
		/**< USB communication response open request body */
		struct usb_comm_response_body_open	open;	

		/**< USB communication response read request body */
		struct usb_comm_response_body_read	read;	
	} body;
};

extern int comm_usb_init(void (* pcb_func)(struct usb_comm_response *, void *), void *puser);
extern void comm_usb_fin(void);

extern void comm_usb_end(void);

extern int comm_usb_request(struct usb_comm_request * request);
extern int comm_usb_request_connect(u32 boot);
extern int comm_usb_request_open(u32 mode);
extern int comm_usb_request_close(u32 mode);
extern int comm_usb_request_disconnect(void);
extern int comm_usb_request_read(u32 offset, u32 size);


extern int comm_usb_cdrom_interruptible(void);



extern int comm_usb_is_connect_polling(void);


#endif /* __USB_GADGET_COMM_USB_H */
