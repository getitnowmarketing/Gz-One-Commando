/*
 * synopex.c - Synopex SPI touchscreen
 *
 * Copyright (C) 2010 NEC Corporation
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

 
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/kthread.h>
#include <linux/input/msm_ts.h>


#include <linux/proc_fs.h>
#include <asm/uaccess.h>


#include <mach/synopex.h>
#include <mach/gpio.h>
#include <linux/touch_panel_cmd.h>
#include <linux/earlysuspend.h>


#include <linux/debugfs.h>
#include <linux/delay.h>

#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

#include <linux/oemnc_info.h>

#define SYNOPEX_BUTTON_NUM   4

#define SYNOPEX_REG_TOUCH    0x00
#define SYNOPEX_REG_X1_H     0x01
#define SYNOPEX_REG_X1_L     0x02
#define SYNOPEX_REG_Y1_H     0x03
#define SYNOPEX_REG_Y1_L     0x04
#define SYNOPEX_REG_X2_H     0x05
#define SYNOPEX_REG_X2_L     0x06
#define SYNOPEX_REG_Y2_H     0x07
#define SYNOPEX_REG_Y2_L     0x08
#define SYNOPEX_REG_REV      0x09
#define SYNOPEX_REG_SETUP    0x0A
#define SYNOPEX_REG_SETUP_WR 0xC0


#define SYNOPEX_MODE_ENTER_DEEP   0x20
#define SYNOPEX_MODE_EXIT_DEEP    0x00
#define SYNOPEX_MODE_ENTER_SLEEP  0x10
#define SYNOPEX_MODE_EXIT_SLEEP   ~0x10
#define SYNOPEX_MODE_MULTI        0x08
#define SYNOPEX_MODE_SINGLE       0x00
#define SYNOPEX_MODE_ENTER_2SCAN   0x04  /* GPS noise measures  add  */
#define SYNOPEX_MODE_SOFT_RESET   0x01

#define SYNOPEX_MODE_BUTTON3   0x80
#define SYNOPEX_MODE_BUTTON2   0x40
#define SYNOPEX_MODE_BUTTON1   0x20
#define SYNOPEX_MODE_BUTTON0   0x10
#define SYNOPEX_MODE_BUTTON    0x01 /* Single BUTTON Touch */
#define SYNOPEX_MODE_ERR_TOUCH 0x02 /* ERROR */
#define SYNOPEX_MODE_2ND_TOUCH 0x02 /* Second Touch */
#define SYNOPEX_MODE_1ST_TOUCH 0x04 /* First Touch */

#define SYNOPEX_MODE_BUTTON_CHECK	0x07	/* Button Check */

#define SYNOPEX_MODE_MASK  (SYNOPEX_MODE_1ST_TOUCH | SYNOPEX_MODE_2ND_TOUCH | SYNOPEX_MODE_BUTTON)

#define SYNOPEX_MODE_BUTTON_MASK      \
	(SYNOPEX_MODE_BUTTON3 | SYNOPEX_MODE_BUTTON2 \
	 | SYNOPEX_MODE_BUTTON1 | SYNOPEX_MODE_BUTTON2)


#define SYNOPEX_CMD_TOUCH_RD    SYNOPEX_REG_TOUCH
#define SYNOPEX_CMD_X1_H_RD     SYNOPEX_REG_X1_H
#define SYNOPEX_CMD_X1_L_RD     SYNOPEX_REG_X1_L
#define SYNOPEX_CMD_Y1_H_RD     SYNOPEX_REG_Y1_H
#define SYNOPEX_CMD_Y1_L_RD     SYNOPEX_REG_Y1_L
#define SYNOPEX_CMD_X2_H_RD     SYNOPEX_REG_X2_H
#define SYNOPEX_CMD_X2_L_RD     SYNOPEX_REG_X2_L
#define SYNOPEX_CMD_Y2_H_RD     SYNOPEX_REG_Y2_H
#define SYNOPEX_CMD_Y2_L_RD     SYNOPEX_REG_Y2_L
#define SYNOPEX_CMD_REV_RD      SYNOPEX_REG_REV
#define SYNOPEX_CMD_SETUP_RD    SYNOPEX_REG_SETUP
#define SYNOPEX_CMD_DUMMY       (0)
#define SYNOPEX_CMD_ENTER_SLEEP_WR (SYNOPEX_REG_SETUP_WR | SYNOPEX_MODE_ENTER_SLEEP | SYNOPEX_MODE_MULTI)
#define SYNOPEX_CMD_ENTER_DEEP_WR  (SYNOPEX_REG_SETUP_WR | SYNOPEX_MODE_ENTER_DEEP | SYNOPEX_MODE_MULTI)
#define SYNOPEX_CMD_EXIT_SLEEP_WR  ((SYNOPEX_REG_SETUP_WR & SYNOPEX_MODE_EXIT_SLEEP) | SYNOPEX_MODE_MULTI)
#define SYNOPEX_CMD_EXIT_DEEP_RD   SYNOPEX_MODE_EXIT_DEEP
#define SYNOPEX_CMD_SOFT_RESET_WR  (SYNOPEX_REG_SETUP_WR | SYNOPEX_MODE_SOFT_RESET | SYNOPEX_MODE_MULTI)
#define SYNOPEX_CMD_MULTI_WR       (SYNOPEX_REG_SETUP_WR | SYNOPEX_MODE_MULTI)
#define SYNOPEX_CMD_SINGLE_WR      (SYNOPEX_REG_SETUP_WR | SYNOPEX_MODE_SINGLE)
/* GPS noise measures  add start */
#define SYNOPEX_CMD_ENTER_2SCAN_WR (SYNOPEX_REG_SETUP_WR | SYNOPEX_MODE_ENTER_SLEEP | SYNOPEX_MODE_MULTI | SYNOPEX_MODE_ENTER_2SCAN)
#define SYNOPEX_CMD_EXIT_2SCAN_WR  (SYNOPEX_REG_SETUP_WR  | SYNOPEX_MODE_MULTI)
/* GPS noise measures  add end */


#define DATA_INDEX(reg)  (reg + 1)

#define SYNOPEX_STATE_NONE          0x00000000
#define SYNOPEX_STATE_INIT_WAIT     0x00000001
#define SYNOPEX_STATE_INIT_DONE     0x00000002
#define SYNOPEX_STATE_SUSPEND       0x00000004

#define SYNOPEX_FWUP_STATE_NONE     0x00000000
#define SYNOPEX_FWUP_STATE_EXEC     0x00000001

#define SYNOPEX_FWUP_RET_OK					0x00
#define SYNOPEX_FWUP_RET_ERR_SRESET			0x01	/* Init Error (Soft Reset)		*/
#define SYNOPEX_FWUP_RET_ERR_GPIO			0x02	/* Init Error (GPIO Setting)	*/
#define SYNOPEX_FWUP_RET_ERR_INIT_I2C		0x03	/* Init Error (Change I2C)		*/
#define SYNOPEX_FWUP_RET_ERR_I2C_READ_ACK	0x10	/* I2C Read Error (No ACK)		*/
#define SYNOPEX_FWUP_RET_ERR_I2C_READ_OTHER	0x11	/* I2C Read Error (Other)		*/
#define SYNOPEX_FWUP_RET_ERR_I2C_WRITE_ACK	0x12	/* I2C Write Error (No ACK)		*/
#define SYNOPEX_FWUP_RET_ERR_I2C_WRITE_OTHER 0x13	/* I2C Write Error (Other)		*/
#define SYNOPEX_FWUP_RET_ERR_FW_NO_FILE		0x20	/* Update file doesn't exist	*/
#define SYNOPEX_FWUP_RET_ERR_BUSY_TOUT		0x21	/* Busy signal timeout			*/
#define SYNOPEX_FWUP_RET_ERR_CSUM			0x22	/* Check sum error				*/
#define SYNOPEX_FWUP_RET_ERR_UNINIT			0x23	/* The error during start-up	*/
#define SYNOPEX_FWUP_RET_ERR_FW_SAME		0x99	/* FW is the same version	 	*/
#define SYNOPEX_FWUP_RET_ERR_OTHER			0xFF	/* Other Error					*/


#define	SYNOPEX_I2C_IDA_ADDR				0x01
#define	SYNOPEX_I2C_IDA_DATA				0x00

#define	SYNOPEX_I2C_IDA_MEM					0x02
#define	SYNOPEX_I2C_MEM_CODE				0x00
#define	SYNOPEX_I2C_MEM_XDATA				0x01

#define	SYNOPEX_I2C_IDA_CTRL				0x03
#define	SYNOPEX_I2C_CTRL_MASK				0xBB
#define	SYNOPEX_I2C_CTRL_UNMASK				0x5A

#define	SYNOPEX_I2C_IDA_RESPONSE			0x04

#define	SYNOPEX_I2C_IDA_ERASE				0xE0

#define	SYNOPEX_I2C_IDA_RUN					0xB0
#define	SYNOPEX_I2C_IDA_RUN2				0xB2
#define	SYNOPEX_I2C_IDA_RUN0				0xBF

#define	SYNOPEX_I2C_IDA_VERSION				0xF1

#define	SYNOPEX_I2C_IDA_CHECKSUM			0x30

#define	SYNOPEX_AP_BASE						0x1000
#define	SYNOPEX_AP_END						0x7999
#define	SYNOPEX_SIZE_1KB					0x0400
#define	SYNOPEX_SIZE_32						32

#define	SYNOPEX_FW_REV						0x100


static u32 spi_delay = 100;

static u32 max_speed_hz;
static u8 rx_data[11]; 
static u8 setup_data = 0;

static int x1_pos, y1_pos, x2_pos, y2_pos;

static const struct {
	const char *name;
	mode_t mode;
	void *data;
} debugfs_synopex_env[] = {
	{"max_speed_hz",  S_IRUGO | S_IWUSR, &max_speed_hz},
	{"spi_delay",  S_IRUGO | S_IWUSR, &spi_delay},
};


static struct workqueue_struct *synopex_wq;

struct synopex_ts_abs {
    int                         finger;
    int                         x1;
    int                         y1;
    int                         x2;
    int                         y2;
};

