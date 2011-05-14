/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>

#include <linux/leds-bd6082gul.h>
#include <linux/leds_cmd.h>

/*============================================================================
        DEFINE & ENUM
============================================================================*/
//#define LED_DRV_OPT_RGB_RED_ONLY
#define LED_DRV_OPT_RGB_GREEN_USE
//#define LED_DRV_OPT_RGB_BLUE_USE
#define LED_DRV_OPT_KEYBL_LED4_DISABLE


enum eled_devices_ids {
	LED_DRV_IC_DEVICE_ID_RGB_RED		= 0,			/* RGB(RED) LED		*/
	LED_DRV_IC_DEVICE_ID_RGB_GREEN,						/* RGB(GREEN) LED	*/

	LED_DRV_IC_DEVICE_ID_RGB_PATERN,					/*  */
	LED_DRV_IC_DEVICE_ID_RGB_TRGB,						/*  */
	LED_DRV_IC_DEVICE_ID_RGB_RGB1OS,					/*  */

	LED_DRV_IC_DEVICE_ID_KEYBL,							/* Key backlight	*/
	LED_DRV_IC_DEVICE_ID_FLASH_TORCH,					/* Flash LED(Torch)	*/
	LED_DRV_IC_DEVICE_ID_FLASH_FLASH,					/* Flash LED(Flash)	*/
	LED_DRV_IC_DEVICE_ID_INDICATOR_RED,					/* Indicator(RED) LED*/
	LED_DRV_IC_DEVICE_ID_INDICATOR_GREEN,				/* Indicator(GREEN) LED*/
	LED_DRV_IC_DEVICE_ID_BTNBL,							/* Button Backlight	*/
	LED_DRV_IC_DEVICE_ID_NUM
};

enum eled_devices_ids_sub_rgb {
	LED_DRV_IC_DEVICE_ID_SUB_RGB_RED		= 0,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM
};

enum eled_devices_ids_sub_rgb_opt {
	LED_DRV_IC_DEVICE_ID_SUB_RGB_PATERN			= 0,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_TRGB,
	LED_DRV_IC_DEVICE_ID_SUB_RGB_RGB1OS,
};

enum eled_devices_ids_sub_flash {
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH		= 0,
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH,
	LED_DRV_IC_DEVICE_ID_SUB_FLASH_NUM
};

enum eled_devices_ids_sub_indicator {
	LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED		= 0,
	LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN,
	LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_NUM
};


enum eled_rgb_lighting_num {
	LED_DRV_IC_RGB_LIGHTING_1		= 0,				
	LED_DRV_IC_RGB_LIGHTING_2,							
	LED_DRV_IC_RGB_LIGHTING_3,							
	LED_DRV_IC_RGB_LIGHTING_NUM
};

enum eled_keybl_brightness_num {
	LED_DRV_IC_KEYBL_BRIGHTNESS_1	= 0,				
	LED_DRV_IC_KEYBL_BRIGHTNESS_2,						
	LED_DRV_IC_KEYBL_BRIGHTNESS_NUM
};
/*
   Flash type
*/
enum eled_flash_type_num {
	LED_DRV_IC_FLASH_TYPE_STILL	= 0,					
	LED_DRV_IC_FLASH_TYPE_MOVIE,						
	LED_DRV_IC_FLASH_TYPE_IDLE,							
	LED_DRV_IC_FLASH_TYPE_NUM
};
/*
   Flash mode
*/
enum eled_flash_mode_num {
	LED_DRV_IC_FLASH_MODE_TORCH	= 0,					/* Torch mode		*/
	LED_DRV_IC_FLASH_MODE_FLASH,						/* Flash mode		*/
	LED_DRV_IC_FLASH_MODE_NUM
};

enum eled_flash_temp_num {
	LED_DRV_IC_FLASH_TEMP_1	= 0,					
	LED_DRV_IC_FLASH_TEMP_2,						
	LED_DRV_IC_FLASH_TEMP_3,						
	LED_DRV_IC_FLASH_TEMP_NUM
};

enum eled_brightness_value {
	LED_DRV_IC_BRIGHTNESS_VALUE_OFF	= 0,			
	LED_DRV_IC_BRIGHTNESS_VALUE_FULL,				/* 1/1					*/
	LED_DRV_IC_BRIGHTNESS_VALUE_HALF				/* 1/2					*/
};


