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


#include <linux/kernel.h>
#include <linux/i2c.h>
#include <mach/camera.h>
#include "camcmp_cmdmaker_ius006f.h"


#define CAMCMP_POLL_TIME      10
#define CAMCMP_POLL_RETRY     500

#define CAMCMP_OTP_DAC_MASK   0x000003FF

/* <002><010><011> add-start */
#define CAMCMP_AF_SEARCH_OFFSET				253
#define CAMCMP_AF_LENSPOS_ON_AFNG			258
#define CAMCMP_AF_DRV_AMOUNT_TONEAR_F		259
#define CAMCMP_AF_DRV_AMOUNT_TONEAR_S		260
#define CAMCMP_AF_DRV_AMOUNT_TOFAR_F		261
#define CAMCMP_AF_DRV_AMOUNT_TOFAR_S		262
#define CAMCMP_AF_AREA_LOW_TYPE1			264
#define CAMCMP_AF_AREA_HIGH_TYPE1			265
#define CAMCMP_AF_AREA_LOW_TYPE2			266
#define CAMCMP_AF_AREA_HIGH_TYPE2			267
#define CAMCMP_AF_AREA_LOW_TYPE3			268
#define CAMCMP_AF_AREA_HIGH_TYPE3			269
#define CAMCMP_AF_CAF_PARAM_WOBBLE_STEP		297
#define CAMCMP_AF_CAF_CLIMB_STEP			298
#define CAMCMP_AF_CAF_WOBBLE_MAX_MOVE		412
/* <002><010><011> add-end */

#define CAMCMP_AF_CORRECT_NUM				15






static struct i2c_client* g_p_camcmp_ius006f_i2c = NULL;

static int16_t g_camcmp_af_search_offset			= CAMCMP_AF_SEARCH_OFFSET;
static int16_t g_camcmp_af_lenspos_on_afng			= CAMCMP_AF_LENSPOS_ON_AFNG;
static int16_t g_camcmp_af_drv_amount_tonear_f		= CAMCMP_AF_DRV_AMOUNT_TONEAR_F;
static int16_t g_camcmp_af_drv_amount_tonear_s		= CAMCMP_AF_DRV_AMOUNT_TONEAR_S;
static int16_t g_camcmp_af_drv_amount_tofar_f		= CAMCMP_AF_DRV_AMOUNT_TOFAR_F;
static int16_t g_camcmp_af_drv_amount_tofar_s		= CAMCMP_AF_DRV_AMOUNT_TOFAR_S;
static int16_t g_camcmp_af_area_low_type1			= CAMCMP_AF_AREA_LOW_TYPE1;
static int16_t g_camcmp_af_area_high_type1			= CAMCMP_AF_AREA_HIGH_TYPE1;
static int16_t g_camcmp_af_area_low_type2			= CAMCMP_AF_AREA_LOW_TYPE2;	
static int16_t g_camcmp_af_area_high_type2			= CAMCMP_AF_AREA_HIGH_TYPE2;
static int16_t g_camcmp_af_area_low_type3			= CAMCMP_AF_AREA_LOW_TYPE3;
static int16_t g_camcmp_af_area_high_type3			= CAMCMP_AF_AREA_HIGH_TYPE3;
static int16_t g_camcmp_af_caf_param_wobble_step	= CAMCMP_AF_CAF_PARAM_WOBBLE_STEP;
static int16_t g_camcmp_af_caf_climb_step			= CAMCMP_AF_CAF_CLIMB_STEP;
static int16_t g_camcmp_af_caf_wobble_max_move		= CAMCMP_AF_CAF_WOBBLE_MAX_MOVE;



extern camcmp_makecmd_type g_camcmp_ius006f_cmd_list[];

extern camcmp_ius006f_af_val_type* g_p_camcmp_ius006f_af;

extern uint16_t* g_p_camcmp_ius006f_ver;

extern camcmp_data_type g_camcmp_ius006f_poll_list[];




