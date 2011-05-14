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


#ifndef CAMCMP_CMDSENDER_H
#define CAMCMP_CMDSENDER_H


#include <linux/types.h>
#include <linux/kernel.h>



#define CAMCMP_LOG_ENABLE


#define CAMCOMBINE2(a,b) a##b
#define CAMCOMBINE3(a,b,c) a##b##c

/* <002> mod start */


#define CAMCMP_LOG_DBG(fmt, args...) printk(KERN_INFO "msm_camera:%s(%d) " fmt, __func__, __LINE__, ##args)

/* <011> mod start */
#define CAMCMP_LOG_INF(fmt, args...) do { } while (0)
/* <011> mod end */


#define CAMCMP_LOG_ERR(fmt, args...) pr_err("msm_camera:%s(%d) " fmt, __func__, __LINE__, ##args)
/* <002> mod end */

/* <007> mod start */
#define CAMCMP_CALC_10000		10000 /* <008> */
#define CAMCMP_CALC_1000		1000
#define CAMCMP_CALC_100			100
#define CAMCMP_CALC_10			10
#define CAMCMP_CALC_ROUND_OFF	5


enum otp_rom {
	CAMCMP_OTP0L = 0,
	CAMCMP_OTP0M,
	CAMCMP_OTP0H,
	CAMCMP_OTP1L,
	CAMCMP_OTP1M,
	CAMCMP_OTP1H,

	CAMCMP_OTP_ROM_NUM
};
/* <007> mod end */

typedef enum {
	CAMCMP_CID_CHK_DEV_STS,
	CAMCMP_CID_INIT_SET_1,
	CAMCMP_CID_INIT_SET_2,
	CAMCMP_CID_INIT_SET_3,
	CAMCMP_CID_ROM_CORCT,
	CAMCMP_CID_INIT_SET_4,
	CAMCMP_CID_INIT_SET_5,
	CAMCMP_CID_WB_AUTO,
	CAMCMP_CID_WB_INCAND,
	CAMCMP_CID_WB_DAYLIGHT,
	CAMCMP_CID_WB_FLUORE,
	CAMCMP_CID_WB_CLOUDY,
	CAMCMP_CID_WB_LED,
	CAMCMP_CID_NIGHT_OFF,
	CAMCMP_CID_NIGHT_ON,
	CAMCMP_CID_MOVIE,				/* <013> */
	CAMCMP_CID_CAP_SIZE,
	CAMCMP_CID_MONI_SIZE,
	CAMCMP_CID_MODE_CHG,
	CAMCMP_CID_ZOOM_5M,
	CAMCMP_CID_ZOOM_3M,
	CAMCMP_CID_ZOOM_2M,
	CAMCMP_CID_ZOOM_1M,
	CAMCMP_CID_ZOOM_QH,
	CAMCMP_CID_ZOOM_QL,
	CAMCMP_CID_BRIGHTNESS,
	CAMCMP_CID_EFFECT,
	CAMCMP_CID_QUALITY,
	CAMCMP_CID_INT_CLR,
	CAMCMP_CID_AF_CANCEL,
	CAMCMP_CID_AF_MODE,
	CAMCMP_CID_MF_START,
	CAMCMP_CID_AE_NOMAL,
	CAMCMP_CID_AE_LONG,
	CAMCMP_CID_PTNCHK_OFF,
	CAMCMP_CID_PTNCHK_ON,
	CAMCMP_CID_CHK_AF_LOCK,
	CAMCMP_CID_CONF_THUMBNAIL,		/* <005> */
	CAMCMP_CID_JPEG_LIMIT,  		/* <009> */
	CAMCMP_CID_AF_RESTART_F, 		/* <012> */
	CAMCMP_CID_JPEG_INTSTS,			/* <014> */

	CAMCMP_CID_GET_OTP0L,
	CAMCMP_CID_GET_OTP0M,
	CAMCMP_CID_GET_OTP0H,
	CAMCMP_CID_GET_OTP1L,
	CAMCMP_CID_GET_OTP1M,
	CAMCMP_CID_GET_OTP1H,
	CAMCMP_CID_GET_OTP,
	CAMCMP_CID_GET_INTSTS,

/*<004>start*/
	CAMCMP_CID_GET_EVALUATE,
	CAMCMP_CID_GET_LENSPOS,
	CAMCMP_CID_GET_AF_STEP,
	CAMCMP_CID_GET_AESCL,
	CAMCMP_CID_GET_SHT_TIME_H,
	CAMCMP_CID_GET_SHT_TIME_L,
	CAMCMP_CID_GET_AGC_SCL,
/*<004>end*/
/* <006> add start */
	CAMCMP_CID_GET_AF_RESULT,
	CAMCMP_CID_GET_AF_STATE,
/* <006> add end */

/*<010>add start*/
	CAMCMP_CID_GET_MANUAL_STEP,
	CAMCMP_CID_GET_AREA_LOW_TYPE1,
	CAMCMP_CID_GET_AREA_HIGH_TYPE1,
/*<010>add end*/
	CAMCMP_CID_GET_JPG_STS,			/* <014> */
	CAMCMP_CID_GET_JPG_RETRY_CTL,	/* <014> */
	CAMCMP_CID_MAX
} camcmp_cmd_id_enum;

