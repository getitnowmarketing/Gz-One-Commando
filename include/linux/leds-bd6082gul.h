/*
 * leds-bd6082gul.h - platform data structure for bd6082gul led controller
 *
 * Copyright (C) 2010 NEC Corporation
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

 
#ifndef __LINUX_BD6082GUL_H
#define __LINUX_BD6082GUL_H

#include <linux/leds.h>

/* define ------------------------------*/

#define BD6082GUL_LED_SET_OK	0
#define BD6082GUL_LED_SET_NG	(-1)
#define BD6082GUL_LED_SET_RESERVED	(1)

#define BD6082GUL_LED_REG_WRITE_NG	(0x80)

#define BD6082GUL_LED_ON		(1)
#define BD6082GUL_LED_OFF		(0)

/* structure ------------------------------*/


struct led_backlight_ctrl {
	unsigned char	mleden:1	; 
	unsigned char	dmy2:3		; 
	unsigned char	sleden:1	; 
	unsigned char	dmy1:2		; 
	unsigned char	wpwmen:1	; 
};


struct led_main_slope_ctrl {
	unsigned char	tlh:3		; 
	unsigned char	dmy2:1		; 
	unsigned char	thl:3		; 
	unsigned char	dmy1:1		; 
};


struct led_rgb_ctrl {
	unsigned char	rgb1en:1	; 
	unsigned char	rgb1os:1	; 
	unsigned char	rgb1eml:1	; 
	unsigned char	dmy1:5		; 
};


struct led_rgb_cnf_ctrl {
	unsigned char	trgb:3		; 
	unsigned char	dmy1:1		; 
	unsigned char	srrgb:2		; 
	unsigned char	sfrgb:2		; 
};


struct led_rgb_bright {
	unsigned char	bright:7		; 
	unsigned char	set_flag:1		; 
};


struct led_group1_ctrl {
	unsigned char	dmy2:2		; 
	unsigned char	flleden:1	; 
	unsigned char	dmy1:4		; 
	unsigned char	flmd:1		; /* Mobile Light Flash mode / Torch mode */
};


struct led_group2_ctrl {
	unsigned char	led1en:1	; 
	unsigned char	led2en:1	; 
	unsigned char	led3en:1	; 
	unsigned char	led4en:1	; 
	unsigned char	led5en:1	; 
	unsigned char	led6en:1	; 
	unsigned char	dmy1:2		; 
};


struct led_key_bright {
	unsigned char	bright:5	; 
	unsigned char	dmy1:2		; 
	unsigned char	set_flag:1	; 
};


struct led_indicator_bright {
	unsigned char	bright:5	; 
	unsigned char	dmy1:2		; 
	unsigned char	set_flag:1	; 
};


struct led_flash_bright {
	unsigned char	bright:5	; 
	unsigned char	dmy1:2		; 
	unsigned char	set_flag:1	; 
};

/* extern */
extern int bd6082gul_main_lcd_set(struct led_backlight_ctrl *request);
extern int bd6082gul_main_lcd_bright (unsigned char lcd_bright);
extern int bd6082gul_main_lcd_slope (struct led_main_slope_ctrl *slope);
extern int bd6082gul_sub_lcd_set(struct led_backlight_ctrl *request);
extern int bd6082gul_sub_lcd_bright (unsigned char lcd_bright);
extern int bd6082gul_rgb_led_set(struct led_rgb_ctrl *request);
extern int bd6082gul_rgb_led_bright(struct led_rgb_bright *red_bright,
									struct led_rgb_bright *green_bright,
									struct led_rgb_bright *blue_bright);
extern int bd6082gul_rgb_led_cnf(struct led_rgb_cnf_ctrl *request);
extern int bd6082gul_rgb_led_patern(unsigned char red_patern,
									unsigned char green_patern,
									unsigned char blue_patern);
extern int bd6082gul_flash_led_set(struct led_group1_ctrl *request);
extern int bd6082gul_flash_led_bright(struct led_flash_bright *led_low_bright,
									  struct led_flash_bright *led_high_bright);
extern int bd6082gul_indicator_led_set(struct led_group2_ctrl *request);
extern int bd6082gul_indicator_led_bright(struct led_indicator_bright *led_ind_red_bright,
										  struct led_indicator_bright *led_ind_green_bright);
extern int bd6082gul_key_led_set(struct led_group2_ctrl *request);
extern int bd6082gul_key_led_bright (struct led_key_bright *led_key_bright,
									 struct led_key_bright *led_key_bright2);
extern int bd6082gul_reg_write(unsigned char sub2,
								unsigned char inf1);
extern int bd6082gul_main_lcd_don(unsigned char sub2);

#endif /* __LINUX_BD6082GUL_H */

/* File END */