static int32_t camcmp_i2c_transfer(unsigned char* p_txdata, int tx_size,
                                   unsigned char* p_rxdata, int rx_size)
{
	struct i2c_msg msgs[] = {
		{
			.addr  = g_p_camcmp_ius006f_i2c->addr,
			.flags = 0,
			.len   = tx_size,
			.buf   = p_txdata,
		},
		{
			.addr  = g_p_camcmp_ius006f_i2c->addr,
			.flags = I2C_M_RD,
			.len   = rx_size,
			.buf   = p_rxdata,
		}
	};

	int32_t num, rc;

	num = sizeof(msgs)/sizeof(struct i2c_msg);
	if ( (p_rxdata == NULL) || (rx_size == 0) ) {
		num = 1;
	}

	rc = i2c_transfer(g_p_camcmp_ius006f_i2c->adapter, msgs, num);
	if (rc < 0) {
		CAMCMP_LOG_ERR("i2c failed! rc=%d\n", rc);
		return -EFAULT;
	}

	return 0;
}


static int32_t camcmp_ius006f_write_single(unsigned short addr, unsigned int val, unsigned short size)
{
	int32_t rc = 0;
	unsigned char buf[6];

	buf[0] = (addr & 0xFF00) >> 8;
	buf[1] = (addr & 0x00FF);

	buf[2] = (val & 0x000000FF);
	buf[3] = (val & 0x0000FF00) >> 8;
	buf[4] = (val & 0x00FF0000) >> 16;
	buf[5] = (val & 0xFF000000) >> 24;

	CAMCMP_LOG_INF("i2c write addr,val,size: W,0x%04X,0x%08X,%d\n", addr, val, size);

	rc = camcmp_i2c_transfer(buf, (size+2), NULL, 0);
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_i2c_transfer failed! rc=%d\n", rc);
		return rc;
	}

	return 0;
}


static int32_t camcmp_ius006f_write_sequential(unsigned short addr, unsigned char* p_data, unsigned short size)
{
	int32_t i, j, w_size, max;   /* <010> */
	int32_t rc = 0;
	unsigned char buf[128];      /* <010> */

	if (p_data == NULL) {
		CAMCMP_LOG_ERR("input NULL!\n");
		return -EINVAL;
	}

	/* <010> mod start */
	max = (size / 126);
	if ((size % 126) != 0) {
		max++;
	}

	for (i = 0; i < max; i++) {
		buf[0] = (addr & 0xFF00) >> 8;
		buf[1] = (addr & 0x00FF);

		w_size = 126;
		if (size < 126) {
			w_size = size;
		}
		w_size += 2;

		for (j = 2; j < w_size; j++) {
			buf[j] = *p_data;
			CAMCMP_LOG_INF("i2c write addr,val,size: W,0x%04X,0x%08X,%d\n", addr, buf[j], 1);

			addr++;
			p_data++;
		}

		CAMCMP_LOG_INF("sequential write: 0x%04X,%d\n", ((buf[0] << 8) + buf[1]), (w_size-2));

		rc = camcmp_i2c_transfer(buf, w_size, NULL, 0);
		if (rc < 0) {
			CAMCMP_LOG_ERR("camcmp_i2c_transfer failed! num=%d rc=%d\n", i, rc);
			return rc;
		}

		size -= 126;
	}
	/* <010> mod start */

	return 0;
}


static int32_t camcmp_ius006f_read(unsigned short addr, unsigned short size, uint32_t* p_rev)    /* <010> */
{
	int32_t rc = 0;
	unsigned char w_buf[2];
	unsigned char r_buf[4];

	if (p_rev == NULL) {
		CAMCMP_LOG_ERR("input NULL!\n");
		return -EINVAL;
	}

	if ( (size > CAMCMP_D_LEN_LWORD) || (size == 0) ) {
		CAMCMP_LOG_ERR("input size failed! size=%d", size);
		return -EINVAL;
	}

	w_buf[0] = (addr & 0xFF00) >> 8;
	w_buf[1] = (addr & 0x00FF);

	memset(r_buf, 0x00, sizeof(r_buf));

	rc = camcmp_i2c_transfer(w_buf, sizeof(w_buf), r_buf, size);
	if (rc < 0) {
		CAMCMP_LOG_ERR("camcmp_i2c_transfer failed! rc=%d\n", rc);
		return rc;
	}

	*p_rev = r_buf[3] << 24 |
	         r_buf[2] << 16 |
	         r_buf[1] <<  8 |
	         r_buf[0];

	CAMCMP_LOG_INF("i2c read  addr,val,size: R,0x%04X,0x%08X,%d\n", addr, *p_rev, size);

	return 0;
}

