/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#ifndef __LINUX_TOUCH_PANEL_CMD_H
#define __LINUX_TOUCH_PANEL_CMD_H

typedef struct
{
	unsigned char	reg0;	// Touch Mode Register
	unsigned char	reg1;	// X1 Register(High)
	unsigned char	reg2;	// X1 Register(Low)
	unsigned char	reg3;	// Y1 Register(High)
	unsigned char	reg4;	// Y1 Register(Low)
	unsigned char	reg5;	// X2 Register(High)
	unsigned char	reg6;	// X2 Register(Low)
	unsigned char	reg7;	// Y2 Register(High)
	unsigned char	reg8;	// Y2 Register(Low)
} touch_panel_cmd_callback_param_coord;


#define TOUCH_PANEL_CMD_TYPE_RESET				0x01
#define TOUCH_PANEL_CMD_TYPE_ENTR_SLEEP			0x02
#define TOUCH_PANEL_CMD_TYPE_EXIT_SLEEP			0x03
#define TOUCH_PANEL_CMD_TYPE_ENTR_DEEP			0x04
#define TOUCH_PANEL_CMD_TYPE_EXIT_DEEP			0x05
#define TOUCH_PANEL_CMD_TYPE_GET_REVISION		0x06
#define TOUCH_PANEL_CMD_TYPE_GET_COORD			0x07
#define TOUCH_PANEL_CMD_TYPE_START_TEST			0x08
#define TOUCH_PANEL_CMD_TYPE_GET_TEST_RSLT		0x09
#define TOUCH_PANEL_CMD_TYPE_FW_UPDATE			0x0a
#define TOUCH_PANEL_CMD_TYPE_GET_CHECKSUM		0x0b

extern unsigned char touch_panel_cmd(unsigned char type, unsigned char *val);
extern unsigned char touch_panel_cmd_callback(unsigned char type, unsigned char val, void (*func)(void *));

#endif /* __LINUX_TOUCH_PANEL_CMD_H */
