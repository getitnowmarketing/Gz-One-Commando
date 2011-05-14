/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#include <linux/delay.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <media/msm_camera.h>
#include <mach/gpio.h>
#include <mach/vreg.h>  /* <001> add */
#include <mach/camera.h>
#include "ius006f.h"
#include "camcmp_cmdsender.h"  /* <003> add */

/*=============================================================
// local option macro
==============================================================*/
#define RAWSNAPSHOT_ENABLE		1

/*=============================================================
    SENSOR REGISTER DEFINES
==============================================================*/
/* <013> add start */
#define IUS006F_CHECK_STATE_WAITMAX	800			/* <026>mod */
#define IUS006F_CHECK_STATE_WAIT	1			/* <036> mod */

#define IUS006F_AF_STATE_ERR		255
#define IUS006F_AF_LOCK_DONE		0x00000010	/* <022> add */

static struct timer_list timer;
/* <013> add end */

enum ius006f_fps_mode {
	FPS_MODE_MONI = 0,
	/* <012> mod start */
	FPS_MODE_CAPT,
	FPS_MODE_MOVIE,
	/* <012> mod end */

	FPS_MODE_NUM
};


/* b2.0 add start --> */
enum otp_area_1{
	OTP0 = 0,
	OTP1 = 1
};

enum otp_area_2{
	OTP_L = 0,
	OTP_M,
	OTP_H
};
/* --> b2.0 add end   */

#define IUS006F_WAIT_20					20
#define IUS006F_WAIT_50					50
#define IUS006F_POWER_WAIT				5		/* <001> add */
#define IUS006F_POWER_OFF_IOVD			15		/* <001> add */
#define IUS006F_POWER_ON_NCYC			5
#define IUS006F_POWER_ON_T1				5
#define IUS006F_POWER_ON_T2				200
#define IUS006F_POWER_ON_T3				15
#define IUS006F_POWER_ON_T4				6
#define IUS006F_POWER_OFF_T1			41

#define IUS006F_GPIO_CAM_CLK_EN			19
#define IUS006F_GPIO_CAM_STBY			25
#define IUS006F_GPIO_VDD_CAM_EN2		34
#define IUS006F_GPIO_VDD_CAM_EN3		35		/* <001> */

#define IUS006F_FPS_15					150
#define IUS006F_FPS_75					75
#define IUS006F_FRAME_DEF				67

/* b2.0 add start --> */
#define IUS006F_DIAG_SUCCESS            0
#define IUS006F_DIAG_ACT_ERR            1
#define IUS006F_DIAG_PARAM_ERR          2
#define IUS006F_DIAG_STATE_ERR          3
#define IUS006F_OTP_BIT_MIN             0
#define IUS006F_OTP_BIT_MAX             31

#define IUS006F_SENSOR_PAT_CHG_SLEEP	5
#define IUS006F_SENSOR_PAT_CHG_COUNT	300

#define IUS006F_GPIO_HIGH		1
#define IUS006F_GPIO_LOW		0
#define IUS006F_GPIO_4			4
#define IUS006F_GPIO_5			5
#define IUS006F_GPIO_6			6
#define IUS006F_GPIO_7			7
#define IUS006F_GPIO_8			8
#define IUS006F_GPIO_9			9
#define IUS006F_GPIO_10			10
#define IUS006F_GPIO_11			11
#define IUS006F_GPIO_CNT		8
/* --> b2.0 add end */

static int ius006f_set_af_mode(struct msm_sync *sync,struct af_mode_cfg *af_mode_chg);		/*<002>*//* <009> *//* <013> */
static int ius006f_set_default_focus(void);													/*<002>*//* <013> */
static int ius006f_af_start(struct msm_sync *sync);											/*<002>*//*<009>*//* <013> */
static int ius006f_get_af_pos(struct get_af_pos_cfg *af_pos);								/*<009>*//*<010>*/
static int ius006f_get_moni_gain(struct get_monigain_cfg *moni_gain);						/*<009>*//*<010>*/
static int ius006f_get_reg_ver(struct get_reg_ver_cfg *get_reg_ver);						/*<009>*//*<010>*/
static int ius006f_set_wb(int8_t wb);														/* <018> */
static int ius006f_set_jpeg_quality(int8_t jpeg_quality);									/* <019> */

int ius006f_sensor_release(void);															/* <035> */


struct ius006f_work {
	struct work_struct work;
};
static struct ius006f_work *ius006f_sensorw;
static struct i2c_client *ius006f_client;

struct ius006f_ctrl {
	const struct msm_camera_sensor_info *sensordata;

	int sensormode;
	int af_mode;
	uint32_t otp_rom_data[CAMCMP_OTP_ROM_NUM];
	int prev_res;									/* <012> */
	uint8_t af_cancel_onoff;						/* <013> add */
	uint16_t init_curr_lens_pos;					/* <013> add */
    int8_t night_mode;								/* <016> add */
};

static uint8_t ius006f_fps_list[] = { IUS006F_FPS_15, IUS006F_FPS_75, IUS006F_FPS_15};

static struct ius006f_ctrl *ius006f_ctrl;
static DECLARE_WAIT_QUEUE_HEAD(ius006f_wait_queue);
DEFINE_MUTEX(ius006f_mut);

static uint32_t af_start_after_cnt = 0;				/* <013> add */

static uint16_t ius006f_calc_msec_from_fps(uint8_t frame)
{
	uint32_t msec = 0;
	uint32_t calc = 0;
	uint16_t mode_fps = 0;
	uint8_t vcnt = 1;

	switch (ius006f_ctrl->sensormode) {
	case SENSOR_PREVIEW_MODE:
	case SENSOR_HALF_MODE:
		mode_fps = ius006f_fps_list[FPS_MODE_MONI];
		vcnt = 1;    /* <030> */
		break;
	case SENSOR_SNAPSHOT_MODE:
		mode_fps = ius006f_fps_list[FPS_MODE_CAPT];
		vcnt = 1;    /* <030> */
		break;
	case SENSOR_MOVIE_MODE:
		mode_fps = ius006f_fps_list[FPS_MODE_MOVIE];
		break;
	default:
		return 0;
	}

	if (ius006f_ctrl->night_mode == CAMERA_SCENE_MODE_NIGHT) {
		vcnt = 4;
	}

	/* <025> */
	msec  = CAMCMP_CALC_1000 * frame * CAMCMP_CALC_10;
	calc = msec;
	msec = (msec / mode_fps) * vcnt;
	if((calc % mode_fps) != 0) {
		msec++;
	}

	return msec;
}