struct synopex_ts_data {
	struct spi_device			*spi;
	struct input_dev			*input_dev;
	struct work_struct			work;
	struct task_struct			*ts_task;
	u8							key_down[SYNOPEX_BUTTON_NUM];
	int							touch[2];
	int							key_code[SYNOPEX_BUTTON_NUM];
	unsigned char				rev;
	uint32_t					hw_rev;

	struct dentry 				*dent_synopex_root;
	struct dentry				*dent_synopex_env[ARRAY_SIZE(debugfs_synopex_env)];

	struct hrtimer timer;
	struct early_suspend early_suspend;
	u8							is_suspend;
};

static struct msm_gpio synopex_i2c_gpio[] = {
	{ GPIO_CFG(45, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "synopex_i2c_scl"   },
	{ GPIO_CFG(47, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "synopex_boot_cond" },
	{ GPIO_CFG(48, 0, GPIO_CFG_OUTPUT,  GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "synopex_reset"     },
	{ GPIO_CFG(89, 0, GPIO_CFG_INPUT,   GPIO_CFG_PULL_UP, GPIO_CFG_2MA), "synopex_i2c_sda"   },
};

static void synopex_early_suspend(struct early_suspend *h);
static void synopex_late_resume(struct early_suspend *h);

static struct spi_device	*g_spi = NULL;
static volatile int g_touch_state = SYNOPEX_STATE_NONE;
static volatile int g_touch_fw_update_state = SYNOPEX_FWUP_STATE_NONE;
static void (*g_tp_cmd_callback)(void *) = NULL;
static touch_panel_cmd_callback_param_coord g_touch_coord;
static volatile bool g_TouchDeepState = false;


static int synopex_cmd_txrx(struct spi_device *spi, u8 *ptxdata, u8 *prxdata, int len, int txrx_delay )
{
	u8 tx_buf;
	u8 rx_buf;
	struct spi_message m;
	struct spi_transfer t;
	int rc = 0,cnt;

	memset(&t, 0, sizeof t);
	t.tx_buf = &tx_buf;
	t.rx_buf = &rx_buf;
	t.len = 1;

	t.speed_hz = max_speed_hz;

	for(cnt=0; cnt<len; cnt++)
	{

		if( txrx_delay )
			udelay(txrx_delay);

		tx_buf = *(ptxdata + cnt);
		rx_buf = 0;

		spi_message_init(&m);
		spi_message_add_tail(&t, &m);

		rc = spi_sync(spi, &m);

		if (rc)
			goto error_exit;

		if( prxdata )
			*(prxdata + cnt) = rx_buf;
	}

error_exit:
	printk(KERN_ERR "[synopex]%s: Enter cmd tx Exit (rc:%d)\n", __func__, rc);
	return rc;
}


/*============================================================================
     LOCAL I2C MODULE
==============================================================================*/
/* DEBUG DEFINE */
//#define	I2C_LOCAL_DEBUG
//#define	I2C_LOCAL_DEBUG_NO_GPIO
//#define	I2C_LOCAL_DEBUG_SEND_BIT
#define	I2C_LOCAL_DEBUG_MAICRO_SEC
//#define	I2C_LOCAL_DEBUG_READ

/*---------------------------------------------------------------------------*/
#define	ON									1
#define	OFF									0

#define	SDA_READ_MODE						1
#define	SDA_WRITE_MODE						0

#define	GPIO_BIT_MASK						0x01

#define	GPIO_LOCAL_SCL_PORT					45
#define	GPIO_LOCAL_SDA_PORT					89

#define	I2C_SYNOPEX_SLAVE_ADDRESS			0x40

#define	WAIT_1_4_CYCLE						25
#define	WAIT_1_2_CYCLE						( WAIT_1_4_CYCLE * 2 ) 
#define	WAIT_3_4_CYCLE						( WAIT_1_4_CYCLE * 3 ) 
#define	WAIT_1_1_CYCLE						( WAIT_1_4_CYCLE * 4 ) 

#define	I2C_LOCAL_DATA_MAX					64

#define	I2C_LOCAL_OK						0x00 ;
#define	I2C_LOCAL_ERROR_DATA_SIZE			0x80 ;
#define	I2C_LOCAL_ERROR_WRITE_ADDRESS		0x81 ;
#define	I2C_LOCAL_ERROR_WRITE_DATA			0xA0 ;/* A0-BF */
#define	I2C_LOCAL_ERROR_READ_DATA			0xC0 ;/* C0-DF */

/*---------------------------------------------------------------------------*/
struct i2c_local_data_b_st{
	unsigned short	bit0:1 ;
	unsigned short	non2:7 ;
	unsigned short	bit8:1 ;
	unsigned short	non1:7 ;
};

struct i2c_local_data_c_st{
	unsigned char		uc0;
	unsigned char		uc1;
};

union u_i2c_local_data_us{
	struct i2c_local_data_b_st	st_b;
	struct i2c_local_data_c_st	st_c;
	unsigned short				us;
};

/*---------------------------------------------------------------------------*/

static char i2c_local_sda_mode = SDA_WRITE_MODE ;

/*---------------------------------------------------------------------------*/

int i2c_local_write_block( int d_size, unsigned char *data );
int i2c_local_read_block( int d_size, unsigned char *data );

/*---------------------------------------------------------------------------*/

/* GPS noise measures 15s -------------- add start */
static struct timer_list touch_rel_timer;
unsigned char touch_rel_timer_flg = 0;
/* GPS noise measures 15s -------------- add end */

/* GPS noise measures -------------- add start */
struct work_struct			work2;
struct work_struct			work3;

void spi_touch_2scan_enter(struct work_struct *data);
void spi_touch_2scan_exit(struct work_struct *data);


bool GPS_android_loc_state = false;
bool GPS_vzw_loc_state = false;
bool GPS_2scan_state = false;


void spi_touch_2scan_enter(struct work_struct *data)
{
	int rc = 0;
	u8 tx_buf;


	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	tx_buf = SYNOPEX_CMD_ENTER_2SCAN_WR;

	/* GPS noise measures 15s ----------- add start */
	if( touch_rel_timer_flg) {
		del_timer(&touch_rel_timer);
		touch_rel_timer_flg = 0;

		printk(KERN_DEBUG "del_timer  ,%d\n",touch_rel_timer_flg);

	}
	/* GPS noise measures 15s ----------- add end */

	if( g_TouchDeepState == false )
		rc = synopex_cmd_txrx( g_spi, &tx_buf, NULL, 1, 0 );
	else
		rc = 0;

	GPS_2scan_state = true;

	if (rc){
		printk(KERN_ERR "[synopex]%s: enter with error %d\n", __func__, rc);
		return;
	}

	printk(KERN_DEBUG "spi_touch_2scan_enter ,%d\n",rc);

	mdelay(25);
}

void spi_touch_2scan_exit(struct work_struct *data)
{
	int rc = 0;
	u8 tx_buf;


	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	tx_buf = SYNOPEX_CMD_EXIT_2SCAN_WR;

	/* GPS noise measures 15s ----------- add start */
	if( touch_rel_timer_flg) {
		del_timer(&touch_rel_timer);
		touch_rel_timer_flg = 0;

		printk(KERN_DEBUG "del_timer  ,%d\n",touch_rel_timer_flg);

	}
	/* GPS noise measures 15s ----------- add end */

	if( g_TouchDeepState == false )
		rc = synopex_cmd_txrx( g_spi, &tx_buf, NULL, 1, 0 );
	else
		rc = 0;

	GPS_2scan_state = false;

	if (rc){
		printk(KERN_ERR "[synopex]%s: enter with error %d\n", __func__, rc);
		return;
	}

	printk(KERN_DEBUG "spi_touch_2scan_exit ,%d\n",rc);

	mdelay(25);
}


void nec_touch_android_lbs_started(bool started)
{

	if(started){
		if(!GPS_vzw_loc_state){
			printk(KERN_DEBUG "nec_touch_android_lbs_started ,%d\n",started);
			schedule_work(&work2);
		}
		GPS_android_loc_state = started;
	}
	else{
		if(!GPS_vzw_loc_state){
			printk(KERN_DEBUG "nec_touch_android_lbs_started ,%d\n",started);
			schedule_work(&work3);
		}
		GPS_android_loc_state = started;
	}

	printk(KERN_DEBUG "nec_touch_android_lbs_started,%d\n",GPS_android_loc_state);

}
EXPORT_SYMBOL(nec_touch_android_lbs_started);

void nec_touch_vzw_lbs_started(bool started)
{
	if(started){
		if(!GPS_android_loc_state){
			printk(KERN_DEBUG "nec_touch_vzw_lbs_started ,%d\n",started);
			schedule_work(&work2);
		}
		GPS_vzw_loc_state = started;
	}
	else{
		if(!GPS_android_loc_state){
			printk(KERN_DEBUG "nec_touch_vzw_lbs_started ,%d\n",started);
			schedule_work(&work3);
		}
		GPS_vzw_loc_state = started;
	}
	printk(KERN_DEBUG "nec_touch_vzw_lbs_started,%d\n",GPS_vzw_loc_state);

}
EXPORT_SYMBOL(nec_touch_vzw_lbs_started);

/* GPS noise measures -------------- add end */


/* GPS noise measures 15s -------------- add start */
static void touch_rel_timeout(unsigned long arg)
{

	schedule_work(&work2);

}

static void touch_reltimer_init(void)
{
	init_timer(&touch_rel_timer);

	touch_rel_timer.function = touch_rel_timeout;
	touch_rel_timer.data = 0;
	touch_rel_timer.expires = jiffies + 15*HZ;
	printk(KERN_DEBUG "test__log_TOUCHKEY1\n");
}

/* GPS noise measures 15s -------------- add end */


static void i2c_local_scl_set( unsigned char scl, unsigned int n_time)
{
	/* gpio set */
	if (scl){
		gpio_direction_input(GPIO_LOCAL_SCL_PORT);
	}
	else{
		gpio_direction_output(GPIO_LOCAL_SCL_PORT, 0 );
	}

	/* Wait time */
	if( n_time != 0 ){
		udelay(n_time);
	}
}

static void i2c_local_sda_set( unsigned char sda, unsigned int n_time)
{
	/* gpio set */
	/* From i2c-gpio.c #20 i2c_gpio_setsda_dir() */
	if (sda){
		gpio_direction_input(GPIO_LOCAL_SDA_PORT);
	}
	else{
		gpio_direction_output(GPIO_LOCAL_SDA_PORT, 0);
	}
	/* Wait time */
	if( n_time != 0 ){
		udelay(n_time);
	}
}

static unsigned char i2c_local_sda_get( unsigned int n_time)
{
	unsigned char	data;
	
	/* SDA */
	/* gpio get */
	data = gpio_get_value(GPIO_LOCAL_SDA_PORT);		/* SDA */
	/* Wait time */
	if( n_time != 0 ){
		udelay(n_time);
	}
	return data;
}

static void i2c_local_scl_write_mode_set(void)
{
	
	/* SCL Write mode */
	gpio_direction_output(GPIO_LOCAL_SCL_PORT, 0);
}

static void i2c_local_sda_write_mode_set(void)
{
	
	/* SDA Write mode */
	gpio_direction_output(GPIO_LOCAL_SDA_PORT, 0);
	i2c_local_sda_mode = SDA_WRITE_MODE ;
}

static void i2c_local_sda_read_mode_set(void)
{
	
	/* SDA Read mode */
	gpio_direction_input(GPIO_LOCAL_SDA_PORT);
	i2c_local_sda_mode = SDA_READ_MODE ;
}

static int i2c_local_write( unsigned char data )
{
	int				bit_cnt ;
	union u_i2c_local_data_us	s_data ;
	unsigned char		ret = 0 ;
	
	i2c_local_scl_set( OFF, OFF );					/* SDA set OK */
	
	/* SDA Write mode(SDA = 0) */
	i2c_local_sda_write_mode_set();
	
	/* data */
	s_data.us = 0;
	s_data.st_c.uc0 = data;
	
	for( bit_cnt = 8 ; bit_cnt > 0 ; bit_cnt-- ){
		i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );	/* SDA set OK */
		/* Master Data Set */
		s_data.us = s_data.us << 1 ;
		i2c_local_sda_set( s_data.st_b.bit8 , WAIT_1_4_CYCLE );
		i2c_local_scl_set( ON,  WAIT_1_2_CYCLE );	/* SDA set NG */
		/* Slave Data Read */
	}
	i2c_local_scl_set( OFF, 0 );		/* SDA set OK */
	
	/* SDA Read mode */
	i2c_local_sda_read_mode_set();
	
	i2c_local_scl_set( OFF, WAIT_1_2_CYCLE );		/* SDA set OK */
	i2c_local_scl_set( ON,  WAIT_1_4_CYCLE );		/* SDA set NG */
	
	/* ACK ? */
	ret = i2c_local_sda_get( WAIT_1_4_CYCLE );
	i2c_local_scl_set( OFF, 0 );					/* SDA set OK */
	
	if( ( ret & GPIO_BIT_MASK ) != OFF ){
		/* Error */
		printk(KERN_ERR
			   "i2c_local: Ack NG Error \n" );
	}
	return ret;
}

static int i2c_local_read( unsigned char *data )
{
	int				bit_cnt ;
	union u_i2c_local_data_us	s_data ;
	unsigned char		ret = 0 ;
	
	i2c_local_scl_set( OFF, OFF );					/* SDA set OK */
	
	/* SDA Read mode */
	i2c_local_sda_read_mode_set();
	
	/* data */
	s_data.us = 0;
	for( bit_cnt = 8 ; bit_cnt > 0 ; bit_cnt-- ){
		i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );	/* SDA set OK */
		i2c_local_scl_set( ON,  WAIT_1_4_CYCLE );	/* SDA set NG */
		/* Master Data Read */
		s_data.us = s_data.us  << 1 ;
		s_data.st_c.uc1 = i2c_local_sda_get( WAIT_1_4_CYCLE );
		s_data.st_b.bit0 = s_data.st_b.bit8 ;
		/* Slave Data Read */
	}
	i2c_local_scl_set( OFF, WAIT_1_2_CYCLE );		/* SDA set OK */
	