#define LED_DRV_IC_DEVICE_NAME_RGB_RED				"red"
#define LED_DRV_IC_DEVICE_NAME_RGB_GREEN			"green"

#define LED_DRV_IC_DEVICE_NAME_RGB_PATERN			"rgb-patern"
#define LED_DRV_IC_DEVICE_NAME_RGB_TRGB				"rgb-trgb"
#define LED_DRV_IC_DEVICE_NAME_RGB_RGB1OS			"rgb-rgb1os"

#define LED_DRV_IC_DEVICE_NAME_KEYBL				"keyboard-backlight"
#define LED_DRV_IC_DEVICE_NAME_FLASH_TORCH			"torch-led"
#define LED_DRV_IC_DEVICE_NAME_FLASH_FLASH			"flash-led"
#define LED_DRV_IC_DEVICE_NAME_INDICATOR_RED		"indicator-red-led"
#define LED_DRV_IC_DEVICE_NAME_INDICATOR_GREEN		"indicator-green-led"
#define LED_DRV_IC_DEVICE_NAME_BTNBL				"button-backlight"

/*============================================================================
        STRUCTURE
============================================================================*/
struct drv_ic_led_rgb_brightness {
	unsigned char					brightness[LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM];
};

struct drv_ic_led_indicator_brightness {
	unsigned char					brightness[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_NUM];
};

/***** Config data *****/
/* RGB LED Config */
struct drv_ic_led_rgb_config {
	struct drv_ic_led_rgb_brightness		lighting[LED_DRV_IC_RGB_LIGHTING_NUM];
	struct led_rgb_ctrl						reg_07;
};
/* Key Backlight Config */
struct drv_ic_led_keybl_config {
	unsigned char			 		brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_NUM];
};
/* Flash LED Config */
struct drv_ic_led_flash_mode_config {
	unsigned char					brightness[LED_DRV_IC_FLASH_TYPE_NUM];
};

struct drv_ic_led_flash_config {
	struct drv_ic_led_flash_mode_config		temp[LED_DRV_IC_FLASH_TEMP_NUM];
};


struct drv_ic_led_configs {
	struct drv_ic_led_rgb_config			rbg_config;
	struct drv_ic_led_keybl_config			keybl_config;
	struct drv_ic_led_flash_config			flash_config;
	struct drv_ic_led_indicator_brightness	indicator_config;
};

struct drv_ic_data {
	struct led_classdev					leds[LED_DRV_IC_DEVICE_ID_NUM];
};


/*============================================================================
        LOCAL DATA
============================================================================*/
static struct drv_ic_led_configs				*pg_led_congig = NULL;
static struct drv_ic_led_rgb_brightness		g_rgb_brightness;
static struct drv_ic_led_indicator_brightness	g_indicator_brightness;
/*============================================================================
        LOCAL FUNCTIONS
============================================================================*/
static void led_brightness_set_rgb(int device_no, enum led_brightness value);
static void led_brightness_set_rgb_opt(int device_no, enum led_brightness value);
static void led_brightness_set_keybl(int device_no, enum led_brightness value);
static void led_brightness_set_flash(int device_no, enum led_brightness value);
static void led_brightness_set_indicator(int device_no, enum led_brightness value);
/*============================================================================
        EXTERNAL FUNCTIONS
============================================================================*/
unsigned char leds_cmd(unsigned char type, unsigned char val)
{
	int			device_no;
	char		ret = LEDS_CMD_RET_OK;

	switch(type)
	{
		case LEDS_CMD_TYPE_RGB_RED:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RED;
			if(val){
				val = 0x1E ;
			}
			led_brightness_set_rgb( device_no, (enum led_brightness)val );
			break;
		case LEDS_CMD_TYPE_RGB_GREEN:
			device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN;
			if(val){
				val = 0x1E ;
			}
			led_brightness_set_rgb( device_no, (enum led_brightness)val );
			break;
		case LEDS_CMD_TYPE_KEY:
			device_no = LED_DRV_IC_DEVICE_ID_KEYBL;
			led_brightness_set_keybl( device_no, (enum led_brightness)val );
			break;
		case LEDS_CMD_TYPE_FLASH:
			device_no = LED_DRV_IC_DEVICE_ID_FLASH_TORCH;
			led_brightness_set_flash( device_no, (enum led_brightness)val );
			break;
		default:
			ret = LEDS_CMD_RET_NG;
			break;
	}
	return ret;
}
EXPORT_SYMBOL(leds_cmd);
/*============================================================================
        LOCAL FUNCTIONS
============================================================================*/