static int32_t camcmp_ius006f_polling(camcmp_ius006f_reg_val_type* p_reg, int32_t num)
{
	int32_t i, j;
	int32_t rc = 0;
	uint32_t val;     /* <010> */
	camcmp_ius006f_reg_val_type reg_val;

	/* <007> add start */
	if ( (p_reg == NULL) || (num == 0) ) {
		CAMCMP_LOG_ERR("input NULL! reg=%p, num=%d \n", p_reg, num);
		return -EINVAL;
	}
	/* <007> add end */

	for (i = 0; i < CAMCMP_POLL_RETRY; i++) {
		for (j = 0; j < num; j++) {
			reg_val = *(p_reg + j);

			rc = camcmp_ius006f_read(reg_val.waddr, reg_val.size, &val);
			if (rc < 0) {
				CAMCMP_LOG_ERR("camcmp_ius006f_read failed! addr=0x%04X rc=%d\n", reg_val.waddr, rc);
				return rc;
			}

			if ( (reg_val.cmd_kind == CAMCMP_REG_POLL1) &&
			     ((val & reg_val.lwdata) == reg_val.lwdata) ) {
				return j;
			} else if ((reg_val.cmd_kind == CAMCMP_REG_POLL2) && (val == reg_val.lwdata)) {
				return j;
			} else {
				/* Do Nothing */
			}
		}

		CAMCMP_LOG_INF("i2c polling wait time=%d\n", CAMCMP_POLL_TIME);

		msleep(CAMCMP_POLL_TIME);
	}

	CAMCMP_LOG_ERR("polling time out! \n");
	return -ETIMEDOUT;
}


static int32_t camcmp_ius006f_set_blank(camcmp_data_type blank_info, void* p_in_data)
{
	int32_t i;
	camcmp_ius006f_blank_val_type *p_blank;
	unsigned short *p_w  = NULL;
	unsigned int   *p_lw = NULL;
	uint32_t       val = 0;

	if ( (blank_info.p_data == NULL) || (blank_info.size == 0) ) {
		CAMCMP_LOG_ERR("input NULL! pinf=0x%08X, num=%d\n", (int32_t)blank_info.p_data, blank_info.size);
		return -EINVAL;
	}

	p_blank = (camcmp_ius006f_blank_val_type*)blank_info.p_data;

	if (p_in_data != NULL) {
		if (blank_info.size > 1) {
			CAMCMP_LOG_ERR("input size failed! size=%d", blank_info.size);
			return -EINVAL;
		}

		val = *((uint32_t*)(p_in_data));   /* <009> */
	}

	for (i = 0; i < blank_info.size; i++) {
		switch (p_blank->size) {
		case CAMCMP_D_LEN_WORD:
			p_w = (unsigned short*)p_blank->p_copy;
			if (p_in_data == NULL) {
				*p_w = (unsigned short)p_blank->org_val;
			} else {
				*p_w = (unsigned short)val;
			}
			break;

		case CAMCMP_D_LEN_LWORD:
			p_lw = (unsigned int*)p_blank->p_copy;
			if (p_in_data == NULL) {
				*p_lw = p_blank->org_val;
			} else {
				*p_lw = val;
			}
			break;

		default:
			CAMCMP_LOG_ERR("blank table size failed! size=%d", p_blank->size);
			return -EINVAL;
		}

		p_blank++;
	}

	return 0;
}