	/* SDA Write mode(SDA = 0) */
	i2c_local_sda_write_mode_set();
	
	/* Ack Set */
	i2c_local_scl_set( ON,  WAIT_1_2_CYCLE );		/* SDA set NG */
	i2c_local_scl_set( OFF, 0 );					/* SDA set OK */
	
	*data = s_data.st_c.uc0 ;
	
	return ret ;
}

int i2c_local_write_block( int d_size, unsigned char *data )
{
	int		ret ;
	int		index ;
	
	if( d_size > I2C_LOCAL_DATA_MAX ){
		/* Error */
		printk(KERN_ERR
			   "i2c_local: data_size_error \n" );
		return I2C_LOCAL_ERROR_DATA_SIZE ;
	}
	
	i2c_local_scl_write_mode_set();
	i2c_local_sda_write_mode_set();
	
	i2c_local_sda_set( ON,  OFF );
	i2c_local_scl_set( ON,  WAIT_1_1_CYCLE );
	msleep(1);
	
	/* Start */
	i2c_local_sda_set( OFF, WAIT_1_4_CYCLE );
	/* address */
	ret = i2c_local_write( I2C_SYNOPEX_SLAVE_ADDRESS | SDA_WRITE_MODE );
	if( ret != OFF ){
		/* Error */
		i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );
		
		/* SDA Write mode(SDA = 0) */
		i2c_local_sda_write_mode_set();
		
		i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );
		i2c_local_scl_set( ON,  WAIT_1_4_CYCLE );
		/* stop */
		i2c_local_sda_set( ON,  WAIT_1_4_CYCLE );
		
		printk(KERN_ERR
			   "i2c_local: i2c_local_write_block Write Address Error \n" );
		return I2C_LOCAL_ERROR_WRITE_ADDRESS ;
	}
	/* data */
	for( index = 0 ; index < d_size ; index++ ){
		ret = i2c_local_write( data[index] );
		if( ret != OFF ){
			/* Error */
			printk(KERN_ERR
				   "i2c_local: i2c_local_write_block Write Data Error (%d) \n", index );
			ret = I2C_LOCAL_ERROR_WRITE_DATA + (char)index ;
			return ret;
		}
	}
	i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );
	
	/* SDA Write mode(SDA = 0) */
	i2c_local_sda_write_mode_set();
	
	i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );
	i2c_local_scl_set( ON,  WAIT_1_4_CYCLE );
	/* stop */
	i2c_local_sda_set( ON,  WAIT_1_4_CYCLE );
	
	return I2C_LOCAL_OK;
}

int i2c_local_read_block( int d_size, unsigned char *data )
{
	int		ret ;
	int		index ;
	
	if( d_size > I2C_LOCAL_DATA_MAX ){
		/* Error */
		printk(KERN_ERR
			   "i2c_local: data_size_error \n" );
		return I2C_LOCAL_ERROR_DATA_SIZE ;
	}
	
	i2c_local_scl_write_mode_set();
	i2c_local_sda_write_mode_set();
	
	i2c_local_sda_set( ON,  OFF );
	i2c_local_scl_set( ON,  WAIT_1_1_CYCLE );
	msleep(1);
	
	/* Start */
	i2c_local_sda_set( OFF, WAIT_1_4_CYCLE );
	/* address */
	ret = i2c_local_write( I2C_SYNOPEX_SLAVE_ADDRESS | SDA_READ_MODE );
	if( ret != OFF ){
		/* Error */
		printk(KERN_ERR
			   "i2c_local: i2c_local_read_block Write Address Error \n" );
		return I2C_LOCAL_ERROR_WRITE_ADDRESS ;
	}
	/* data */
	for( index = 0 ; index < d_size ; index++ ){
		ret = i2c_local_read( &data[index] );
		if( ret != OFF ){
			/* Error */
			printk(KERN_ERR
				   "i2c_local: i2c_local_read_block Read Data Error (%d) \n", index );
			ret = I2C_LOCAL_ERROR_READ_DATA + (char)index ;
			return ret ;
		}
	}
	i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );
	
	/* SDA Write mode(SDA = 0) */
	i2c_local_sda_write_mode_set();
	
	i2c_local_scl_set( OFF, WAIT_1_4_CYCLE );
	i2c_local_scl_set( ON,  WAIT_1_4_CYCLE );
	/* stop */
	i2c_local_sda_set( ON,  WAIT_1_4_CYCLE );
	
	return I2C_LOCAL_OK;
}

/*============================================================================*/

static int synopex_i2c_busy_check( void )
{
	int	cnt;
	for(cnt=0;cnt<2000;cnt++) {
		if(gpio_get_value(92)) {
			return SYNOPEX_FWUP_RET_OK;
		}
		msleep(1);				//delay 1ms
	}
	return SYNOPEX_FWUP_RET_ERR_BUSY_TOUT;
}
static int synopex_i2c_write_cmd(unsigned char *cmd,int size)
{
	int ret = SYNOPEX_FWUP_RET_OK;
	char		dbg_str[256];
	int			dbg_cnt;

	ret = i2c_local_write_block( size, cmd );
	if( ret ){
		return SYNOPEX_FWUP_RET_ERR_I2C_WRITE_ACK;
	}

	for(dbg_cnt=0; dbg_cnt<size; dbg_cnt++)
		sprintf( &dbg_str[dbg_cnt*3],"%02x,",*(cmd+dbg_cnt));

	if( synopex_i2c_busy_check())
		ret = SYNOPEX_FWUP_RET_ERR_BUSY_TOUT;

	return ret;
}
static int synopex_i2c_read_cmd(unsigned char *cmd,int size)
{
	int ret = SYNOPEX_FWUP_RET_OK;

	ret = i2c_local_read_block( size, cmd );
	if( ret ){
		return SYNOPEX_FWUP_RET_ERR_I2C_READ_ACK;
	}

	if( synopex_i2c_busy_check())
		ret = SYNOPEX_FWUP_RET_ERR_BUSY_TOUT;

	return ret;
}