static void led_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	int			device_no;

	/* check:device */
	if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_RED))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RED;
		led_brightness_set_rgb( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_GREEN))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_GREEN;
		led_brightness_set_rgb( device_no, value );
	}

	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_PATERN))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_PATERN;
		led_brightness_set_rgb_opt( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_TRGB))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_TRGB;
		led_brightness_set_rgb_opt( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_RGB_RGB1OS))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_RGB_RGB1OS;
		led_brightness_set_rgb_opt( device_no, value );
	}

	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_KEYBL) ||
	         !strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_BTNBL) )
	{
		device_no = LED_DRV_IC_DEVICE_ID_KEYBL;
		led_brightness_set_keybl( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_FLASH_TORCH))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_TORCH;
		led_brightness_set_flash( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_FLASH_FLASH))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH;
		led_brightness_set_flash( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_INDICATOR_RED))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED;
		led_brightness_set_indicator( device_no, value );
	}
	else if (!strcmp(led_cdev->name, LED_DRV_IC_DEVICE_NAME_INDICATOR_GREEN))
	{
		device_no = LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN;
		led_brightness_set_indicator( device_no, value );
	}
}

static void led_brightness_set_rgb(int device_no, enum led_brightness value)
{
	struct led_rgb_bright	set_brigh[3];
	struct led_rgb_ctrl		set_onoff;
	int						lighting_num = 0;
	int						cnt;
	int						ret;

	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( set_brigh, 0, sizeof( set_brigh ) );

	g_rgb_brightness.brightness[device_no] = value;

	set_brigh[device_no].set_flag = BD6082GUL_LED_ON;
	set_brigh[device_no].bright   = g_rgb_brightness.brightness[device_no];

	ret = bd6082gul_rgb_led_bright(	&set_brigh[0],&set_brigh[1],&set_brigh[2] );	// Red,Green,Bule

	if( ret != BD6082GUL_LED_SET_OK )
		printk(KERN_INFO "led_brightness_set_rgb: bd6082gul_rgb_led_bright = %d \n",ret);


	for( cnt= 0; cnt<LED_DRV_IC_DEVICE_ID_SUB_RGB_NUM; cnt++)
	{
		if( g_rgb_brightness.brightness[cnt] != LED_DRV_IC_BRIGHTNESS_VALUE_OFF )
		{
			lighting_num++;
		}
	}


	if( lighting_num )
	{
		/* ON */
		set_onoff.rgb1os = pg_led_congig->rbg_config.reg_07.rgb1os;
		if( set_onoff.rgb1os == BD6082GUL_LED_ON ){
			/* 1st  */
			
			set_onoff.rgb1en = BD6082GUL_LED_OFF; /* OFF */
		}
		else{
			set_onoff.rgb1en = BD6082GUL_LED_ON; /* ON */
		}
	}
	else
	{
		/* OFF */
		set_onoff.rgb1os = pg_led_congig->rbg_config.reg_07.rgb1os;
		set_onoff.rgb1en = BD6082GUL_LED_OFF; /* OFF */
	}

	ret = bd6082gul_rgb_led_set( &set_onoff );
	if( ret != BD6082GUL_LED_SET_OK )
		printk(KERN_INFO "led_brightness_set_rgb: bd6082gul_rgb_led_set(%d) = %d \n",set_onoff.rgb1en,ret);

}

static void led_brightness_set_rgb_opt(int device_no, enum led_brightness value)
{
	struct led_rgb_cnf_ctrl request;
	
	if( device_no == LED_DRV_IC_DEVICE_ID_SUB_RGB_PATERN ){
		bd6082gul_rgb_led_patern(value ,value, value );
		return;
	}
	else if( device_no == LED_DRV_IC_DEVICE_ID_SUB_RGB_TRGB ){
		memset( &request, 0, sizeof( request ) );
		request.trgb = value ;
		bd6082gul_rgb_led_cnf(&request);
		return;
	}
	else if( device_no == LED_DRV_IC_DEVICE_ID_SUB_RGB_RGB1OS ){
		pg_led_congig->rbg_config.reg_07.rgb1os  = value ;
	}
	else{
		printk(KERN_INFO "led_brightness_set_rgb_opt: device_no Error = %d \n",device_no );
	}
}


