/*
 * diag_wrapper_key.h
 *
 * Copyright (C) 2010 NEC Corporation
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#ifndef DIAGWRAPPER_SHARED
#define DIAGWRAPPER_SHARED

#include <linux/ioctl.h>

#include <linux/keypad_cmd.h>
#include <linux/leds_cmd.h>
#include <linux/touch_panel_cmd.h>

#define DIAG_WRAPPER_FD_PATH    "/dev/diag_wrapper"

#define DIAG_WRAPPER_IOCTL_OK   0
#define DIAG_WRAPPER_IOCTL_NG   -1

#define DIAG_WRAPPER_IOCTL_A01  0x10
#define DIAG_WRAPPER_IOCTL_A02  0x11
#define DIAG_WRAPPER_IOCTL_A03  0x12
#define DIAG_WRAPPER_IOCTL_A04  0x13

#define DIAG_WRAPPER_IOCTL_B01  0x20
#define DIAG_WRAPPER_IOCTL_B02  0x21
#define DIAG_WRAPPER_IOCTL_B03  0x22
#define DIAG_WRAPPER_IOCTL_B04  0x23
#define DIAG_WRAPPER_IOCTL_B05  0x24
#define DIAG_WRAPPER_IOCTL_B06  0x25
#define DIAG_WRAPPER_IOCTL_B07  0x26
#define DIAG_WRAPPER_IOCTL_B08  0x27
#define DIAG_WRAPPER_IOCTL_B09  0x28
#define DIAG_WRAPPER_IOCTL_B10  0x29
#define DIAG_WRAPPER_IOCTL_B11  0x2A
#define DIAG_WRAPPER_IOCTL_B12  0x2B

#define DIAG_WRAPPER_IOCTL_C01  0x30
#define DIAG_WRAPPER_IOCTL_C02  0x31
#define DIAG_WRAPPER_IOCTL_C03  0x32
#define DIAG_WRAPPER_IOCTL_C04  0x33
#define DIAG_WRAPPER_IOCTL_C05  0x34
#define DIAG_WRAPPER_IOCTL_C06  0x35
#define DIAG_WRAPPER_IOCTL_C07  0x36
#define DIAG_WRAPPER_IOCTL_C08  0x37
#define DIAG_WRAPPER_IOCTL_C09  0x38
#define DIAG_WRAPPER_IOCTL_C10  0x39
#define DIAG_WRAPPER_IOCTL_C11  0x3A

#define KEY_DIAG_EMULATION_KEY     1
#define KEY_DIAG_EMULATION_HANDSET 2

union u_diag_pw_led_drv_ret{
    char    ch[sizeof(int)];
    int     it;
};

#endif /* DIAGWRAPPER_SHARED */