static int synopex_set_protection(int protect)
{
	unsigned char	cmd_string[2];
	cmd_string[0]=SYNOPEX_I2C_IDA_CTRL;

	if(protect) {
		cmd_string[1]=SYNOPEX_I2C_CTRL_MASK;
	}
	else {
		cmd_string[1]=SYNOPEX_I2C_CTRL_UNMASK;
	}

	return synopex_i2c_write_cmd(cmd_string, 2);
}

static int synopex_erase_address(int addr)
{
	unsigned char	cmd_string[5];

	cmd_string[0]=SYNOPEX_I2C_IDA_ERASE;
	cmd_string[1]=(unsigned char)(addr>>8);
	cmd_string[2]=(unsigned char)(addr);
	cmd_string[3]=(unsigned char)(addr>>8);
	cmd_string[4]=(unsigned char)(addr);
	return synopex_i2c_write_cmd(cmd_string, 5);
}

static int synopex_erase_tsp(int addr,int size)
{
	int		i;
	int		result;

	for(i=0;i<size;i+=SYNOPEX_SIZE_1KB) {
		result=synopex_erase_address(addr+i);
		if(result!=0) {
			printk(KERN_ERR "[synopex]%s erase address failed: %x\n", __func__, result);
			return result;
		}
	}
	return SYNOPEX_FWUP_RET_OK;
}
static int synopex_set_address(int addr)
{
	unsigned char	cmd_string[3];

	cmd_string[0]=SYNOPEX_I2C_IDA_ADDR;
	cmd_string[1]=(unsigned char)(addr>>8);
	cmd_string[2]=(unsigned char)addr;
	return synopex_i2c_write_cmd(cmd_string,3);
}

static int synopex_write_data(const unsigned char *dt,int size)
{
	int		i;
	unsigned char	cmd_string[256];

	cmd_string[0]=SYNOPEX_I2C_IDA_DATA;
	for(i=0;i<size;i++)
		cmd_string[i+1]=dt[i];
	return synopex_i2c_write_cmd(cmd_string,size+1);
}

static int synopex_program_tsp(const unsigned char *code,int addr,int size)
{
	int				i;
	int				result;
	unsigned char	tmp_code[SYNOPEX_SIZE_32];
	unsigned char	*pcode;

	result=synopex_set_address(addr);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s set address failed: %x\n", __func__, result);
		return result;
	}
	for(i=0;i<size;i+=SYNOPEX_SIZE_32) {
		pcode = (unsigned char *)(code+i);

		if( i==0 ) {
			memcpy(tmp_code,code+i,SYNOPEX_SIZE_32);
			tmp_code[0] = 0xFF;
			pcode = tmp_code;
		}

		if( SYNOPEX_SIZE_32 > (size-i) )
		{
			memset(tmp_code,0xFF,SYNOPEX_SIZE_32);
			memcpy(tmp_code,code+i,(size-i));
			pcode = tmp_code;
		}

		result=synopex_write_data(pcode,SYNOPEX_SIZE_32);
		if(result!=0) {
			printk(KERN_ERR "[synopex]%s write data failed: %x\n", __func__, result);
			return result;
		}
	}
	return SYNOPEX_FWUP_RET_OK;
}

static int synopex_check_checksum(const unsigned char *code,int addr,int size)
{
	unsigned char	cmd_string[6];
	uint16_t		hw_checksum = 0;
	uint16_t		sw_checksum = 0;
	int				cnt;
	int				rc;

	cmd_string[0]=SYNOPEX_I2C_IDA_CHECKSUM;
	cmd_string[1]=(unsigned char)(addr>>8);
	cmd_string[2]=(unsigned char)(addr   );
	cmd_string[3]=(unsigned char)((addr+size-1)>>8);
	cmd_string[4]=(unsigned char)((addr+size-1)   );
	rc = synopex_i2c_write_cmd(cmd_string,5);
	if (rc){
		printk(KERN_ERR "[synopex]%s check sum command write failed: %x\n", __func__, rc);
		return rc;
	}

	rc = synopex_i2c_read_cmd(cmd_string,6);
	if (rc){
		printk(KERN_ERR "[synopex]%s check sum command read failed: %x\n", __func__, rc);
		return rc;
	}
	hw_checksum = ((uint16_t)cmd_string[0]<<8) + cmd_string[1];

	sw_checksum = 0;

	sw_checksum += (unsigned char)~0xFF;
	for( cnt= 1; cnt<size; cnt++ )
	{
		sw_checksum += (unsigned char)~*(code+cnt);
	}

	if( hw_checksum == sw_checksum )
		return SYNOPEX_FWUP_RET_OK;
	else
		return SYNOPEX_FWUP_RET_ERR_CSUM;
}

static int synopex_set_ljmp( void )
{
	unsigned char	cmd_string[3];
	int				result;

	result=synopex_set_address(SYNOPEX_AP_BASE);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s set address failed: %x\n", __func__, result);
		return result;
	}

	cmd_string[0]=SYNOPEX_I2C_IDA_DATA;
	cmd_string[1]=0x02;

	result = synopex_i2c_write_cmd(cmd_string,2);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s write data failed: %x\n", __func__, result);
		return result;
	}

	return SYNOPEX_FWUP_RET_OK;

}

static int synopex_check_ljmp( void )
{
	unsigned char	cmd_string[1];
	int				result;

	result=synopex_set_address(SYNOPEX_AP_BASE);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s set address failed: %x\n", __func__, result);
		return result;
	}

	cmd_string[0]=SYNOPEX_I2C_MEM_CODE;
	result=synopex_i2c_write_cmd(cmd_string,1);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s write data(mem code) failed: %x\n", __func__, result);
		return result;
	}

	result=synopex_i2c_read_cmd(cmd_string,1);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s write data failed: %x\n", __func__, result);
		return result;
	}

	if(cmd_string[0] == 0x02)
		return SYNOPEX_FWUP_RET_OK;
	else
		return SYNOPEX_FWUP_RET_ERR_OTHER;
}

static int synopex_jump_reset( void )
{
	unsigned char	cmd_string[1];
	int				result;

	cmd_string[0]=SYNOPEX_I2C_IDA_RUN0;
	result=synopex_i2c_write_cmd(cmd_string,1);
	if(result!=0) {
		printk(KERN_ERR "[synopex]%s write data failed: %x\n", __func__, result);
		return result;
	}

	return SYNOPEX_FWUP_RET_OK;
}

