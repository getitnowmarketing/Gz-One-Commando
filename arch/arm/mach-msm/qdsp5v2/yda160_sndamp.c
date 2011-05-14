/*===========================================================================
 FILENAME : yda160_sndamp.c

 Copyright (C) 2010 NEC Corporation.
===========================================================================*/

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/






/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/

#include <asm/uaccess.h>

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include <linux/mfd/pmic8058.h>

#include <asm/param.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#define MSM_MM_DEBUG_LEVEL 2
#define MSM_MM_MODULE "yda160-sndamp" /* module name used in log messages */
#include <mach/debug_mm.h>

#include "mach/msm_battery.h"    

#include "yda160_sndamp.h"
#include "yda160_sndampi.h"
#include "yda160_i2c_driver.h"
#include "yda160_sndamp_diag.h"


/*===========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

There are definitions for LOCAL data.  There is no global data.

===========================================================================*/

static DECLARE_WAIT_QUEUE_HEAD(open_wq);

static atomic_t reserve_open_flag;
static atomic_t open_count;
static atomic_t open_flag;


static  unsigned char  sndamp_yda160_reg_buffer[SNDAMP_REG_MAX] = {
    SNDAMP_REG_0_POWERDOWN1_INIT_VAL,
                                      /* SNDAMP_REG_0_POWERDOWN1           */
    SNDAMP_REG_1_POWERDOWN2_INIT_VAL,
                                      /* SNDAMP_REG_1_POWERDOWN2           */
    SNDAMP_REG_2_POWERDOWN3_INIT_VAL,
                                      /* SNDAMP_REG_2_POWERDOWN3           */
    SNDAMP_REG_3_POWERDOWN4_INIT_VAL,
                                      /* SNDAMP_REG_3_POWERDOWN4           */
    SNDAMP_REG_4_NONCLIP2_INIT_VAL,
                                      /* SNDAMP_REG_4_NONCLIP2             */
    SNDAMP_REG_5_ATTACK_RELEASE_TIME_INIT_VAL,
                                      /* SNDAMP_REG_5_ATTACK_RELEASE_TIME  */
    SNDAMP_REG_6_RESERVE_INIT_VALD,
                                      /* SNDAMP_REG_6_RESERVED             */
    SNDAMP_REG_7_MONAURAL_INIT_VAL,
                                      /* SNDAMP_REG_7_MONAURAL             */
    SNDAMP_REG_8_LINE1_LCH_INIT_VAL,
                                      /* SNDAMP_REG_8_LINE1_LCH            */
    SNDAMP_REG_9_LINE1_RCH_INIT_VAL,
                                      /* SNDAMP_REG_9_LINE1_RCH            */
    SNDAMP_REG_10_LINE2_LCH_INIT_VAL,
                                      /* SNDAMP_REG_10_LINE2_LCH           */
    SNDAMP_REG_11_LINE2_RCH_INIT_VAL,
                                      /* SNDAMP_REG_11_LINE2_RCH           */
    SNDAMP_REG_12_HEADPHONE_MIXER_INIT_VAL,
                                      /* SNDAMP_REG_12_HEADPHONE_MIXER     */
    SNDAMP_REG_13_SPEAKER_MIXER_INIT_VAL,
                                      /* SNDAMP_REG_13_SPEAKER_MIXER       */
    SNDAMP_REG_14_SPEAKER_ATT_INIT_VAL,
                                      /* SNDAMP_REG_14_SPEAKER_ATT         */
    SNDAMP_REG_15_HEADPHONE_ATT_LCH_INIT_VAL,
                                      /* SNDAMP_REG_15_HEADPHONE_ATT_LCH   */
    SNDAMP_REG_16_HEADPHONE_ATT_RCH_INIT_VAL,
                                      /* SNDAMP_REG_16_HEADPHONE_ATT_RCH   */
    SNDAMP_REG_17_ERROR_FLAG_INIT_VAL
                                      /* SNDAMP_REG_17_ERROR_FLAG          */
};



struct pm8058_gpio aud_ddc_en[SNDAMP_DCDC_MAX] = {
  {                                   
    PM_GPIO_DIR_OUT,                  /* direction                         */
    PM_GPIO_OUT_BUF_CMOS,             /* output buffer                     */
    0,                                /* output value                      */
    PM_GPIO_PULL_NO,                  /* pull                              */
    6,                                /* vin Sel                           */
    PM_GPIO_STRENGTH_LOW,             /* out strength                      */
    PM_GPIO_FUNC_NORMAL,              /* function                          */
    0                                 /* invert interrupt polarity         */
  },
  {                                   
    PM_GPIO_DIR_OUT,                  /* direction                         */
    PM_GPIO_OUT_BUF_CMOS,             /* output buffer                     */
    1,                                /* output value                      */
    PM_GPIO_PULL_NO,                  /* pull                              */
    6,                                /* vin Sel                           */
    PM_GPIO_STRENGTH_LOW,             /* out strength                      */
    PM_GPIO_FUNC_NORMAL,              /* function                          */
    0                                 /* invert interrupt polarity         */
  },
};




/*-----------------------------------------------------*/

/* Device : SNDAMP_DEVICE_HEADSET_STEREO               */
/*-----------------------------------------------------*/
sndamp_cal_type sndamp_device_headset_stereo_cal = {
    0x15,                             /* input volume(SVLA)                */
    0x15,                             /* input volume(SVRA)                */
    0x1E,                             /* output volume(SALA)               */
    0x1E,                             /* output volume(SARA)               */
    0x00,                             /* output volume(MNA)                */
    0x00,                             /* Non-Clip2                         */
    0x00                              /* Attack/Release Time               */
};

/*-----------------------------------------------------*/

/* Device : SNDAMP_DEVICE_SPEAKER_STEREO               */
/*-----------------------------------------------------*/
sndamp_cal_type sndamp_device_speaker_stereo_cal = {
    0x15,                             /* input volume(SVLA)                */
    0x15,                             /* input volume(SVRA)                */
    0x00,                             /* output volume(SALA)               */
    0x00,                             /* output volume(SARA)               */
    0x1D,                             /* output volume(MNA)                */
    0x01,                             /* Non-Clip2                         */
    0x09                              /* Attack/Release Time               */
};

/*-----------------------------------------------------*/

