/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#ifndef __LINUX_LEDS_CMD_H
#define __LINUX_LEDS_CMD_H

/* Command ID */
#define LEDS_CMD_TYPE_RGB_RED		0x01
#define LEDS_CMD_TYPE_RGB_GREEN		0x02
#define LEDS_CMD_TYPE_RGB_BLUE		0x03
#define LEDS_CMD_TYPE_KEY			0x04
#define LEDS_CMD_TYPE_FLASH			0x05

/* Return value */
#define LEDS_CMD_RET_OK				1
#define LEDS_CMD_RET_NG				0

/* Command function */
extern unsigned char leds_cmd(unsigned char type, unsigned char val);

#endif /* __LINUX_LEDS_CMD_H */