static unsigned short synopex_fw_get_checksum( void )
{
	struct synopex_ts_data *ts;
	int rc;
	u8 tx_buf;
	unsigned char	cmd_string[6];
	unsigned short 	ret = 0xFFFF;
	
	printk(KERN_DEBUG "[synopex]%s Start ------------------- \n", __func__);

	if( g_touch_state == SYNOPEX_STATE_INIT_DONE && g_spi )
	{
		ts = spi_get_drvdata(g_spi);

		/*--- Cancel work que ---*/
		cancel_work_sync(&ts->work);

		/*--- Disable interrupt ---*/
		disable_irq(g_spi->irq);

		/*--- Firmware update state ---*/
		g_touch_fw_update_state = SYNOPEX_FWUP_STATE_EXEC;

		/*--- Soft Reset ---*/
		tx_buf = SYNOPEX_CMD_SOFT_RESET_WR;
		rc = synopex_cmd_txrx( g_spi, &tx_buf, NULL, 1, 0 );
		if (rc < 0)
		{
			printk(KERN_ERR "[synopex]%s soft reset failed: %x\n", __func__, rc);
			goto error_sip_exit ;
		}

		/*--- Set I2C GPIO Setting ---*/
		gpio_free(45);
		gpio_free(47);
		gpio_free(48);
		gpio_free(89);

		rc = msm_gpios_request_enable(synopex_i2c_gpio,ARRAY_SIZE(synopex_i2c_gpio));

		if (rc < 0)
		{
			printk(KERN_ERR "[synopex]%s gpio i2c enable failed: %x\n", __func__, rc);
			goto error_i2c_exit ;
		}
		gpio_set_value( 47 , 1 );
		gpio_set_value( 48 , 1 );
		
		mdelay(620);
		
		cmd_string[0]=SYNOPEX_I2C_IDA_CHECKSUM;
		cmd_string[1]=(unsigned char)(SYNOPEX_AP_BASE>>8);
		cmd_string[2]=(unsigned char)(SYNOPEX_AP_BASE   );
		cmd_string[3]=(unsigned char)(SYNOPEX_AP_END>>8);
		cmd_string[4]=(unsigned char)(SYNOPEX_AP_END   );
		rc = synopex_i2c_write_cmd(cmd_string,5);
		
		if (rc){
			printk(KERN_ERR "[synopex]%s check sum command write failed: %x\n", __func__, rc);
			goto error_i2c_exit ;
		}

		rc = synopex_i2c_read_cmd(cmd_string,6);
		if (rc){
			printk(KERN_ERR "[synopex]%s check sum command read failed: %x\n", __func__, rc);
			goto error_i2c_exit ;
		}
		ret = ((uint16_t)cmd_string[0]<<8) + cmd_string[1];
		
error_i2c_exit:

		gpio_set_value( 47 , 0 );
		rc = synopex_jump_reset() ;

		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_jump_reset failed: %x\n", __func__, rc);
		}

error_sip_exit:

		g_touch_fw_update_state = SYNOPEX_STATE_INIT_DONE ;
		enable_irq(g_spi->irq);
	}

	printk(KERN_DEBUG "[synopex]%s End ret = %x ------------------- \n", __func__, ret );

	return ret;
}
static unsigned char synopex_fw_update( void )
{
	unsigned char ret = SYNOPEX_FWUP_RET_ERR_UNINIT;
	struct synopex_ts_data *ts;
	int rc;
	u8 tx_buf;
	const struct firmware *fw_entry;

	printk(KERN_DEBUG "[synopex]%s Start ------------------- \n", __func__);

	if( g_touch_state == SYNOPEX_STATE_INIT_DONE && g_spi )
	{
		ts = spi_get_drvdata(g_spi);

		/*-----------------------------------------
		           Touch Panel Firmware
		              Initialization
		-----------------------------------------*/
		/*--- Firmware read ---*/
		rc = request_firmware(&fw_entry, "synopex_fw.bin", &g_spi->dev);
		if (rc != 0) {
			printk(KERN_ERR "[synopex]%s firmware read failed: %x\n", __func__, rc);
			ret = SYNOPEX_FWUP_RET_ERR_FW_NO_FILE;
			goto error_exit;
		}

		/* Check FW Revision */
		printk(KERN_DEBUG "[synopex]%s FW Rev Device = %x synopex_fw.bin = %x", __func__, ts->rev, *(fw_entry->data + SYNOPEX_FW_REV) );

		if ( ts->rev == *(fw_entry->data + SYNOPEX_FW_REV) )
		{
			printk(KERN_DEBUG "[synopex]%s The same revision as Update file", __func__);
			ret = SYNOPEX_FWUP_RET_ERR_FW_SAME;
			goto error_exit_and_free;
		}

		/*--- Cancel work que ---*/
		cancel_work_sync(&ts->work);

		/*--- Disable interrupt ---*/
		disable_irq(g_spi->irq);

		/*--- Firmware update state ---*/
		g_touch_fw_update_state = SYNOPEX_FWUP_STATE_EXEC;

		/*--- Soft Reset ---*/
		tx_buf = SYNOPEX_CMD_SOFT_RESET_WR;
		rc = synopex_cmd_txrx( g_spi, &tx_buf, NULL, 1, 0 );
		if (rc < 0)
		{
			printk(KERN_ERR "[synopex]%s soft reset failed: %x\n", __func__, rc);
		}

		/*--- Set I2C GPIO Setting ---*/
		gpio_free(45);
		gpio_free(47);
		gpio_free(48);
		gpio_free(89);

		rc = msm_gpios_request_enable(synopex_i2c_gpio,ARRAY_SIZE(synopex_i2c_gpio));

		if (rc < 0)
		{
			printk(KERN_ERR "[synopex]%s gpio enable failed: %x\n", __func__, rc);
			ret = SYNOPEX_FWUP_RET_ERR_GPIO;
			goto error_exit_and_free;
		}
		gpio_set_value( 47 , 1 );
		gpio_set_value( 48 , 1 );

		mdelay(620);

		/*-----------------------------------------
		           Touch Panel Firmware
		                AP Update
		-----------------------------------------*/
		/*- 1. PGM Protection release			-*/
		rc = synopex_set_protection(0);
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_set_protection failed: %x\n", __func__, rc);
			ret = rc;
			goto error_exit_and_free;
		}

		/*- 2. Erases flash area				-*/
		rc = synopex_erase_tsp(SYNOPEX_AP_BASE,(SYNOPEX_AP_END-SYNOPEX_AP_BASE));
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_erase_tsp failed: %x\n", __func__, rc);
			ret = rc;
			goto error_exit_and_free;
		}

		/*- 3. Write program					-*/
		rc = synopex_program_tsp(fw_entry->data,SYNOPEX_AP_BASE,fw_entry->size);
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_program_tsp failed: %x\n", __func__, rc);
			ret = rc;
			goto error_exit_and_free;
		}

		/*- 4. Check Sum			.			-*/
		rc = synopex_check_checksum(fw_entry->data,SYNOPEX_AP_BASE,fw_entry->size );
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_check_checksum failed: %x\n", __func__, rc);
			ret = rc;
			goto error_exit_and_free;
		}

		/*- 5. Write 0x02 in 0x100 address		-*/
		rc = synopex_set_ljmp( );
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_set_ljmp failed: %x\n", __func__, rc);
			ret = rc;
			goto error_exit_and_free;
		}

		/*- 6. Check 0x100 address				-*/
		rc = synopex_check_ljmp( );
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s synopex_check_ljmp failed: %x\n", __func__, rc);
			ret = SYNOPEX_FWUP_RET_OK;
		}
		else {
			
			rc = synopex_jump_reset( );
			if (rc)
			{
				printk(KERN_ERR "[synopex]%s synopex_jump_reset failed: %x\n", __func__, rc);
				ret = rc;
				goto error_exit_and_free;
			}
		}
		ret = SYNOPEX_FWUP_RET_OK;
	}
	else {
		printk(KERN_ERR "[synopex]%s g_touch_state(%0x) or g_spi failed\n", __func__, g_touch_state);
		return SYNOPEX_FWUP_RET_ERR_OTHER;
	}
error_exit_and_free:
	release_firmware(fw_entry);
error_exit:
	printk(KERN_DEBUG "[synopex]%s End (Error:%0x) -------- \n", __func__,ret);
	return ret;
}

static int synopex_soft_reset(struct spi_device *spi)
{
	int rc = 0;
	u8 tx_buf;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	tx_buf = SYNOPEX_CMD_SOFT_RESET_WR;

	rc = synopex_cmd_txrx( spi, &tx_buf, NULL, 1, 0 );
	if(rc)
		return rc;

	mdelay(600);

	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);

	return 0;
}

static int synopex_exit_sleep(struct spi_device *spi)
{
	int rc = 0;
	u8 tx_buf;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	tx_buf = SYNOPEX_CMD_EXIT_SLEEP_WR;
	rc = synopex_cmd_txrx( spi, &tx_buf, NULL, 1, 0 );
	if (rc){
		printk(KERN_ERR "[synopex]%s: exit with error %d\n", __func__, rc);
		return rc;
	}
	mdelay(10);
	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);

	return 0;
}

static int synopex_enter_deep(struct spi_device *spi)
{
	int rc = 0;
	int ret;
	u8 tx_buf;
	struct synopex_ts_data *ts;
	struct synopex_platform_data *pdata = spi->dev.platform_data;

	ts = spi_get_drvdata(spi);

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	if( ts == NULL )
	{
		printk(KERN_ERR "[synopex]%s: The ts date doesn't exist\n", __func__);
		return 0;
	}

	ts->is_suspend = 1;

	disable_irq(spi->irq);
	ret = cancel_work_sync(&ts->work);
	if (ret){
		printk(KERN_ERR "[synopex]%s: cancel work\n", __func__);
		enable_irq(spi->irq);
	}

	tx_buf = SYNOPEX_CMD_EXIT_2SCAN_WR;

	rc = synopex_cmd_txrx( spi, &tx_buf, NULL, 1, 0 );
	if (rc){
		printk(KERN_ERR "[synopex]%s: enter with error %d\n", __func__, rc);
	}

	mdelay(25);

	printk(KERN_DEBUG "[synopex]%s: EXIT 2Scan mode \n", __func__);

	g_TouchDeepState = true;

	tx_buf = SYNOPEX_CMD_ENTER_DEEP_WR;

	rc = synopex_cmd_txrx( spi, &tx_buf, NULL, 1, 0 );
	if (rc){
		printk(KERN_ERR "[synopex]%s: enter with error %d\n", __func__, rc);
		ts->is_suspend = 0;
		enable_irq(spi->irq);
		return rc;
	}

	mdelay(10);

	if (pdata && pdata->powerdown)
		pdata->powerdown(&spi->dev);

	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);

	return 0;
}

static int synopex_exit_deep(struct spi_device *spi)
{
	int rc = 0;
	u8 tx_buf;
	struct synopex_platform_data *pdata = spi->dev.platform_data;
	struct synopex_ts_data *ts;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	ts = spi_get_drvdata(spi);
	if( ts == NULL )
	{
		printk(KERN_ERR "[synopex]%s: The ts date doesn't exist\n", __func__);
		return 0;
	}

	if (pdata && pdata->powerup)
		pdata->powerup(&spi->dev);
	udelay(400);

	enable_irq(spi->irq);

	tx_buf = SYNOPEX_CMD_EXIT_DEEP_RD;
	rc = synopex_cmd_txrx( spi, &tx_buf, NULL, 1, spi_delay );
	if (rc){
		printk(KERN_ERR "[synopex]%s: exit with error (read)%d\n", __func__, rc);
		ts->is_suspend = 0;
		enable_irq(spi->irq); /* enable interrupt */
		return rc;
	}
	mdelay(220);

	ts->is_suspend = 0;

	g_TouchDeepState = false;

	printk(KERN_DEBUG "[synopex]%s: rev %0x\n", __func__, ts->rev);
	if( ts->rev < 0x20 && ts->rev != 0x00 )
	{
		rc = synopex_exit_sleep(spi);
		if (rc)
		{
			printk(KERN_ERR "[synopex]%s: exit with error (exit sleep)%d\n", __func__, rc);
			return rc;
		}
	}

	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);

	return 0;
}

static unsigned char synopex_rev(struct spi_device *spi)
{
	int rc = 0;
	u8 tx_buf[2];
	u8 rx_buf[2];

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);
	
	tx_buf[0] = SYNOPEX_CMD_REV_RD;
	tx_buf[1] = SYNOPEX_CMD_DUMMY;

	rc = synopex_cmd_txrx( spi, tx_buf, rx_buf, 2, spi_delay );

	if (rc){
		printk(KERN_ERR "[synopex]%s: rev read with error %d\n", __func__, rc);
		return 0xFF;
	}
	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);

	return rx_buf[1];
}

static int synopex_debugfs_env_set(void *data, u64 val)
{
	*(u32*)data = (u32)val;
	return 0;
}