static void led_brightness_set_keybl(int device_no, enum led_brightness value)
{
	int				ret;

	struct led_group2_ctrl	set_onoff;
	struct led_key_bright	set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_NUM];

	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( set_brigh, 0, sizeof( set_brigh ) );

	/* check:brightness */
	if( value > LED_OFF )
	{
		/* ON */
		set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1].bright = value;
		set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1].set_flag = BD6082GUL_LED_ON;

		ret = bd6082gul_key_led_bright(	&set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_1],
										&set_brigh[LED_DRV_IC_KEYBL_BRIGHTNESS_2] );
		if( ret != BD6082GUL_LED_SET_OK )
			printk(KERN_INFO "led_brightness_set_keybl: bd6082gul_key_led_bright = %d \n",ret);

		set_onoff.led3en = BD6082GUL_LED_ON;
	}
	else
	{
		/* OFF */
		set_onoff.led3en = BD6082GUL_LED_OFF;
	}

	ret = bd6082gul_key_led_set( &set_onoff );
	if( ret != BD6082GUL_LED_SET_OK )
		printk(KERN_INFO "led_brightness_set_keybl: bd6082gul_key_led_set(%d,%d) = %d \n",set_onoff.led3en,set_onoff.led4en,ret);

}

static void led_brightness_set_flash(int device_no, enum led_brightness value)
{
	int				ret;
	unsigned char	mode ;

	struct led_group1_ctrl	set_onoff;
	struct led_flash_bright	set_brigh[LED_DRV_IC_FLASH_MODE_NUM];

	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( set_brigh, 0, sizeof( set_brigh ) );

	/* check:brightness */
	if( value > LED_OFF )
	{
		/* ON */
		if( device_no == LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH ){
			set_brigh[LED_DRV_IC_FLASH_MODE_FLASH].set_flag = BD6082GUL_LED_ON;
			set_brigh[LED_DRV_IC_FLASH_MODE_FLASH].bright = value ;
			mode = BD6082GUL_LED_ON ;
		}
		else{
			set_brigh[LED_DRV_IC_FLASH_MODE_TORCH].set_flag = BD6082GUL_LED_ON;
			set_brigh[LED_DRV_IC_FLASH_MODE_TORCH].bright = value ;
			mode = BD6082GUL_LED_OFF ;
		}
		ret = bd6082gul_flash_led_bright(	&set_brigh[LED_DRV_IC_FLASH_MODE_TORCH],
											&set_brigh[LED_DRV_IC_FLASH_MODE_FLASH] );
		if( ret != BD6082GUL_LED_SET_OK )
			printk(KERN_INFO "led_brightness_set_flash: bd6082gul_flash_led_bright = %d \n",ret);

		set_onoff.flleden = BD6082GUL_LED_ON;
		set_onoff.flmd = mode;
	}
	else
	{
		/* OFF */
		if( device_no == LED_DRV_IC_DEVICE_ID_SUB_FLASH_FLASH ){
			mode = BD6082GUL_LED_ON ;
		}
		else{
			mode = BD6082GUL_LED_OFF ;
		}
		set_onoff.flleden = BD6082GUL_LED_OFF;
		set_onoff.flmd = mode;
	}

	ret = bd6082gul_flash_led_set( &set_onoff );
	if( ret != BD6082GUL_LED_SET_OK )
		printk(KERN_INFO "led_brightness_set_flash: bd6082gul_flash_led_set(%d,%d) = %d \n",set_onoff.flleden,set_onoff.flmd,ret);
}