/* Device : SNDAMP_DEVICE_FMRADIO_SPEAKER              */
/*-----------------------------------------------------*/
sndamp_cal_type sndamp_device_fmradio_speaker_cal = {
    0x15,                             /* input volume(SVLA)                */
    0x15,                             /* input volume(SVRA)                */
    0x00,                             /* output volume(SALA)               */
    0x00,                             /* output volume(SARA)               */
    0x1D,                             /* output volume(MNA)                */
    0x01,                             /* Non-Clip2                         */
    0x09                              /* Attack/Release Time               */
};

/*-----------------------------------------------------*/

/* Device : SNDAMP_DEVICE_TTY_HEADSET_MONO             */
/*-----------------------------------------------------*/
sndamp_cal_type sndamp_device_tty_headset_mono_cal = {
    0x17,                             /* input volume(SVLA)                */
    0x00,                             /* input volume(SVRA)                */
    0x1F,                             /* output volume(SALA)               */
    0x00,                             /* output volume(SARA)               */
    0x00,                             /* output volume(MNA)                */
    0x00,                             /* Non-Clip2                         */
    0x00                              /* Attack/Release Time               */
};


/*-----------------------------------------------------*/

/* Device : SNDAMP_DEVICE_HEADSET_STEREO_SPEAKER_STEREO */
/*-----------------------------------------------------*/
sndamp_cal_type sndamp_device_headset_stereo_speaker_stereo_cal = {
    0x15,                             /* input volume(SVLA)                */
    0x15,                             /* input volume(SVRA)                */
    0x1B,                             /* output volume(SALA)               */
    0x1B,                             /* output volume(SARA)               */
    0x1E,                             /* output volume(MNA)                */
    0x01,                             /* Non-Clip2                         */
    0x09                              /* Attack/Release Time               */
};

/*-----------------------------------------------------*/

/* Device : SNDAMP_DEVICE_PTT_SPEAKER_STEREO           */
/*-----------------------------------------------------*/
sndamp_cal_type sndamp_device_ptt_speaker_stereo_cal = {
    0x15,                             /* input volume(SVLA)                */
    0x15,                             /* input volume(SVRA)                */
    0x00,                             /* output volume(SALA)               */
    0x00,                             /* output volume(SARA)               */
    0x1D,                             /* output volume(MNA)                */
    0x01,                             /* Non-Clip2                         */
    0x09                              /* Attack/Release Time               */
};


sndamp_cal_type* sndamp_cal_tbl[SNDAMP_DEVICE_MAX] = {
    &sndamp_device_headset_stereo_cal, 
                                      /* SNDAMP_DEVICE_HEADSET_STEREO      */
    &sndamp_device_speaker_stereo_cal,
                                      /* SNDAMP_DEVICE_SPEAKER_STEREO      */
    &sndamp_device_fmradio_speaker_cal,
                                      /* SNDAMP_DEVICE_FMRADIO_SPEAKER     */
    &sndamp_device_tty_headset_mono_cal,
                                      /* SNDAMP_DEVICE_TTY_HEADSET_MONO    */
    &sndamp_device_headset_stereo_speaker_stereo_cal,
                                      /* SNDAMP_DEVICE_HEADSET_STEREO_SPEAKER_STEREO */
    &sndamp_device_ptt_speaker_stereo_cal,
                                      /* SNDAMP_DEVICE_PTT_SPEAKER_STEREO  */
};




sndamp_device_type g_sndamp_cur_path = SNDAMP_DEVICE_HEADSET_STEREO;


sndamp_sp_state_type g_snd_amp_sp_state = SNDAMP_SP_OFF;


static struct timer_list sndamp_common_power_down_delay_timer;


bool g_sndamp_common_power_down_delay_enable = false;


bool g_sndamp_init_enable = false;


void sndamp_common_power_down_work_func(struct work_struct *work);
DECLARE_WORK(work_create_sndamp, sndamp_common_power_down_work_func);



sndamp_proc_device_type sndamp_yda160_getCurrentDevice( void )
{
    sndamp_proc_device_type device;    

    
    switch( g_sndamp_cur_path )
    {
        
        case SNDAMP_DEVICE_HEADSET_STEREO :
            printk( KERN_INFO "[yda160] Current Device HEADSET : %d \n",
                    g_sndamp_cur_path );
            
            device = SNDAMP_PROC_DEVICE_HEADSET;
            break;

        
        case SNDAMP_DEVICE_SPEAKER_STEREO :
        case SNDAMP_DEVICE_FMRADIO_SPEAKER :
        case SNDAMP_DEVICE_PTT_SPEAKER_STEREO :
            printk( KERN_INFO "[yda160] Current Device SPEAKER : %d \n",
                    g_sndamp_cur_path );
            
            device = SNDAMP_PROC_DEVICE_SPEAKER;
            break;

        
        case SNDAMP_DEVICE_TTY_HEADSET_MONO :
            printk( KERN_INFO "[yda160] Current Device TTY HEADSET : %d \n",
                    g_sndamp_cur_path );
            
            device = SNDAMP_PROC_DEVICE_TTY_HEADSET;
            break;

        
        case SNDAMP_DEVICE_HEADSET_STEREO_SPEAKER_STEREO :
            printk( KERN_INFO "[yda160] Current Device HEADSET SPEAKER : %d \n",
                    g_sndamp_cur_path );
            
            device = SNDAMP_PROC_DEVICE_HEADSET_SPEAKER;
            break;

        
        default:
            printk( KERN_ERR "[yda160] Current Device OTHER : %d \n",
                    g_sndamp_cur_path );
            
            device = SNDAMP_PROC_DEVICE_INVALID;
            break;
    }

    
    return device;

}/* end of sndamp_yda160_getCurrentDevice */
/* <EJECT> */


int sndamp_yda160_WriteReg(sndamp_yda160_register_type r)
{
    int                 ret = 0;    

    
    if ( yda160_write( (u8)SNDAMP_ADDR(r), (u8)SNDAMP_BUF(r) ) ) 
    {
        

        printk( KERN_INFO "[yda160] Write REG : ADDR=0x%x DATA=0x%x \n",
                SNDAMP_ADDR(r),
                SNDAMP_BUF(r) );
        
        ret = 0; 
    }else{
        

        printk( KERN_ERR "[yda160] yda160_WriteReg ERR \n" );
        
        ret = 1;
    }

    
    return ret;

}/* end of sndamp_yda160_WriteReg */
/* <EJECT> */