static int synopex_debugfs_env_get(void *data, u64 *val)
{
	*val = (u64)(*(u32*)data);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(synopex_debugfs_fops,
						synopex_debugfs_env_get,
						synopex_debugfs_env_set,
						"0x%08llx\n");

static void __devinit synopex_debugfs_init(struct synopex_ts_data *ts)
{
	ts->dent_synopex_root = debugfs_create_dir(dev_name(&ts->spi->dev), NULL);
	if (ts->dent_synopex_root) {
		int i;
		for (i = 0; i < ARRAY_SIZE(debugfs_synopex_env); i++) {
			ts->dent_synopex_env[i] =
				debugfs_create_file(
					debugfs_synopex_env[i].name,
					debugfs_synopex_env[i].mode,
					ts->dent_synopex_root,
					debugfs_synopex_env[i].data,
					&synopex_debugfs_fops
					);
		}
	}
}

static void __devexit synopex_debugfs_remove(struct synopex_ts_data *ts)
{
	if (ts->dent_synopex_root) {
		int i;
		debugfs_remove_recursive(ts->dent_synopex_root);
		ts->dent_synopex_root = NULL;
		for (i = 0; i < ARRAY_SIZE(debugfs_synopex_env); i++)
			ts->dent_synopex_env[i] = NULL;
	}
}

static ssize_t synopex_show_stats(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	return snprintf(buf, PAGE_SIZE,
					"dummy : 0x%x\n"
					"status: 0x%x\n"
					"x1_h  : 0x%x\n"
					"x1_l  : 0x%x\n"
					"y1_h  : 0x%x\n"
					"y1_l  : 0x%x\n"
					"x2_h  : 0x%x\n"
					"x2_l  : 0x%x\n"
					"y2_h  : 0x%x\n"
					"y2_l  : 0x%x\n"
					"x1,y1 : %d(0x%04x), %d(0x%04x)\n"
					"x2,y2 : %d(0x%04x), %d(0x%04x)\n"
					"button0: %c\n"
					"button1: %c\n"
					"button2: %c\n"
					"button3: %c\n"
					"setup : 0x%04x\n",
					rx_data[0],
					rx_data[1],
					rx_data[2],
					rx_data[3],
					rx_data[4],
					rx_data[5],
					rx_data[6],
					rx_data[7],
					rx_data[8],
					rx_data[9],
					x1_pos, x1_pos, y1_pos, y1_pos,
					x2_pos, x2_pos, y2_pos, y2_pos,
					(rx_data[1]&SYNOPEX_MODE_BUTTON0)?'1':'-',
					(rx_data[1]&SYNOPEX_MODE_BUTTON1)?'1':'-',
					(rx_data[1]&SYNOPEX_MODE_BUTTON2)?'1':'-',
					(rx_data[1]&SYNOPEX_MODE_BUTTON3)?'1':'-',
					setup_data
			);
}

static DEVICE_ATTR(stats, S_IRUGO | S_IWUSR, synopex_show_stats, NULL);
static struct attribute *dev_attrs[] = {
	&dev_attr_stats.attr,
	NULL,
};

static struct attribute_group dev_attr_grp = {
	.attrs = dev_attrs,
};

static void rx_debug_fs(u8 data[], int len)
{
	int i;
	for (i = 0; i < len; i++) {
		rx_data[i] = data[i];
	}
}

static void synopex_power_down(struct spi_device *spi)
{
	struct synopex_platform_data *pdata = spi->dev.platform_data;

	if (pdata && pdata->teardown) {
		pdata->teardown(&spi->dev);
	}

	mdelay(21);

}

static void synopex_key_scan(struct synopex_ts_data *ts, u8 status, int down)
{
	if (down) {
		if ((status & SYNOPEX_MODE_BUTTON0) && (ts->key_down[0] == 0)) {
			ts->key_down[0] = 1;
			input_report_key(ts->input_dev, ts->key_code[0], 1);
			input_sync(ts->input_dev);
		}
		else if (((status & SYNOPEX_MODE_BUTTON0) == 0) && ts->key_down[0]) {
			input_report_key(ts->input_dev, ts->key_code[0], 0);
			input_sync(ts->input_dev);
			ts->key_down[0] = 0;
		}

		if ((status & SYNOPEX_MODE_BUTTON1) && (ts->key_down[1] == 0)) {
			ts->key_down[1] = 1;
			input_report_key(ts->input_dev, ts->key_code[1], 1);
			input_sync(ts->input_dev);
		}
		else if (((status & SYNOPEX_MODE_BUTTON1) == 0) && ts->key_down[1]) {
			input_report_key(ts->input_dev, ts->key_code[1], 0);
			input_sync(ts->input_dev);
			ts->key_down[1] = 0;
		}

		if ((status & SYNOPEX_MODE_BUTTON2) && (ts->key_down[2] == 0)) {
			ts->key_down[2] = 1;
			input_report_key(ts->input_dev, ts->key_code[2], 1);
			input_sync(ts->input_dev);
		}
		else if (((status & SYNOPEX_MODE_BUTTON2) == 0) && ts->key_down[2]) {
			input_report_key(ts->input_dev, ts->key_code[2], 0);
			input_sync(ts->input_dev);
			ts->key_down[2] = 0;
		}

		if ((status & SYNOPEX_MODE_BUTTON3) && (ts->key_down[3] == 0)) {
			ts->key_down[3] = 1;
			input_report_key(ts->input_dev, ts->key_code[3], 1);
			input_sync(ts->input_dev);
		}
		else if (((status & SYNOPEX_MODE_BUTTON3) == 0) && ts->key_down[3]) {
			input_report_key(ts->input_dev, ts->key_code[3], 0);
			input_sync(ts->input_dev);
			ts->key_down[3] = 0;
		}
	}
	else {
		if (ts->key_down[0]) {
			input_report_key(ts->input_dev, ts->key_code[0], 0);
			input_sync(ts->input_dev);
		}
		if (ts->key_down[1]) {
			input_report_key(ts->input_dev, ts->key_code[1], 0);
			input_sync(ts->input_dev);
		}
		if (ts->key_down[2]) {
			input_report_key(ts->input_dev, ts->key_code[2], 0);
			input_sync(ts->input_dev);
		}
		if (ts->key_down[3]) {
			input_report_key(ts->input_dev, ts->key_code[3], 0);
			input_sync(ts->input_dev);
		}
		ts->key_down[0] = ts->key_down[1] = ts->key_down[2] = ts->key_down[3] = 0;
	}
}

unsigned char touch_panel_cmd(unsigned char type, unsigned char *val)
{
	unsigned char	ret = 0xFF; /* err */
	unsigned short	checksum = 0;
	int				rc;
	u8 tx_buf[2];

	if( g_spi == NULL )
		return ret;

	if( g_touch_fw_update_state == SYNOPEX_FWUP_STATE_EXEC )
		return ret;

	switch( type )
	{
		case TOUCH_PANEL_CMD_TYPE_RESET:
			tx_buf[0] = SYNOPEX_CMD_SOFT_RESET_WR;
			rc = synopex_cmd_txrx( g_spi, tx_buf, NULL, 1, 0 );
			if( !rc )
				ret = 0;
			break;
		case TOUCH_PANEL_CMD_TYPE_ENTR_SLEEP:
			tx_buf[0] = SYNOPEX_CMD_ENTER_SLEEP_WR;
			rc = synopex_cmd_txrx( g_spi, tx_buf, NULL, 1, 0 );
			if( !rc )
				ret = 0;
			break;
		case TOUCH_PANEL_CMD_TYPE_EXIT_SLEEP:
			tx_buf[0] = SYNOPEX_CMD_EXIT_SLEEP_WR;
			rc = synopex_cmd_txrx( g_spi, tx_buf, NULL, 1, 0 );
			if( !rc )
				ret = 0;
			break;
		case TOUCH_PANEL_CMD_TYPE_ENTR_DEEP:
			tx_buf[0] = SYNOPEX_CMD_ENTER_DEEP_WR;
			rc = synopex_cmd_txrx( g_spi, tx_buf, NULL, 1, 0 );
			if( !rc )
				ret = 0;
			break;
		case TOUCH_PANEL_CMD_TYPE_EXIT_DEEP:
			tx_buf[0] = SYNOPEX_CMD_EXIT_DEEP_RD;
			rc = synopex_cmd_txrx( g_spi, tx_buf, NULL, 1, 0 );
			if( !rc )
				ret = 0;
			break;
		case TOUCH_PANEL_CMD_TYPE_GET_REVISION:
			ret = synopex_rev(g_spi);
			break;
		case TOUCH_PANEL_CMD_TYPE_FW_UPDATE:
			ret = synopex_fw_update();
			break;
		case TOUCH_PANEL_CMD_TYPE_GET_CHECKSUM:
			if(val){
				checksum = synopex_fw_get_checksum();
				val[1] = (unsigned char)((checksum >> 8) & 0xFF);
				val[2] = (unsigned char)(checksum & 0xFF);
			}
			ret = 0;
			break;
		default:
			break;
	}

	return ret;

}
EXPORT_SYMBOL(touch_panel_cmd);

unsigned char touch_panel_cmd_callback(unsigned char type, unsigned char val, void (*func)(void *))
{
	unsigned char ret = 0xFF; /* err */

	switch( type )
	{
		case TOUCH_PANEL_CMD_TYPE_GET_COORD:
			if( func )
			{
				g_tp_cmd_callback = func;
				ret = 0;
			}
			break;
		default:
			break;
	}
	return ret;

}
EXPORT_SYMBOL(touch_panel_cmd_callback);

static void touch_panel_callback( u8 data[] )
{
	if( g_tp_cmd_callback ) {

		g_touch_coord.reg0 = data[1];
		g_touch_coord.reg1 = data[2];
		g_touch_coord.reg2 = data[3];
		g_touch_coord.reg3 = data[4];
		g_touch_coord.reg4 = data[5];
		g_touch_coord.reg5 = data[6];
		g_touch_coord.reg6 = data[7];
		g_touch_coord.reg7 = data[8];
		g_touch_coord.reg8 = data[9];

		g_tp_cmd_callback(&g_touch_coord);
		g_tp_cmd_callback = NULL;
	}
}
static void synopex_abs_report(struct synopex_ts_data *ts, struct synopex_ts_abs *src )
{
	if( src->finger )
	{
		input_report_abs(ts->input_dev, ABS_MT_POSITION_X, src->x1);
		input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, src->y1);
		input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 5);
		input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 122);
		input_mt_sync(ts->input_dev);
		ts->touch[0] = 1;

		if( src->finger > 1 )
		{
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, src->x2);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, src->y2);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 10);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 122);
			input_mt_sync(ts->input_dev);
			ts->touch[1] = 1;
		}
		else
		{
			if( ts->touch[1] )
			{
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, 0);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, 0);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(ts->input_dev);
				ts->touch[1] = 0;
			}
		}
	}
	else
	{
		if( ts->touch[0] )
		{
			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, 0);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, 0);
			input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
			input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			input_mt_sync(ts->input_dev);
			ts->touch[0] = 0;

			if( ts->touch[1] )
			{
				input_report_abs(ts->input_dev, ABS_MT_POSITION_X, 0);
				input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, 0);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(ts->input_dev);
				ts->touch[1] = 0;
			}
		}
	}
	input_sync(ts->input_dev);

}