static void led_brightness_set_indicator(int device_no, enum led_brightness value)
{
	int				ret;

	struct led_group2_ctrl		set_onoff;
	struct led_indicator_bright	set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_NUM];

	memset( &set_onoff, 0, sizeof( set_onoff ) );
	memset( set_brigh, 0, sizeof( set_brigh ) );

	g_indicator_brightness.brightness[device_no] = value;

	set_brigh[device_no].bright = value ;
	set_brigh[device_no].set_flag =BD6082GUL_LED_ON;

	if( g_indicator_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED] != LED_DRV_IC_BRIGHTNESS_VALUE_OFF )
	{
//		set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED].bright = value ;
//		set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED].set_flag =BD6082GUL_LED_ON;
		set_onoff.led6en = BD6082GUL_LED_ON;
	}
	else
	{
		set_onoff.led6en = BD6082GUL_LED_OFF;
	}

	if( g_indicator_brightness.brightness[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN] != LED_DRV_IC_BRIGHTNESS_VALUE_OFF )
	{
//		set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN].bright = value ;
//		set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN].set_flag =BD6082GUL_LED_ON;
		set_onoff.led5en = BD6082GUL_LED_ON;
	}
	else
	{
		set_onoff.led5en = BD6082GUL_LED_OFF;
	}

	if( (set_onoff.led6en == BD6082GUL_LED_ON) || (set_onoff.led5en == BD6082GUL_LED_ON) )
	{
		ret = bd6082gul_indicator_led_bright(	&set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED],
												&set_brigh[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN] );
		if( ret != BD6082GUL_LED_SET_OK )
			printk(KERN_INFO "led_brightness_set_indicator: bd6082gul_indicator_led_bright = %d \n",ret);
	}

	ret = bd6082gul_indicator_led_set( &set_onoff );
	if( ret != BD6082GUL_LED_SET_OK )
		printk(KERN_INFO "led_brightness_set_indicator: bd6082gul_indicator_led_set(%d,%d) = %d \n",set_onoff.led5en,set_onoff.led6en,ret);

}

static int drv_ic_led_get_config(void)
{
	if( !pg_led_congig )
	{
		pg_led_congig = (struct drv_ic_led_configs *)kzalloc(sizeof(struct drv_ic_led_configs), GFP_KERNEL);
		if (pg_led_congig == NULL) {
			printk(KERN_ERR "drv_ic_led_get_config: no memory for device\n");
			return -ENOMEM;
		}
	}
	/* Read config data */

	/* RGB */
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]=0x32;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_1].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=0x32;

	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]=0x1E;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_2].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=0x1E;

	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_RED]=0x0E;
	pg_led_congig->rbg_config.lighting[LED_DRV_IC_RGB_LIGHTING_3].brightness[LED_DRV_IC_DEVICE_ID_RGB_GREEN]=0x0E;

	pg_led_congig->rbg_config.reg_07.rgb1en  = 0 ;
	pg_led_congig->rbg_config.reg_07.rgb1os  = 0 ;
	pg_led_congig->rbg_config.reg_07.rgb1eml = 0 ;
	pg_led_congig->rbg_config.reg_07.dmy1    = 0 ;

	/* Key Backlight */
	pg_led_congig->keybl_config.brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_1]=0x09;
	pg_led_congig->keybl_config.brightness[LED_DRV_IC_KEYBL_BRIGHTNESS_2]=0x09;

	/* Flash */
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_1].brightness[LED_DRV_IC_FLASH_TYPE_STILL] =0x13; 
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_1].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE] =0x01;
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_1].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ] =0x0B; 

	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_2].brightness[LED_DRV_IC_FLASH_TYPE_STILL] =0x09; 
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_2].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE] =0x01;
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_2].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ] =0x05; 

	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_3].brightness[LED_DRV_IC_FLASH_TYPE_STILL] =0x01; 
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_3].brightness[LED_DRV_IC_FLASH_TYPE_MOVIE] =0x01;
	pg_led_congig->flash_config.temp[LED_DRV_IC_FLASH_TEMP_3].brightness[LED_DRV_IC_FLASH_TYPE_IDLE ] =0x01; 

	/* Indicator */
	pg_led_congig->indicator_config.brightness[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_RED  ] =0x04;
	pg_led_congig->indicator_config.brightness[LED_DRV_IC_DEVICE_ID_SUB_INDICATOR_GREEN] =0x04;

	return 0;
}

static int drv_ic_led_free_config(void)
{
	if( pg_led_congig )
		kfree(pg_led_congig);
	pg_led_congig = NULL;

	return 0;
}