int sndamp_yda160_ReadReg(sndamp_yda160_register_type r)
{
    int                    ret = 0;     
    unsigned char          read_buf;    
    int                    i2c_ret;     

    
    i2c_ret = yda160_read( (u8)(SNDAMP_ADDR(r)), (u8 *)&(read_buf) );

    
    if(i2c_ret)
    {
        

        
        memcpy( (void *)&SNDAMP_BUF(r), (void*)&read_buf, SNDAMP_BUFFER_LENGTH );
        printk( KERN_INFO "[yda160] Read REG : ADDR=0x%x DATA=0x%x \n",
                SNDAMP_ADDR(r),
                SNDAMP_BUF(r) );
        
        ret =  0;
    }else{
        

        printk( KERN_ERR "[yda160] yda160_ReadReg ERR \n" );
        
        ret = 1;
    }

    
    return ret;

} /* end of sndamp_yda160_ReadReg */
/* <EJECT> */


void sndamp_yda160_amp_set_speaker_att_gain(unsigned char out_vol)
{
    printk( KERN_INFO "[yda160] set speaker ATT MNA = 0x%x \n", out_vol );

    
    SNDAMP_SET_REG_14_SPEAKER_ATT_MNA( out_vol );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_14_SPEAKER_ATT );

} /* end of sndamp_yda160_amp_set_speaker_att_gain */
/* <EJECT> */


void sndamp_yda160_amp_set_headphone_att_gain(unsigned char out_sala, unsigned char out_sara)
{
    printk( KERN_INFO "[yda160] set headphone ATT out(SALA)=0x%x / out(SARA)=0x%x \n",
            out_sala,
            out_sara );

    
    SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_SALA( out_sala );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_15_HEADPHONE_ATT_LCH );

    
    SNDAMP_SET_REG_16_HEADPHONE_ATT_RCH_SARA( out_sara );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_16_HEADPHONE_ATT_RCH );

} /* end of sndamp_yda160_amp_set_headphone_att_gain */
/* <EJECT> */


void sndamp_yda160_amp_set_input_volume(unsigned char in_svla, unsigned char in_svra)
{
    printk( KERN_INFO "[yda160] set input SVLA=0x%x, SVRA=0x%x \n",
            in_svla,
            in_svra );

    

    
    SNDAMP_SET_REG_8_LINE1_LCH_SVLA( in_svla );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_8_LINE1_LCH );

    
    SNDAMP_SET_REG_9_LINE1_RCH_SVRA( in_svra );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_9_LINE1_RCH );

} /* end of sndamp_yda160_amp_set_input_volume */
/* <EJECT> */


void sndamp_yda160_amp_set_sp_state( sndamp_sp_state_type state )
{
    printk( KERN_INFO "[yda160] set sp state %d \n", state );

    
    g_snd_amp_sp_state = state;

    if( g_snd_amp_sp_state == SNDAMP_SP_ON )
    {
        

        
        pm_obs_a_speaker( SNDAMP_PM_OBS_SPEAKER_ON );
    }
    else
    {
        

        
        pm_obs_a_speaker( SNDAMP_PM_OBS_SPEAKER_OFF );
    }
} /* end of sndamp_yda160_amp_set_sp_state */
/* <EJECT> */


void sndamp_yda160_amp_common_power_down( void )
{
    printk( KERN_INFO "[yda160] common power down \n");

    
    SNDAMP_SET_REG_0_POWER_DOWN1_PDPC( SNDAMP_REG_ON );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_0_POWERDOWN1 );

    
    SNDAMP_SET_REG_0_POWER_DOWN1_PDP( SNDAMP_REG_ON );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_0_POWERDOWN1 );

} /* end of sndamp_yda160_amp_common_power_down */
/* <EJECT> */


void sndamp_yda160_amp_input_power_down( void )
{
    printk( KERN_INFO "[yda160] input power down \n");

    
    sndamp_yda160_amp_set_input_volume( SNDAMP_VOLUME_INPUT_MUTE,
                                        SNDAMP_VOLUME_INPUT_MUTE );

} /* end of sndamp_yda160_amp_input_power_down */
/* <EJECT> */


void sndamp_yda160_amp_headphone_att_power_down( void )
{
    printk( KERN_INFO "[yda160] headphone att power down \n" );

    
    SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_ZCS_HPA( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_15_HEADPHONE_ATT_LCH );

    
    sndamp_yda160_amp_set_headphone_att_gain( SNDAMP_VOLUME_OUTPUT_MUTE,
                                              SNDAMP_VOLUME_OUTPUT_MUTE );

    
    msleep( SNDAMP_HEADPHONE_ATT_TO_MUTE_DELAY_TIME );

} /* end of sndamp_yda160_amp_headphone_att_power_down */
/* <EJECT> */


void sndamp_yda160_amp_speaker_att_power_down( void )
{
    printk( KERN_INFO "[yda160] speaker att power down \n" );

    
    SNDAMP_SET_REG_14_SPEAKER_ATT_ZCS_SPA( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_14_SPEAKER_ATT );

    
    sndamp_yda160_amp_set_speaker_att_gain( SNDAMP_VOLUME_OUTPUT_MUTE );

    
    msleep( SNDAMP_SPEAKER_ATT_TO_MUTE_DELAY_TIME );

} /* end of sndamp_yda160_amp_speaker_att_power_down */
/* <EJECT> */


void sndamp_yda160_amp_headphone_mixier_vdd_power_down( void )
{
    printk( KERN_INFO "[yda160] headphone mixier power down \n" );

    
    
    SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPL_AMIX( SNDAMP_REG_OFF );
    
    SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_AMIX( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_12_HEADPHONE_MIXER );

    
    SNDAMP_SET_REG_2_POWER_DOWN3_PD_REG( SNDAMP_REG_ON );

    
    SNDAMP_SET_REG_2_POWER_DOWN3_PD_CHP( SNDAMP_REG_ON );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_2_POWERDOWN3 );

} /* end of sndamp_yda160_amp_headphone_mixier_vdd_power_down */
/* <EJECT> */


void sndamp_yda160_amp_speaker_mixier_vdd_power_down( void )
{
    printk( KERN_INFO "[yda160] speaker mixier power down \n" );

    
    
    SNDAMP_SET_REG_13_SPEAKER_MIXER_SPL_AMIX( SNDAMP_REG_OFF );
    
    SNDAMP_SET_REG_13_SPEAKER_MIXER_SPR_AMIX( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_13_SPEAKER_MIXER );

    
    pm8058_gpio_config( SNDAMP_PMIC_GPIO_AUD_DDC_EN, &aud_ddc_en[SNDAMP_DCDC_OFF] );
    printk( KERN_INFO "[yda160] AUDDDC_EN:OFF \n" );
    

} /* end of sndamp_yda160_amp_speaker_mixier_vdd_power_down */
/* <EJECT> */


