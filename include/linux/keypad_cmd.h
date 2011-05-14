/*
 * keypad_cmd.h
 *
 * Copyright (C) 2010 NEC Corporation
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#ifndef __LINUX_KEY_DIAG_H
#define __LINUX_KEY_DIAG_H

/* define ------------------------------*/
#define KEYPAD_CMD_TYPE_MASK	0x01
#define KEYPAD_CMD_TYPE_GET_KEYCODE	0x02
#define KEYPAD_CMD_TYPE_KEY_EMULATION	0x04
#define KEY_DIAG_FLG_RAND_IN		0x01
#define KEY_DIAG_FLG_KEYMASK		0x02
/* structure ---------------------------*/
typedef struct
{
	int key_code;      // Key code
} keypad_cmd_callback_param_keycode;

/* extern ------------------------------*/
extern unsigned char keypad_cmd(unsigned char type, int *val);
extern unsigned char keypad_cmd_callback(unsigned char type,
				 unsigned char val, void (*func)(void *));
extern void keypad_diag_func(int code);
extern int keypad_mask_get(void);
#endif