typedef enum {
	CAMCMP_BID_CAP_SIZE_5M,
	CAMCMP_BID_CAP_SIZE_3M,
	CAMCMP_BID_CAP_SIZE_2M,
	CAMCMP_BID_CAP_SIZE_1M
} camcmp_blank_cap_size_enum;

typedef enum {
	CAMCMP_BID_MONI_SIZE_VGA,
	CAMCMP_BID_MONI_SIZE_QH,
	CAMCMP_BID_MONI_SIZE_QL
} camcmp_blank_moni_size_enum;

typedef enum {
	CAMCMP_BID_MONI_MODE,
	CAMCMP_BID_MONI_REFRESH,
	CAMCMP_BID_HALF_MODE,
	CAMCMP_BID_CAP_MODE
} camcmp_blank_chg_mode_enum;

typedef enum {
	CAMCMP_BID_ZOOM_0,
	CAMCMP_BID_ZOOM_1,
	CAMCMP_BID_ZOOM_2,
	CAMCMP_BID_ZOOM_3,
	CAMCMP_BID_ZOOM_4,
	CAMCMP_BID_ZOOM_5,
	CAMCMP_BID_ZOOM_6,
	CAMCMP_BID_ZOOM_7,
	CAMCMP_BID_ZOOM_8,
	CAMCMP_BID_ZOOM_9,
} camcmp_blank_zoom_enum;

typedef enum {
	CAMCMP_BID_BRIGHT_M5,
	CAMCMP_BID_BRIGHT_M4,
	CAMCMP_BID_BRIGHT_M3,
	CAMCMP_BID_BRIGHT_M2,
	CAMCMP_BID_BRIGHT_M1,
	CAMCMP_BID_BRIGHT_0,
	CAMCMP_BID_BRIGHT_P1,
	CAMCMP_BID_BRIGHT_P2,
	CAMCMP_BID_BRIGHT_P3,
	CAMCMP_BID_BRIGHT_P4,
	CAMCMP_BID_BRIGHT_P5
} camcmp_blank_bright_enum;

typedef enum {
	CAMCMP_BID_EFFECT_OFF,
	CAMCMP_BID_EFFECT_MONO,
	CAMCMP_BID_EFFECT_SEPIA,
	CAMCMP_BID_EFFECT_NEGA
} camcmp_blank_effect_enum;

typedef enum {
	CAMCMP_BID_Q_NORMAL,
	CAMCMP_BID_Q_FINE,
	CAMCMP_BID_Q_SFINE
} camcmp_blank_quality_enum;

typedef enum {
	CAMCMP_BID_CLR_MODE,
	CAMCMP_BID_CLR_AF_LOCK,
	CAMCMP_BID_CLR_MODE_AF
} camcmp_blank_int_clr_enum;

typedef enum {
	CAMCMP_BID_MODE_OFF,			/* <003> */
	CAMCMP_BID_MODE_SAF,
	CAMCMP_BID_MODE_MF
} camcmp_blank_af_mode_enum;

typedef enum {
	CAMCMP_BID_MF_1M,
	CAMCMP_BID_MF_MACRO,
	CAMCMP_BID_MF_INF,
	CAMCMP_BID_MF_0,
	CAMCMP_BID_MF_CUSTOM
} camcmp_blank_mf_pos_enum;
/* <009> add-start */
typedef enum {
	CAMCMP_BID_JPEG_LIMIT_SIZE_5M,
	CAMCMP_BID_JPEG_LIMIT_SIZE_3M,
	CAMCMP_BID_JPEG_LIMIT_SIZE_2M,
	CAMCMP_BID_JPEG_LIMIT_SIZE_1M
} camcmp_blank_jpeg_size_enum;
/* <009> */



typedef struct {
	unsigned char*	p_buff;
	uint32_t		size;
} camcmp_buff_type;

typedef struct {
	uint32_t		blank_id;
	void*			p_blank_data;
} camcmp_blank_info_type;

typedef struct {
	uint32_t					cmd_id;
	camcmp_blank_info_type*		p_blank_info;
	uint32_t					blank_num;
	void*						p_recieve_data;
	uint32_t					recieve_num;
} camcmp_cmd_info_type;

#define CAMCMP_SIZE_CMD(a)     (sizeof(a)/sizeof(camcmp_cmd_info_type))
#define CAMCMP_SIZE_BLKINF(a)  (sizeof(a)/sizeof(camcmp_blank_info_type))
#define CAMCMP_SIZE_REVINF(a)  (sizeof(a)/sizeof(uint32_t))



int32_t camcmp_i2c_init(void* p_client);
int32_t camcmp_makecmd_init(void);
void camcmp_makecmd_get_version(uint16_t* p_ver, uint8_t* p_flg);
int32_t camcmp_makecmd_correction(void* p_val, uint32_t num);
int32_t camcmp_isp_trans(camcmp_cmd_info_type* p_info, uint32_t num);

#endif /* CAMCMP_CMDSENDER_H */