static int32_t camcmp_ius006f_send_cmd(camcmp_data_type send_cmd, void* p_rev, uint32_t* p_rev_num, uint32_t max_num)
{
	int32_t rc = 0;
	int32_t i;
	uint32_t                    *p_rev_data;    /* <010> */
	camcmp_ius006f_reg_val_type *p_info;
	camcmp_ius006f_reg_val_type *p_poll;

	if (g_p_camcmp_ius006f_i2c == NULL) {
		CAMCMP_LOG_ERR("i2c driver NULL!\n");
		return -EFAULT;
	}

	if ( (send_cmd.p_data == NULL) || (send_cmd.size == 0) ) {
		CAMCMP_LOG_ERR("input NULL! pinf=0x%08X, num=%d\n", (int32_t)send_cmd.p_data, send_cmd.size);
		return -EINVAL;
	}

	p_info = (camcmp_ius006f_reg_val_type*)send_cmd.p_data;

	for (i = 0; i < send_cmd.size; i++) {
		switch (p_info->cmd_kind) {
		case CAMCMP_REG_WRITE:
			if (p_info->size > CAMCMP_D_LEN_LWORD) {
				rc = camcmp_ius006f_write_sequential(p_info->waddr, p_info->p_data, p_info->size);
			} else {
				rc = camcmp_ius006f_write_single(p_info->waddr, p_info->lwdata, p_info->size);
			}
			if (rc < 0) {
				CAMCMP_LOG_ERR("camcmp_ius006f_write failed! size=%d rc=%d\n", p_info->size, rc);
				return rc;
			}
			break;

		case CAMCMP_REG_READ:
			/* <007> add start */
			if ( (p_rev == NULL) || (p_rev_num == NULL) ) {
				CAMCMP_LOG_ERR("input NULL! rev=%p, rev_num=%p\n", p_rev, p_rev_num);
				return -EINVAL;
			}
			/* <007> add end */

			if (*p_rev_num > max_num) {
				CAMCMP_LOG_ERR("recieve size failed! num=%d\n", *p_rev_num);
		 		return -EINVAL;
			}

			p_rev_data = ((uint32_t*)p_rev) + *p_rev_num;   /* <010> */
			rc = camcmp_ius006f_read(p_info->waddr, p_info->size, p_rev_data);
			if (rc < 0) {
				CAMCMP_LOG_ERR("camcmp_ius006f_read failed! rc=%d\n", rc);
				return rc;
			}
			(*p_rev_num)++;
			break;

		case CAMCMP_REG_POLL1:
		case CAMCMP_REG_POLL2:
			rc = camcmp_ius006f_polling(p_info, 1);
			if (rc < 0) {
				CAMCMP_LOG_ERR("camcmp_ius006f_polling failed! rc=%d\n", rc);
				return rc;
			}
			break;

		/* <001> add start */
		case CAMCMP_REG_POLL3:
			p_poll = (camcmp_ius006f_reg_val_type*)g_camcmp_ius006f_poll_list[p_info->waddr].p_data;
			rc = camcmp_ius006f_polling(p_poll,
			                            g_camcmp_ius006f_poll_list[p_info->waddr].size);
			if (rc < 0) {
				CAMCMP_LOG_ERR("camcmp_ius006f_polling 3 failed! rc=%d\n", rc);
				return rc;
			}
			if (rc != p_info->lwdata) {
				CAMCMP_LOG_ERR("camcmp_ius006f_polling 3 %d != %d\n", rc, p_info->lwdata);
				return -EFAULT;
			}
			break;
		/* <001> add end */

		default:
			CAMCMP_LOG_ERR("cmd kind failed! kind=%d\n", p_info->cmd_kind);

			return -EFAULT;
		}


		p_info++;
	}

	return 0;
}


static camcmp_makecmd_type* camcmp_ius006f_make_cmd(void)
{
	return g_camcmp_ius006f_cmd_list;
}