void sndamp_yda160_amp_headphone_power_down( void )
{
    printk( KERN_INFO "[yda160] headphone power down \n" );

    
    sndamp_yda160_amp_headphone_att_power_down();

    
    sndamp_yda160_amp_headphone_mixier_vdd_power_down();

} /* end of sndamp_yda160_amp_headphone_power_down */
/* <EJECT> */


void sndamp_yda160_amp_speaker_power_down( void )
{
    printk( KERN_INFO "[yda160] speaker power down \n" );

    
    sndamp_yda160_amp_speaker_att_power_down();

    
    sndamp_yda160_amp_speaker_mixier_vdd_power_down();

    
    sndamp_yda160_amp_set_sp_state( SNDAMP_SP_OFF );

} /* end of sndamp_yda160_amp_speaker_power_down */
/* <EJECT> */


void sndamp_yda160_amp_headphone_speaker_power_down( void )
{
    printk( KERN_INFO "[yda160] headphone speaker power down \n" );

    
    sndamp_yda160_amp_headphone_att_power_down();

    
    sndamp_yda160_amp_speaker_att_power_down();

    
    sndamp_yda160_amp_headphone_mixier_vdd_power_down();

    
    sndamp_yda160_amp_speaker_mixier_vdd_power_down();

    
    sndamp_yda160_amp_set_sp_state( SNDAMP_SP_OFF );

} /* end of sndamp_yda160_amp_headphone_speaker_power_down */
/* <EJECT> */


void sndamp_common_power_down_delay_timer_handler( unsigned long data )
{
    u64 now = get_jiffies_64();         

    
    schedule_work( &work_create_sndamp );

    printk( KERN_INFO "[yda160] common power down timer expired now(%lld) \n",
            now );

} /* end of sndamp_common_power_down_delay_timer_handler */
/* <EJECT> */


void sndamp_common_power_down_delay_timer_start( void )
{
    u64 now = get_jiffies_64();         

    if( g_sndamp_common_power_down_delay_enable == false )
    {
        

        
        g_sndamp_common_power_down_delay_enable = true;

        
        init_timer( &sndamp_common_power_down_delay_timer );

        
        sndamp_common_power_down_delay_timer.expires = now + SNDAMP_KERNEL_TIMER_TIMEOUT_VAL( SNDAMP_POWER_DOWN_DELAY_TIME );

        
        sndamp_common_power_down_delay_timer.data = (unsigned long)now;

        
        sndamp_common_power_down_delay_timer.function = sndamp_common_power_down_delay_timer_handler;

        
        
        add_timer( &sndamp_common_power_down_delay_timer );

        printk( KERN_INFO "[yda160] common power down timer start %d(%d) now(%lld) \n",
                SNDAMP_POWER_DOWN_DELAY_TIME,
                SNDAMP_KERNEL_TIMER_TIMEOUT_VAL( SNDAMP_POWER_DOWN_DELAY_TIME ),
                now );
    }

} /* end of sndamp_common_power_down_delay_timer_start */
/* <EJECT> */


void sndamp_common_power_down_delay_timer_delete( void )
{
    
    if ( g_sndamp_common_power_down_delay_enable != false )
    {
        

        
        g_sndamp_common_power_down_delay_enable = false;

        
        del_timer( &sndamp_common_power_down_delay_timer );

        printk( KERN_INFO "[yda160] common power down timer delete \n" );

    }
    else
    {
        
        printk( KERN_INFO "[yda160] common power down timer is not started \n" );
    }

} /* end of sndamp_common_power_down_delay_timer_delete */
/* <EJECT> */


void sndamp_common_power_down_work_func(struct work_struct *work)
{
    
    if( g_sndamp_common_power_down_delay_enable != false )
    {
        

        
        g_sndamp_common_power_down_delay_enable = false;

        
        sndamp_yda160_amp_common_power_down();

        printk( KERN_INFO "[yda160] sndamp_common_power_down_work_func() common power down DONE \n" );

    }
    else
    {
        
        printk( KERN_INFO "[yda160] common part not power down \n" );
    }

} /* end of sndamp_common_power_down_work_func */
/* <EJECT> */


void sndamp_shutdown( void )
{
    sndamp_proc_device_type  curDev;    

    printk( KERN_INFO "[yda160] sndamp shutdown dev=%d \n", g_sndamp_cur_path );

    curDev = sndamp_yda160_getCurrentDevice();

    switch( curDev )
    {
        
        case SNDAMP_PROC_DEVICE_HEADSET:
        case SNDAMP_PROC_DEVICE_TTY_HEADSET:
            
            sndamp_yda160_amp_headphone_power_down();
            break;

        
        case SNDAMP_PROC_DEVICE_SPEAKER:
            
            sndamp_yda160_amp_speaker_power_down();
            break;

        
        case SNDAMP_PROC_DEVICE_HEADSET_SPEAKER:
            
            sndamp_yda160_amp_headphone_speaker_power_down();
            break;

        
        default:
            printk( KERN_ERR "[yda160] power down device invalid dev=%d \n", curDev );
            break;
    }

    
    sndamp_yda160_amp_input_power_down();

    
    sndamp_yda160_amp_common_power_down();

} /* end of sndamp_shutdown */
/* <EJECT> */