static int32_t ius006f_change_mode_af(int mode, enum msm_sensor_onoff af_on)
{
	int32_t rc = 0;
	camcmp_cmd_info_type cmd_info[] = {{0, NULL, 0, NULL, 0}, {0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type intclr   = {0, NULL};
	camcmp_blank_info_type modechg  = {0, NULL};

	intclr.blank_id = CAMCMP_BID_CLR_MODE;
	if (af_on == SENSOR_ON) { /* <013> add */
		intclr.blank_id = CAMCMP_BID_CLR_MODE_AF;
	}
	cmd_info[0].cmd_id = CAMCMP_CID_INT_CLR;
	cmd_info[0].p_blank_info = &intclr;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(intclr);


	switch (mode) {
	case SENSOR_PREVIEW_MODE:
	case SENSOR_MOVIE_MODE:
		if ( (ius006f_ctrl->sensormode == SENSOR_PREVIEW_MODE) || 
		     (ius006f_ctrl->sensormode == SENSOR_MOVIE_MODE) ) {
			modechg.blank_id = CAMCMP_BID_MONI_REFRESH;
		} else {
			modechg.blank_id = CAMCMP_BID_MONI_MODE;
		}
		break;
	case SENSOR_HALF_MODE:
		modechg.blank_id = CAMCMP_BID_HALF_MODE;
		break;
	case SENSOR_SNAPSHOT_MODE:
		modechg.blank_id = CAMCMP_BID_CAP_MODE;
		break;
		
	default:
		CAMCMP_LOG_ERR("input failed! mode=%d\n", mode);
		return -EINVAL;
	}

	cmd_info[1].cmd_id = CAMCMP_CID_MODE_CHG;
	cmd_info[1].p_blank_info = &modechg;
	cmd_info[1].blank_num = CAMCMP_SIZE_BLKINF(modechg);

	CAMCMP_LOG_DBG("%d -> %d \n", ius006f_ctrl->sensormode, mode);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	ius006f_ctrl->sensormode = mode;

	return 0;
}


static int32_t ius006f_change_mode(int mode)
{
	return ius006f_change_mode_af(mode, SENSOR_OFF); /* <013> add */
}


static int32_t ius006f_video_config(int prev_mode, int res)     /* <039> mod */
{
	int32_t rc;
	int mode = SENSOR_PREVIEW_MODE;
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}, {0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type moni_size = {0, NULL};

	/* <039> mod-start */
	switch (res)
	{
		case SENSOR_CAMERA_VGA_SIZE:
			moni_size.blank_id = CAMCMP_BID_MONI_SIZE_VGA;
			break;
		case SENSOR_CAMERA_QH_SIZE:
			moni_size.blank_id = CAMCMP_BID_MONI_SIZE_QH;
			break;
		case SENSOR_CAMERA_QL_SIZE:
			moni_size.blank_id = CAMCMP_BID_MONI_SIZE_QL;
			break;
		default:
			CAMCMP_LOG_ERR("sensor previwe size failed res = %d\n", res);
			return -EINVAL;
	}

	if (prev_mode == SENSOR_PREVIEW_MODE) {
		CAMCMP_LOG_DBG("sensor previwe mode -PREVIEW-\n");
		/* <034> add start */
		cmd_info[1].cmd_id = CAMCMP_CID_NIGHT_OFF;
		if (ius006f_ctrl->night_mode == CAMERA_SCENE_MODE_NIGHT) {
			cmd_info[1].cmd_id = CAMCMP_CID_NIGHT_ON;
		}
		/* <034> add end */
		mode = SENSOR_PREVIEW_MODE;
	} else if (prev_mode == SENSOR_MOVIE_MODE) {
		CAMCMP_LOG_DBG("sensor previwe mode -MOVIE-\n");
		cmd_info[1].cmd_id = CAMCMP_CID_MOVIE;      /* <034> */
		mode = SENSOR_MOVIE_MODE;
	} else {
		CAMCMP_LOG_ERR("sensor previwe mode -failed-\n");
    return -EINVAL;
	}
	/* <039> mod-end */

	cmd_info[0].cmd_id = CAMCMP_CID_MONI_SIZE;
	cmd_info[0].p_blank_info = &moni_size;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(moni_size);

	CAMCMP_LOG_DBG("sensor mode %d->%d\n", ius006f_ctrl->sensormode, mode);
	CAMCMP_LOG_DBG("moni size %d->%d\n", ius006f_ctrl->prev_res, res);

	/* <012> */
	if ( ius006f_ctrl->sensormode != mode || ius006f_ctrl->prev_res != res ) {
		rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (rc < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
			return -EFAULT;
		}
		ius006f_ctrl->prev_res = res;		/* <012> */

		rc = ius006f_change_mode(mode);
		if (rc < 0) {
			CAMCMP_LOG_ERR("ius006f_change_mode failed. rc = %d\n", rc);
			return -EFAULT;
		}
	}

/* <005> add start */
	rc = ius006f_set_default_focus();
/* <005> add end */

	return rc;
}


/*<010> add-start */
static int32_t ius006f_snapshot_config(int32_t res)
{

	camcmp_cmd_info_type cmd_info[]  = { {0, NULL, 0, NULL, 0} }; /* <023><028><037> */
	camcmp_blank_info_type cap_size  = {0, NULL};
	/* <037> add-start */
	uint32_t               jpg_sts       = 0;
	uint32_t               jpg_retry     = 0;
	int32_t                cap_cnt       = 0;
	uint8_t                intsts_clear  = false;
	uint32_t               intsts        = 0;
	int32_t                check_cnt     = 0;
	/* <037> add-end */
	int32_t ret = 0;

	CAMCMP_LOG_DBG("start here !!!\n");

	switch (res)
	{
		case SENSOR_CAMERA_1MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_1M;
			break;
		case SENSOR_CAMERA_2MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_2M;
			break;
		case SENSOR_CAMERA_3MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_3M;
			break;
		case SENSOR_CAMERA_5MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_5M;
			break;
		default:
			return -EINVAL;
	}

	cmd_info[0].cmd_id = CAMCMP_CID_CAP_SIZE;
	cmd_info[0].p_blank_info = &cap_size;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(cap_size);

	ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (ret < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans error [ret:%d ]\n",ret);
		return ret;
	}

	CAMCMP_LOG_DBG("start mode change => capture\n");
	ret = ius006f_change_mode(SENSOR_SNAPSHOT_MODE);
	if (ret < 0) {
		CAMCMP_LOG_ERR("ius006f_change_mode error [ret:%d ]\n",ret);
		return ret;
	}

	/* <037> add-start */
	memset( cmd_info, 0, sizeof(cmd_info) );
	cmd_info[0].cmd_id         = CAMCMP_CID_GET_JPG_RETRY_CTL;
	cmd_info[0].p_recieve_data = &jpg_retry;
	cmd_info[0].recieve_num    = 1;
	ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (ret < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
		return ret;
	}

	if ( (jpg_retry&0x00000080) ) {
		jpg_retry     = jpg_retry & 0x0000007F;
	} else {
		jpg_retry = 0;
	}

	for (cap_cnt=0; cap_cnt < jpg_retry+1; cap_cnt++) {
		memset( cmd_info, 0, sizeof(cmd_info) );
		cmd_info[0].cmd_id       = CAMCMP_CID_JPEG_INTSTS;
		cmd_info[0].p_blank_info = NULL;
		cmd_info[0].blank_num    = 0;
		ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (ret < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
			return ret;
		}

		memset( cmd_info, 0, sizeof(cmd_info) );
		cmd_info[0].cmd_id       = CAMCMP_CID_GET_INTSTS;
		cmd_info[0].p_recieve_data = &intsts;
		cmd_info[0].recieve_num    = 1;
		intsts_clear = true;
		check_cnt    = 0;
		while(intsts_clear) {
			msleep(10);
			CAMCMP_LOG_DBG("CAMCMP_CID_GET_INTSTS Clear Check \n");
			ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
			if (ret < 0) {
				CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
				return ret;
			}
			if ((intsts & 0x00000004)==0){
				CAMCMP_LOG_DBG("CAMCMP_CID_GET_INTSTS Clear Check OK \n");
				intsts_clear = false;
			}
			check_cnt++;
			if ( check_cnt == 13 ){
				CAMCMP_LOG_DBG("CAMCMP_CID_GET_INTSTS Clear Check Over \n");
				intsts_clear = false;
			}
		}

		memset( cmd_info, 0, sizeof(cmd_info) );
		cmd_info[0].cmd_id       = CAMCMP_CID_GET_JPG_STS;
		cmd_info[0].p_recieve_data = &jpg_sts;
		cmd_info[0].recieve_num    = 1;

		ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (ret < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
			return -EFAULT;
		}

		CAMCMP_LOG_DBG("jpg_sts [%d] cap_cnt[%d] \n",jpg_sts, cap_cnt);
		if ( jpg_sts == 0x00000000 ) {
			CAMCMP_LOG_DBG("image start \n");
			break;
		} else if ( jpg_sts == 0x00000002 || jpg_sts == 0x00000003 ) {
			CAMCMP_LOG_DBG("retry \n");
			continue;
		} else {
			CAMCMP_LOG_ERR("jpeg sts err \n");
			return -EFAULT;
		}
	}
	if ( cap_cnt == jpg_retry+1 )	{
		CAMCMP_LOG_DBG("jpeg retry over image start \n");
	}
	/* <037> add-end */
	CAMCMP_LOG_DBG("succsess\n");
	return ret;


}
/* <010> add-end */

static int32_t ius006f_raw_snapshot_config(int res)
{
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0} }; /* <023><028><037> */
	camcmp_blank_info_type cap_size  = {0, NULL};
	/* <037> add-start */
	uint32_t               jpg_sts       = 0;
	uint32_t               jpg_retry     = 0;
	int32_t                cap_cnt       = 0;
	uint8_t                intsts_clear  = false;
	uint32_t               intsts        = 0;
	int32_t                check_cnt     = 0;
	/* <037> add-end */

	int32_t                ret = 0;              /* <038> */
	CAMCMP_LOG_DBG("start here !!!\n");

	switch (res)
	{
		case SENSOR_CAMERA_1MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_1M;
			break;
		case SENSOR_CAMERA_2MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_2M;
			break;
		case SENSOR_CAMERA_3MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_3M;
			break;
		case SENSOR_CAMERA_5MP_SIZE:
			cap_size.blank_id = CAMCMP_BID_CAP_SIZE_5M;
			break;
		default:
			return -EINVAL;
	}

	cmd_info[0].cmd_id = CAMCMP_CID_CAP_SIZE;
	cmd_info[0].p_blank_info = &cap_size;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(cap_size);

	ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (ret < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans fail [RC:%d]\n",ret);
		return -EFAULT;
	}

	CAMCMP_LOG_DBG("start mode change => capture\n");
	ret = ius006f_change_mode(SENSOR_SNAPSHOT_MODE);
	if (ret < 0) {
		CAMCMP_LOG_ERR("ius006f_change_mode fail [RC:%d]\n",ret);
		return ret;
	}

	/* <037> add-start */
	memset( cmd_info, 0, sizeof(cmd_info) );
	cmd_info[0].cmd_id         = CAMCMP_CID_GET_JPG_RETRY_CTL;
	cmd_info[0].p_recieve_data = &jpg_retry;
	cmd_info[0].recieve_num    = 1;
	ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (ret < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
		return ret;
	}

	if ( (jpg_retry&0x00000080) ) {
		jpg_retry     = jpg_retry & 0x0000007F;
	} else {
		jpg_retry = 0;
	}

	for (cap_cnt=0; cap_cnt < jpg_retry+1; cap_cnt++) {
		memset( cmd_info, 0, sizeof(cmd_info) );
		cmd_info[0].cmd_id       = CAMCMP_CID_JPEG_INTSTS;
		cmd_info[0].p_blank_info = NULL;
		cmd_info[0].blank_num    = 0;
		ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (ret < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
			return ret;
		}

		memset( cmd_info, 0, sizeof(cmd_info) );
		cmd_info[0].cmd_id       = CAMCMP_CID_GET_INTSTS;
		cmd_info[0].p_recieve_data = &intsts;
		cmd_info[0].recieve_num    = 1;
		intsts_clear = true;
		check_cnt    = 0;
		while(intsts_clear) {
			msleep(10);
			CAMCMP_LOG_DBG("CAMCMP_CID_GET_INTSTS Clear Check \n");
			ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
			if (ret < 0) {
				CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
				return ret;
			}
			if ((intsts & 0x00000004)==0){
				CAMCMP_LOG_DBG("CAMCMP_CID_GET_INTSTS Clear Check OK \n");
				intsts_clear = false;
			}
			check_cnt++;
			if ( check_cnt == 13 ){
				CAMCMP_LOG_DBG("CAMCMP_CID_GET_INTSTS Clear Check Over \n");
				intsts_clear = false;
			}
		}

		memset( cmd_info, 0, sizeof(cmd_info) );
		cmd_info[0].cmd_id       = CAMCMP_CID_GET_JPG_STS;
		cmd_info[0].p_recieve_data = &jpg_sts;
		cmd_info[0].recieve_num    = 1;
		ret = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (ret < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed [RC:%d]\n",ret);
			return -EFAULT;
		}

		CAMCMP_LOG_DBG("jpg_sts [%d] cap_cnt[%d] \n",jpg_sts, cap_cnt);
		if ( jpg_sts == 0x00000000 ) {
			CAMCMP_LOG_DBG("image start \n");
			break;
		} else if ( jpg_sts == 0x00000002 || jpg_sts == 0x00000003 ) {
			CAMCMP_LOG_DBG("retry \n");
			continue;
		} else {
			CAMCMP_LOG_ERR("jpeg sts err \n");
			return -EFAULT;
		}
	}
	if ( cap_cnt == jpg_retry+1 )	{
		CAMCMP_LOG_DBG("jpeg retry over image start \n");
	}
	/* <037> add-end */


	CAMCMP_LOG_DBG("succsess\n");
	return ret;
}

