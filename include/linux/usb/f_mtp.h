/*
 * Gadget Driver for Android MTP
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


#ifndef __LINUX_USB_F_MTP_H
#define __LINUX_USB_F_MTP_H

struct mtp_event {
	size_t		length;	/* size of the event */
	void  		*data;	/* event data to send */
};

#define USB_MTP_IOC_MAGIC 0xFF

#define USB_MTP_FUNC_IOC_CANCEL_REQUEST_SET _IOW(USB_MTP_IOC_MAGIC, 0x20, int)
#define USB_MTP_FUNC_IOC_CANCEL_REQUEST_GET _IOW(USB_MTP_IOC_MAGIC, 0x21, int)
#define USB_MTP_FUNC_IOC_GET_EXTENDED_EVENT_DATA_SET    _IOW(USB_MTP_IOC_MAGIC, 0x22, int)
#define USB_MTP_FUNC_IOC_GET_EXTENDED_EVENT_DATA_GET    _IOW(USB_MTP_IOC_MAGIC, 0x23, int)
#define USB_MTP_FUNC_IOC_DEVICE_RESET_REQUEST_SET   _IOW(USB_MTP_IOC_MAGIC, 0x24, int)
#define USB_MTP_FUNC_IOC_DEVICE_RESET_REQUEST_GET   _IOW(USB_MTP_IOC_MAGIC, 0x25, int)
#define USB_MTP_FUNC_IOC_GET_DEVICE_STATUS_SET  _IOW(USB_MTP_IOC_MAGIC, 0x26, int)
#define USB_MTP_FUNC_IOC_GET_DEVICE_STATUS_GET  _IOW(USB_MTP_IOC_MAGIC, 0x27, int)

void mtp_offline(void);

#endif /* __LINUX_USB_F_MTP_H */