void sndamp_yda160_amp_speaker_mixier_vdd_power_down_release( void )
{
    printk( KERN_INFO "[yda160] speaker mixier power down release \n" );

    
    
    pm8058_gpio_config( SNDAMP_PMIC_GPIO_AUD_DDC_EN, &aud_ddc_en[SNDAMP_DCDC_ON] );
    printk( KERN_INFO "[yda160] AUDDDC_EN:ON \n" );
    

    /* Waits for 4msec(According to specifications) */
    msleep( SNDAMP_DCDC_START_DELAY_TIME );

    
    SNDAMP_SET_REG_14_SPEAKER_ATT_ZCS_SPA( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_14_SPEAKER_ATT );

    
    
    SNDAMP_SET_REG_13_SPEAKER_MIXER_SPL_AMIX( SNDAMP_REG_ON );
    
    SNDAMP_SET_REG_13_SPEAKER_MIXER_SPR_AMIX( SNDAMP_REG_ON );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_13_SPEAKER_MIXER );

} /* end of sndamp_yda160_amp_speaker_mixier_vdd_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_headphone_mixier_vdd_power_down_release( void )
{
    sndamp_proc_device_type  curDev;    

    printk( KERN_INFO "[yda160] headphone mixier power down release \n" );

    
    SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_ZCS_HPA( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_15_HEADPHONE_ATT_LCH );

    
    SNDAMP_SET_REG_2_POWER_DOWN3_PD_REG( SNDAMP_REG_OFF );

    
    SNDAMP_SET_REG_2_POWER_DOWN3_PD_CHP( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_2_POWERDOWN3 );

    
    msleep( SNDAMP_CHARGE_PUMP_WAKE_UP_DELAY_TIME );

    
    
    SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPL_AMIX( SNDAMP_REG_ON );

    curDev = sndamp_yda160_getCurrentDevice();

    switch( curDev )
    {
        case SNDAMP_PROC_DEVICE_HEADSET:    
        case SNDAMP_PROC_DEVICE_SPEAKER:    
        case SNDAMP_PROC_DEVICE_HEADSET_SPEAKER: 
            printk( KERN_INFO "[yda160] headphone power down release not TTY device %d \n",
                    curDev );
            
            SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_AMIX( SNDAMP_REG_ON );
            
            SNDAMP_SET_REG_12_HEADPHONE_MIXER_MONO_HP( SNDAMP_REG_OFF );
            break;
        case SNDAMP_PROC_DEVICE_TTY_HEADSET:    
            printk( KERN_INFO "[yda160] headphone power down release TTY device %d \n",
                    curDev );
            
            SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_AMIX( SNDAMP_REG_OFF );
            
            SNDAMP_SET_REG_12_HEADPHONE_MIXER_MONO_HP( SNDAMP_REG_ON );
            break;
        default:  
            printk( KERN_ERR "[yda160] headphone power down release device invalid %d \n",
                    curDev );
            
            SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_AMIX( SNDAMP_REG_ON );
            
            SNDAMP_SET_REG_12_HEADPHONE_MIXER_MONO_HP( SNDAMP_REG_OFF );
            break;
    }

    
    sndamp_yda160_WriteReg( SNDAMP_REG_12_HEADPHONE_MIXER );

} /* end of sndamp_yda160_amp_headphone_mixier_vdd_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_speaker_att_power_down_release( unsigned char out_vol )
{
    printk( KERN_INFO "[yda160] speaker att power down release OUTVOL=0x%x \n",
            out_vol );

    
    sndamp_yda160_amp_set_speaker_att_gain( out_vol );

    
    msleep( SNDAMP_SPEAKER_MUTE_TO_ATT_DELAY_TIME );

    
    SNDAMP_SET_REG_14_SPEAKER_ATT_ZCS_SPA( SNDAMP_REG_ON );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_14_SPEAKER_ATT );

} /* end of sndamp_yda160_amp_speaker_att_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_headphone_att_power_down_release(
    unsigned char out_sala,
    unsigned char out_sara
)
{
    printk( KERN_INFO "[yda160] headphone att power down release OUTVOL(SALA)=0x%x, OUTVOL(SARA)=0x%x \n",
            out_sala,
            out_sara );

    
    sndamp_yda160_amp_set_headphone_att_gain( out_sala, out_sara );

    
    msleep( SNDAMP_HEADPHONE_MUTE_TO_ATT_DELAY_TIME );

    
    SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_ZCS_HPA( SNDAMP_REG_ON );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_15_HEADPHONE_ATT_LCH );

} /* end of sndamp_yda160_amp_headphone_att_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_speaker_power_down_release( unsigned char out_vol )
{
    printk( KERN_INFO "[yda160] speaker power down release OUTVOL=0x%x \n",
            out_vol );

    
    sndamp_yda160_amp_speaker_mixier_vdd_power_down_release();

    
    sndamp_yda160_amp_speaker_att_power_down_release( out_vol );

    
    sndamp_yda160_amp_set_sp_state( SNDAMP_SP_ON );

} /* end of sndamp_yda160_amp_speaker_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_headphone_power_down_release(
    unsigned char out_sala,
    unsigned char out_sara
)
{
    printk( KERN_INFO "[yda160] headphone power down release OUTVOL(SALA)=0x%x, OUTVOL(SARA)=0x%x \n",
            out_sala,
            out_sara );

    
    sndamp_yda160_amp_headphone_mixier_vdd_power_down_release();

    
    sndamp_yda160_amp_headphone_att_power_down_release( out_sala, out_sara );

} /* end of sndamp_yda160_amp_headphone_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_headphone_speaker_power_down_release(
    unsigned char out_sala,
    unsigned char out_sara,
    unsigned char out_mna
)
{
    printk( KERN_INFO "[yda160] headphone power down release OUTVOL(SALA)=0x%x, OUTVOL(SARA)=0x%x, OUTVOL(MNA)=0x%x \n",
            out_sala,
            out_sara,
            out_mna );

    
    sndamp_yda160_amp_speaker_mixier_vdd_power_down_release();

    
    sndamp_yda160_amp_headphone_mixier_vdd_power_down_release();

    
    sndamp_yda160_amp_speaker_att_power_down_release( out_mna );

    
    sndamp_yda160_amp_set_sp_state( SNDAMP_SP_ON );

    
    sndamp_yda160_amp_headphone_att_power_down_release( out_sala, out_sara );

} /* end of sndamp_yda160_amp_headphone_speaker_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_input_power_down_release( unsigned char in_svla, unsigned char in_svra )
{
    printk( KERN_INFO "[yda160] input power down release VOL(SVLA)=0x%x, VOL(SVRA)=0x%x \n",
            in_svla,
            in_svra );

    sndamp_yda160_amp_set_input_volume( in_svla, in_svra );

    
    
    msleep( SNDAMP_CHARGE_INOUT_DC_CAP_DELAY_TIME );

} /* end of sndamp_yda160_amp_input_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_set_non_clip(
    unsigned char nonclip_mode,       /* Non-Clip2                         */
    unsigned char att_rel_time        /* Attack/Release Time               */
)
{
    printk( KERN_INFO "[yda160] set Non-Clip nonclip_mode=0x%02x att_rel_time=0x%02x \n",
            nonclip_mode,
            att_rel_time );

    
    SNDAMP_BUF(SNDAMP_REG_4_NONCLIP2) = nonclip_mode;

    
    sndamp_yda160_WriteReg(SNDAMP_REG_4_NONCLIP2);

    
    SNDAMP_BUF(SNDAMP_REG_5_ATTACK_RELEASE_TIME) = att_rel_time;

    
    sndamp_yda160_WriteReg( SNDAMP_REG_5_ATTACK_RELEASE_TIME );

} /* end of sndamp_yda160_amp_set_non_clip */
/* <EJECT> */