static void synopex_do_work(struct work_struct *work)
{
#define TXRX_BUF_MAX_SIZE  10
	u8 tx_buf[TXRX_BUF_MAX_SIZE];
	u8 rx_buf[TXRX_BUF_MAX_SIZE];
	int rc = 0;
	struct synopex_ts_data *ts = container_of(work, struct synopex_ts_data, work);
	u8 status = 0;
	struct synopex_ts_abs src;
	int finger = 0, buf_len = 0;

	if( ts->is_suspend )
	{
		printk(KERN_DEBUG "[synopex]%s: Invalid interrupt\n", __func__);
		enable_irq(ts->spi->irq);
		return;
	}

	memset( rx_buf, 0xFF, TXRX_BUF_MAX_SIZE );
	memset( &src  , 0x00, sizeof(src));

	tx_buf[0] = SYNOPEX_CMD_TOUCH_RD;
	tx_buf[1] = SYNOPEX_CMD_DUMMY;

	rc = synopex_cmd_txrx( ts->spi, tx_buf, rx_buf, 2, spi_delay );
	if(rc)
		goto error_exit;

	status = rx_buf[1];

	if(((status & SYNOPEX_MODE_MASK) == SYNOPEX_MODE_ERR_TOUCH) ||
		((status & SYNOPEX_MODE_MASK) == (SYNOPEX_MODE_ERR_TOUCH | SYNOPEX_MODE_BUTTON)))
		goto status_error_exit;

	/* Button Check */
	if( ( status & SYNOPEX_MODE_BUTTON_CHECK ) == SYNOPEX_MODE_BUTTON ){
		synopex_key_scan(ts, status, 1);
	}else{
		synopex_key_scan(ts, status, 0);
	}

	if( ( status & SYNOPEX_MODE_2ND_TOUCH ) == SYNOPEX_MODE_2ND_TOUCH )
	{
		finger  = 2;
		buf_len = 9;
		tx_buf[1] = SYNOPEX_CMD_X1_H_RD;
		tx_buf[2] = SYNOPEX_CMD_X1_L_RD;
		tx_buf[3] = SYNOPEX_CMD_Y1_H_RD;
		tx_buf[4] = SYNOPEX_CMD_Y1_L_RD;
		tx_buf[5] = SYNOPEX_CMD_X2_H_RD;
		tx_buf[6] = SYNOPEX_CMD_X2_L_RD;
		tx_buf[7] = SYNOPEX_CMD_Y2_H_RD;
		tx_buf[8] = SYNOPEX_CMD_Y2_L_RD;
		tx_buf[9] = SYNOPEX_CMD_DUMMY;
	}
	else if( ( status & SYNOPEX_MODE_1ST_TOUCH ) == SYNOPEX_MODE_1ST_TOUCH )
	{
		finger  = 1;
		buf_len = 5;
		tx_buf[1] = SYNOPEX_CMD_X1_H_RD;
		tx_buf[2] = SYNOPEX_CMD_X1_L_RD;
		tx_buf[3] = SYNOPEX_CMD_Y1_H_RD;
		tx_buf[4] = SYNOPEX_CMD_Y1_L_RD;
		tx_buf[5] = SYNOPEX_CMD_DUMMY;
	}

	if( finger )
	{
		rc = synopex_cmd_txrx( ts->spi, &tx_buf[1], &rx_buf[1], buf_len, spi_delay );
		if (rc)
			goto error_exit;

		src.x1 = (rx_buf[2] << 8) | rx_buf[3];
		src.y1 = (rx_buf[4] << 8) | rx_buf[5];

		if( finger >= 2 )
		{
			src.x2 = (rx_buf[6] << 8) | rx_buf[7];
			src.y2 = (rx_buf[8] << 8) | rx_buf[9];
		}
		src.finger = finger;

	}
	synopex_abs_report( ts, &src );

	/* GPS noise measures 15s ----------- add start */
	if( touch_rel_timer_flg) {
		del_timer(&touch_rel_timer);
		touch_rel_timer_flg = 0;

		printk(KERN_DEBUG "del_timer  ,%d\n",touch_rel_timer_flg);

	}

	if( !finger) {

		if( GPS_2scan_state == true ){
			touch_rel_timer.function = touch_rel_timeout;
			touch_rel_timer.data = 0;
			touch_rel_timer.expires = jiffies + 15*HZ;

			add_timer(&touch_rel_timer);
			touch_rel_timer_flg = 1;

			printk(KERN_DEBUG "add_timer on ,%d\n",touch_rel_timer_flg);
		}
	}
	/* GPS noise measures 15s ----------- add end */

	rx_debug_fs(rx_buf, TXRX_BUF_MAX_SIZE);

	touch_panel_callback(rx_buf);

	enable_irq(ts->spi->irq);

	return;

status_error_exit:
	printk(KERN_ERR "[synopex]%s: exit with error (status)\n", __func__);

error_exit:
	printk(KERN_ERR "[synopex]%s: exit with error \n", __func__);

	synopex_key_scan(ts, status, 0);
	input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_mt_sync(ts->input_dev);
	input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_mt_sync(ts->input_dev);
	ts->touch[0] = 0;
	ts->touch[1] = 0;
	input_sync(ts->input_dev);
	enable_irq(ts->spi->irq);
}

static irqreturn_t synopex_irq(int irq, void *dev_id)
{
	struct synopex_ts_data *ts = dev_id;
	if( g_touch_fw_update_state != SYNOPEX_FWUP_STATE_EXEC )
	{
		disable_irq_nosync(ts->spi->irq);
		queue_work(synopex_wq, &ts->work);
	}

	return IRQ_HANDLED;
}

void synopex_init_request( void )
{
	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	printk(KERN_DEBUG "[synopex]%s: g_touch_state(Bfr):%0x\n", __func__,g_touch_state);

	if( g_touch_state != SYNOPEX_STATE_INIT_DONE )
		g_touch_state = SYNOPEX_STATE_INIT_WAIT;

	printk(KERN_DEBUG "[synopex]%s: g_touch_state(Afr):%0x\n", __func__,g_touch_state);

	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);
	return ;

}
EXPORT_SYMBOL(synopex_init_request);

void synopex_suspend(struct spi_device *spi, pm_message_t mesg)
{
	if( g_touch_state != SYNOPEX_STATE_INIT_DONE )
		g_touch_state |= SYNOPEX_STATE_SUSPEND;
	else if( g_touch_fw_update_state != SYNOPEX_FWUP_STATE_EXEC )
		synopex_enter_deep(spi);

}

void synopex_resume(struct spi_device *spi)
{
	if( (g_touch_state & SYNOPEX_STATE_SUSPEND) == SYNOPEX_STATE_SUSPEND )
		g_touch_state ^= SYNOPEX_STATE_SUSPEND;
	else if( g_touch_fw_update_state != SYNOPEX_FWUP_STATE_EXEC )
		synopex_exit_deep(spi);

}

static int synopex_ts_thread(void *_ts)
{
	struct synopex_ts_data *ts = _ts;
	int rc = 0;
	int time_cnt = 0;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);
	printk(KERN_DEBUG "[synopex]%s: g_touch_state:%0x\n", __func__,g_touch_state);

	while( (g_touch_state & SYNOPEX_STATE_INIT_WAIT ) != SYNOPEX_STATE_INIT_WAIT )
	{
		msleep(100);
		if( time_cnt >= 20 ){ // 2s
		printk(KERN_ERR "[synopex]%s: Wait time out!! (%d)\n", __func__,time_cnt);
			break;
		}
		time_cnt++;
	}

	printk(KERN_DEBUG "[synopex]%s: Wait DONE(%d)\n", __func__,time_cnt);

	if( ts->spi )
	{
		printk(KERN_DEBUG "[synopex]%s: ts->spi enable\n", __func__);
		synopex_soft_reset(ts->spi);

		ts->rev = synopex_rev(ts->spi);
		printk(KERN_DEBUG "[synopex]%s: rev:0x%x\n", __func__,ts->rev);

		if( ts->rev == 0x00 || ts->rev >= 0x20 )
		{
			spi_delay = 100;
		}
		else
		{
			spi_delay = 400;
		}
		printk(KERN_DEBUG "[synopex]%s: spi_delay:%d\n", __func__,spi_delay);

		rc = request_irq(ts->spi->irq, synopex_irq,
				  IRQF_TRIGGER_FALLING | IRQF_DISABLED,
				  "msm_touchscreen", ts);
		if (rc != 0)
			pr_err("%s: Cannot register irq1 (%d)\n", __func__, rc);

		printk(KERN_DEBUG "[synopex]%s: g_touch_state:%0x\n", __func__,g_touch_state);

		if( (g_touch_state & SYNOPEX_STATE_SUSPEND) == SYNOPEX_STATE_SUSPEND )
			synopex_suspend(ts->spi, PMSG_SUSPEND);

		g_touch_state = SYNOPEX_STATE_INIT_DONE;

		printk(KERN_DEBUG "[synopex]%s: g_touch_state:%0x\n", __func__,g_touch_state);

	}

	ts->ts_task = NULL;
	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);

	return 0;
}
static int synopex_open(struct input_dev *dev)
{
	struct synopex_ts_data *ts = input_get_drvdata(dev);

	printk(KERN_DEBUG "[synopex]%s: Enter(%0x)\n", __func__, g_touch_state);

	ts->ts_task = kthread_run(synopex_ts_thread, ts, "synopex_ts");
	if (IS_ERR(ts->ts_task)) {
		printk(KERN_ERR "[synopex]%s: exit with error \n", __func__);
		ts->ts_task = NULL;
	}
	
	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);
	return 0;
}

static void synopex_close(struct input_dev *dev)
{
	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);
	printk(KERN_DEBUG "[synopex]%s: Exit\n", __func__);
}

