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


#ifndef CAMCMP_CMDMAKER_IUS006F_H
#define CAMCMP_CMDMAKER_IUS006F_H

#include <linux/types.h>
#include "camcmp_cmdmaker.h"



typedef enum {
	CAMCMP_REG_WRITE = 0,
	CAMCMP_REG_READ,
	CAMCMP_REG_POLL1,
	CAMCMP_REG_POLL2,
	CAMCMP_REG_POLL3 /* <001> */
} camcmp_cmd_kind_enum;

typedef enum {
	CAMCMP_D_LEN_BYTE  = 1,
	CAMCMP_D_LEN_WORD  = 2,
	CAMCMP_D_LEN_LWORD = 4
} camcmp_data_len_enum;


typedef struct {
	camcmp_cmd_kind_enum	cmd_kind;
	unsigned short			waddr;
	unsigned int			lwdata;
	unsigned char*			p_data;
	unsigned short			size;
} camcmp_ius006f_reg_val_type;

typedef struct {
	void*					p_copy;
	unsigned int			org_val;
	unsigned short			size;
} camcmp_ius006f_blank_val_type;

typedef struct {
/* <002> add start */
	uint16_t				z1m;
	uint16_t				z10cm;
	uint16_t				dacovermacro;
	uint16_t				dacoffset;
	uint16_t				fstep;
	uint16_t				sstep;
	uint16_t				frange;
	uint16_t				wstep;
	uint16_t				cstep;
 /* <002> add end */
} camcmp_ius006f_af_val_type;




int32_t camcmp_ius006f_i2c_init(void* p_client);

void camcmp_ius006f_get_info(camcmp_cmdmaker_func_type* p_func);


#endif 