static int ius006f_set_mode(int mode, int res)   /*<014>*/
{
	int rc = 0;		/*<014>*/

	CAMCMP_LOG_DBG("start here !!!\n");

	switch (mode) {
/* <039> mod-start */
	case SENSOR_PREVIEW_MODE:
	case SENSOR_MOVIE_MODE:
		CAMCMP_LOG_DBG("SENSOR_MODE mode = %d\n", mode);
		rc = ius006f_video_config(mode, res);
		break;
/* <039> mod-end */

	case SENSOR_RAW_SNAPSHOT_MODE:
		CAMCMP_LOG_DBG("SENSOR_RAW_SNAPSHOT_MODE\n");
		rc = ius006f_raw_snapshot_config(res);
		break;

/* <010> add-start */
	case SENSOR_SNAPSHOT_MODE:
		rc = ius006f_snapshot_config(res);
		break;
/* <010> add-end */

	default:
		CAMCMP_LOG_ERR("param error. default\n");
		rc = -EINVAL;
		break;
	}

	return rc;
}


/*<002>start*/
static int ius006f_set_af_mode_off(struct af_mode_cfg *af_mode_chg) /* <007> add *//* <013> add */
{
	int rc = 0;  /* <013> add */
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type mf_pos = {0, NULL};
	camcmp_blank_info_type af_mode = {0, NULL}; /* <007> add */

	/* <007> add start */
	af_mode.blank_id = CAMCMP_BID_MODE_MF;
	cmd_info[0].cmd_id = CAMCMP_CID_AF_MODE;
	cmd_info[0].p_blank_info = &af_mode;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(af_mode);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}
	/* <007> add end */

	rc = ius006f_change_mode(SENSOR_HALF_MODE);
	if (rc < 0) {
		return rc;
	}

	switch (af_mode_chg->af_mode) /* <007> add */
	{
	case CAMERA_AF_MODE_NORMAL:
		mf_pos.blank_id = CAMCMP_BID_MF_1M;
		break;
	case CAMERA_AF_MODE_MACRO:
		mf_pos.blank_id = CAMCMP_BID_MF_MACRO;
		break;
	case CAMERA_AF_MODE_INFINITY:
		mf_pos.blank_id = CAMCMP_BID_MF_INF;
		break;
	default:
		mf_pos.blank_id = CAMCMP_BID_MF_INF;
		break;
	}

	cmd_info[0].cmd_id = CAMCMP_CID_MF_START;
	cmd_info[0].p_blank_info = &mf_pos;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(mf_pos);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	return rc;
}
/*<002>end*/

/*<002>start*/
static int ius006f_set_af_mode_on(struct msm_sync *sync) /* <013> add */
{
    int rc = 0;  /* <013> add */

	/* <007> add start */
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type af_mode = {0, NULL};

	af_mode.blank_id = CAMCMP_BID_MODE_SAF;
	cmd_info[0].cmd_id = CAMCMP_CID_AF_MODE;
	cmd_info[0].p_blank_info = &af_mode;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(af_mode);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}
	/* <007> add end */

    rc = ius006f_af_start(sync); /*<009>*//*<013>*/
    
    return rc;
}
/*<002> end*/


static int ius006f_set_af_mode(struct msm_sync *sync, struct af_mode_cfg *af_mode_chg) /* <007> */ /* <013> add */
{
	int rc = 0; /* <013> add */

/*<002>start*/
	if (ius006f_ctrl->sensormode == SENSOR_HALF_MODE)
	{
		rc = ius006f_set_default_focus();
		/* <007> add start */
		if (rc < 0) {
			CAMCMP_LOG_ERR("ius006f_set_default_focus failed. rc = %d\n", rc);
			return -EFAULT;
		}
		/* <007> add end */
	}
	else
	{
		// nop.
	}
/*<002>end*/

	if ((af_mode_chg->af_ends) == SENSOR_OFF) {	/*<013>*/
		rc = ius006f_set_af_mode_off(af_mode_chg);  /*<002>*/
	}
	else if ((af_mode_chg->af_ends) == SENSOR_ON) {	/*<013>*/
		rc = ius006f_set_af_mode_on(sync);  /*<009>*//*<013>*/
	}
	else {
		rc = -EINVAL;
	}
	return rc;
}

/* b2.0 add start --> */
static int ius006f_get_otp(struct get_otp_cfg *get_otp)   /*<014>*/
{
	int  rc  = 0;		/*<014>*/
	int8_t   idx = 0;
	int8_t   i   = 0;
	uint32_t read_otp_data = 0;
	uint32_t read_mask = 0x1;
	uint32_t get_otp_data = 0;

	if(get_otp->rom_area_1 > OTP1){

		get_otp->result = IUS006F_DIAG_PARAM_ERR;

		CAMCMP_LOG_ERR("error root %d\n", __LINE__);
		return -EINVAL;
	}
	if(get_otp->rom_area_2 > OTP_H){

		get_otp->result = IUS006F_DIAG_PARAM_ERR;

		CAMCMP_LOG_ERR("error root %d\n", __LINE__);
		return -EINVAL;
	}
	if(get_otp->start_bit > (IUS006F_OTP_BIT_MAX - 1)){

		get_otp->result = IUS006F_DIAG_PARAM_ERR;

		CAMCMP_LOG_ERR("error root %d\n", __LINE__);
		return -EINVAL;
	}
	if(get_otp->end_bit > IUS006F_OTP_BIT_MAX || 
	   get_otp->end_bit < (IUS006F_OTP_BIT_MIN + 1)){

		get_otp->result = IUS006F_DIAG_PARAM_ERR;

		CAMCMP_LOG_ERR("error root %d\n", __LINE__);
		return -EINVAL;
	}

	idx = (get_otp->rom_area_1 * 3) + get_otp->rom_area_2;
	read_otp_data = ius006f_ctrl->otp_rom_data[idx];

	for(i = 0; i <= IUS006F_OTP_BIT_MAX; i++){
		if(i >= get_otp->start_bit && i <= get_otp->end_bit){
			get_otp_data |= read_otp_data & read_mask;
		}
		read_mask = read_mask << 1;
	}

	get_otp_data = get_otp_data >> get_otp->start_bit;

	get_otp->data_0 = (get_otp_data & 0xff000000) >> 24;
	get_otp->data_1 = (get_otp_data & 0x00ff0000) >> 16;
	get_otp->data_2 = (get_otp_data & 0x0000ff00) >> 8;
	get_otp->data_3 = (get_otp_data & 0x000000ff);

	return rc;
}

static int ius006f_sensor_pat_chg(struct sensor_pat_chg *pat_chg)  /*<014>*/
{
	int      rc            = 0;    /*<014>*/
	int32_t  i             = 0;
	int32_t  j             = 0;
	uint8_t  port_high_chk = 0x00;
	uint8_t  port_low_chk  = 0x00;
	uint8_t  port_chk      = 0x00;
	uint8_t  mask          = 0x01;
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};

	CAMCMP_LOG_DBG("start\n");

	cmd_info[0].cmd_id = CAMCMP_CID_PTNCHK_ON;
	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}

	for(i = 0; i < IUS006F_SENSOR_PAT_CHG_COUNT; i++){
		msleep(IUS006F_SENSOR_PAT_CHG_SLEEP);

		for(j = IUS006F_GPIO_4, mask = 0x01; j <= IUS006F_GPIO_11; j++, mask = mask << 1){
			if(gpio_get_value(j)){
				port_high_chk |= mask;
			}
			else{
				port_low_chk |= mask;
			}
		}

		port_chk = (port_high_chk & port_low_chk);

		if(port_chk == 0xff){
			break;
		}
	}

	if(port_chk != 0xff){
		for(i = 0, mask = 0x01; i < IUS006F_GPIO_CNT; i++, mask = mask << 1){
			if((port_chk & mask) == 0){
				pat_chg->port_no[pat_chg->err_num] = i+IUS006F_GPIO_4;
				pat_chg->err_num++;
			}
		}
	}

	cmd_info[0].cmd_id = CAMCMP_CID_PTNCHK_OFF;
	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		return -EFAULT;
	}

	CAMCMP_LOG_DBG("end\n");

	return rc;
}
/* --> b2.0 add end */