void sndamp_yda160_amp_common_power_down_release( void )
{
    printk( KERN_INFO "[yda160] common power down release \n" );

    
    SNDAMP_SET_REG_0_POWER_DOWN1_PDPC( SNDAMP_REG_OFF );

    
    SNDAMP_SET_REG_0_POWER_DOWN1_PDP( SNDAMP_REG_OFF );

    
    sndamp_yda160_WriteReg( SNDAMP_REG_0_POWERDOWN1 );

} /* end of sndamp_yda160_amp_common_power_down_release */
/* <EJECT> */


void sndamp_yda160_amp_init(void)
{
    printk( KERN_INFO "[yda160] sndamp_init \n");

    
    sndamp_yda160_WriteReg( SNDAMP_REG_0_POWERDOWN1 );
    sndamp_yda160_WriteReg( SNDAMP_REG_1_POWERDOWN2 );
    sndamp_yda160_WriteReg( SNDAMP_REG_2_POWERDOWN3 );
    sndamp_yda160_WriteReg( SNDAMP_REG_3_POWERDOWN4 );
    sndamp_yda160_WriteReg( SNDAMP_REG_4_NONCLIP2 );
    sndamp_yda160_WriteReg( SNDAMP_REG_5_ATTACK_RELEASE_TIME );
    sndamp_yda160_WriteReg( SNDAMP_REG_7_MONAURAL );
    sndamp_yda160_WriteReg( SNDAMP_REG_8_LINE1_LCH );
    sndamp_yda160_WriteReg( SNDAMP_REG_9_LINE1_RCH );
    sndamp_yda160_WriteReg( SNDAMP_REG_10_LINE2_LCH );
    sndamp_yda160_WriteReg( SNDAMP_REG_11_LINE2_RCH );
    sndamp_yda160_WriteReg( SNDAMP_REG_12_HEADPHONE_MIXER );
    sndamp_yda160_WriteReg( SNDAMP_REG_13_SPEAKER_MIXER );
    sndamp_yda160_WriteReg( SNDAMP_REG_14_SPEAKER_ATT );
    sndamp_yda160_WriteReg( SNDAMP_REG_15_HEADPHONE_ATT_LCH );
    sndamp_yda160_WriteReg( SNDAMP_REG_16_HEADPHONE_ATT_RCH );

} /* end of sndamp_yda160_amp_init */
/* <EJECT> */


void sndamp_startup( sndamp_device_type path )
{
    sndamp_cal_type          *vol_tbl;  
    sndamp_proc_device_type   dev;

    printk( KERN_INFO "[yda160] sndamp startup dev=%d \n", path );

    
    g_sndamp_cur_path = path;

    
    vol_tbl = sndamp_cal_tbl[g_sndamp_cur_path];

    
    if ( g_sndamp_init_enable == false )
    {
        
        sndamp_yda160_amp_init();

        
        g_sndamp_init_enable = true;
    }

    
    sndamp_yda160_amp_common_power_down_release();

    
    dev = sndamp_yda160_getCurrentDevice();
    switch( dev )
    {
        case SNDAMP_PROC_DEVICE_SPEAKER :     
        case SNDAMP_PROC_DEVICE_HEADSET_SPEAKER : 
            
            sndamp_yda160_amp_set_non_clip( vol_tbl->nonclip_mode,
                                            vol_tbl->att_rel_time );
            break;

        default :  
            printk( KERN_INFO "[yda160] not setting Non-Clip dev=%d \n", dev );
            break;
    }

    
    sndamp_yda160_amp_input_power_down_release( vol_tbl->in_svla, vol_tbl->in_svra );

    switch( dev )
    {
        
        case SNDAMP_PROC_DEVICE_HEADSET :
        case SNDAMP_PROC_DEVICE_TTY_HEADSET :
            
            sndamp_yda160_amp_headphone_power_down_release( vol_tbl->out_sala,
                                                            vol_tbl->out_sara );
            break;
        
        case SNDAMP_PROC_DEVICE_SPEAKER :
            
            sndamp_yda160_amp_speaker_power_down_release( vol_tbl->out_mna );
            break;
        
        case SNDAMP_PROC_DEVICE_HEADSET_SPEAKER :
            
            sndamp_yda160_amp_headphone_speaker_power_down_release( vol_tbl->out_sala,
                                                                    vol_tbl->out_sara,
                                                                    vol_tbl->out_mna );
            break;
        
        default:
            printk( KERN_ERR "[yda160] Current Device ERR dev=%d \n", dev );
            break;
    }

} /* end of sndamp_startup */
/* <EJECT> */


void sndamp_headset_stereo_power_on( void )
{
    
    sndamp_startup( SNDAMP_DEVICE_HEADSET_STEREO );
    return;

} /* end of sndamp_headset_stereo_power_on */
/* <EJECT> */


void sndamp_speaker_stereo_power_on( void )
{
    
    sndamp_startup( SNDAMP_DEVICE_SPEAKER_STEREO );
    return;

} /* end of sndamp_speaker_stereo_power_on */
/* <EJECT> */


void sndamp_fmradio_speaker_power_on( void )
{
    
    sndamp_startup( SNDAMP_DEVICE_FMRADIO_SPEAKER );
    return;

} /* end of sndamp_fmradio_speaker_power_on */
/* <EJECT> */


void sndamp_tty_headset_mono_power_on( void )
{
    
    sndamp_startup( SNDAMP_DEVICE_TTY_HEADSET_MONO );
    return;

} /* end of sndamp_tty_headset_mono_power_on */
/* <EJECT> */


void sndamp_headset_stereo_speaker_stereo_power_on( void )
{
    
    sndamp_startup( SNDAMP_DEVICE_HEADSET_STEREO_SPEAKER_STEREO );
    return;

} /* end of sndamp_headset_stereo_speaker_stereo_power_on */
/* <EJECT> */


