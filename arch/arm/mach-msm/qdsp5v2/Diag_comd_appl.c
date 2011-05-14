/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#include <linux/module.h>
#include <linux/platform_device.h>

//#include <linux/mfd/marimba-codec.h>
#include <linux/mfd/msm-adie-codec.h>

#include <linux/mfd/marimba.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <asm/uaccess.h>

#include <mach/qdsp5v2/snddev_icodec.h>

#include "Diag_com_appl.h"


#include "yda160_sndamp_diag.h"



/* data load function */
static void _diag_calibration_data_load( void );
Diag_snd_calib_load_func_type Diag_snd_calib_data_load_func = _diag_calibration_data_load;


short Diag_Audio_earjack( Diag_Audio_earjack_type *data )
{
	return ( 0x0000 ) ;
}
EXPORT_SYMBOL(Diag_Audio_earjack);


static s32 _diag_tool_snd_vol_bcd_decode(char *data)
{
	s32 ret_val;

	ret_val  = ((data[1] >> 4) & 0xF) * 1000;
	ret_val += (data[1] & 0xF) * 100;
	ret_val += ((data[2] >> 4) & 0xF) * 10;
	ret_val += data[2] & 0xF;
	if ( data[0] ) ret_val = -ret_val;
	return ret_val;
}


static void _diag_tool_snd_vol_bcd_encode(s32 val, char *data)
{
	s32 tmp;

	if ( val < 0 ){
		data[0] = 1;
		val = -val;
	} else {
		data[0] = 0;
	}

	data[1]  = ((val / 1000) &0xF) << 4;
	tmp = val % 1000;
	data[1] |= (tmp / 100) & 0xF;
	tmp = tmp %100;
	data[2]  = ((tmp / 10) & 0xF) << 4;
	tmp = tmp % 10;
	data[2] |= tmp;
	return;
}


void Diag_Audio_vol_limit_wrapper(char * req_data, char * rsp_data )
{
    Diag_Audio_vol_limit_type vol_info;			
    unsigned char ret_val;						

	printk(KERN_INFO "Diag_Audio_vol_limit_wrapper\n");
	
	vol_info.modeRW  = req_data[1];				
	vol_info.dev_idx = req_data[2];				/* device index */
	vol_info.max_nb  = _diag_tool_snd_vol_bcd_decode(&req_data[3]);
	vol_info.min_nb  = _diag_tool_snd_vol_bcd_decode(&req_data[6]);
	vol_info.max_wb  = _diag_tool_snd_vol_bcd_decode(&req_data[9]);
	vol_info.min_wb  = _diag_tool_snd_vol_bcd_decode(&req_data[12]);

	
	ret_val = Diag_Audio_vol_limit(&vol_info);

	if (req_data[1] == 0x00){					
		rsp_data[0]  = ret_val;
	} else {									
		
		rsp_data[0]  = req_data[0];
		rsp_data[1]  = vol_info.modeRW;
		rsp_data[2]  = vol_info.dev_idx;
		_diag_tool_snd_vol_bcd_encode(vol_info.max_nb, &rsp_data[3]);
		_diag_tool_snd_vol_bcd_encode(vol_info.min_nb, &rsp_data[6]);
		_diag_tool_snd_vol_bcd_encode(vol_info.max_wb, &rsp_data[9]);
		_diag_tool_snd_vol_bcd_encode(vol_info.min_wb, &rsp_data[12]);
		rsp_data[15] = ret_val;
	}
}


void Diag_Audio_qtr_cal_wrapper(char * req_data, char * rsp_data )
{
    Diag_Audio_qtr_cal_type cal_info;      
    unsigned char ret_val;                 

	printk(KERN_INFO "Diag_Audio_qtr_cal_wrapper\n");
	
	cal_info.modeRW     = req_data[1];				
	cal_info.act_idx    = req_data[2];				
	cal_info.in_act_idx = req_data[3];				
	cal_info.act_type   = 							
			((u32)req_data[4]) << 24 |
			((u32)req_data[5]) << 16 |
			((u32)req_data[6]) << 8  |
			req_data[7];

	cal_info.data       = 							/* ACTION DATA                      */
			((u32)req_data[8])<<24 |
			((u32)req_data[9])<<16 |
			((u32)req_data[10])<<8 |
				req_data[11];

	
	ret_val = Diag_Audio_qtr_cal(&cal_info);

	if (req_data[1] == 0x00){					
		rsp_data[0]  = ret_val;
	} else {									
		
		rsp_data[0]  = req_data[0];
		rsp_data[1]  = cal_info.modeRW;						
		rsp_data[2]  = cal_info.act_idx;					
		rsp_data[3]  = cal_info.in_act_idx;					
		rsp_data[4]  = (cal_info.act_type >> 24) & 0xFF;	
		rsp_data[5]  = (cal_info.act_type >> 16) & 0xFF;
		rsp_data[6]  = (cal_info.act_type >> 8)  & 0xFF;
		rsp_data[7]  = cal_info.act_type & 0xFF;
		rsp_data[8]  = (cal_info.data >> 24) & 0xFF;		/* ACTION DATA                      */
		rsp_data[9]  = (cal_info.data >> 16) & 0xFF;
		rsp_data[10] = (cal_info.data >> 8)  & 0xFF;
		rsp_data[11] = cal_info.data & 0xFF;
		rsp_data[12] = rsp_data[13] = rsp_data[14] = 0;
		rsp_data[15] = ret_val;
	}
}