/*<009> start*/
/*014 start*/
static int ius006f_get_af_pos(struct get_af_pos_cfg *af_pos)
{
	int rc = 0;
	uint32_t af_evaluate = 0;
	uint32_t af_result = 0;        /* <017> */
	uint32_t af_lens_pos = 0;      /* <017> */
	uint32_t af_adjust_step = 0;   /* <017> */
	camcmp_cmd_info_type cmd_info[] = {{0, NULL, 0, NULL, 0}, {0, NULL, 0, NULL, 0}};

	cmd_info[0].cmd_id = CAMCMP_CID_GET_EVALUATE;
	cmd_info[0].p_recieve_data = &af_evaluate;
	cmd_info[0].recieve_num = 1;/* <013> */

	cmd_info[1].cmd_id = CAMCMP_CID_GET_AF_RESULT;
	cmd_info[1].p_recieve_data = &af_result;
	cmd_info[1].recieve_num = 1;

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}
	CAMCMP_LOG_DBG("af_evaluate=%x, \n", af_evaluate);
	CAMCMP_LOG_DBG("af_result=%x, \n", af_result);

	af_pos->af_evaluate_0 = (af_evaluate & 0xff000000) >> 24;
	af_pos->af_evaluate_1 = (af_evaluate & 0x00ff0000) >> 16;
	af_pos->af_evaluate_2 = (af_evaluate & 0x0000ff00) >> 8;
	af_pos->af_evaluate_3 = (af_evaluate & 0x000000ff);

	/* <013> add start */
	if (af_result == AF_RESULT_OK) {
		af_pos->af_result = AF_SUCCESS;
	} else {
		af_pos->af_result = AF_FAILED;
	}
	/* <013> add end */

	cmd_info[0].cmd_id = CAMCMP_CID_GET_LENSPOS;
	cmd_info[0].p_recieve_data = &af_lens_pos;
	cmd_info[0].recieve_num = 1; /* <013> */
	cmd_info[1].cmd_id = CAMCMP_CID_GET_AF_STEP;
	cmd_info[1].p_recieve_data = &af_adjust_step;
	cmd_info[1].recieve_num = 1; /* <013> */

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}
	CAMCMP_LOG_DBG("af_lens_pos=%x, \n", af_lens_pos);
	CAMCMP_LOG_DBG("af_adjust_step=%x, \n", af_adjust_step);

	af_pos->af_lens_pos_0 = (af_lens_pos & 0x0000ff00) >> 8;
	af_pos->af_lens_pos_1 = (af_lens_pos & 0x000000ff);
	af_pos->af_adjust_step_0 = (af_adjust_step & 0x0000ff00) >> 8;
	af_pos->af_adjust_step_1 = (af_adjust_step & 0x000000ff);

	return rc;
}


static int ius006f_get_moni_gain(struct get_monigain_cfg *moni_gain)
{
	int rc = 0;
	uint32_t shut_speed0 = 0;    /* <017> */
	uint32_t shut_speed1 = 0;    /* <017> */
	uint32_t aescl = 0 ;         /* <017> */
	uint32_t agc_scl = 0 ;       /* <017> */
	camcmp_cmd_info_type cmd_info1[] = {{0, NULL, 0, NULL, 0}};
	camcmp_cmd_info_type cmd_info2[] = {{0, NULL, 0, NULL, 0}, {0, NULL, 0, NULL, 0}};

	cmd_info1[0].cmd_id = CAMCMP_CID_GET_SHT_TIME_H;
	cmd_info1[0].p_recieve_data = &shut_speed0;
	cmd_info1[0].recieve_num = 1; /* <013> */

	rc = camcmp_isp_trans(cmd_info1, CAMCMP_SIZE_CMD(cmd_info1));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}
	moni_gain->shutter_speed_0 = (shut_speed0 & 0x0000ff00) >> 8;
	moni_gain->shutter_speed_1 = (shut_speed0 & 0x000000ff);
	CAMCMP_LOG_DBG("shut_speed0=%x\n", shut_speed0);

	cmd_info1[0].cmd_id = CAMCMP_CID_GET_SHT_TIME_L;
	cmd_info1[0].p_recieve_data = &shut_speed1;
	cmd_info1[0].recieve_num = 1; /* <013> */

	rc = camcmp_isp_trans(cmd_info1, CAMCMP_SIZE_CMD(cmd_info1));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}
	moni_gain->shutter_speed_2 = (shut_speed1 & 0x0000ff00) >> 8;
	moni_gain->shutter_speed_3 = (shut_speed1 & 0x000000ff);
	CAMCMP_LOG_DBG("shut_speed1=%x \n", shut_speed1);


	cmd_info2[0].cmd_id = CAMCMP_CID_GET_AGC_SCL;
	cmd_info2[0].p_recieve_data = &agc_scl;
	cmd_info2[0].recieve_num = 1; /* <013> */

	cmd_info2[1].cmd_id = CAMCMP_CID_GET_AESCL;
	cmd_info2[1].p_recieve_data = &aescl;
	cmd_info2[1].recieve_num = 1; /* <013> */

	rc = camcmp_isp_trans(cmd_info2, CAMCMP_SIZE_CMD(cmd_info2));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}

	moni_gain->AGC_gain_0 = (agc_scl & 0x0000ff00) >> 8;
	moni_gain->AGC_gain_1 = (agc_scl & 0x000000ff);

	moni_gain->aescl_0 = (aescl & 0x0000ff00) >> 8;
	moni_gain->aescl_1 = (aescl & 0x000000ff);

	CAMCMP_LOG_DBG("AGC_gain_0=%x \n", agc_scl);
	CAMCMP_LOG_DBG("aescl=%x \n", aescl);

	return rc;

}
/*014 end*/

static int ius006f_get_reg_ver(struct get_reg_ver_cfg *get_reg_ver)     /*<014>*/
{
	int			rc = 0;       /*<014>*/
	uint8_t		sd_type=0;    /*<014>*/
	uint16_t	reg_ver=0;    /*<014>*/

	camcmp_makecmd_get_version(&reg_ver, &sd_type);

	get_reg_ver->reg_ver_0 = (reg_ver&0xFF00)>>8;
	get_reg_ver->reg_ver_1 = reg_ver&0xFF;
	get_reg_ver->sd_type = sd_type;
	return rc;
}
/*<009> end*/
/*<027>add start*/
static int ius006f_set_manual_step(struct set_manual_step_cfg *set_manual_step)
{
	int      rc = 0;
	uint32_t af_evaluate = 0;
	int32_t  manual_step_val=0;
	int32_t  af_lens_pos = 0;
	int32_t  serch_area_max=0;
	int32_t  serch_area_min=0;
	camcmp_cmd_info_type cmd_info1[] = {{0, NULL, 0, NULL, 0}};
	camcmp_cmd_info_type cmd_info2[] = {{0, NULL, 0, NULL, 0}, {0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type mf_pos = {0, NULL};
	camcmp_blank_info_type af_mode = {0, NULL};

	cmd_info2[0].cmd_id = CAMCMP_CID_GET_AREA_LOW_TYPE1;
	cmd_info2[0].p_recieve_data = &serch_area_min;
	cmd_info2[0].recieve_num = 1;
	cmd_info2[1].cmd_id = CAMCMP_CID_GET_AREA_HIGH_TYPE1;
	cmd_info2[1].p_recieve_data = &serch_area_max;
	cmd_info2[1].recieve_num = 1;
	rc = camcmp_isp_trans(cmd_info2, CAMCMP_SIZE_CMD(cmd_info2));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}

	cmd_info2[0].cmd_id = CAMCMP_CID_GET_MANUAL_STEP;
	cmd_info2[0].p_recieve_data = &manual_step_val;
	cmd_info2[0].recieve_num = 1;
	cmd_info2[1].cmd_id = CAMCMP_CID_GET_LENSPOS;
	cmd_info2[1].p_recieve_data = &af_lens_pos;
	cmd_info2[1].recieve_num = 1;
	rc = camcmp_isp_trans(cmd_info2, CAMCMP_SIZE_CMD(cmd_info2));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EINVAL;
	}

	if(set_manual_step->direct){
		af_lens_pos -= (set_manual_step->step*manual_step_val);
	}
	else{
		af_lens_pos += (set_manual_step->step*manual_step_val);
	}
	
	if ((af_lens_pos<serch_area_min)||
	    (af_lens_pos>serch_area_max)){
		set_manual_step->step_check = 0;
		CAMCMP_LOG_ERR("af_lens_pos invalid. af_lens_pos=%d.\n", af_lens_pos);
		return rc;
	}
	
	rc = ius006f_change_mode(SENSOR_PREVIEW_MODE);
	if (rc < 0) {
		CAMCMP_LOG_ERR("ius006f_change_mode failed. rc = %d\n", rc);
		return -EFAULT;
	}

	af_mode.blank_id = CAMCMP_BID_MODE_MF;
	cmd_info1[0].cmd_id = CAMCMP_CID_AF_MODE;
	cmd_info1[0].p_blank_info = &af_mode;
	cmd_info1[0].blank_num = CAMCMP_SIZE_BLKINF(af_mode);

	rc = camcmp_isp_trans(cmd_info1, CAMCMP_SIZE_CMD(cmd_info1));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	rc = ius006f_change_mode(SENSOR_HALF_MODE);
	if (rc < 0) {
		return rc;
	}

	mf_pos.blank_id = CAMCMP_BID_MF_CUSTOM;
	mf_pos.p_blank_data = &af_lens_pos;
	cmd_info1[0].cmd_id = CAMCMP_CID_MF_START;
	cmd_info1[0].p_blank_info = &mf_pos;
	cmd_info1[0].blank_num = CAMCMP_SIZE_BLKINF(mf_pos);

	rc = camcmp_isp_trans(cmd_info1, CAMCMP_SIZE_CMD(cmd_info1));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	cmd_info2[0].cmd_id = CAMCMP_CID_GET_EVALUATE;
	cmd_info2[0].p_recieve_data = &af_evaluate;
	cmd_info2[0].recieve_num = 1;
	cmd_info2[1].cmd_id = CAMCMP_CID_GET_LENSPOS;
	cmd_info2[1].p_recieve_data = &af_lens_pos;
	cmd_info2[1].recieve_num = 1;

	rc = camcmp_isp_trans(cmd_info2, CAMCMP_SIZE_CMD(cmd_info2));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}

	set_manual_step->step_check = 1;
	set_manual_step->af_evaluate_0 = (af_evaluate & 0xff000000) >> 24;
	set_manual_step->af_evaluate_1 = (af_evaluate & 0x00ff0000) >> 16;
	set_manual_step->af_evaluate_2 = (af_evaluate & 0x0000ff00) >> 8;
	set_manual_step->af_evaluate_3 = (af_evaluate & 0x000000ff);

	set_manual_step->af_lens_pos_0 = (af_lens_pos & 0x0000ff00) >> 8;
	set_manual_step->af_lens_pos_1 = (af_lens_pos & 0x000000ff);

	return rc;
}
/*<027>add end*/