static int __devinit synopex_probe(struct spi_device *spi)
{
	struct synopex_ts_data *ts = NULL;
	int rc = 0;
	int cnt = 0;
	struct synopex_platform_data *pdata = spi->dev.platform_data;
	struct msm_ts_platform_data *mtpdata = pdata->msm_ts_data;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);

	ts = kzalloc(sizeof(struct synopex_ts_data), GFP_KERNEL);

	if (ts == NULL) {
		pr_err("%s: No memory for struct msm_ts\n", __func__);
		return -ENOMEM;
	}
	INIT_WORK(&ts->work, synopex_do_work);

	INIT_WORK(&work2, spi_touch_2scan_enter );	/* GPS noise measures add */
	INIT_WORK(&work3, spi_touch_2scan_exit );	/* GPS noise measures add */


	ts->spi = spi;
	ts->is_suspend = 0;

	g_spi   = spi;
	g_touch_state = SYNOPEX_STATE_NONE;

	if (pdata && pdata->setup) {
		rc = pdata->setup(&spi->dev);
		if (rc)
			goto err_hw_setup;
	}
	udelay(200);

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		pr_err("failed to allocate touchscreen input device\n");
		rc = -ENOMEM;
		goto err_alloc_input_dev;
	}
	printk(KERN_DEBUG "ts=%p, input_dev=%p\n", ts, ts->input_dev);
	ts->input_dev->name = "synopex-touchscreen";
	input_set_drvdata(ts->input_dev, ts);

	ts->hw_rev = hw_revision_read();
	printk(KERN_DEBUG "[synopex]%s: hw rev:0x%x\n", __func__,ts->hw_rev);

	if( ts->hw_rev == HW_REVISION_READ_ERR )
		ts->hw_rev = 0x00;  /* set default */

	if( ts->hw_rev <= HARDWARE_REV_09 ){
		// <= ES2-1(ES2-A)
		ts->key_code[0] = KEY_HOME;
		ts->key_code[1] = KEY_MENU;
		ts->key_code[2] = KEY_BACK;
		ts->key_code[3] = KEY_SEARCH;
	}
	else{
		// ES2-2(ES2-B) <=
		ts->key_code[0] = KEY_MENU;
		ts->key_code[1] = KEY_HOME;
		ts->key_code[2] = KEY_BACK;
		ts->key_code[3] = KEY_SEARCH;
	}

	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);

	for(cnt=0; cnt<SYNOPEX_BUTTON_NUM ;cnt++){
		set_bit(ts->key_code[cnt], ts->input_dev->keybit);
	}

	input_set_abs_params(ts->input_dev, ABS_X, mtpdata->min_x, mtpdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_Y, mtpdata->min_y, mtpdata->max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_PRESSURE, mtpdata->min_press, mtpdata->max_press, 0, 0);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, mtpdata->min_x, mtpdata->max_x, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, mtpdata->min_y, mtpdata->max_y, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, mtpdata->min_press, mtpdata->max_press, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);

	ts->input_dev->open  = synopex_open;
	ts->input_dev->close = synopex_close;

	rc = input_register_device(ts->input_dev);
	if (rc != 0) {
		pr_err("%s: failed to register input device\n", __func__);
		goto err_input_dev_reg;
	}

	spi_set_drvdata(spi, ts);

	max_speed_hz = 1000000;
	rc = sysfs_create_group(&spi->dev.kobj, &dev_attr_grp);
	if (rc) {
		dev_err(&spi->dev, "failed to create dev. attrs : %d\n", rc);
		goto err_attrs;
	}
	synopex_debugfs_init(ts);

	printk(KERN_DEBUG "[synopex]%s: Leave\n", __func__);

	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = synopex_early_suspend;
	ts->early_suspend.resume = synopex_late_resume;
	register_early_suspend(&ts->early_suspend);

	touch_reltimer_init();		/* GPS noise measures 15s add */

	return 0;

err_attrs:
	spi_set_drvdata(spi, NULL);
	input_unregister_device(ts->input_dev);

err_input_dev_reg:
	input_set_drvdata(ts->input_dev, NULL);
	input_free_device(ts->input_dev);

err_alloc_input_dev:
err_hw_setup:
	kfree(ts);
	return rc;
}

static int __devexit synopex_remove(struct spi_device *spi)
{
	struct synopex_ts_data *ts;
	struct synopex_platform_data *pdata = spi->dev.platform_data;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);
	ts = spi_get_drvdata(spi);

	sysfs_remove_group(&spi->dev.kobj, &dev_attr_grp);
	synopex_debugfs_remove(ts);

	synopex_power_down(spi);
	input_unregister_device(ts->input_dev);
	input_free_device(ts->input_dev);
	spi_set_drvdata(spi, NULL);
	if (pdata && pdata->teardown)
		pdata->teardown(&spi->dev);
	kfree(ts);

	printk(KERN_DEBUG "[synopex]%s: Leave\n", __func__);
	return 0;
}

static void synopex_early_suspend(struct early_suspend *h)
{
	struct synopex_ts_data *ts;
	ts = container_of(h, struct synopex_ts_data, early_suspend);
	synopex_suspend(ts->spi, PMSG_SUSPEND);
}

static void synopex_late_resume(struct early_suspend *h)
{
	struct synopex_ts_data *ts;
	ts = container_of(h, struct synopex_ts_data, early_suspend);
	synopex_resume(ts->spi);
}

static struct spi_driver synopex_driver = {
	.driver = {
		.name = "msm-touchscreen",
		.owner = THIS_MODULE,
	},
	.probe = synopex_probe,
	.remove = __devexit_p(synopex_remove),
};




#define NEC_PROC_TOUCH_ANDROID_LBS       "nec_touch_android_lbs"
#define NEC_PROC_TOUCH_VZW_LBS           "nec_touch_vzw_lbs"
#define NEC_WRITEBUF_SIZE 64

DECLARE_MUTEX(sem_nec_touch_android_lbs);
DECLARE_MUTEX(sem_nec_touch_vzw_lbs);

static int nec_touch_android_lbs_proc_write(struct file* filp, const char* buffer, unsigned long count, void* data )
{
	int copy_len;
	int android_lbs_started;
	char buf[NEC_WRITEBUF_SIZE];

	if ( down_interruptible( &sem_nec_touch_android_lbs ) ) {
		printk( KERN_INFO "proctest : down_interruptible for write failed\n");
		return -ERESTARTSYS;
	}

	if ( count > NEC_WRITEBUF_SIZE ) {
		copy_len = NEC_WRITEBUF_SIZE;
	}
	else {
 		copy_len = count;
	}

	if ( copy_from_user( buf, buffer, copy_len ) ) {
		up( &sem_nec_touch_android_lbs );
 		printk( KERN_INFO "proctest : copy_from_user failed\n");
		return -EFAULT;
 	}
 	android_lbs_started = (int)(buf[0]-'0');
 	printk( KERN_INFO "android_lbs_started(%d) \n",android_lbs_started);
	if (android_lbs_started == 1) {
		nec_touch_android_lbs_started(true);
	}
	else {
		nec_touch_android_lbs_started(false);
	}
	
	up( &sem_nec_touch_android_lbs );

	return copy_len;
}

static int nec_touch_vzw_lbs_proc_write(struct file* filp, const char* buffer, unsigned long count, void* data )
{
	int copy_len;
	int vzw_lbs_started;
	char buf[NEC_WRITEBUF_SIZE];

	if ( down_interruptible( &sem_nec_touch_vzw_lbs ) ) {
		printk( KERN_INFO "proctest : down_interruptible for write failed\n");
		return -ERESTARTSYS;
	}

	if ( count > NEC_WRITEBUF_SIZE ) {
		copy_len = NEC_WRITEBUF_SIZE;
	}
	else {
 		copy_len = count;
	}

	if ( copy_from_user( buf, buffer, copy_len ) ) {
		up( &sem_nec_touch_vzw_lbs );
 		printk( KERN_INFO "proctest : copy_from_user failed\n");
		return -EFAULT;
 	}
 	vzw_lbs_started = (int)(buf[0]-'0');
 	printk( KERN_INFO "vzw_lbs_started(%d) \n",vzw_lbs_started);
	if (vzw_lbs_started == 1) {
		nec_touch_vzw_lbs_started(true);
	}
	else {
		nec_touch_vzw_lbs_started(false);
	}
	
	up( &sem_nec_touch_vzw_lbs );

	return copy_len;
}


static int __init nec_proc_init(void)
{
	struct proc_dir_entry *dirp;

	dirp = (struct proc_dir_entry *)
		create_proc_entry(NEC_PROC_TOUCH_ANDROID_LBS, 0222, 0);
	if (dirp == 0) {
		return(-EINVAL);
	}
	dirp->write_proc = (write_proc_t *) nec_touch_android_lbs_proc_write;
	
	dirp = (struct proc_dir_entry *)
		create_proc_entry(NEC_PROC_TOUCH_VZW_LBS, 0222, 0);
	if (dirp == 0) {
		return(-EINVAL);
	}
	dirp->write_proc = (write_proc_t *) nec_touch_vzw_lbs_proc_write;

	return 0;
}

static void nec_proc_exit(void)
{
	remove_proc_entry(NEC_PROC_TOUCH_ANDROID_LBS, NULL);
	remove_proc_entry(NEC_PROC_TOUCH_VZW_LBS, NULL);
}




static int __init synopex_init(void)
{
	int rc = 0;

	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);
	synopex_wq = create_singlethread_workqueue("synopex_wq");
	if (!synopex_wq)
		return -ENOMEM;
	rc = spi_register_driver(&synopex_driver);

	if (rc) {
		destroy_workqueue(synopex_wq);
		synopex_wq = NULL;
	}
	
	
	nec_proc_init();
	
	
	printk(KERN_DEBUG "[synopex]%s: Leave with %d\n", __func__, rc);

	return rc;
}

static void __exit synopex_exit(void)
{
	printk(KERN_DEBUG "[synopex]%s: Enter\n", __func__);
	spi_unregister_driver(&synopex_driver);
	if (synopex_wq)
		destroy_workqueue(synopex_wq);
	
	
	nec_proc_exit();
	
	
	printk(KERN_DEBUG "[synopex]%s: Leave\n", __func__);
}

module_init(synopex_init);
module_exit(synopex_exit);
MODULE_DESCRIPTION("Qualcomm MSM/QSD Touchscreen controller driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:synopex_touchscreen");