void Diag_Audio_yda160_cal_wrapper(char * req_data, char * rsp_data )
{
    sndamp_volume_ctrl_type amp_vol_info;
    
    printk(KERN_INFO "Diag_Audio_yda160_cal_wrapper \n" );
    
    
    memset( &amp_vol_info, 0x00, sizeof(sndamp_volume_ctrl_type) );
    
    
    memcpy( amp_vol_info.data, req_data, SND_CALIB_PKG_LEN );
    
    
    amp_vol_info.data[SNDAMP_DIAG_REQ_POS_VOL_SUB_CMD] = SNDAMP_DIAG_VOL_SUB_CMD;
    
    
    sndamp_RW_SoundParam( &amp_vol_info );
    
    
    rsp_data[0]  = amp_vol_info.data[SNDAMP_DIAG_RES_POS_VOL_RES_STATUS];

}



static int _diag_calibration_read_file(const char *filename, unsigned  char *buf)
{
	struct file 	*filp;
	mm_segment_t	oldfs;
	int	read_out_sz = 0;

	printk( KERN_INFO "_diag_calibration_read_file :%s\n", filename );
	
	
	filp = filp_open( filename, O_RDONLY, 0 );

	
	if ( IS_ERR(filp) || (filp==NULL) ){
		printk(KERN_INFO "_diag_calibration_read_file:filp=%08x, read failed\n",(unsigned int)filp );
		return read_out_sz;
	}

	
	if ( (filp->f_op == NULL) || (filp->f_op->read==NULL) ){
		printk(KERN_INFO "_diag_calibration_read_file: read function not supported\n");
		return read_out_sz;
	}

	
	oldfs = get_fs();
	
	set_fs(KERNEL_DS);

	
	filp->f_pos = 0;
	
	read_out_sz = filp->f_op->read( filp, buf, SND_CALIB_DATA_LOAD_BUF, &filp->f_pos );

	
	set_fs( oldfs );

	
	fput( filp );

	printk( KERN_INFO "_diag_calibration_read_file :ret=%d\n", read_out_sz );
	return read_out_sz;
}


static int _diag_calibration_restore(unsigned  char *buf, int sz)
{
	int i, ret=0;
	char rsp_data[SND_CALIB_PKG_LEN];		/* dummy respone */

	for ( i = 0 ; i < sz ; i += SND_CALIB_PKG_LEN ) {
		
		if ( buf[i+1] != 0x00 ) continue;

		switch( buf[i] ) {
		case 0x01:	
			Diag_Audio_vol_limit_wrapper( buf+i, rsp_data );
			
			if ( rsp_data[0] == 0x00 ) ret ++;
			break;

		case 0x02:	
			Diag_Audio_qtr_cal_wrapper( buf+i, rsp_data );
			
			if ( rsp_data[0] == 0x00 ) ret ++;
			break;

		
		case 0x81:	
			Diag_Audio_yda160_cal_wrapper( buf+i, rsp_data );
			
			if ( rsp_data[0] == SNDAMP_DIAG_STATUS_NORMAL ) ret ++;
			break;
		

		default:
			printk( KERN_ERR "_diag_calibration_restore :unsupported type:%d\n", buf[i] );
			break;
		}
	}
	return ret;
}



static void _diag_calibration_data_load( void )
{
	unsigned char *buf;
	int read_out_sz = 0;
	int ret;

	
	buf = kmalloc( SND_CALIB_DATA_LOAD_BUF, GFP_KERNEL );

	if ( buf == NULL ) {
		printk( KERN_ERR " Diag_calibration_data_load: malloc buf failed\n" );
		return ;
	}

	
	read_out_sz = _diag_calibration_read_file( SND_CALIB_DATA_FILENAME, buf );

	
	if ( read_out_sz > 0 ) {
		ret = _diag_calibration_restore( buf, read_out_sz );
		printk( KERN_INFO "_diag_calibration_restore ret=%d\n", ret );
	}

	
	kfree( buf );

	return ;
}