/*<002>start*/
static int ius006f_set_default_focus(void) /* <013> add */
{
	/* <013> add start */
	int rc = 0;
	uint32_t af_lens_pos = 0;    /* <017> */
	/* <013> add end */

	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type mf_pos = {0, NULL};

	CAMCMP_LOG_DBG("start\n"); /* <013> add */

	/* <005> add start */
	if ( (ius006f_ctrl->sensormode != SENSOR_PREVIEW_MODE) && 
	     (ius006f_ctrl->sensormode != SENSOR_MOVIE_MODE) ) {
		rc = ius006f_change_mode(SENSOR_PREVIEW_MODE);
		if (rc < 0) {
			CAMCMP_LOG_ERR("ius006f_change_mode failed. rc = %d\n", rc);
			return rc;
		}
	}
	/* <005> add end */

	/* <013> add start */
	cmd_info[0].cmd_id = CAMCMP_CID_GET_LENSPOS;
	cmd_info[0].p_recieve_data = &af_lens_pos;
	cmd_info[0].recieve_num = 1;

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	if ((af_lens_pos == 0) ||
	    (ius006f_ctrl->init_curr_lens_pos != af_lens_pos)){
		mf_pos.blank_id = CAMCMP_BID_MF_INF; /* <005> add */
		cmd_info[0].cmd_id = CAMCMP_CID_MF_START;
		cmd_info[0].p_blank_info = &mf_pos;
		cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(mf_pos);
		cmd_info[0].p_recieve_data = NULL;
		cmd_info[0].recieve_num = 0;

		rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (rc < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
			return -EFAULT;
		}

		/*<022> add start */
		if(ius006f_ctrl->init_curr_lens_pos == 0){
			cmd_info[0].cmd_id = CAMCMP_CID_GET_LENSPOS;
			cmd_info[0].p_recieve_data = &af_lens_pos;
			cmd_info[0].recieve_num = 1;
			cmd_info[0].p_blank_info = NULL;
			cmd_info[0].blank_num = 0;

			rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
			if (rc < 0) {
				CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
				return -EFAULT;
			}
			ius006f_ctrl->init_curr_lens_pos = (uint16_t)af_lens_pos;    /* <017> */
		}
		/*<022> add end */
	}

	CAMCMP_LOG_DBG("end\n");
	/* <013> add end */

	return rc;
}

static int ius006f_af_start(struct msm_sync *sync) /*<009>*//*<013>*/
{
/* <013> add start */
	int rc = 0;
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}}; /* <031> add start */

	CAMCMP_LOG_DBG("start\n");

	ius006f_ctrl->af_cancel_onoff = SENSOR_OFF;

	/* <031> add start */
	if(ius006f_ctrl->sensormode == SENSOR_HALF_MODE){
		cmd_info[0].cmd_id = CAMCMP_CID_AF_RESTART_F;

		rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (rc < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
			return rc;
		}
	} else {
		rc = ius006f_change_mode_af(SENSOR_HALF_MODE, SENSOR_ON);
		if (rc < 0) {
			CAMCMP_LOG_ERR("ius006f_change_mode_af failed. rc = %d\n", rc);
			return rc;
		}
	}
	/* <031> add end */

	init_timer(&timer);
	timer.expires = jiffies + IUS006F_CHECK_STATE_WAIT;
	timer.data=(unsigned long)sync;
	timer.function = msm_af_start_poll;
	add_timer(&timer);

	CAMCMP_LOG_DBG("end rc = %d\n",rc);
/* <013> add end */

	return rc;
}
/*<002>end*/

static int ius006f_af_start_after(struct msm_sync *sync)
{

	int rc = 0;
	uint32_t af_result = 0;				/* <017> */
	uint32_t af_state  = 0;				/* <017> */
	uint32_t af_lock   = 0;				/* <022> add */
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0},{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type intclr   = {0, NULL};

	CAMCMP_LOG_DBG("start\n");

	if ((ius006f_ctrl->af_cancel_onoff) == SENSOR_ON) {
		CAMCMP_LOG_ERR("cancel. cancel = %d\n", ius006f_ctrl->af_cancel_onoff);
		af_start_after_cnt = 0;
		return AF_RESULT_NG;
	}

	af_start_after_cnt++;
	if (af_start_after_cnt > IUS006F_CHECK_STATE_WAITMAX){
		CAMCMP_LOG_ERR("cut over. cnt = %d\n", af_start_after_cnt);
		af_start_after_cnt = 0;
		return -EFAULT;
	}

	cmd_info[0].cmd_id = CAMCMP_CID_GET_AF_STATE;
	cmd_info[0].p_recieve_data = &af_state;
	cmd_info[0].recieve_num = 1;

	/* <022> add start */
	cmd_info[1].cmd_id = CAMCMP_CID_GET_INTSTS;
	cmd_info[1].p_recieve_data = &af_lock;
	cmd_info[1].recieve_num = 1;
	/* <022> add end */

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		af_start_after_cnt = 0;
		return -EFAULT;
	}

	if(af_state == IUS006F_AF_STATE_ERR){
		CAMCMP_LOG_ERR("AF_STATE_ERR\n");
		af_start_after_cnt = 0;
		return -EFAULT; /* <033> mod */
	}

	if((af_lock & IUS006F_AF_LOCK_DONE ) > 0){ /* <022> add  */
		CAMCMP_LOG_DBG("AF_LOCK_DONE\n");

		af_start_after_cnt = 0;

		intclr.blank_id = CAMCMP_BID_CLR_AF_LOCK;
		cmd_info[0].cmd_id = CAMCMP_CID_INT_CLR;
		cmd_info[0].p_blank_info = &intclr;
		cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(intclr);
		cmd_info[0].p_recieve_data = NULL;
		cmd_info[0].recieve_num = 0;

		cmd_info[1].cmd_id = CAMCMP_CID_GET_AF_RESULT;
		cmd_info[1].p_recieve_data = &af_result;
		cmd_info[1].recieve_num = 1;

		rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (rc < 0) {
			CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
			return -EFAULT;
		}

		CAMCMP_LOG_DBG("af_result = %d\n", af_result);

		if (af_result == AF_RESULT_OK) {
			rc = AF_RESULT_OK;
		} else if (af_result == AF_RESULT_NG) {
			/* <032> add start */
			rc = ius006f_change_mode(SENSOR_PREVIEW_MODE);
			if (rc < 0) {
				CAMCMP_LOG_ERR("AF_RESULT_NG Change mode failed. rc = %d\n", rc);
				return -EFAULT;
			}
			/* <032> add end */
			rc = AF_RESULT_NG;
		} else {
			rc = -EFAULT;
			CAMCMP_LOG_ERR("af_result failed. rc = %d\n", rc);
		}
	}else{
		CAMCMP_LOG_DBG("AF_CHK_POOLING\n");

		timer.expires = jiffies + IUS006F_CHECK_STATE_WAIT;
		timer.data=(unsigned long)sync;
		timer.function = msm_af_start_poll;
		add_timer(&timer);
		rc = AF_RESULT_DURING;
	}

	CAMCMP_LOG_DBG("end rc = %d\n",rc);
	return rc;
}

static int ius006f_af_cancel(void)
{
	int rc = 0;

	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};

	CAMCMP_LOG_DBG("start\n");

	ius006f_ctrl->af_cancel_onoff = SENSOR_ON; /* <038> mod */

	cmd_info[0].cmd_id = CAMCMP_CID_AF_CANCEL;

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return rc;
	}

	rc = ius006f_set_default_focus();

	CAMCMP_LOG_DBG("end rc = %d\n",rc);

	return rc;
}
/* <013> add end */

static int ius006f_set_brightness(int8_t brightness)
{
	int rc = 0;
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type set_brightness = {0, NULL};

	CAMCMP_LOG_DBG("brightness=%x \n", brightness);
	
	if ((brightness < CAMCMP_BID_BRIGHT_M5) || (brightness > CAMCMP_BID_BRIGHT_P5))
	{
		CAMCMP_LOG_ERR("param error. brightness=%d\n", brightness);
		return -EINVAL;
	}

	set_brightness.blank_id = (camcmp_blank_bright_enum)brightness;
	cmd_info[0].cmd_id = CAMCMP_CID_BRIGHTNESS;
	cmd_info[0].p_blank_info = &set_brightness;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(set_brightness);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}

	return rc;
}

static int ius006f_set_effect(int8_t effect)
{
	int rc = 0;
	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type set_effect = {0, NULL};

	CAMCMP_LOG_DBG("effect=%x \n", effect);

	switch (effect)
	{
		case CAMERA_EFFECT_OFF:
			set_effect.blank_id = CAMCMP_BID_EFFECT_OFF;
			break;
		case CAMERA_EFFECT_MONO:
			set_effect.blank_id = CAMCMP_BID_EFFECT_MONO;
			break;
		case CAMERA_EFFECT_NEGATIVE:
			set_effect.blank_id = CAMCMP_BID_EFFECT_NEGA;
			break;
		case CAMERA_EFFECT_SEPIA:
			set_effect.blank_id = CAMCMP_BID_EFFECT_SEPIA;
			break;
		default:
			CAMCMP_LOG_DBG("param error. default\n");
			return -EINVAL;
	}

	cmd_info[0].cmd_id = CAMCMP_CID_EFFECT;
	cmd_info[0].p_blank_info = &set_effect;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(set_effect);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("error root:%d\n", __LINE__);
		return -EFAULT;
	}

	return rc;
}
/*<014>end*/

