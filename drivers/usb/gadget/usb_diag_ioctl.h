/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

/*******************************************************************/
/*                      usb_diag_ioctl.h                           */
/*                                                                 */
/* 20100614 add Definition of for Diagnosis USB Enumeration ioctl  */

/*                                                                 */
/*******************************************************************/
#ifndef _USB_DIAG_H_
#define _USB_DIAG_H_

#include <linux/ioctl.h>

#define IOC_MAGIC_USB_DIAG 'u'        /* usb_diag_driver */

#define IOCTL_USB_DIAG_FS_ENUM _IOR(IOC_MAGIC_USB_DIAG, 1, char) /* ioctl command */
#define IOCTL_USB_DIAG_HS_ENUM _IOR(IOC_MAGIC_USB_DIAG, 3, char) /* ioctl command */

#define DEVICE_COUNTS          1    /* counts of reserve to use (minor number) */
#define USB_DEV_MAJOR          0    /* default major number */
#define USB_DEV_MINOR          0    /* default minor number */

struct usb_diag_ioctl_cmd {
	char rsp_data[1];
};

struct ioctl_cmd {
	char rsp_data[1];
};

#endif /* _USB_DIAG_H_ */