void sndamp_ptt_speaker_stereo_power_on( void )
{
    
    sndamp_startup( SNDAMP_DEVICE_PTT_SPEAKER_STEREO );
    return;

} /* end of sndamp_ptt_speaker_stereo_power_on */
/* <EJECT> */


void sndamp_power_off( void )
{
    
    sndamp_shutdown();
    return;

} /* end of sndamp_power_off */
/* <EJECT> */


DEFINE_MUTEX(msm_sndamp_sem);


static int sound_amp_ioctl(struct inode *inode, struct file *filp,
                           unsigned int cmd, unsigned long arg )
{
    sndamp_diag_ctrl_type  diag_ctrl;           

    
    
    void __user *argp = (void __user *)arg;   

    printk( KERN_INFO "[yda160] I/O Control Start \n" );


    
    if( (cmd == SNDAMP_IOCTL_AMPCTL_READ)    ||
        (cmd == SNDAMP_IOCTL_AMPCTL_WRITE)   ||
        (cmd == SNDAMP_IOCTL_REGISTER_READ)  ||
        (cmd == SNDAMP_IOCTL_REGISTER_WRITE) ||
        (cmd == SNDAMP_IOCTL_DRIVE) ) {

        

        /* TODO:unsigned long copy_from_user(void *to, const void __user *from, unsigned long n) */
        
        
        
        
        

        
        if ( copy_from_user( &diag_ctrl, argp, sizeof(diag_ctrl) ) ) {
            

            printk( KERN_ERR "[yda160] I/O Control : copy_from_user ERROR \n" );
            
            return -EFAULT;
        }
    }

    
    mutex_lock(&msm_sndamp_sem);

    
    switch (cmd) {
        
        case SNDAMP_IOCTL_AMPCTL_READ:
        
        case SNDAMP_IOCTL_AMPCTL_WRITE:
            printk( KERN_INFO "[yda160] I/O Control : Cmd = SNDAMP_IOCTL_AMPCTL_READ/WRITE \n");
            
            sndamp_RW_SoundParam( &diag_ctrl.vol );
            break;

        
        case SNDAMP_IOCTL_REGISTER_READ:
            printk( KERN_INFO "[yda160] I/O Control : Cmd = SNDAMP_IOCTL_REGISTER_READ \n" );
            
            sndamp_read_reg_diag( diag_ctrl.reg.reg_addr, (unsigned char *)&(diag_ctrl.reg.reg_data) );

            break;

        
        case SNDAMP_IOCTL_REGISTER_WRITE:
            printk( KERN_INFO "[yda160] I/O Control : Cmd = SNDAMP_IOCTL_REGISTER_WRITE \n" );
            
            sndamp_write_reg_diag( diag_ctrl.reg.reg_addr, diag_ctrl.reg.reg_data );
            break;

        
        case SNDAMP_IOCTL_DRIVE:
            printk( KERN_INFO "[yda160] I/O Control : Cmd = SNDAMP_IOCTL_DRIVE \n" );
            
            sndamp_Drive_Control( &diag_ctrl.drv );
            break;

        
        default:
            printk( KERN_ERR "[yda160] I/O Control : Cmd = default \n" );
            mutex_unlock(&msm_sndamp_sem);
            
            return -EFAULT;
    }
    
    mutex_unlock(&msm_sndamp_sem);

    
    if( (cmd == SNDAMP_IOCTL_AMPCTL_READ)   ||
        (cmd == SNDAMP_IOCTL_AMPCTL_WRITE)  ||
        (cmd == SNDAMP_IOCTL_REGISTER_READ) ) {

        

        /* TODO:unsigned long copy_to_user(void __user *to, const void *from, unsigned long n)   */
        
        
        

        
        if ( copy_to_user(argp, &diag_ctrl, sizeof(diag_ctrl) ) ) {
            

            printk( KERN_ERR "[yda160] I/O Control : copy_to_user ERROR \n" );
            
            return -EFAULT;
        }
    }

    
    return 1;

} /* end of sound_amp_ioctl */
/* <EJECT> */


static int sound_amp_open(struct inode *inode, struct file *file)
{
    if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
        if (atomic_cmpxchg(&open_flag, 0, 2) == 0) {
            atomic_set(&reserve_open_flag, 2);
            wake_up(&open_wq);
            printk( KERN_INFO "[yda160] sndamp driver open : OK \n" );
            return 0;
        }
    }
    printk( KERN_ERR "[yda160] sndamp driver open : ERROR \n" );
    return -1;

} /* end of sound_amp_open */
/* <EJECT> */


static int sound_amp_release(struct inode *inode, struct file *file)
{
    atomic_set(&reserve_open_flag, 0);
    atomic_set(&open_flag, 0);
    atomic_set(&open_count, 0);
    wake_up(&open_wq);
    printk( KERN_INFO "[yda160] sndamp driver release : OK \n" );
    return 0;

} /* end of sound_amp_release */
/* <EJECT> */

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
static struct file_operations sound_amp_ctl_fops = {
    .owner    = THIS_MODULE,
    .ioctl    = sound_amp_ioctl,
    .open     = sound_amp_open,
    .release  = sound_amp_release,
};

static struct miscdevice sndamp_dev = {
    .minor = MISC_DYNAMIC_MINOR ,
    .name = SNDAMP_DEVICECHR_NAME ,
    .fops = &sound_amp_ctl_fops ,
};







static int __init sound_amp_init(void)
{
    int ret;

    ret = misc_register(&sndamp_dev);
    if ( ret ) {
        printk( KERN_ERR "[yda160] sound_amp_INIT : FAIL to misc_register \n" );
        return ret;
    }

    
    
    
    
    
    
    
    
    

    
    mutex_init(&msm_sndamp_sem);
    printk( KERN_INFO "[yda160] sound_amp_INIT : OK to misc_register \n" );

    
    

    
    gpio_set_value( SNDAMP_GPIO_AUD_AMP_RST, SNDAMP_GPIO_LOW_VALUE );

    /* 1ms wait */
    mdelay( SNDAMP_AUD_AMP_RST_DELAY_TIME );

    
    gpio_set_value( SNDAMP_GPIO_AUD_AMP_RST, SNDAMP_GPIO_HIGH_VALUE );

    return 0;

} /* end of sound_amp_init */
/* <EJECT> */