/* <016> start */
static int32_t ius006f_set_zoom(int32_t zoom, int size)
{
	int32_t rc = 0;

	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type zoom_blank = {0, NULL};

	CAMCMP_LOG_DBG("start\n");
	CAMCMP_LOG_DBG("zoom = %d, size = %d \n", zoom, size);

	switch (size)
	{
		case SENSOR_CAMERA_5MP_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_5M;
			break;
		case SENSOR_CAMERA_3MP_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_3M;
			break;
		case SENSOR_CAMERA_2MP_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_2M;
			break;
		case SENSOR_CAMERA_1MP_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_1M;
			break;
		case SENSOR_CAMERA_QH_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_QH;
			break;
		case SENSOR_CAMERA_QL_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_QL;
			break;
		/* <039> start */
		case SENSOR_CAMERA_VGA_SIZE:
			cmd_info[0].cmd_id = CAMCMP_CID_ZOOM_QH;
			break;
		/* <039> end */
		default:
			CAMCMP_LOG_ERR("size failed.\n");
			return -EINVAL;
	}

    /* <021> */
	if ((cmd_info[0].cmd_id == CAMCMP_CID_ZOOM_5M) ||
		(cmd_info[0].cmd_id == CAMCMP_CID_ZOOM_QH) ||
		(cmd_info[0].cmd_id == CAMCMP_CID_ZOOM_QL))
	{
		if (zoom > 0) {
			CAMCMP_LOG_ERR("zoom param err\n");
			return -EINVAL;
		}
	}

	zoom_blank.blank_id = (camcmp_blank_zoom_enum)zoom;

	cmd_info[0].p_blank_info = &zoom_blank;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(zoom_blank);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	CAMCMP_LOG_DBG("end rc = %d\n", rc);

	return rc;
}

static int32_t ius006f_set_picture_mode(int8_t pict_mode)
{
	int32_t rc = 0;

	camcmp_cmd_info_type cmd_info[]  = {{0, NULL, 0, NULL, 0}};

	CAMCMP_LOG_DBG("start here !!!\n");

	switch (pict_mode)
	{
		case CAMERA_SCENE_MODE_AUTO:
			ius006f_ctrl->night_mode = CAMERA_SCENE_MODE_AUTO;
			cmd_info[0].cmd_id = CAMCMP_CID_NIGHT_OFF;
			break;
		case CAMERA_SCENE_MODE_NIGHT:
			ius006f_ctrl->night_mode = CAMERA_SCENE_MODE_NIGHT;
			cmd_info[0].cmd_id = CAMCMP_CID_NIGHT_ON;
			break;
		default:
			CAMCMP_LOG_ERR("pict_mode failed.\n");
			return -EINVAL;
	}

	/* <034> add start */
	if ( ius006f_ctrl->sensormode == SENSOR_MOVIE_MODE ) {
		cmd_info[0].cmd_id = CAMCMP_CID_MOVIE;
	}
	/* <034> add end */

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed. rc = %d\n", rc);
		return -EFAULT;
	}

	CAMCMP_LOG_DBG("end rc = %d\n", rc);

	return rc;
}
/* <016> end */

/* <018> add start */
static int ius006f_set_wb(int8_t wb)
{
	int32_t rc = -EFAULT;
	camcmp_cmd_info_type cmd_info[] = {{0, NULL, 0, NULL, 0}};

	switch (wb) {
		case MSM_CAM_WB_AUTO:          /* <024> modify */
			cmd_info[0].cmd_id = CAMCMP_CID_WB_AUTO;
			break;

		case MSM_CAM_WB_INCANDESCENT:  /* <024> modify */
			cmd_info[0].cmd_id = CAMCMP_CID_WB_INCAND;
			break;

		case MSM_CAM_WB_FLUORESCENT:   /* <024> modify */
			cmd_info[0].cmd_id = CAMCMP_CID_WB_FLUORE;
			break;

		case MSM_CAM_WB_DAYLIGHT:      /* <024> modify */
			cmd_info[0].cmd_id = CAMCMP_CID_WB_DAYLIGHT;
			break;

		case MSM_CAM_WB_CLOUDY:        /* <024> modify */
			cmd_info[0].cmd_id = CAMCMP_CID_WB_CLOUDY;
			break;

		case MSM_CAM_WB_LED:           /* <024> modify */
			cmd_info[0].cmd_id = CAMCMP_CID_WB_LED;
			break;

		default:
			CAMCMP_LOG_ERR("%s: invalid parameter (%d)\n", __func__, wb);
			return -EINVAL;
	}

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));

	if (rc != 0) {
		CAMCMP_LOG_ERR("%s: camcmp_isp_trans failed (%d)\n", __func__, rc);
		return -EFAULT;  /* <024> add */
	}

	return 0;  /* <024> modify */
}
/* <018> add end */

/* <019> add start */
static int ius006f_set_jpeg_quality(int8_t jpeg_quality)
{
	int32_t rc = -EFAULT;
	camcmp_cmd_info_type cmd_info[] = {{0, NULL, 0, NULL, 0}};
	camcmp_blank_info_type quality  = {0, NULL};     /* <020> add */

	switch (jpeg_quality) {
		case SENSOR_JPEG_Q_SUPER_FINE:
			quality.blank_id = CAMCMP_BID_Q_SFINE;   /* <020> modify */
			break;

		case SENSOR_JPEG_Q_FINE:
			quality.blank_id = CAMCMP_BID_Q_FINE;    /* <020> modify */
			break;

		case SENSOR_JPEG_Q_STANDARD:
			quality.blank_id = CAMCMP_BID_Q_NORMAL;  /* <020> modify */
			break;

		default:
			CAMCMP_LOG_ERR("%s: invalid parameter (%d)\n", __func__, jpeg_quality);
			return -EINVAL;
	}

/* <020> add start */
	cmd_info[0].cmd_id       = CAMCMP_CID_QUALITY;
	cmd_info[0].p_blank_info = &quality;
	cmd_info[0].blank_num    = CAMCMP_SIZE_BLKINF(quality);
/* <020> add end */

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));

	if (rc != 0) {
		CAMCMP_LOG_ERR("%s: camcmp_isp_trans failed (%d)\n", __func__, rc);
		return -EFAULT;  /* <024> add */
	}
/* <029> add-start */
	rc = ius006f_change_mode(SENSOR_PREVIEW_MODE);
	if (rc != 0) {
		CAMCMP_LOG_ERR("ius006f_change_mode failed (%d)\n", rc); /* <029> */
		return -EFAULT;
	}
/* <029> add-end */
	return 0;  /* <024> modify */
}
/* <019> add end */
/* <028> add start */
static int ius006f_set_pict_size( int pict_size ) /*<029>*/
{
	int32_t rc = -EFAULT;
	camcmp_cmd_info_type cmd_info[] = { {0, NULL, 0, NULL, 0} };
	camcmp_blank_info_type jpeg_limit = {0, NULL};

	CAMCMP_LOG_DBG("start\n"); /* <029> */

	switch (pict_size)
	{
		case SENSOR_CAMERA_1MP_SIZE:
			jpeg_limit.blank_id = CAMCMP_BID_JPEG_LIMIT_SIZE_1M;
			break;
		case SENSOR_CAMERA_2MP_SIZE:
			jpeg_limit.blank_id = CAMCMP_BID_JPEG_LIMIT_SIZE_2M;
			break;
		case SENSOR_CAMERA_3MP_SIZE:
			jpeg_limit.blank_id = CAMCMP_BID_JPEG_LIMIT_SIZE_3M;
			break;
		case SENSOR_CAMERA_5MP_SIZE:
			jpeg_limit.blank_id = CAMCMP_BID_JPEG_LIMIT_SIZE_5M;
			break;
		default:
			return -EINVAL;
	}

	cmd_info[0].cmd_id = CAMCMP_CID_JPEG_LIMIT;
	cmd_info[0].p_blank_info = &jpeg_limit;
	cmd_info[0].blank_num = CAMCMP_SIZE_BLKINF(jpeg_limit);

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));

	if (rc != 0) {
		CAMCMP_LOG_ERR("camcmp_isp_trans failed (%d)\n",rc);     /* <029> */
		return -EFAULT;
	}


	rc = ius006f_change_mode(SENSOR_PREVIEW_MODE);
	if (rc != 0) {
		CAMCMP_LOG_ERR("ius006f_change_mode failed (%d)\n", rc); /* <029> */
		return -EFAULT;
	}

	CAMCMP_LOG_DBG("end rc = %d\n",rc);                          /* <029> */

	return rc;
}
/* <028> add-end */

