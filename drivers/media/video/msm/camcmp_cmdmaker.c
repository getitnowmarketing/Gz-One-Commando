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
#include <mach/camera.h>
#include "camcmp_cmdsender.h"
#include "camcmp_cmdmaker_ius006f.h"








static camcmp_makecmd_type* g_p_camcmp_cmd = NULL;

static camcmp_cmdmaker_func_type g_camcmp_cmd_func = {NULL, NULL, NULL, NULL, NULL};





int32_t camcmp_i2c_init(void* p_client)
{
	int32_t rc = 0;
	rc = camcmp_ius006f_i2c_init(p_client);
	return rc;
}

int32_t camcmp_makecmd_init(void)
{
	camcmp_ius006f_get_info(&g_camcmp_cmd_func);

	/* <004> add start */
	if (g_camcmp_cmd_func.make_cmd == NULL) {
		return -EFAULT;
	}
	/* <004> add end */

	g_p_camcmp_cmd = g_camcmp_cmd_func.make_cmd();
	if (g_p_camcmp_cmd == NULL) {
		CAMCMP_LOG_ERR("make_cmd failed! cmd=NULL\n");
		return -EFAULT;
	}
	return 0;
}

int32_t camcmp_makecmd_correction(void* p_val, uint32_t num)
{
/* <004> mod start */
	int32_t rc = 0;

	if (g_camcmp_cmd_func.correct_cmd != NULL) {
		rc = g_camcmp_cmd_func.correct_cmd(p_val, num);
	}
	return rc;
/* <004> mod end */
}

void camcmp_makecmd_get_version(uint16_t* p_ver, uint8_t* p_flg)
{
/* <004> mod start */
	if (g_camcmp_cmd_func.get_version != NULL) {
		g_camcmp_cmd_func.get_version(p_ver, p_flg);
	}
/* <004> mod end */
}

int32_t camcmp_isp_trans(camcmp_cmd_info_type* p_info, uint32_t num)
{
	int32_t rc;
	int32_t i, j;
	int32_t rev_cnt;

	camcmp_blank_info_type *p_in_blank;
	camcmp_makecmd_type    *p_cmd;
	camcmp_data_type       *p_data;

	if ( (p_info == NULL) || (num == 0) ) {
		CAMCMP_LOG_ERR("input NULL! pinf=0x%08X, num=%d\n", (int32_t)p_info, num);
		return -EINVAL;
	}

	if (g_p_camcmp_cmd == NULL) {
		CAMCMP_LOG_ERR("cmd failed! cmd=NULL\n");
		return -EFAULT;
	}

	for (i = 0; i < num; i++) {

		CAMCMP_LOG_DBG("cmd_id=%d, blk_num=%d, rev_num=%d\n",
		               p_info->cmd_id, p_info->blank_num, p_info->recieve_num);

		if ( p_info->cmd_id > CAMCMP_CID_MAX ) {
			CAMCMP_LOG_ERR("input cmd_id failed! id=%d\n", p_info->cmd_id);
			return -EINVAL;
		}

		if ( (p_info->blank_num > 0) && (p_info->p_blank_info == NULL) ) {
			CAMCMP_LOG_ERR("input blank failed! blk_num=%d\n", p_info->blank_num);
			return -EINVAL;
		}

		if ( (p_info->recieve_num > 0) && (p_info->p_recieve_data == NULL) ) {
			CAMCMP_LOG_ERR("input recieve failed! rev_num=%d\n", p_info->recieve_num);
			return -EINVAL;
		}

		p_cmd = (g_p_camcmp_cmd + p_info->cmd_id);

		if ( ((p_info->blank_num > 0) && (p_cmd->p_blank_data == NULL)) ||
		     ((p_cmd->p_blank_data != NULL) && (p_info->blank_num == 0)) )
		{
			CAMCMP_LOG_ERR("input blank table failed! blk_num=%d, blk_tbl=0x%08X\n",
			     p_info->blank_num, (int32_t)p_cmd->p_blank_data);
			return -EINVAL;
		}

		for (j = 0; j < p_info->blank_num; j++) {
			p_in_blank = (p_info->p_blank_info + j);

			if (p_in_blank->blank_id > p_cmd->blank_num) {
				CAMCMP_LOG_ERR("input blk_id failed! cmd_id=%d, blk_id=%d\n", p_info->cmd_id, p_in_blank->blank_id);
				return -EINVAL;
			}

			p_data = (p_cmd->p_blank_data + p_in_blank->blank_id);

			rc = g_camcmp_cmd_func.set_blank(*p_data, p_in_blank->p_blank_data);
			if (rc < 0) {
				CAMCMP_LOG_ERR("set_blank failed! cmd_id=%d, blk_id=%d rc=%d\n", p_info->cmd_id, p_in_blank->blank_id, rc);
				return rc;
			}
		}

		rev_cnt = 0;

		for (j = 0; j < p_cmd->send_cmd_num; j++) {
			p_data = (p_cmd->p_send_cmd + j);

			rc = g_camcmp_cmd_func.send_cmd(*p_data, p_info->p_recieve_data, &rev_cnt, p_info->recieve_num);
			if (rc < 0) {
				CAMCMP_LOG_ERR("send_cmd failed! cmd_id=%d, cnt=%d rc=%d\n", p_info->cmd_id, j, rc);
				return rc;
			}
		}

		p_info->recieve_num = rev_cnt;
		p_info++;
	}

	return 0;
}