static void __exit sound_amp_exit(void)
{
    
    mutex_destroy( &msm_sndamp_sem );

    
    misc_deregister(&sndamp_dev);

    printk( KERN_INFO "[yda160] sound_amp_EXIT \n" );

    return;

} /* end of sound_amp_exit */
/* <EJECT> */

module_init(sound_amp_init);
module_exit(sound_amp_exit);


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


int sndamp_write_reg_diag(unsigned char addrReg, unsigned char buf)
{
    
    int result = 0;

    
    sndamp_yda160_register_type reg_e = addrReg - SNDAMP_ADDR_BASE;

    
    SNDAMP_BUF(reg_e) = buf;

    
    result = sndamp_yda160_WriteReg( reg_e );

    return result;

} /* end of sndamp_write_reg_diag */
/* <EJECT> */


int sndamp_read_reg_diag(unsigned char addrReg, unsigned char* buf)
{
    
    int result = 0;

    
    sndamp_yda160_register_type reg_e = addrReg - SNDAMP_ADDR_BASE;

    
    result = sndamp_yda160_ReadReg( reg_e );

    
    if(result == 0)
    {
        
        *buf = SNDAMP_BUF( reg_e );
    }
    else
    {
        
        *buf = 0xFF;
    }

    return result;

} /* end of sndamp_read_reg_diag */
/* <EJECT> */


void sndamp_Drive_Control( sndamp_drive_ctrl_type  *ctrl )
{
    
    if(ctrl != NULL)
    {
        printk( KERN_INFO "[yda160] sndamp_Drive_Control() ctrl != NULL \n" );

        
        if(ctrl->vol_id < SNDAMP_DEVICE_MAX)
        {
            switch( ctrl->drv_ctl )
            {
                
                case SNDAMP_DIAG_DRV_POWER_DOWN:
                    sndamp_shutdown();
                    break;
                
                case SNDAMP_DIAG_DRV_POWER_DOWN_RELEASE:
                    sndamp_startup( (sndamp_device_type)(ctrl->vol_id) );
                    break;
                
                default:
                    printk( KERN_ERR "[yda160] sndamp_Drive_Control() drive parameter ERROR : 0x%x \n", ctrl->drv_ctl );
                    break;
            }

        } else {
            printk( KERN_ERR "[yda160] sndamp_Drive_Control() volume id ERROR : %d \n", ctrl->vol_id );
        }

    } else {
        printk( KERN_ERR "[yda160] sndamp_Drive_Control()  ERROR : ctrl == NULL \n" );
    }

    return;

} /* end of sndamp_Drive_Control */
/* <EJECT> */


void sndamp_RW_SoundParam( sndamp_volume_ctrl_type  *ctrl )
{
    
    sndamp_cal_type   *target_tbl = NULL;

    
    if(ctrl != NULL)
    {
        printk( KERN_INFO "[yda160] sndamp_RW_SoundParam() ctrl != NULL \n" );

        
        ctrl->data[SNDAMP_DIAG_RES_POS_VOL_RES_STATUS] = SNDAMP_DIAG_STATUS_NORMAL;

        
        if(ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_DEV_ID] < SNDAMP_DEVICE_MAX)
        {
            printk( KERN_INFO "[yda160] sndamp_RW_SoundParam() volume id : %d \n",
                    ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_DEV_ID] );

            
            target_tbl = sndamp_cal_tbl[(ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_DEV_ID])];

            
            switch( ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_RW] )
            {
                
                case SNDAMP_DIAG_VOL_WRITE:

                    
                    target_tbl->in_svla      = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_IN_SVLA];
                    target_tbl->in_svra      = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_IN_SVRA];
                    target_tbl->out_sala     = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_OUT_SALA];
                    target_tbl->out_sara     = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_OUT_SARA];
                    target_tbl->out_mna      = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_OUT_MNA];
                    target_tbl->nonclip_mode = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_NONCLIP_MODE];
                    target_tbl->att_rel_time = ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_ATT_REL_TIME];
                    printk( KERN_INFO "[yda160] sndamp_RW_SoundParam : WRITE OK \n");
                    break;

                
                case SNDAMP_DIAG_VOL_READ:

                    
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_IN_SVLA]      = target_tbl->in_svla;
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_IN_SVRA]      = target_tbl->in_svra;
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_OUT_SALA]     = target_tbl->out_sala;
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_OUT_SARA]     = target_tbl->out_sara;
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_OUT_MNA]      = target_tbl->out_mna;
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_NONCLIP_MODE] = target_tbl->nonclip_mode;
                    ctrl->data[SNDAMP_DIAG_RES_POS_VOL_ATT_REL_TIME] = target_tbl->att_rel_time;
                    printk( KERN_INFO "[yda160] sndamp_RW_SoundParam : READ OK \n");
                    break;

                
                default:

                  
                  ctrl->data[SNDAMP_DIAG_RES_POS_VOL_RES_STATUS] = SNDAMP_DIAG_STATUS_ERR_PROCESS_MODE;
                  printk( KERN_ERR "[yda160] sndamp_RW_SoundParam() read/write parameter ERROR : 0x%x \n",
                                   ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_RW] );
                  break;
            }

        } else {

            
            ctrl->data[SNDAMP_DIAG_RES_POS_VOL_RES_STATUS] = SNDAMP_DIAG_STATUS_ERR_INVALID_ID;
            printk( KERN_ERR "[yda160] sndamp_RW_SoundParam() volume id ERROR : %d \n",
                    ctrl->data[SNDAMP_DIAG_REQ_POS_VOL_DEV_ID] );
        }
    } else {
        printk( KERN_ERR "[yda160] sndamp_RW_SoundParam()  ERROR : ctrl == NULL \n" );
    }

    return;

} /* end of sndamp_RW_SoundParam */
/* <EJECT> */




/* snd@fsi-iakira4:/sbin$ modinfo fuse                                                                            */
/* filename:       /lib/modules/2.6.24-23-generic/kernel/fs/fuse/fuse.ko                                          */
/* alias:          char-major-10-229                                                                              */
/* license:        GPL                                                                                            */
/* description:    Filesystem in Userspace                                                                        */
/* author:         Miklos Szeredi <miklos@szeredi.hu>                                                             */
/* srcversion:     E541E638476D5621E6382F8                                                                        */
/* depends:                                                                                                       */
/* vermagic:       2.6.24-23-generic SMP mod_unload 586                                                           */



MODULE_AUTHOR("");
MODULE_DESCRIPTION("yda160 soundamp driver");
MODULE_LICENSE("GPL");