static int32_t ius006f_setting(void)
{
	int32_t rc = 0;
	uint16_t power_on_t4 = 0;	/* T4 */

	camcmp_cmd_info_type cmd_info[] = {{0, NULL, 0, NULL, 0}};

	struct timeval starttv = {0};
	struct timeval endtv   = {0};
	uint32_t elapsedtime   = 0;

	cmd_info[0].cmd_id = CAMCMP_CID_INIT_SET_2;
	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("S3:Initial Setting2 pict out off failed. rc = %d\n", rc);
		return -EFAULT;
	}

	if ( (ius006f_ctrl->otp_rom_data[CAMCMP_OTP0M] & 0x0003FFC0) != 0 ) {
		cmd_info[0].cmd_id = CAMCMP_CID_INIT_SET_3;
		rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
		if (rc < 0) {
			CAMCMP_LOG_ERR("S4:Initial Setting3 failed. rc = %d\n", rc);
			return -EFAULT;
		}
	}

	cmd_info[0].cmd_id = CAMCMP_CID_ROM_CORCT;
	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("S5:Rom Correcttion Data Setting failed. rc = %d\n", rc);
		return -EFAULT;
	}

	rc = gpio_request(IUS006F_GPIO_CAM_STBY, "ius006f");
	if (!rc) {
		gpio_direction_output(IUS006F_GPIO_CAM_STBY, 1);
	}
	else {
		return -EINVAL;
	}

	mdelay(IUS006F_POWER_ON_T3);

	do_gettimeofday(&starttv);

	cmd_info[0].cmd_id = CAMCMP_CID_INIT_SET_4;
	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("S6:Initial Setting4 failed. rc = %d\n", rc);
		return -EFAULT;
	}

	power_on_t4 = IUS006F_FRAME_DEF * IUS006F_POWER_ON_T4;
	do_gettimeofday(&endtv);
	elapsedtime = ((endtv.tv_sec - starttv.tv_sec) * CAMCMP_CALC_1000) +
		      ((endtv.tv_usec - starttv.tv_usec) / CAMCMP_CALC_1000);
	CAMCMP_LOG_DBG("T4: output time = %d, elapsed time = %d\n", power_on_t4, elapsedtime);
	if (elapsedtime < power_on_t4) {
		msleep((power_on_t4 - elapsedtime));
	}

	cmd_info[0].cmd_id = CAMCMP_CID_INIT_SET_5;
	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("S7,8:af drv Initial failed. rc = %d\n", rc);
		return -EFAULT;
	}

	return rc;
}

/* <001> add start */
static int ius006f_vreg_control_lvs1(enum msm_sensor_onoff onoff)
{
	struct vreg *sensor_vreg;
	int rc = 0;

	sensor_vreg = vreg_get(NULL, "lvsw1");
	if ( IS_ERR(sensor_vreg) || sensor_vreg == NULL ) {
		CAMCMP_LOG_ERR("vreg_get(%s) failed (%ld)\n", "lvsw1", PTR_ERR(sensor_vreg));
		return -EFAULT;
	}

	if (onoff == SENSOR_ON) { /* <013> add */
		rc = vreg_set_level(sensor_vreg, 1800);
		if (rc) {
			CAMCMP_LOG_ERR("vreg_set_level failed (%d)\n", rc);
			return -EFAULT;
		}
		rc = vreg_enable(sensor_vreg);
		if (rc) {
			CAMCMP_LOG_ERR("vreg_enable failed = (%d)\n", rc);
			return -EFAULT;
		}
	} else {
		rc = vreg_disable(sensor_vreg);
		if (rc) {
			CAMCMP_LOG_ERR("vreg disable failed (%d)\n", rc);
			return -EFAULT;
		}
	}

	return 0;
}
/* <001> add end */


static void ius006f_probe_init_done(const struct msm_camera_sensor_info *data)
{
	gpio_direction_output(data->sensor_reset, 0);
	gpio_free(data->sensor_reset);

	gpio_direction_output(IUS006F_GPIO_CAM_CLK_EN, 0);
	gpio_free(IUS006F_GPIO_CAM_CLK_EN);

	gpio_direction_output(IUS006F_GPIO_VDD_CAM_EN2, 0);
	gpio_free(IUS006F_GPIO_VDD_CAM_EN2);
	/* <001> add start */
	mdelay(IUS006F_POWER_WAIT);
	gpio_direction_output(IUS006F_GPIO_VDD_CAM_EN3, 0);
	gpio_free(IUS006F_GPIO_VDD_CAM_EN3);

	mdelay(IUS006F_POWER_WAIT);
	(void)ius006f_vreg_control_lvs1(SENSOR_OFF); /* <013> add */

	mdelay(IUS006F_POWER_OFF_IOVD);
	/* <001> add end */
	gpio_direction_output(data->sensor_pwd, 0);
	gpio_free(data->sensor_pwd);
}

static int ius006f_probe_init_sensor(const struct msm_camera_sensor_info *data)
{
	int32_t rc = 0;
	camcmp_cmd_info_type cmd_info[] = {{0, NULL, 0, NULL, 0}, {0, NULL, 0, NULL, 0}};

	rc = gpio_request(data->sensor_pwd, "ius006f");
	if (!rc) {
		gpio_direction_output(data->sensor_pwd, 1);
	}
	else {
		CAMCMP_LOG_ERR("sensor_pwd Error, rc = %d\n", rc);
		return -EFAULT;
	}

	mdelay(IUS006F_POWER_WAIT);

	rc = ius006f_vreg_control_lvs1(SENSOR_ON); /* <013> add */
	if (rc < 0) {
		/* <001> */
		ius006f_probe_init_done(data);
		CAMCMP_LOG_ERR("VDD_CAM_EN2 Error, rc = %d\n", rc);
		return -EFAULT;
	}

	mdelay(IUS006F_POWER_WAIT);

	rc = gpio_request(IUS006F_GPIO_VDD_CAM_EN3, "ius006f");
	if (!rc) {
		gpio_direction_output(IUS006F_GPIO_VDD_CAM_EN3, 1);
	}
	else {
		/* <001> */
		ius006f_probe_init_done(data);
		CAMCMP_LOG_ERR("VDD_CAM_EN2 Error, rc = %d\n", rc);
		return -EFAULT;
	}

	mdelay(IUS006F_POWER_WAIT);
	/* <001> add end */

	rc = gpio_request(IUS006F_GPIO_VDD_CAM_EN2, "ius006f");
	if (!rc) {
		gpio_direction_output(IUS006F_GPIO_VDD_CAM_EN2, 1);
	}
	else {
		/* <001> */
		ius006f_probe_init_done(data);
		CAMCMP_LOG_ERR("VDD_CAM_EN2 Error, rc = %d\n", rc);
		return -EFAULT;
	}

	mdelay(IUS006F_POWER_WAIT);		/* <001> add end */

	rc = gpio_request(IUS006F_GPIO_CAM_CLK_EN, "ius006f");
	if (!rc) {
		gpio_direction_output(IUS006F_GPIO_CAM_CLK_EN, 1);
	}
	else {
		/* <001> */
		ius006f_probe_init_done(data);
		CAMCMP_LOG_ERR("CAM_CLK_EN Error, rc = %d\n", rc);
		return -EFAULT;
	}

	mdelay(IUS006F_POWER_ON_NCYC);

	rc = gpio_request(data->sensor_reset, "ius006f");
	if (!rc) {
		gpio_direction_output(data->sensor_reset, 1);
	}
	else {
		/* <001> */
		ius006f_probe_init_done(data);
		CAMCMP_LOG_ERR("sensor_reset Error, rc = %d\n", rc);
		return -EFAULT;
	}

	mdelay(IUS006F_POWER_ON_T1);

	cmd_info[0].cmd_id = CAMCMP_CID_CHK_DEV_STS;
	cmd_info[1].cmd_id = CAMCMP_CID_INIT_SET_1;

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("Read I2C status or S1:Initial Setting1_inclk failed. rc = %d\n", rc);
		ius006f_probe_init_done(data);
		return -EFAULT;
	}

	udelay(IUS006F_POWER_ON_T2);

	cmd_info[0].cmd_id = CAMCMP_CID_CHK_DEV_STS;
	cmd_info[1].cmd_id = CAMCMP_CID_GET_OTP;
	cmd_info[1].p_recieve_data = (void*)ius006f_ctrl->otp_rom_data;
	cmd_info[1].recieve_num = CAMCMP_OTP_ROM_NUM;

	rc = camcmp_isp_trans(cmd_info, CAMCMP_SIZE_CMD(cmd_info));
	if (rc < 0) {
		CAMCMP_LOG_ERR("Read I2C status or S2:OTP Read failed. rc = %d\n", rc);
		ius006f_probe_init_done(data);
		return -EFAULT;
	}

	rc = camcmp_makecmd_correction((void*)ius006f_ctrl->otp_rom_data, CAMCMP_OTP_ROM_NUM);
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_makecmd_correction failed. rc = %d\n", rc);
		ius006f_probe_init_done(data);
		return -EFAULT;
	}

	return 0;
}

static int ius006f_sensor_open_init(const struct msm_camera_sensor_info *data)
{
	int32_t rc;

	ius006f_ctrl = kzalloc(sizeof(struct ius006f_ctrl), GFP_KERNEL);
	if (!ius006f_ctrl) {
		CAMCMP_LOG_ERR("ius006f_init failed!\n");
		return -ENOMEM;
	}
	if (data) {
		ius006f_ctrl->sensordata = data;
	}

	ius006f_ctrl->sensormode = SENSOR_PREVIEW_MODE;

	ius006f_ctrl->prev_res = SENSOR_CAMERA_VGA_SIZE;

	/* <013> add start */
	ius006f_ctrl->init_curr_lens_pos = 0;

	ius006f_ctrl->af_cancel_onoff = SENSOR_OFF;

	/* <016> */
	ius006f_ctrl->night_mode = 0;

	af_start_after_cnt =0;
	/* <013> add end */

	msm_camio_camif_pad_reg_reset();
	mdelay(IUS006F_WAIT_20);

	/* <003> add start */
	rc = camcmp_makecmd_init();
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_makecmd_init failed\n");
		return -EFAULT;
	}
	/* <003> add end */

	rc = ius006f_probe_init_sensor(data);
	if (rc < 0) {
		if (ius006f_ctrl) {
			kfree(ius006f_ctrl);
			ius006f_ctrl = NULL;
		}
		return rc;
	}

	rc = ius006f_setting();
	if (rc < 0) {
		CAMCMP_LOG_ERR("ius006f_setting failed. rc = %d\n", rc);
		ius006f_sensor_release();    /* <035> */
	}

	return rc;
}