static int drv_ic_led_probe(struct platform_device *pdev)
{
	struct drv_ic_data		*pdrvdata;
	int						ret,cnt,cnt_rev;

	/***** Initialization *****/
	memset( &g_rgb_brightness, 0, sizeof( struct drv_ic_led_rgb_brightness ));
	memset( &g_indicator_brightness, 0, sizeof( struct drv_ic_led_indicator_brightness ));
	

	ret = drv_ic_led_get_config();							/* get config data	*/
	if( ret == -ENOMEM )
		goto err_alloc_failed;

	pdrvdata = (struct drv_ic_data *)kzalloc(sizeof(struct drv_ic_data), GFP_KERNEL);
	if (pdrvdata == NULL) {
		printk(KERN_ERR "drv_ic_led_probe: no memory for device\n");
		ret = -ENOMEM;
		goto err_alloc_failed;
	}

	memset(pdrvdata, 0, sizeof(struct drv_ic_data));

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RED].name = LED_DRV_IC_DEVICE_NAME_RGB_RED;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RED].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RED].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_GREEN].name = LED_DRV_IC_DEVICE_NAME_RGB_GREEN;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_GREEN].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_GREEN].brightness = LED_OFF;


	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_PATERN].name = LED_DRV_IC_DEVICE_NAME_RGB_PATERN;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_PATERN].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_PATERN].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TRGB].name = LED_DRV_IC_DEVICE_NAME_RGB_TRGB;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TRGB].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_TRGB].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RGB1OS].name = LED_DRV_IC_DEVICE_NAME_RGB_RGB1OS;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RGB1OS].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_RGB_RGB1OS].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEYBL].name = LED_DRV_IC_DEVICE_NAME_KEYBL;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEYBL].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_KEYBL].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH].name = LED_DRV_IC_DEVICE_NAME_FLASH_TORCH;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_TORCH].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH].name = LED_DRV_IC_DEVICE_NAME_FLASH_FLASH;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_FLASH_FLASH].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_INDICATOR_RED].name = LED_DRV_IC_DEVICE_NAME_INDICATOR_RED;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_INDICATOR_RED].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_INDICATOR_RED].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_INDICATOR_GREEN].name = LED_DRV_IC_DEVICE_NAME_INDICATOR_GREEN;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_INDICATOR_GREEN].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_INDICATOR_GREEN].brightness = LED_OFF;

	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_BTNBL].name = LED_DRV_IC_DEVICE_NAME_BTNBL;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_BTNBL].brightness_set = led_brightness_set;
	pdrvdata->leds[LED_DRV_IC_DEVICE_ID_BTNBL].brightness = LED_OFF;

	for (cnt = 0; cnt < LED_DRV_IC_DEVICE_ID_NUM; cnt++) {	/* red, green, blue jogball */
		ret = led_classdev_register(&pdev->dev, &pdrvdata->leds[cnt]);
		if (ret) {
			printk(KERN_ERR
			       "drv_ic_led_probe: led_classdev_register failed\n");
			goto err_led_classdev_register_failed;
		}
	}

	dev_set_drvdata(&pdev->dev, pdrvdata);

	return 0;

err_led_classdev_register_failed:
	for (cnt_rev = 0; cnt_rev < cnt; cnt_rev++)
		led_classdev_unregister(&pdrvdata->leds[cnt_rev]);

err_alloc_failed:
	drv_ic_led_free_config();
	return ret;

}

static int __devexit drv_ic_led_remove(struct platform_device *pdev)
{
	struct drv_ic_data		*pdrvdata;
	int						cnt;

	drv_ic_led_free_config();

	pdrvdata = (struct drv_ic_data *)platform_get_drvdata(pdev);

	for (cnt = 0; cnt < LED_DRV_IC_DEVICE_ID_NUM; cnt++) 
		led_classdev_unregister(&pdrvdata->leds[cnt]);

	if( pdrvdata )
		kfree(pdrvdata);
	return 0;
}
/*----------------------------------------------------------------------------
    platform_driver
----------------------------------------------------------------------------*/
#ifndef WIN32
static struct platform_driver g_drv_ic_led_driver = {
	.probe		= drv_ic_led_probe,
	.remove		= __devexit_p(drv_ic_led_remove),
	.driver		= {
		.name	= "led-drv-ic",
		.owner	= THIS_MODULE,
	},
};
#else
static struct platform_driver g_drv_ic_led_driver;
#endif


static int __init drv_ic_led_init(void)
{
	return platform_driver_register(&g_drv_ic_led_driver);
}

static void __exit drv_ic_led_exit(void)
{
	platform_driver_unregister(&g_drv_ic_led_driver);
}

MODULE_DESCRIPTION("LED Drv IC Driver");
MODULE_LICENSE("GPL");

module_init(drv_ic_led_init);
module_exit(drv_ic_led_exit);

