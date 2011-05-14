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


#ifndef CAMCMP_CMDMAKER_H
#define CAMCMP_CMDMAKER_H


#include <linux/types.h>
#include "camcmp_cmdsender.h"




typedef struct {
	void*			p_data;
	unsigned short	size;
} camcmp_data_type;

typedef struct {
	camcmp_data_type*	p_send_cmd;
	unsigned short		send_cmd_num;
	camcmp_data_type*	p_blank_data;
	unsigned short		blank_num;
} camcmp_makecmd_type;

typedef struct {
	camcmp_makecmd_type* (*make_cmd)(void);
	int32_t  (*correct_cmd)(void*, uint32_t);
	void     (*get_version)(uint16_t*, uint8_t*);
	int32_t  (*set_blank)(camcmp_data_type, void*);
	int32_t  (*send_cmd)(camcmp_data_type, void*, uint32_t*, uint32_t);
} camcmp_cmdmaker_func_type;




#endif /* CAMCMP_CMDMAKER_H */