static int32_t camcmp_ius006f_correct_cmd(void* p_val, uint32_t num)
{
	int32_t *p_otp;
	camcmp_ius006f_reg_val_type    *p_reg;
	camcmp_ius006f_blank_val_type  *p_blk;


	int32_t dac1m;
	int32_t dac10cm;
	int32_t vcm_slope;
	int32_t dac_overinf;
	int32_t dis_bet_pos = 106;	/* <004> add */ /* <009> add */

	/* <004> add start */

	int32_t opt_inf_pos;
	int32_t macro_pos;
	int32_t stand_pos;
	int32_t af_drv_search_f;
	int32_t af_drv_search_s;
	int32_t low_search_area;
	int32_t high_search_area;
	/* <004> add end */


	if ( (p_val == NULL) || (num < CAMCMP_OTP_ROM_NUM) ) {
		CAMCMP_LOG_ERR("camcmp_ius006f_correct_cmd input NULL! p_otp=0x%08X, num=%d\n", (int32_t)p_val, num);
		return -EINVAL;
	}

	p_otp = (int32_t*)p_val;

	p_reg = (camcmp_ius006f_reg_val_type*)
	        g_camcmp_ius006f_cmd_list[CAMCMP_CID_INIT_SET_3].p_send_cmd->p_data;

	p_reg[0].lwdata = ((p_otp[CAMCMP_OTP0H] & 0x00000003) << 14) |
	                   ((p_otp[CAMCMP_OTP0M] & 0xFFFC0000) >> 18);
	p_reg[0].lwdata -= 0x0600;


	p_reg[1].lwdata = ((p_otp[CAMCMP_OTP0H] & 0x0003FFFC) >> 2);


	p_reg[2].lwdata = ((p_otp[CAMCMP_OTP0M] & 0x0000003F) << 6) | ((p_otp[CAMCMP_OTP0L] & 0xFC000000) >> 26);


	p_reg[3].lwdata = ((p_otp[CAMCMP_OTP0M] & 0x0003FFC0) >> 6);

/* <004> add start */

	dac1m       = ((p_otp[CAMCMP_OTP1L] >> 5) & CAMCMP_OTP_DAC_MASK) * CAMCMP_CALC_100;
	dac10cm     = ((p_otp[CAMCMP_OTP0L] >> 16) & CAMCMP_OTP_DAC_MASK) * CAMCMP_CALC_100;
	vcm_slope   = ((p_otp[CAMCMP_OTP1H] >> 6) & CAMCMP_OTP_DAC_MASK) * CAMCMP_CALC_100;

	dac_overinf = ((g_p_camcmp_ius006f_af->fstep * vcm_slope / 100) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_1000)) / CAMCMP_CALC_10000; /* <009> add */

	opt_inf_pos = ((dac1m - ((dis_bet_pos * (dac10cm - dac1m) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_100)) / CAMCMP_CALC_1000)
	                - g_p_camcmp_ius006f_af->dacoffset) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_10)) / CAMCMP_CALC_100; /* <009> add */

	macro_pos = ((dac10cm - g_p_camcmp_ius006f_af->dacoffset) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_10)) / CAMCMP_CALC_100; /* <009> add */

	stand_pos = ((dac1m - g_p_camcmp_ius006f_af->dacoffset) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_10)) / CAMCMP_CALC_100; /* <009> add */

	af_drv_search_f = ((g_p_camcmp_ius006f_af->fstep * vcm_slope / 100) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_1000)) / CAMCMP_CALC_10000; /* <009> add */

	af_drv_search_s = ((g_p_camcmp_ius006f_af->sstep * vcm_slope / 100) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_1000)) / CAMCMP_CALC_10000; /* <009> add */

	low_search_area = opt_inf_pos - dac_overinf;

	high_search_area = (((macro_pos * CAMCMP_CALC_100) + g_p_camcmp_ius006f_af->dacovermacro) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_10)) / CAMCMP_CALC_100; /* <009> add */


	p_reg = (camcmp_ius006f_reg_val_type*)
	        g_camcmp_ius006f_cmd_list[CAMCMP_CID_INIT_SET_4].p_send_cmd->p_data;

	if( p_reg[g_camcmp_af_search_offset].waddr == 0x4844 ) {
		p_reg[g_camcmp_af_search_offset].lwdata = ((g_p_camcmp_ius006f_af->frange * vcm_slope / 100) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_1000)) / CAMCMP_CALC_10000; /* <009> add */
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_search_offset].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_search_offset].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_lenspos_on_afng].waddr == 0x486A ) {
		p_reg[g_camcmp_af_lenspos_on_afng].lwdata = opt_inf_pos;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_lenspos_on_afng].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_lenspos_on_afng].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_drv_amount_tonear_f].waddr == 0x486C ) {
		p_reg[g_camcmp_af_drv_amount_tonear_f].lwdata = af_drv_search_f;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_drv_amount_tonear_f].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_drv_amount_tonear_f].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_drv_amount_tonear_s].waddr == 0x486E ) {
		p_reg[g_camcmp_af_drv_amount_tonear_s].lwdata = af_drv_search_s;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_drv_amount_tonear_s].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_drv_amount_tonear_s].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_drv_amount_tofar_f].waddr == 0x4870 ) {
		p_reg[g_camcmp_af_drv_amount_tofar_f].lwdata = af_drv_search_f;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_drv_amount_tofar_f].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_drv_amount_tofar_f].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_drv_amount_tofar_s].waddr == 0x4872 ) {
		p_reg[g_camcmp_af_drv_amount_tofar_s].lwdata = af_drv_search_s;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_drv_amount_tofar_s].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_drv_amount_tofar_s].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_area_low_type1].waddr == 0x4876 ) {
		p_reg[g_camcmp_af_area_low_type1].lwdata = low_search_area;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_area_low_type1].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_area_low_type1].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_area_high_type1].waddr == 0x4878 ) {
		p_reg[g_camcmp_af_area_high_type1].lwdata = high_search_area;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_area_high_type1].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_area_high_type1].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_area_low_type2].waddr == 0x487A ) {
		p_reg[g_camcmp_af_area_low_type2].lwdata = low_search_area;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_area_low_type2].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_area_low_type2].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_area_high_type2].waddr == 0x487C ) {
		p_reg[g_camcmp_af_area_high_type2].lwdata = high_search_area;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_area_high_type2].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_area_high_type2].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_area_low_type3].waddr == 0x487E ) {
		p_reg[g_camcmp_af_area_low_type3].lwdata = low_search_area;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_area_low_type3].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_area_low_type3].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_area_high_type3].waddr == 0x4880 ) {
		p_reg[g_camcmp_af_area_high_type3].lwdata = high_search_area;
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_area_high_type3].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_area_high_type3].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_caf_param_wobble_step].waddr == 0x4822 ) {
		p_reg[g_camcmp_af_caf_param_wobble_step].lwdata = ((g_p_camcmp_ius006f_af->wstep * vcm_slope / 100) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_1000)) / CAMCMP_CALC_10000; /* <009> add */
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_caf_param_wobble_step].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_caf_param_wobble_step].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_caf_climb_step].waddr == 0x4824 ) {
		p_reg[g_camcmp_af_caf_climb_step].lwdata = ((g_p_camcmp_ius006f_af->cstep * vcm_slope / 100) + (CAMCMP_CALC_ROUND_OFF * CAMCMP_CALC_1000)) / CAMCMP_CALC_10000; /* <009> add */
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_caf_climb_step].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_caf_climb_step].waddr);
		return -EINVAL;
	}

	if( p_reg[g_camcmp_af_caf_wobble_max_move].waddr == 0x4838 ) {
		p_reg[g_camcmp_af_caf_wobble_max_move].lwdata = p_reg[g_camcmp_af_caf_param_wobble_step].lwdata * 2; /* <009> add */
	} else {
		CAMCMP_LOG_ERR("p_reg[g_camcmp_af_caf_wobble_max_move].waddr = %x\n", (int32_t)p_reg[g_camcmp_af_caf_wobble_max_move].waddr);
		return -EINVAL;
	}

	p_blk = (camcmp_ius006f_blank_val_type*)
	        g_camcmp_ius006f_cmd_list[CAMCMP_CID_MF_START].p_blank_data[CAMCMP_BID_MF_1M].p_data;
	p_blk->org_val = stand_pos;

	p_blk = (camcmp_ius006f_blank_val_type*)
	        g_camcmp_ius006f_cmd_list[CAMCMP_CID_MF_START].p_blank_data[CAMCMP_BID_MF_MACRO].p_data;
	p_blk->org_val = macro_pos;

	p_blk = (camcmp_ius006f_blank_val_type*)
	        g_camcmp_ius006f_cmd_list[CAMCMP_CID_MF_START].p_blank_data[CAMCMP_BID_MF_INF].p_data;
	p_blk->org_val = opt_inf_pos;
/* <004> add end */

	return 0;
}

static void camcmp_ius006f_get_version(uint16_t* p_ver, uint8_t* p_flg)
{

	/* <007> add start */
	if ( (p_ver == NULL) || (p_flg == NULL) ) {
		CAMCMP_LOG_ERR("input NULL! ver=%p, flg=%p\n", p_ver, p_flg);
		return;
	}
	/* <007> add end */

	*p_ver = *g_p_camcmp_ius006f_ver;
	*p_flg = 0;

}

int32_t camcmp_ius006f_i2c_init(void* p_client)
{
	g_p_camcmp_ius006f_i2c = (struct i2c_client*)p_client;
	return 0;
}

void camcmp_ius006f_get_info(camcmp_cmdmaker_func_type* p_func)
{
	p_func->make_cmd    = camcmp_ius006f_make_cmd;
	p_func->correct_cmd = camcmp_ius006f_correct_cmd;
	p_func->get_version = camcmp_ius006f_get_version;
	p_func->set_blank   = camcmp_ius006f_set_blank;
	p_func->send_cmd    = camcmp_ius006f_send_cmd;
}