int ius006f_sensor_release(void)
{
	uint16_t power_off_t1 = 0;
	uint16_t calc = 0;

	mutex_lock(&ius006f_mut);

	gpio_direction_output(IUS006F_GPIO_CAM_STBY, 0);
	gpio_free(IUS006F_GPIO_CAM_STBY);

	power_off_t1 = ius006f_calc_msec_from_fps(IUS006F_POWER_OFF_T1);
	calc = power_off_t1;
	power_off_t1 /= CAMCMP_CALC_10;
	if((calc % CAMCMP_CALC_10) != 0) {
		power_off_t1++;
	}
	CAMCMP_LOG_DBG("T1: Sleep wait time = %d\n", power_off_t1);
	msleep(power_off_t1);

	ius006f_probe_init_done(ius006f_ctrl->sensordata);

	if (ius006f_ctrl) {
		kfree(ius006f_ctrl);
		ius006f_ctrl = NULL;
	}

	CAMCMP_LOG_DBG("ius006f_release completed\n");

	mutex_unlock(&ius006f_mut);

	return 0;
}

int ius006f_sensor_config(void *__sync, void __user *argp) /* <013> add */
{
	struct sensor_cfg_data cdata;
	struct msm_sync *sync = (struct msm_sync *) __sync; /* <013> add */
	int rc = 0;

	CAMCMP_LOG_DBG("start here !!!\n");
	if (copy_from_user(&cdata,
			   (void *)argp, sizeof(struct sensor_cfg_data))) {
		return -EFAULT;
	}

	mutex_lock(&ius006f_mut);

	CAMCMP_LOG_DBG("cfgtype = %d\n", cdata.cfgtype);
	switch (cdata.cfgtype) {
	case CFG_SET_MODE:
	
		CAMCMP_LOG_DBG("CFG_SET_MODE\n");
		rc = ius006f_set_mode(cdata.mode, cdata.rs);
		break;

	case  CFG_SET_AF_MODE:
		CAMCMP_LOG_DBG("CFG_SET_AF_MODE\n");
		rc = ius006f_set_af_mode(sync, &(cdata.cfg.af_mode)); /* <013> */
/*<009>start*/
		if (copy_to_user((void *)argp, &cdata,
				 sizeof(struct sensor_cfg_data))) {
			rc = -EFAULT;
		}
/*<009>end*/
		break;

/* b2.0 add start --> */
	case CFG_GET_EEPROM_READ:
		rc = ius006f_get_otp(&(cdata.cfg.get_otp));
		if (copy_to_user((void *)argp, &cdata,
				 sizeof(struct sensor_cfg_data))) {
			rc = -EFAULT;
		}
		break;
	case CFG_SENSOR_PAT_CHG:
		rc = ius006f_sensor_pat_chg(&(cdata.cfg.pat_chg));
		if (copy_to_user((void *)argp, &cdata,
				 sizeof(struct sensor_cfg_data))) {
			rc = -EFAULT;
		}
		break;
/* --> b2.0 add end   */

/*<009> start*/
    case CFG_GET_AF_POS:
		CAMCMP_LOG_DBG("CFG_GET_AF_POS\n");
		rc = ius006f_get_af_pos(&(cdata.cfg.get_af_pos));
		if (rc >= 0) {
			if (copy_to_user((void *)argp, &cdata,
					 sizeof(struct sensor_cfg_data))) {
				rc = -EFAULT;
			}
		}
		break;
    case CFG_GET_MONI_GAIN:
		CAMCMP_LOG_DBG("CFG_GET_MONI_GAIN\n");
		rc = ius006f_get_moni_gain(&(cdata.cfg.get_monigain));
		if (rc >= 0) {
			if (copy_to_user((void *)argp, &cdata,
					 sizeof(struct sensor_cfg_data))) {
				rc = -EFAULT;
			}
		}
		break;
    case CFG_GET_REGVER:
		CAMCMP_LOG_DBG("CFG_GET_REGVER\n");
		rc = ius006f_get_reg_ver(&(cdata.cfg.get_reg_ver));
		if (rc >= 0) {
			if (copy_to_user((void *)argp, &cdata,
					 sizeof(struct sensor_cfg_data))) {
				rc = -EFAULT;
			}
		}
		break;
/*<009> end*/

/* <013> add start */
	case CFG_SET_DEFAULT_FOCUS:
		CAMCMP_LOG_DBG("CFG_SET_DEFAULT_FOCUS\n");
		rc = ius006f_set_default_focus();
		break;

	case CFG_AUTO_FOCUS:
		CAMCMP_LOG_DBG("CFG_AUTO_FOCUS\n");
		rc = ius006f_af_start(sync);
		break;

	case CFG_AUTO_FOCUS_AFTER:
		CAMCMP_LOG_DBG("CFG_AUTO_FOCUS_AFTER\n");
		rc = ius006f_af_start_after(sync);
		break;

	case CFG_AUTO_FOCUS_CANCEL:
		CAMCMP_LOG_DBG("CFG_AUTO_FOCUS_CANCEL\n");
		rc = ius006f_af_cancel();
		break;
/* <013> add end */

/*<014> start*/
	case CFG_SET_BRIGHTNESS:
		CAMCMP_LOG_DBG("CFG_SET_BRIGHTNESS\n");
		rc = ius006f_set_brightness(cdata.cfg.brightness);
		break;

	case CFG_SET_EFFECT:
		CAMCMP_LOG_DBG("CFG_SET_EFFECT\n");
		rc = ius006f_set_effect(cdata.cfg.effect);
		break;
/*<014> end*/

/* <016> start */
    case CFG_SET_ZOOM:
		CAMCMP_LOG_DBG("CFG_SET_ZOOM\n");
		rc = ius006f_set_zoom( cdata.cfg.zoom, cdata.rs );
		break;

    case CFG_SET_PICTURE_MODE:
		CAMCMP_LOG_DBG("CFG_SET_PICTURE_MODE\n");
		rc = ius006f_set_picture_mode( cdata.cfg.picture_mode );
		break;
/* <016> end */

/* <018> add start */
	case CFG_SET_WB:
		CAMCMP_LOG_DBG("ius006f_sensor_config CFG_SET_WB\n");
		rc = ius006f_set_wb(cdata.cfg.wb);
		break;
/* <018> add end */

/* <019> add start */
	case CFG_SET_JPEG_QUALITY:
		CAMCMP_LOG_DBG("ius006f_sensor_config CFG_SET_JPEG_QUALITY\n");
		rc = ius006f_set_jpeg_quality(cdata.cfg.jpeg_quality);
		break;
/* <019> add end */
/*<027>add start*/
    case CFG_SET_MANUAL_STEP:
		CAMCMP_LOG_DBG("CFG_SET_MANUAL_STEP\n");
		rc = ius006f_set_manual_step(&(cdata.cfg.set_manual_step));
		if (rc >= 0) {
			if (copy_to_user((void *)argp, &cdata,
					 sizeof(struct sensor_cfg_data))) {
				rc = -EFAULT;
			}
		}
		break;
/*<027>add end*/
/* <028> add-start */
	case CFG_SET_PICTURE_SIZE:
		CAMCMP_LOG_DBG("CFG_SET_PICTURE_SIZE\n");
		rc = ius006f_set_pict_size( cdata.rs );
		break;
/* <028> add-end */

	default:
		rc = -EINVAL;
		break;
	}

	mutex_unlock(&ius006f_mut);
	return rc;
}


static void ius006f_init_client(struct i2c_client *client)
{
	init_waitqueue_head(&ius006f_wait_queue);
}

static int ius006f_i2c_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{
	int rc = 0;
	CAMCMP_LOG_DBG("ius006f_probe called!\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		CAMCMP_LOG_ERR("i2c_check_functionality failed\n");
		rc = -EFAULT;
		goto probe_failure;
	}

	ius006f_sensorw = kzalloc(sizeof(struct ius006f_work), GFP_KERNEL);
	if (!ius006f_sensorw) {
		CAMCMP_LOG_ERR("kzalloc failed.\n");
		rc = -ENOMEM;
		goto probe_failure;
	}

	i2c_set_clientdata(client, ius006f_sensorw);
	ius006f_init_client(client);
	ius006f_client = client;
	
	/* <003> add start */
	rc = camcmp_i2c_init((void*)client);
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_i2c_init failed\n");
		rc = -EFAULT;
		goto probe_failure;
	}
	/* <003> add end */

	mdelay(IUS006F_WAIT_50);

	CAMCMP_LOG_DBG("ius006f_probe successed! rc = %d\n", rc);
	return rc;

probe_failure:
	CAMCMP_LOG_ERR("ius006f_probe failed! rc = %d\n", rc);
	return rc;
}


static const struct i2c_device_id ius006f_i2c_id[] = {
	{"ius006f", 0},
	{}
};

static struct i2c_driver ius006f_i2c_driver = {
	.id_table = ius006f_i2c_id,
	.probe = ius006f_i2c_probe,
	.remove = __exit_p(ius006f_i2c_remove),
	.driver = {
		   .name = "ius006f",
		   },
};

static int ius006f_sensor_probe(const struct msm_camera_sensor_info *info,
				struct msm_sensor_ctrl *ctrl)
{
	int rc = i2c_add_driver(&ius006f_i2c_driver);
	if (rc < 0 || ius006f_client == NULL) {
		rc = -ENOTSUPP;
		goto probe_done;
	}

/* <001> del start */

	ctrl->s_init = ius006f_sensor_open_init;
	ctrl->s_release = ius006f_sensor_release;
/* <013> add start */
	ctrl->s_config = NULL;
	ctrl->s_config2 = ius006f_sensor_config;
/* <013> add end */

probe_done:
	CAMCMP_LOG_ERR("%s\n", __FILE__);
	return rc;
}

static int __ius006f_probe(struct platform_device *pdev)
{
	return msm_camera_drv_start(pdev, ius006f_sensor_probe);
}


static struct platform_driver msm_camera_driver = {
	.probe = __ius006f_probe,
	.driver = {
		   .name = "msm_camera_ius006f",
		   .owner = THIS_MODULE,
		   },
};

static int __init ius006f_init(void)
{
	return platform_driver_register(&msm_camera_driver);
}


module_init(ius006f_init);
