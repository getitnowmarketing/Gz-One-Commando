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


#include <linux/types.h>
#include "camcmp_cmdmaker_ius006f.h"


#define CAMCMP_REG(a)       CAMCOMBINE2(camcmp_reg_,a)
#define CAMCMP_CMD(a)       CAMCOMBINE2(camcmp_cmd_,a)
#define CAMCMP_BLK(a)       CAMCOMBINE2(camcmp_blk_,a)
#define CAMCMP_BTBL(a)      CAMCOMBINE2(camcmp_btbl_,a)
#define CAMCMP_POLL(a)      CAMCOMBINE2(camcmp_poll_,a)
#define CAMCMP_DAT(a)       CAMCOMBINE2(camcmp_dat_,a)

#define CAMCMP_SIZE_REG(a)  (sizeof(a)/sizeof(camcmp_ius006f_reg_val_type))
#define CAMCMP_SIZE_BLK(a)  (sizeof(a)/sizeof(camcmp_ius006f_blank_val_type))
#define CAMCMP_SIZE_DAT(a)  (sizeof(a)/sizeof(camcmp_data_type))

#define CAMCAMP_BLK_SIZE_DEF 1




static camcmp_ius006f_reg_val_type CAMCMP_REG(chk_dev_sts)[] =
{
	{CAMCMP_REG_POLL1, 0x00F8, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(init_set_1)[] =
{
	{CAMCMP_REG_WRITE, 0x02FA, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02FB, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02FC, 0x0000001B, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02FD, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02FE, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02FF, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0300, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0302, 0x000063CE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0304, 0x0005435D, NULL,                CAMCMP_D_LEN_LWORD},
	{CAMCMP_REG_WRITE, 0x0009, 0x0000001F, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(init_set_2)[] =
{
	{CAMCMP_REG_WRITE, 0x000B, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x033E, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4044, 0x000010E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4046, 0x00002A30, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4048, 0x000032A0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x402B, 0x0000004C, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x402C, 0x0000012F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4050, 0x0000025D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4060, 0x000010E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4062, 0x00002A30, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4064, 0x000032A0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A2, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A8, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0364, 0x00000027, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0366, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C14, 0x00000220, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C16, 0x00000220, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C18, 0x000001E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C1A, 0x000000E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C1C, 0x00000420, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C1E, 0x000002C0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x403E, 0x00001E59, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4040, 0x00001D4C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4074, 0x0000242D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4076, 0x00002320, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4078, 0x0000253A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x407A, 0x0000242D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02C7, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0360, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02B8, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02B9, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02BA, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02BB, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02BC, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02BD, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02BE, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02BF, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02C0, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02C1, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02C2, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02C3, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C10, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0348, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0349, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x034A, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x034B, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x034C, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x034D, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02C4, 0x0000010A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C86, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x444A, 0x00007F9F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x444C, 0x00008387, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0014, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x001C, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x001D, 0x0000001B, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x001E, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x001F, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0020, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0021, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0022, 0x00000280, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0024, 0x00000A20, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0026, 0x00000280, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0028, 0x000001E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x002A, 0x00000798, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x002C, 0x000001E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x021A, 0x00000280, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x021C, 0x000001E0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0032, 0x00000100, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0039, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4406, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x3201, 0x00000006, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0038, 0x0000000F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x003A, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x021E, 0x00001169, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0204, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0205, 0x00000046, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0206, 0x00000050, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0207, 0x0000005A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0208, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0209, 0x00000088, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x020A, 0x000003A7, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x020C, 0x0000078F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x020E, 0x00000B77, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0210, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0212, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0214, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0216, 0x00000014, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0217, 0x00000028, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0218, 0x00000028, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0011, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0380, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0381, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0382, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0383, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0384, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0385, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4001, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0224, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02C8, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0226, 0x00000D80, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02CE, 0x000007A0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0220, 0x00001109, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02CD, 0x00000080, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0222, 0x00009B9A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02D2, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D3, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D4, 0x00009D9C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0386, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(init_set_3)[] =
{
	{CAMCMP_REG_WRITE, 0x4A04, 0x00000F61, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A06, 0x0000121B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A08, 0x0000013F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A0A, 0x0000022E, NULL,                CAMCMP_D_LEN_WORD}
};

static unsigned char CAMCMP_DAT(rom_cor)[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(rom_correct)[] =
{
	{CAMCMP_REG_WRITE, 0xB000, 0x00000000, CAMCMP_DAT(rom_cor), sizeof(CAMCMP_DAT(rom_cor))},
};

static unsigned char CAMCMP_DAT(ae_frm)[] =
{
	0x05, 0x08, 0x0A, 0x0B, 0x0D, 0x0B, 0x0A, 0x08, 0x05, 0x08, 0x09, 0x0D, 0x0E, 0x10, 0x0E, 0x0D, 
	0x09, 0x08, 0x09, 0x0B, 0x17, 0x27, 0x3F, 0x27, 0x17, 0x0B, 0x09, 0x0A, 0x0D, 0x23, 0x52, 0x64, 
	0x52, 0x23, 0x0D, 0x0A, 0x0A, 0x0D, 0x23, 0x52, 0x64, 0x52, 0x23, 0x0D, 0x0A, 0x09, 0x0B, 0x17, 
	0x27, 0x32, 0x27, 0x17, 0x0B, 0x09, 0x05, 0x09, 0x0D, 0x11, 0x13, 0x11, 0x0D, 0x09, 0x05, 0x07, 
	0x0D, 0x0E, 0x10, 0x13, 0x10, 0x0E, 0x0D, 0x07, 0x0D, 0x10, 0x16, 0x1C, 0x1F, 0x1C, 0x16, 0x10, 
	0x0D, 0x12, 0x19, 0x21, 0x32, 0x47, 0x32, 0x21, 0x19, 0x12, 0x14, 0x1C, 0x2B, 0x52, 0x64, 0x52, 
	0x2B, 0x1C, 0x14, 0x14, 0x1C, 0x2B, 0x52, 0x64, 0x52, 0x2B, 0x1C, 0x14, 0x12, 0x19, 0x21, 0x32, 
	0x38, 0x32, 0x21, 0x19, 0x12, 0x11, 0x14, 0x18, 0x1C, 0x1F, 0x1C, 0x18, 0x14, 0x11, 0x03, 0x04, 
	0x07, 0x0B, 0x0E, 0x10, 0x0E, 0x0D, 0x07, 0x05, 0x05, 0x09, 0x14, 0x1B, 0x21, 0x1B, 0x14, 0x0D, 
	0x06, 0x08, 0x0E, 0x26, 0x47, 0x4E, 0x47, 0x26, 0x12, 0x07, 0x09, 0x13, 0x32, 0x58, 0x64, 0x58, 
	0x32, 0x14, 0x06, 0x08, 0x0E, 0x26, 0x47, 0x4E, 0x47, 0x26, 0x12, 0x05, 0x05, 0x09, 0x14, 0x1B, 
	0x21, 0x1B, 0x14, 0x0D, 0x03, 0x04, 0x07, 0x0B, 0x0E, 0x10, 0x0E, 0x0D, 0x07, 0x04, 0x05, 0x06, 
	0x08, 0x09, 0x08, 0x06, 0x05, 0x04, 0x06, 0x08, 0x09, 0x0B, 0x0C, 0x0B, 0x09, 0x08, 0x06, 0x09, 
	0x0E, 0x13, 0x19, 0x21, 0x19, 0x13, 0x0E, 0x09, 0x0C, 0x10, 0x1F, 0x32, 0x4E, 0x32, 0x1F, 0x10, 
	0x0C, 0x0D, 0x14, 0x43, 0x59, 0x64, 0x59, 0x43, 0x14, 0x0D, 0x0C, 0x10, 0x26, 0x32, 0x38, 0x32, 
	0x26, 0x10, 0x0C, 0x09, 0x0E, 0x13, 0x19, 0x1C, 0x19, 0x13, 0x0E, 0x09, 
};

static unsigned char CAMCMP_DAT(awb_frm)[] =
{
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x96, 0x78, 0x6C, 0x5F, 0x55, 0x45, 
	0x3D, 0x39, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30, 0x2F, 0x2E, 0x2D, 0x2C, 0x2B, 0x29, 
	0x25, 0x24, 0x23, 0x22, 0x21, 0x20, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAF, 0xAA, 
	0xB2, 0xB4, 0xB6, 0xB4, 0x9B, 0x82, 0x70, 0x6B, 0x68, 0x65, 0x61, 0x5D, 0x58, 0x53, 0x4E, 0x4A, 
	0x45, 0x40, 0x3D, 0x3A, 0x38, 0x36, 0x3B, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x1F, 0x09, 0x19, 0x10, 
	0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x78, 0x68, 0x51, 
	0x41, 0x3C, 0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30, 0x2F, 0x2E, 0x2C, 
	0x2B, 0x2A, 0x29, 0x28, 0x27, 0x26, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xD9, 0xB7, 0x96, 0x83, 0x76, 0x64, 0x66, 0x64, 0x62, 0x5F, 0x5B, 0x56, 0x51, 0x4C, 0x47, 0x43, 
	0x3F, 0x3B, 0x39, 0x37, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30, 0x2F, 0x2E, 0x1E, 0x0F, 0x18, 0x10, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xCB, 0xA9, 0x90, 0x7D, 0x70, 0x65, 
	0x5C, 0x55, 0x4F, 0x32, 0x30, 0x2F, 0x2D, 0x2B, 0x29, 0x27, 0x25, 0x23, 0x21, 0x1F, 0x1D, 0x1B, 
	0x1A, 0x2A, 0x29, 0x28, 0x27, 0x26, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xD3, 0xB1, 0x98, 0x85, 0x78, 0x6D, 0x64, 0x5D, 0x57, 0x63, 0x5E, 0x5A, 0x56, 0x52, 0x50, 0x4E, 
	0x4C, 0x4A, 0x48, 0x46, 0x44, 0x43, 0x42, 0x32, 0x31, 0x30, 0x2F, 0x2E, 0x1D, 0x14, 0x19, 0x10, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCD, 0xAB, 0x92, 0x7F, 0x72, 0x67, 
	0x5E, 0x57, 0x51, 0x4E, 0x49, 0x45, 0x46, 0x44, 0x41, 0x3E, 0x3D, 0x3B, 0x39, 0x32, 0x30, 0x2E, 
	0x2D, 0x2C, 0x2B, 0x2A, 0x29, 0x28, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0xD3, 0xB1, 0x98, 0x85, 0x78, 0x6D, 0x64, 0x5D, 0x57, 0x52, 0x4D, 0x49, 0x4A, 0x48, 0x45, 0x42, 
	0x41, 0x3F, 0x3D, 0x36, 0x34, 0x34, 0x33, 0x32, 0x31, 0x30, 0x2F, 0x2E, 0x1C, 0x16, 0x19, 0x16, 
};

static unsigned char CAMCMP_DAT(cxc_data)[] =
{
	0x01, 0x10, 0x04, 0x40, 0x10, 0x20, 0xc1, 0x80, 0x05, 0x03, 0x1a, 0x0c, 0x70, 0x20, 0xa0, 0x41, 
	0x00, 0x06, 0x01, 0x18, 0x04, 0x40, 0x10, 0x00, 0x41, 0x80, 0x04, 0x03, 0x16, 0x0c, 0x68, 0x30, 
	0xc0, 0x81, 0x80, 0x06, 0x01, 0x18, 0x04, 0x60, 0x00, 0x60, 0x01, 0x80, 0x05, 0x02, 0x10, 0x00, 
	0x40, 0x40, 0x20, 0x41, 0x00, 0x05, 0x01, 0x16, 0x08, 0x58, 0x20, 0x60, 0xc1, 0x00, 0x04, 0x03, 
	0x10, 0x08, 0x28, 0x20, 0xa0, 0x00, 0x80, 0x02, 0xff, 0x0f, 0x00, 0x40, 0x20, 0x40, 0x81, 0x00, 
	0x05, 0x02, 0x0a, 0x08, 0x28, 0x20, 0x80, 0xc0, 0x80, 0x01, 0x02, 0x08, 0x04, 0x20, 0x10, 0xc0, 
	0x80, 0x80, 0x04, 0x02, 0x12, 0x10, 0x20, 0x40, 0x80, 0xc0, 0x80, 0x01, 0x02, 0x00, 0x04, 0x00, 
	0x10, 0x00, 0x40, 0x80, 0x02, 0xff, 0x0d, 0xfc, 0x37, 0x10, 0x80, 0x40, 0x00, 0x02, 0x03, 0x04, 
	0x04, 0x08, 0x10, 0x20, 0x80, 0x80, 0x00, 0x01, 0x06, 0x00, 0x38, 0x00, 0xe0, 0x80, 0x00, 0x02, 
	0x02, 0x08, 0x0c, 0x08, 0x20, 0x20, 0x80, 0x80, 0x00, 0x02, 0x04, 0x08, 0x10, 0x30, 0xe0, 0xc0, 
	0x80, 0x03, 0xfe, 0x07, 0xf8, 0x1f, 0x40, 0x60, 0x00, 0x01, 0x01, 0x04, 0x06, 0x04, 0x10, 0x30, 
	0xa0, 0x00, 0x01, 0x03, 0x04, 0x0c, 0x0c, 0x18, 0x30, 0x60, 0x40, 0x80, 0x02, 0x02, 0x0a, 0x10, 
	0x18, 0x30, 0xa0, 0x00, 0x01, 0x02, 0x04, 0x10, 0x10, 0x40, 0x30, 0x80, 0xc0, 0x00, 0x02, 0x04, 
	0x0a, 0x10, 0x30, 0x40, 0xe0, 0x00, 0x01, 0x03, 0x03, 0x0c, 0x10, 0x38, 0x40, 0xe0, 0xc0, 0x00, 
	0x02, 0x03, 0x08, 0x10, 0x28, 0x40, 0xc0, 0x00, 0x81, 0x03, 0x04, 0x0c, 0x0c, 0x30, 0x40, 0xe0, 
	0x00, 0x81, 0x03, 0x05, 0xec, 0x17, 0xb0, 0x4f, 0x20, 0xbf, 0x80, 0xfc, 0x01, 0xf2, 0x0b, 0xd8, 
	0x1f, 0x40, 0x7f, 0x00, 0x00, 0x01, 0x00, 0x14, 0xb0, 0x5f, 0xc0, 0x3e, 0x81, 0xfc, 0x02, 0xf2, 
	0x07, 0xc8, 0x2f, 0x60, 0x7f, 0x00, 0xfd, 0x01, 0x00, 0x04, 0x00, 0x40, 0x60, 0x3e, 0x81, 0xf9, 
	0x02, 0xf2, 0x0b, 0xd8, 0x0f, 0x20, 0x3f, 0x00, 0xfb, 0x01, 0xf0, 0x07, 0xd0, 0x1f, 0x40, 0x3f, 
	0x81, 0xfa, 0x04, 0xea, 0x0b, 0xc0, 0x0f, 0x40, 0x3f, 0x00, 0xfd, 0xff, 0xf1, 0xfb, 0xc7, 0x0f, 
	0x80, 0x3f, 0x00, 0xfe, 0x05, 0xf0, 0x17, 0xc0, 0x2f, 0x00, 0x3f, 0x00, 0xfc, 0xff, 0xfb, 0xff, 
	0xdf, 0x0f, 0xa0, 0xff, 0xff, 0xfe, 0xff, 0xfb, 0x13, 0xd0, 0x4f, 0x40, 0x3f, 0x81, 0xfc, 0x01, 
	0xf6, 0x03, 0xe0, 0x0f, 0xe0, 0xff, 0x7f, 0xff, 0x00, 0xfa, 0x03, 0xe8, 0x5f, 0x80, 0x7f, 0x01, 
	0xfe, 0x04, 0xfa, 0x0b, 0xf8, 0x1f, 0x00, 0x40, 0x80, 0x00, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xdf, 
	0xbf, 0x81, 0xfe, 0x06, 0xfa, 0x1b, 0x10, 0x30, 0x60, 0x80, 0x80, 0x00, 0x00, 0x08, 0x0c, 0x18, 
	0x20, 0xc0, 0x80, 0x00, 0x03, 0x09, 0xfe, 0x27, 0xf8, 0x8f, 0x80, 0x00, 0x01, 0x01, 0x04, 0x1a, 
	0x08, 0x30, 0x40, 0xc0, 0x40, 0x81, 0x03, 0x05, 0x0e, 0x24, 0x28, 0x90, 0xa0, 0x40, 0x02, 0x02, 
	0x07, 0x0a, 0x20, 0x30, 0x50, 0x20, 0x81, 0x81, 0x04, 0x05, 0x12, 0x14, 0x48, 0x90, 0x00, 0x41, 
	0x02, 0x04, 0x07, 0x16, 0x1c, 0x40, 0x80, 0x00, 0x41, 0x01, 0x05, 0x06, 0x12, 0x18, 0x28, 0x60, 
	0xa0, 0x40, 0x02, 0x04, 0x09, 0x10, 0x1c, 0x58, 0x70, 0x00, 0x01, 0x02, 0x04, 0x05, 0x14, 0x18, 
	0x48, 0x60, 0xa0, 0x80, 0x81, 0x02, 
};

static unsigned char CAMCMP_DAT(shd_data)[] =
{
	0x24, 0x7a, 0x54, 0x7d, 0xa6, 0x84, 0x1c, 0x0e, 0xed, 0x26, 0x42, 0xb6, 0x79, 0x90, 0x6f, 0x3a, 
	0xa4, 0x1c, 0x16, 0x81, 0xa7, 0x48, 0xe9, 0x89, 0x12, 0x7f, 0xbc, 0xc4, 0x1d, 0x19, 0xf5, 0x46, 
	0x41, 0xaf, 0xe9, 0x4f, 0x6a, 0xf0, 0x33, 0x9b, 0x03, 0x09, 0x87, 0x43, 0xd9, 0xb9, 0x11, 0x78, 
	0x86, 0x44, 0x9c, 0x0e, 0xc5, 0xa6, 0x3f, 0x8c, 0x31, 0x4e, 0x5d, 0x50, 0xf3, 0x16, 0xd0, 0xd8, 
	0x85, 0x35, 0x90, 0x89, 0x0e, 0x6e, 0x0a, 0x44, 0x1c, 0x0c, 0x99, 0x26, 0x3d, 0x8b, 0x11, 0xce, 
	0x57, 0x04, 0xe3, 0x93, 0xab, 0xd0, 0x84, 0x29, 0x3d, 0xc9, 0x8a, 0x57, 0x0e, 0xe3, 0x18, 0xe4, 
	0xa0, 0xc6, 0x3d, 0x8c, 0xf9, 0xcd, 0x5a, 0x24, 0x93, 0x93, 0xa7, 0x68, 0xa4, 0x24, 0x0e, 0xb1, 
	0x48, 0x46, 0x4a, 0x82, 0x13, 0xa8, 0xac, 0xe5, 0x32, 0x8b, 0x21, 0xce, 0x5f, 0x58, 0xb3, 0x15, 
	0xbe, 0x9c, 0xc4, 0x26, 0x09, 0x61, 0x88, 0x3f, 0xfc, 0x91, 0x90, 0x85, 0x98, 0xc4, 0x26, 0x59, 
	0xe1, 0xcb, 0x5e, 0x58, 0xf3, 0x97, 0xd8, 0x70, 0xe5, 0x2f, 0x2a, 0xc9, 0xc9, 0x42, 0x20, 0x12, 
	0x90, 0x80, 0x34, 0xe4, 0x21, 0x2a, 0xc1, 0x49, 0x57, 0x02, 0xf3, 0x17, 0xd9, 0x40, 0x06, 0x39, 
	0x71, 0xd9, 0x8c, 0x50, 0xac, 0x32, 0x92, 0x96, 0x5c, 0xc4, 0x23, 0x23, 0x61, 0x89, 0x50, 0xac, 
	0x62, 0x17, 0xd0, 0x58, 0xa6, 0x39, 0xab, 0x59, 0xcf, 0x64, 0x94, 0xc3, 0x16, 0xc8, 0x30, 0xa5, 
	0x2c, 0x43, 0xc1, 0xca, 0x52, 0xc6, 0xe2, 0x16, 0xc9, 0x78, 0x86, 0x3a, 0xb6, 0xb9, 0x4f, 0x74, 
	0x34, 0xe4, 0x1b, 0x01, 0x6d, 0x06, 0x3b, 0x85, 0xc9, 0x0d, 0x60, 0x5c, 0x73, 0x98, 0xdc, 0x84, 
	0x66, 0x3b, 0xc8, 0x99, 0x50, 0x75, 0x4c, 0x44, 0xa0, 0x2f, 0xa1, 0x87, 0x47, 0xcc, 0xd1, 0x90, 
	0x6f, 0x18, 0x74, 0x1b, 0x02, 0x05, 0xc7, 0x41, 0xd2, 0x19, 0x91, 0x7a, 0x84, 0x04, 0x9f, 0x25, 
	0x99, 0x88, 0x50, 0xf8, 0x99, 0xd2, 0x74, 0x50, 0x64, 0x9c, 0x0d, 0x05, 0xa7, 0x42, 0xca, 0xf9, 
	0xd0, 0x75, 0x5a, 0xd4, 0x1e, 0x24, 0xb5, 0xe7, 0x48, 0xfb, 0x29, 0x51, 0x75, 0xec, 0xd3, 0x9a, 
	0xe6, 0x8c, 0x26, 0x38, 0x9f, 0xd9, 0xcd, 0x6a, 0x88, 0x73, 0x1b, 0xe9, 0x28, 0x87, 0x3d, 0xd2, 
	0xa9, 0x0f, 0x76, 0xf6, 0xc3, 0x1b, 0xee, 0x8c, 0x26, 0x38, 0x99, 0xa1, 0x0d, 0x65, 0x5a, 0x13, 
	0x9a, 0xdc, 0xbc, 0x46, 0x39, 0xc4, 0x19, 0x4f, 0x72, 0xd6, 0x33, 0x9a, 0xe0, 0x50, 0x06, 0x36, 
	0x76, 0x81, 0x8c, 0x59, 0xf6, 0x22, 0x96, 0xb9, 0xb0, 0x65, 0x2f, 0x82, 0xa9, 0xcc, 0x69, 0x7c, 
	0x03, 0x9b, 0xe4, 0x14, 0x06, 0x34, 0x71, 0x41, 0xcc, 0x53, 0xc0, 0x52, 0x93, 0xa0, 0xc4, 0x24, 
	0x27, 0x39, 0x09, 0x8a, 0x55, 0xc6, 0xe2, 0x97, 0xc8, 0x44, 0x26, 0x35, 0x6f, 0x41, 0x8c, 0x55, 
	0xd0, 0xf2, 0x92, 0x9c, 0x54, 0x64, 0x23, 0x0e, 0x89, 0x88, 0x46, 0x3a, 0x42, 0x93, 0x9d, 0x74, 
	0xa5, 0x2d, 0x74, 0x51, 0x4c, 0x58, 0xf2, 0x72, 0x94, 0xab, 0x80, 0xc4, 0x24, 0x07, 0x49, 0xc8, 
	0x3f, 0xfe, 0xb1, 0x10, 0x85, 0x90, 0xe4, 0x24, 0x4d, 0xc9, 0xca, 0x59, 0xf2, 0x42, 0x96, 0xbc, 
	0x24, 0x05, 0x2b, 0x21, 0x41, 0x09, 0x42, 0x16, 0x02, 0x10, 0x80, 0x2c, 0x64, 0x21, 0x24, 0x41, 
	0x09, 0x53, 0xb4, 0x52, 0x96, 0xbd, 0xd8, 0x25, 0x31, 0x5e, 0x59, 0x8b, 0x4d, 0x7c, 0xc2, 0x91, 
	0x90, 0x44, 0x84, 0x22, 0x1c, 0x01, 0xc9, 0x4d, 0x7e, 0xf2, 0x95, 0xb9, 0xd8, 0x45, 0x32, 0x94, 
	0x29, 0x4d, 0x60, 0x1c, 0xd3, 0x15, 0xb3, 0xfc, 0x04, 0x29, 0x37, 0x09, 0x4a, 0x4f, 0x92, 0xa2, 
	0x95, 0xb5, 0xfc, 0x65, 0x33, 0x91, 0x99, 0x8d, 0x6d, 0x98, 0x63, 0x1a, 0xdd, 0x24, 0x26, 0x33, 
	0x74, 0x21, 0x0c, 0x5c, 0xfe, 0x22, 0x17, 0xc4, 0x10, 0x26, 0x34, 0xa3, 0x41, 0x0e, 0x6b, 0xa6, 
	0x43, 0x1e, 0x00, 0x2d, 0xe7, 0x3c, 0xb2, 0x69, 0xce, 0x69, 0x82, 0x13, 0x9a, 0xdd, 0x98, 0xc6, 
	0x38, 0xb0, 0xa9, 0x4e, 0x70, 0xce, 0x53, 0x1c, 0xf5, 0xfc, 0x27, 0x44, 0xda, 0xb9, 0x0f, 0x6e, 
	0xaa, 0xe3, 0x9a, 0xe5, 0xa8, 0xe6, 0x38, 0xad, 0x81, 0x8e, 0x6d, 0xb8, 0x63, 0x9c, 0xf4, 0x14, 
	0x07, 0x3d, 0x6a, 0x9a, 0xd1, 0x7e, 0xdc, 0xd3, 0x1e, 0xfc, 0x10, 0x68, 0x43, 0x26, 0xe2, 0x51, 
	0x84, 0xd4, 0xa3, 0x9d, 0xea, 0x88, 0xa7, 0x3e, 0x0e, 0xba, 0x10, 0x7f, 0xba, 0xa3, 0x9a, 0xc6, 
	0x14, 0x06, 0x32, 0xb0, 0x09, 0x4f, 0x7c, 0x98, 0x73, 0x1a, 0xb7, 0x20, 0xc5, 0x27, 0x48, 0x79, 
	0x4b, 0x6a, 0x96, 0x73, 0x9a, 0xbe, 0x08, 0xc5, 0x23, 0x11, 0xe9, 0x48, 0x50, 0xfc, 0x72, 0x1a, 
	0xcb, 0xa8, 0xc5, 0x25, 0x0a, 0xf1, 0x47, 0x42, 0x5a, 0xa2, 0x16, 0xca, 0x64, 0xa6, 0x2d, 0x31, 
	0x69, 0x48, 0x40, 0x18, 0x02, 0x13, 0xb7, 0x64, 0x06, 0x36, 0x86, 0x51, 0x8a, 0x49, 0x32, 0x62, 
	0x12, 0xa5, 0x24, 0x46, 0x36, 0xd4, 0x91, 0x4d, 0x5f, 0xae, 0xc2, 0x14, 0xab, 0xf8, 0x25, 0x37, 
	0xd9, 0x09, 0x50, 0x7a, 0x7a, 0x13, 0x9a, 0xcb, 0x80, 0xe6, 0x37, 0xf3, 0x29, 0x50, 0x90, 0x3e, 
	0xd4, 0x9f, 0xf6, 0x90, 0xc7, 0x3d, 0x01, 0xf2, 0x50, 0x89, 0xcc, 0x34, 0xa3, 0x03, 0xe5, 0xa7, 
	0x3e, 0xfd, 0x51, 0xd0, 0x88, 0x44, 0x04, 0x13, 0xbc, 0x04, 0x10, 0xa6, 0xd8, 0xca, 0xd4, 0x96, 
	0x96, 0x74, 0xa4, 0x2b, 0x99, 0x09, 0x50, 0x8e, 0x2a, 0x15, 0x9d, 0x88, 0x24, 0x23, 0x15, 0xed, 
	0xa8, 0x4a, 0x75, 0x0a, 0x14, 0x96, 0x66, 0x14, 0x1f, 0xe5, 0xf8, 0xa6, 0x39, 0xf9, 0xe1, 0x51, 
	0x95, 0x38, 0xb4, 0x9e, 0xce, 0xa0, 0x05, 0x2b, 0x68, 0xf9, 0x0c, 0x7c, 0x42, 0xc4, 0x9e, 0xd8, 
	0x74, 0x65, 0x25, 0x19, 0x51, 0x49, 0x57, 0x68, 0x03, 0x9f, 0xea, 0x54, 0x06, 0x28, 0x0e, 0xf1, 
	0x47, 0x43, 0x7c, 0x52, 0x19, 0xeb, 0x68, 0x47, 0x33, 0x43, 0x91, 0x48, 0x40, 0x22, 0x22, 0x94, 
	0xcd, 0x70, 0xe7, 0x3e, 0xbd, 0x39, 0x4b, 0x4d, 0x46, 0x42, 0x93, 0xb3, 0x04, 0xe7, 0x3f, 0x25, 
	0xca, 0xcf, 0x6b, 0xf2, 0x92, 0x16, 0xbc, 0xc4, 0x86, 0x40, 0x33, 0xfa, 0x52, 0x90, 0x10, 0x24, 
	0x1e, 0xea, 0x88, 0xa7, 0x41, 0x53, 0x52, 0x13, 0xab, 0x0a, 0xc5, 0xa5, 0x24, 0xf9, 0x88, 0x49, 
	0x65, 0x42, 0x94, 0xa4, 0xa2, 0xc5, 0x29, 0x36, 0x75, 0xa9, 0x4a, 0x61, 0x82, 0xd3, 0xa2, 0x10, 
	0x85, 0x22, 0xfe, 0x2c, 0x27, 0x37, 0x28, 0xe2, 0xcf, 0x72, 0x72, 0x23, 0x9b, 0xe0, 0x38, 0xc7, 
	0x3c, 0xf4, 0xe9, 0x0f, 0x77, 0x7a, 0xd3, 0x9a, 0xd2, 0xd0, 0xa6, 0x38, 0xde, 0x31, 0x0f, 0x70, 
	0x5a, 0xa3, 0x18, 0xbb, 0xbc, 0x25, 0x2f, 0x93, 0xd9, 0x8d, 0x71, 0x36, 0x63, 0x98, 0xae, 0xfc, 
	0x04, 0x27, 0x40, 0x01, 0x0b, 0x63, 0x48, 0x33, 0x18, 0xb4, 0xe8, 0x64, 0x23, 0x11, 0xe9, 0xc8, 
	0x4e, 0xd0, 0x22, 0x18, 0xb9, 0x58, 0x25, 0x25, 0x0a, 0xf9, 0x07, 0x43, 0x4e, 0x62, 0x15, 0xba, 
	0xd4, 0x05, 0x2b, 0x2a, 0x59, 0x08, 0x40, 0x16, 0x72, 0x12, 0xaa, 0xc8, 0x25, 0x31, 0x6e, 0x09, 
	0x0a, 0x48, 0x24, 0xd2, 0x91, 0x9e, 0xb0, 0x05, 0x31, 0xac, 0xb1, 0x0c, 0x5b, 0x8e, 0xd2, 0x93, 
	0xa1, 0x98, 0x65, 0x32, 0xa9, 0x89, 0x8e, 0x6f, 0x36, 0x23, 0x98, 0xbe, 0x00, 0xe6, 0x32, 0xbb, 
	0x41, 0x4e, 0x81, 0xce, 0x63, 0x9c, 0xdc, 0xcc, 0xe6, 0x36, 0xc7, 0xe1, 0x4e, 0x79, 0x44, 0xb4, 
	0x1f, 0xe7, 0x08, 0xe7, 0x37, 0xc2, 0x71, 0xce, 0x78, 0xce, 0x03, 0x03, 0x18, 0xc4, 0x80, 0x0b, 
};

static unsigned char CAMCMP_DAT(ap_gain)[] =
{
	0x30, 0x30, 0x30, 0x38, 0x38, 0x38, 0x00, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, 0x66, 0x66, 0x00, 
	0x00, 0x40, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x10, 0x10, 
	0x10, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 
	0x10, 0x10, 0x10, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x50, 0x50, 0x20, 0x00, 
	0x00, 0x00, 0x20, 0x20, 0x20, 0x50, 0x50, 0x20, 0x00, 0x00, 0x00, 0x20, 0x20, 0x30, 0x50, 0x50, 
	0x20, 0x80, 0x80, 0x40, 0x40, 0x40, 0x64, 0x64, 0x64, 0x0A, 0x16, 0x16, 0x21, 0x17, 0x0A, 0x21, 
	0x17, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(init_set_4)[] =
{
	{CAMCMP_REG_WRITE, 0x4200, 0x00000000, CAMCMP_DAT(ae_frm),  sizeof(CAMCMP_DAT(ae_frm))},
	{CAMCMP_REG_WRITE, 0x4600, 0x00000000, CAMCMP_DAT(awb_frm), sizeof(CAMCMP_DAT(awb_frm))},
	{CAMCMP_REG_WRITE, 0x0068, 0x00000050, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x9000, 0x00008282, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x9002, 0x00000000, CAMCMP_DAT(cxc_data),sizeof(CAMCMP_DAT(cxc_data))},
	{CAMCMP_REG_WRITE, 0xA000, 0x00009191, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0xA002, 0x00000000, CAMCMP_DAT(shd_data),sizeof(CAMCMP_DAT(shd_data))},
	{CAMCMP_REG_WRITE, 0x4A1A, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A1C, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A1E, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A20, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0068, 0x0000005F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5000, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5002, 0x0000FFE6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5004, 0x0000FFE9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5006, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5008, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x500A, 0x0000FFEF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x500C, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x500E, 0x0000FFE6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5010, 0x0000FFE9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5012, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5014, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5016, 0x0000FFEF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5018, 0x0000FFCB, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x501A, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x501C, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x501E, 0x0000FFF9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5020, 0x00000008, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5022, 0x0000FFDB, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5024, 0x0000FFB3, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5026, 0x0000FFF1, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5028, 0x0000FFF5, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x502A, 0x0000FFFB, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x502C, 0x00000007, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x502E, 0x0000FF98, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5030, 0x0000FFF0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5032, 0x0000FFF3, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5034, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5036, 0x0000FFFA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5038, 0x0000FFEE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x503A, 0x0000FFB8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x503C, 0x0000FFF0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x503E, 0x00000002, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5040, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5042, 0x0000FFF8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5044, 0x0000FFF2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5046, 0x0000FFC0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5048, 0x0000FFF8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x504A, 0x0000FFDC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x504C, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x504E, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5050, 0x0000FFF6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5052, 0x0000FFD8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5054, 0x00000023, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5055, 0x0000001D, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5056, 0x0000002A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5057, 0x0000001B, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5058, 0x00000063, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5059, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x505A, 0x00000075, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x505B, 0x0000000D, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x505C, 0x00000099, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x505D, 0x0000000B, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x505E, 0x0000004A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x505F, 0x00000011, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5060, 0x00000044, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5061, 0x0000000F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5092, 0x0000FFFC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5094, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5096, 0x0000FFFC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5098, 0x0000FFF2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x509A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x509C, 0x0000FFF2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x509E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50A0, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50A2, 0x0000005F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A3, 0x00000057, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A4, 0x0000005F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A5, 0x00000067, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A6, 0x00000066, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A7, 0x00000067, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A8, 0x00000066, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50A9, 0x00000057, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50AA, 0x0000FFF8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50AC, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50AE, 0x0000FFF8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50B0, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50B2, 0x0000FFF8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50B4, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50B6, 0x0000FFF8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50B8, 0x0000FFEA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x50BA, 0x00000053, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50BB, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50BC, 0x00000053, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50BD, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50BE, 0x00000053, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50BF, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50C0, 0x00000053, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50C1, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50C2, 0x00000019, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x50C3, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0105, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5236, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5238, 0x0000000A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x523A, 0x00000013, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x523C, 0x0000001C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x523E, 0x00000025, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5240, 0x0000002D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5242, 0x00000035, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5244, 0x0000003D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5246, 0x00000045, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5248, 0x0000004B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x524A, 0x00000053, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x524C, 0x00000058, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x524E, 0x0000005F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5250, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5252, 0x0000006A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5254, 0x0000006F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5256, 0x00000073, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5258, 0x00000077, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x525A, 0x00000053, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x525C, 0x00000098, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x525E, 0x000000CA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5260, 0x000000DE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5262, 0x000000EB, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5264, 0x000000F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5266, 0x000000FB, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5268, 0x00000104, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x526A, 0x00000109, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52A2, 0x00000510, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52A4, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52A6, 0x00000510, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52A8, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52AA, 0x00000510, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52AC, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x52AE, 0x00001C80, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5436, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5438, 0x0000000A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x543A, 0x00000014, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x543C, 0x00000020, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x543E, 0x00000026, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5440, 0x00000034, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5442, 0x00000038, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5444, 0x00000044, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5446, 0x0000004A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5448, 0x00000054, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x544A, 0x00000058, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x544C, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x544E, 0x00000066, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5450, 0x00000070, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5452, 0x00000072, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5454, 0x0000007C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5456, 0x0000007E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5458, 0x00000086, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x545A, 0x0000005B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x545C, 0x000000A9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x545E, 0x000000CF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5460, 0x000000E3, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5462, 0x000000F1, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5464, 0x000000F7, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5466, 0x00000101, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5468, 0x00000103, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x546A, 0x0000010D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54A2, 0x00000410, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54A4, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54A6, 0x00000410, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54A8, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54AA, 0x00000410, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54AC, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x54AE, 0x00002300, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5636, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5638, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x563A, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x563C, 0x00000006, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x563E, 0x0000000C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5640, 0x00000015, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5642, 0x0000001D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5644, 0x00000027, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5646, 0x00000031, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5648, 0x0000003A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x564A, 0x00000043, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x564C, 0x0000004B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x564E, 0x00000053, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5650, 0x0000005B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5652, 0x00000063, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5654, 0x0000006A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5656, 0x00000071, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5658, 0x0000007B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x565A, 0x0000003E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x565C, 0x000000AC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x565E, 0x000000D4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5660, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5662, 0x000000F2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5664, 0x000000F8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5666, 0x000000FE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5668, 0x00000101, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x566A, 0x00000104, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56A2, 0x00001900, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56A4, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56A6, 0x00001900, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56A8, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56AA, 0x00001900, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56AC, 0x00004005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x56AE, 0x00002300, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0105, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0387, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4A00, 0x00000110, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A02, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x400E, 0x00002400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4012, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4014, 0x00000F27, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4016, 0x00002C13, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4018, 0x00000331, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401C, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4020, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4036, 0x00000331, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x403A, 0x0000000A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4042, 0x00000016, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4043, 0x00000016, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x404C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x404D, 0x000000A0, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x404E, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4080, 0x00000064, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4081, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4082, 0x00000035, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4083, 0x00000026, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4084, 0x00000005, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4085, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4093, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4098, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4099, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x409A, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x409B, 0x00000064, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x409C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x409D, 0x00000007, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x409E, 0x00000006, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x409F, 0x00000062, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x40A0, 0x00000064, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4806, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4807, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x480E, 0x000000FA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4810, 0x000000FA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4814, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4816, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4817, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4818, 0x0000000B, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4819, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x481A, 0x00000008, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x481C, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x481D, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4844, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4856, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4857, 0x00000005, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4864, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4866, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x486A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x486C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x486E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4870, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4872, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4874, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4876, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4878, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x487A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x487C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x487E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4880, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x488C, 0x00000016, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4890, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4891, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4892, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4894, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4896, 0x000003FF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4898, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x490C, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x490E, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4910, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4924, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4926, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4928, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x493C, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x493E, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4940, 0x00000200, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4954, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4956, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4958, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x495A, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x495B, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4964, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4966, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4968, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x496E, 0x000003E8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4970, 0x000007D0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4972, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4822, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4824, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4400, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4401, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4402, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4403, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4404, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4405, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4408, 0x00000100, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x440A, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x440C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x440D, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x440E, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x440F, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4410, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4411, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4412, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4413, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4414, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4415, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4416, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4417, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4418, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4419, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x441A, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x441B, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x441C, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x441D, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x441E, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x441F, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4420, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4421, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4422, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4423, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4424, 0x0000000C, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4425, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4426, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4428, 0x0000FFFF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x442A, 0x00000D2D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x442C, 0x000013F6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x442E, 0x00000C7C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4430, 0x00001775, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4432, 0x00001000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4434, 0x00001000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4436, 0x00000B81, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4438, 0x00001832, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x443A, 0x00000C03, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x443C, 0x0000179E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x443E, 0x00000ACD, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4440, 0x00001AD6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4442, 0x00000C80, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4444, 0x00001300, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4446, 0x000010EF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4448, 0x00000F32, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x444E, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x444F, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4450, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4452, 0x0000001E, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4453, 0x0000007B, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4454, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4456, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4458, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x445A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x445C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x445E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4460, 0x00000300, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4462, 0x00000010, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4464, 0x000003C2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4466, 0x000003C2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4468, 0x000003C2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x446A, 0x00000014, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x446B, 0x00000005, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x446C, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x446D, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x446E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4470, 0x00000005, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4471, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4A22, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A24, 0x0000000E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A26, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A28, 0x0000FFF2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A2A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4A2C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E40, 0x0000154A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E42, 0x000013EC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E44, 0x0000114E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E46, 0x00001004, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E48, 0x000011F8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E4A, 0x000010CC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E4C, 0x00006480, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E4E, 0x00005C80, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E50, 0x00000FA0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E52, 0x00000E74, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E54, 0x00000A28, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E56, 0x000008FC, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E58, 0x00001B58, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E5A, 0x00001A2C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F5C, 0x0000000F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4002, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x402A, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x402E, 0x0000000D, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x402F, 0x0000000F, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x404B, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x404F, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4808, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x480A, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4826, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4828, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x482A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x482C, 0x0000001E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x482E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4830, 0x0000001E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4832, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4834, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4836, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4838, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x483A, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x483B, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x483C, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x483D, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x483E, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x483F, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4840, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4841, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4842, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4846, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4847, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4848, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4849, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x484A, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x484B, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x484C, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x484D, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x484E, 0x00000008, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x484F, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4852, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4854, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4855, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4858, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4859, 0x00000005, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x485A, 0x00002710, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4868, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4884, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4886, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4887, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4888, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4969, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x496A, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x496B, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x496C, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C20, 0x00000489, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C22, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C24, 0x00000345, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C26, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C2C, 0x000004CE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C2E, 0x00000141, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C30, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C32, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C34, 0x000004BA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C36, 0x0000012D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C38, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C3A, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C3C, 0x000002B2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C3E, 0x0000035D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C40, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C42, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C44, 0x0000029E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C46, 0x00000349, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C48, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C4A, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C4C, 0x000004CE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C4E, 0x0000035D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C50, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C52, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C54, 0x000004BA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C56, 0x00000349, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C58, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C5A, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C5C, 0x000006EA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C5E, 0x0000035D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C60, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C62, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C64, 0x000006D6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C66, 0x00000349, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C68, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C6A, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C6C, 0x000004CE, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C6E, 0x00000579, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C70, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C72, 0x000000E6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C74, 0x000004BA, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C76, 0x00000565, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C78, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C7A, 0x0000010E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4C7C, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C7D, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C7E, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C7F, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C80, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C81, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C82, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C83, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C84, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4C85, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E00, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E02, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E04, 0x00000004, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E06, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E07, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E08, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E0A, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E0B, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E0C, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E0E, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E10, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E12, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E14, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E1C, 0x000003FF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E1E, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E1F, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E20, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E21, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x6E8E, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E90, 0x0000012C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E92, 0x00000064, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x6E94, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F24, 0x00000032, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F25, 0x00000009, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F2C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F2E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F30, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F32, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F34, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F36, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4F38, 0x00000008, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F39, 0x0000000E, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F3A, 0x00000014, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F3B, 0x0000000A, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F3C, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F3D, 0x00000016, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F3E, 0x00000008, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F3F, 0x00000018, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F40, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F41, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x5858, 0x00000B29, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x585A, 0x0000072D, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x585C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5888, 0x00000014, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x588A, 0x00000014, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x588C, 0x00000014, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x588E, 0x00000040, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5890, 0x00000040, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5892, 0x00000040, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5894, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5896, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5898, 0x00000002, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x589A, 0x00000009, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x589C, 0x00000007, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x589E, 0x00000006, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58A0, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58A2, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58A4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58A6, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58A8, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58AA, 0x00000002, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58AC, 0x00000009, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58AE, 0x00000007, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58B0, 0x00000006, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58B2, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58B4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58B6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58B8, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58BA, 0x00000004, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58BC, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58BE, 0x00000007, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58C0, 0x00000008, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58C2, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58C4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58C6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58C8, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58CA, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58CC, 0x00000004, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58CE, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58D0, 0x00000007, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58D2, 0x00000008, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58D4, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58D6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58D8, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58DA, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58DC, 0x00000005, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58DE, 0x00000004, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58E0, 0x00000002, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58E2, 0x00000009, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58E4, 0x00000008, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58E6, 0x00000006, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58E8, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58EA, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58EC, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58EE, 0x00000009, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58F0, 0x00000004, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58F2, 0x00000002, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58F4, 0x00000009, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58F6, 0x00000008, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58F8, 0x00000006, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58FA, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58FC, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x58FE, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5900, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5902, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5904, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5906, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5908, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x590A, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x590C, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x590E, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5910, 0x00000320, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5912, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5914, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5916, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5918, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x591A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x591C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x591E, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5920, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5922, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5924, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5926, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5928, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x592A, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x592C, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x592E, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5930, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5932, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5934, 0x00000320, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5936, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5938, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x593A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x593C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x593E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5940, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5942, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5944, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5946, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5948, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x594A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x594C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x594E, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5950, 0x0000015E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5952, 0x00000032, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5954, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5956, 0x000001F4, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5958, 0x000000C8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x595A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x595C, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x595E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5960, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5962, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5964, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5966, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5968, 0x00000003, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x596A, 0x00000001, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x596C, 0x00000000, CAMCMP_DAT(ap_gain), sizeof(CAMCMP_DAT(ap_gain))},
	{CAMCMP_REG_WRITE, 0x59D4, 0x0000FFDF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59D6, 0x0000FFE9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59D8, 0x0000FFF6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59DA, 0x0000FFDF, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59DC, 0x0000FFE9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59DE, 0x0000FFF6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59E0, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59E2, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59E4, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59E6, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59E8, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59EA, 0x0000FFE0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x59EC, 0x000000F9, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59ED, 0x000000F9, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59EE, 0x000000F9, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59EF, 0x000000FE, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59F0, 0x000000FE, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59F1, 0x000000FE, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59F2, 0x000000F9, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59F3, 0x000000F9, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x59F4, 0x000000F9, NULL,                CAMCMP_D_LEN_BYTE},
};


static unsigned char CAMCMP_DAT(af_drv)[] =
{
	0xF7, 0xB5, 0x0E, 0x4D, 0x0E, 0x4C, 0x0F, 0x4A, 0x0F, 0x4F, 0x10, 0x4B, 0x00, 0x21, 0x00, 0x2A,
	0x04, 0xD9, 0x6E, 0x5C, 0x66, 0x54, 0x01, 0x31, 0x91, 0x42, 0xFA, 0xD3, 0x00, 0x21, 0x00, 0x2B,
	0x04, 0xD9, 0x00, 0x22, 0x7A, 0x54, 0x01, 0x31, 0x99, 0x42, 0xFB, 0xD3, 0x08, 0x49, 0x88, 0x60,
	0x01, 0x99, 0x08, 0x48, 0x08, 0x60, 0x03, 0xB0, 0x01, 0x20, 0xF0, 0xBD, 0x14, 0x0D, 0x18, 0x00,
	0x14, 0x0D, 0x18, 0x00, 0x04, 0x00, 0x00, 0x00, 0x18, 0x0D, 0x18, 0x00, 0x0C, 0x00, 0x00, 0x00,
	0x18, 0x0D, 0x18, 0x00, 0x14, 0x0D, 0x18, 0x00, 0x80, 0xB5, 0x1F, 0x78, 0x00, 0x2F, 0x02, 0xD1,
	0x00, 0xF0, 0xB2, 0xF8, 0x80, 0xBD, 0x00, 0xF0, 0x01, 0xF8, 0xFB, 0xE7, 0xFF, 0xB5, 0x83, 0xB0,
	0x0C, 0x1C, 0x1F, 0x1C, 0x00, 0x25, 0x08, 0x23, 0xF8, 0x5E, 0xEB, 0x43, 0x51, 0x4E, 0x98, 0x42,
	0x25, 0xD1, 0xF2, 0x80, 0xA0, 0x1A, 0x02, 0xD5, 0x40, 0x42, 0x01, 0x21, 0x00, 0xE0, 0x00, 0x21,
	0x31, 0x70, 0x00, 0x04, 0x00, 0x0C, 0x70, 0x80, 0xB0, 0x80, 0x18, 0xD1, 0x20, 0x03, 0x30, 0x30,
	0x00, 0x90, 0x00, 0xF0, 0xE4, 0xF8, 0x00, 0x90, 0xB0, 0x68, 0x6A, 0x46, 0x03, 0x21, 0x03, 0x68,
	0x33, 0x20, 0x7F, 0xF6, 0xB2, 0xFB, 0x01, 0x28, 0x04, 0xD0, 0x00, 0x20, 0xC0, 0x43, 0x38, 0x81,
	0x00, 0x20, 0x7C, 0xE0, 0x03, 0x98, 0x04, 0x80, 0x01, 0x20, 0x38, 0x81, 0x77, 0xE0, 0x78, 0x78,
	0x3C, 0x4E, 0x02, 0x90, 0xB9, 0x78, 0x40, 0x00, 0x40, 0x1A, 0xF9, 0x78, 0x40, 0x1A, 0xB1, 0x88,
	0x49, 0x00, 0x7F, 0xF6, 0x9F, 0xFB, 0x04, 0x04, 0x24, 0x0C, 0x38, 0x8A, 0xA0, 0x42, 0x02, 0xDA,
	0x01, 0x21, 0x04, 0x1C, 0x00, 0xE0, 0x00, 0x21, 0x01, 0x91, 0x02, 0x98, 0x00, 0x26, 0x00, 0x28,
	0x0B, 0xDD, 0x30, 0x1C, 0x21, 0x1C, 0x3A, 0x1C, 0x00, 0xF0, 0x96, 0xF8, 0x40, 0x19, 0x05, 0x04,
	0x2D, 0x0C, 0x78, 0x78, 0x01, 0x36, 0xB0, 0x42, 0xF3, 0xDC, 0x2A, 0x4E, 0xB0, 0x88, 0x40, 0x1B,
	0xB0, 0x80, 0x08, 0x23, 0xF8, 0x5E, 0x01, 0x23, 0xD8, 0x42, 0x13, 0xD1, 0x01, 0x99, 0x00, 0x29,
	0x02, 0xD1, 0x01, 0x21, 0x39, 0x81, 0x0D, 0xE0, 0x71, 0x88, 0x28, 0x1C, 0x7F, 0xF6, 0xBC, 0xFA,
	0x38, 0x81, 0x71, 0x88, 0x28, 0x1C, 0x7F, 0xF6, 0xB7, 0xFA, 0x00, 0x29, 0x02, 0xD0, 0x38, 0x89,
	0x01, 0x30, 0x38, 0x81, 0x78, 0x78, 0x00, 0x25, 0x00, 0x28, 0x2F, 0xDD, 0x28, 0x1C, 0x21, 0x1C,
	0x3A, 0x1C, 0x00, 0xF0, 0x69, 0xF8, 0x01, 0x99, 0x00, 0x29, 0x07, 0xD1, 0xB1, 0x88, 0x00, 0x29,
	0x04, 0xDD, 0x01, 0x30, 0x00, 0x04, 0x00, 0x0C, 0x01, 0x39, 0xB1, 0x80, 0x31, 0x78, 0x00, 0x29,
	0x02, 0xD1, 0xF1, 0x88, 0x08, 0x18, 0x01, 0xE0, 0xF1, 0x88, 0x08, 0x1A, 0xF0, 0x80, 0xF0, 0x88,
	0x00, 0x03, 0x30, 0x30, 0x00, 0x90, 0x00, 0xF0, 0x6A, 0xF8, 0x00, 0x90, 0xB0, 0x68, 0x6A, 0x46,
	0x03, 0x21, 0x03, 0x68, 0x33, 0x20, 0x7F, 0xF6, 0x38, 0xFB, 0x01, 0x28, 0x85, 0xD1, 0xF0, 0x88,
	0x03, 0x99, 0x08, 0x80, 0x78, 0x78, 0x01, 0x35, 0xA8, 0x42, 0xCF, 0xDC, 0x01, 0x20, 0x07, 0xB0,
	0xF0, 0xBD, 0x00, 0x00, 0x18, 0x0D, 0x18, 0x00, 0xF8, 0xB5, 0x1C, 0x1C, 0x15, 0x1C, 0x06, 0x1C,
	0x0F, 0x1C, 0x38, 0x03, 0x30, 0x30, 0x00, 0x90, 0x00, 0xF0, 0x49, 0xF8, 0x00, 0x90, 0x0A, 0x48,
	0x6A, 0x46, 0x80, 0x68, 0x03, 0x21, 0x03, 0x68, 0x33, 0x20, 0x7F, 0xF6, 0x16, 0xFB, 0x01, 0x28,
	0x01, 0xD0, 0x00, 0x20, 0x06, 0xE0, 0x38, 0x1C, 0x29, 0x1C, 0x22, 0x1C, 0x37, 0x80, 0x00, 0xF0,
	0x05, 0xF8, 0x01, 0x20, 0xF8, 0xBD, 0x00, 0x00, 0x18, 0x0D, 0x18, 0x00, 0x80, 0xB5, 0x17, 0x1C,
	0x40, 0x1A, 0x00, 0xD5, 0x40, 0x42, 0xB9, 0x89, 0x48, 0x43, 0xF9, 0x89, 0x09, 0x18, 0x78, 0x69,
	0x09, 0x18, 0x7F, 0xF6, 0x49, 0xFA, 0x03, 0x4B, 0x98, 0x42, 0x00, 0xD9, 0x18, 0x1C, 0x38, 0x81,
	0x80, 0xBD, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0x80, 0xB5, 0x93, 0x78, 0x83, 0x42, 0x05, 0xDD,
	0x01, 0x30, 0x48, 0x43, 0x0A, 0x1C, 0x01, 0x1C, 0x58, 0x1C, 0x09, 0xE0, 0x53, 0x78, 0xD2, 0x78,
	0x9F, 0x1A, 0x87, 0x42, 0x09, 0xDC, 0x18, 0x1A, 0x48, 0x43, 0x0B, 0x1C, 0x01, 0x1C, 0x50, 0x1C,
	0x7F, 0xF6, 0xE0, 0xFA, 0x00, 0x04, 0x00, 0x0C, 0x80, 0xBD, 0x08, 0x1C, 0xFC, 0xE7, 0xFF, 0x21,
	0x09, 0x04, 0x01, 0x40, 0x09, 0x0C, 0xFF, 0x22, 0x12, 0x02, 0x02, 0x40, 0x11, 0x43, 0x00, 0x06,
	0x00, 0x0E, 0x00, 0x04, 0x08, 0x43, 0xF7, 0x46, 0xE5, 0x0A, 0x18, 0x00
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(init_af_drv)[] =
{
	{CAMCMP_REG_WRITE, 0xC000, 0x00000000, CAMCMP_DAT(af_drv),  sizeof(CAMCMP_DAT(af_drv))},
	{CAMCMP_REG_WRITE, 0x485E, 0x00008D27, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x485C, 0x0000028C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x000A, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_POLL1, 0x000A, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(init_set_5)[] =
{
	{CAMCMP_REG_WRITE, 0x01BD, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01BE, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01BF, 0x00000008, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01C0, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0084, 0x0000014A, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0086, 0x00000298, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0088, 0x000004E2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x008A, 0x00000A96, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x008C, 0x000007D0, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x008E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0090, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0092, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x01B1, 0x00000080, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01B2, 0x00000065, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01B3, 0x00000080, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0102, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0103, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0104, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0105, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0107, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0108, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0109, 0x0000004D, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x010A, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x010C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x010D, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x010E, 0x00000043, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x010F, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0111, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0112, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0113, 0x0000004D, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0114, 0x00000080, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0116, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0117, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0118, 0x00000043, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0119, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x011B, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x011C, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x011D, 0x000000C3, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x011E, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0120, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0121, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0122, 0x00000043, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0123, 0x000000D0, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0125, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0126, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0127, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0128, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x012A, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x012B, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x012C, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x012D, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x012F, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0130, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0131, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0132, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0134, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0135, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0136, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0137, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0139, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x013A, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x013B, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x013C, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D3, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D4, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D5, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D6, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D7, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D8, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01D9, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01DA, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01DB, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01DC, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01DD, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01DE, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01F8, 0x00000095, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01F9, 0x00000075, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01FA, 0x00000055, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01FB, 0x00000095, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01FC, 0x00000075, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01FD, 0x00000055, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D7, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D8, 0x00000011, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D9, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02DA, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02DB, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02DC, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0004, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0008, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0012, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0013, 0x00000003, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0015, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x002E, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x002F, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0030, 0x00000002, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x005F, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0060, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0061, 0x00000080, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0062, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0080, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00F0, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00F2, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x00F4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0100, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x013E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0140, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0142, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0144, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x01AE, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01AF, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01B0, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01E5, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01E6, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01E7, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01E8, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0278, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x027D, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x027E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0280, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0282, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02AE, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02B0, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02CA, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x034F, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x3208, 0x00000000, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x585E, 0x00000A2B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5860, 0x0000062E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5862, 0x00000231, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5864, 0x00000A2B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5866, 0x0000062E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5868, 0x00000231, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x586A, 0x00000A2B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x586C, 0x0000062E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x586E, 0x00000231, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5870, 0x00000A2B, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5872, 0x0000062E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5874, 0x00000231, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5876, 0x000009F7, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5878, 0x000006A5, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x587A, 0x00000352, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x587C, 0x00000AA2, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x587E, 0x0000074F, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5880, 0x000003FD, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5882, 0x00000B4C, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5884, 0x000007F9, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x5886, 0x000004A7, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E0C, 0x00000331, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E0E, 0x00000B31, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4E10, 0x000000D9, NULL,                CAMCMP_D_LEN_WORD},
};


static camcmp_ius006f_reg_val_type CAMCMP_REG(wb_auto)[] =
{
	{CAMCMP_REG_WRITE, 0x0102, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x446D, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(wb_inc)[] =
{
	{CAMCMP_REG_WRITE, 0x0102, 0x00000028, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4446, 0x00001356, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4448, 0x00000D74, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x446D, 0x00000040, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(wb_sunny)[] =
{
	{CAMCMP_REG_WRITE, 0x0102, 0x00000024, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4436, 0x00000B81, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4438, 0x00001832, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x446D, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(wb_fluor)[] =
{
	{CAMCMP_REG_WRITE, 0x0102, 0x00000027, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4442, 0x00000C80, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4444, 0x00001300, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x446D, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(wb_cloudy)[] =
{
	{CAMCMP_REG_WRITE, 0x0102, 0x00000026, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x443E, 0x00000ACD, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4440, 0x00001AD6, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x446D, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(wb_led)[] =
{
	{CAMCMP_REG_WRITE, 0x0102, 0x00000025, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x443A, 0x00000C03, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x443C, 0x0000179E, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x446D, 0x000000A0, NULL,                CAMCMP_D_LEN_BYTE},
};






static camcmp_ius006f_reg_val_type CAMCMP_REG(night_off)[] =
{
	{CAMCMP_REG_WRITE, 0x404C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4424, 0x0000000C, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02A4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4016, 0x00002C13, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4018, 0x00000331, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401C, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4020, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x01BD, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(night_on)[] =
{
	{CAMCMP_REG_WRITE, 0x404C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4424, 0x0000000C, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02A4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4016, 0x00002C13, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4018, 0x00000331, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401A, 0x00000257, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401C, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401E, 0x000005A8, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4020, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x01BD, 0x00000004, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(movie)[] =
{
	{CAMCMP_REG_WRITE, 0x404C, 0x00000020, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4424, 0x0000000C, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02A4, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02A6, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4016, 0x00002C13, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4018, 0x00000331, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401A, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401C, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x401E, 0x00000000, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x4020, 0x00000400, NULL,                CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x01BD, 0x00000001, NULL,                CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_af_val_type CAMCMP_DAT(af_val) =
{
	1244,
	12906,
	8500,
	3800,
	900,
	500,
	2000,
	250,
	500
};

static uint16_t CAMCMP_DAT(version) = 0x0100;
static camcmp_ius006f_reg_val_type CAMCMP_REG(cap_size)[] =
{
	{CAMCMP_REG_WRITE, 0x0024, 0x00000A20, NULL,  CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x002A, 0x00000798, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(moni_size)[] =
{
	{CAMCMP_REG_WRITE, 0x0022, 0x00000280, NULL,  CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x0028, 0x000001E0, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(chg_mode)[] =
{
	{CAMCMP_REG_WRITE, 0x0011, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_POLL1, 0x00F8, 0x00000002, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000006, NULL,  CAMCMP_D_LEN_BYTE}   /* <025> */
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(zoom)[] =
{
	{CAMCMP_REG_WRITE, 0x0032, 0x00000100, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(brightness)[] =
{
	{CAMCMP_REG_WRITE, 0x0080, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(effect)[] =
{
	{CAMCMP_REG_WRITE, 0x005F, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(quality)[] =
{
	{CAMCMP_REG_WRITE, 0x0204, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(int_clr)[] =
{
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000002, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(af_cancel)[] =
{
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000010, NULL,  CAMCMP_D_LEN_BYTE},   /* <009> add */
	{CAMCMP_REG_WRITE, 0x4885, 0x00000001, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_POLL2, 0x6D76, 0x00000003, NULL,  CAMCMP_D_LEN_BYTE}    /* <009> add *//* <013> add */
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(af_mode)[] =
{
	{CAMCMP_REG_WRITE, 0x002F, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}    /* <004> */
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(mf_pos)[] =
{
	{CAMCMP_REG_WRITE, 0x4852, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(mf_start)[] =
{
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000010, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4850, 0x00000001, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_POLL1, 0x00F8, 0x00000010, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000010, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(ae_normal)[] =
{
	{CAMCMP_REG_WRITE, 0x0103, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(ae_long)[] =
{
	{CAMCMP_REG_WRITE, 0x0103, 0x00000001, NULL,  CAMCMP_D_LEN_BYTE},
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(ptnchk_off)[] =
{
	{CAMCMP_REG_WRITE, 0x2800, 0x00000000, NULL,  CAMCMP_D_LEN_LWORD},
	{CAMCMP_REG_WRITE, 0x3200, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x3205, 0x00000001, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0068, 0x0000005f, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F24, 0x00000032, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F25, 0x00000009, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4E16, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0105, 0x00000040, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01F8, 0x00000095, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01FB, 0x00000095, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x3202, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(ptnchk_on)[] =
{
	{CAMCMP_REG_WRITE, 0x2800, 0x07000000, NULL,  CAMCMP_D_LEN_LWORD},
	{CAMCMP_REG_WRITE, 0x3200, 0x00000006, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x3205, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0068, 0x00000050, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F24, 0x00000010, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4F25, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x4E16, 0x00000004, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x0105, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01F8, 0x00000054, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x01FB, 0x00000054, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x3202, 0x00000001, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(chk_af_lock)[] =
{
	{CAMCMP_REG_POLL3, 0x0000, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000010, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(thumbnail)[] =
{
	{CAMCMP_REG_WRITE, 0x0222, 0x00009B9A, NULL,  CAMCMP_D_LEN_WORD},
	{CAMCMP_REG_WRITE, 0x02D2, 0x00000002, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D3, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x02D4, 0x00009D9C, NULL,  CAMCMP_D_LEN_WORD}
};

/* <014> add-start */
static camcmp_ius006f_reg_val_type CAMCMP_REG(jpeg_limit)[] =
{
	{CAMCMP_REG_WRITE, 0x020A, 0x000003A7, NULL,  CAMCMP_D_LEN_WORD},	/* <015> */
	{CAMCMP_REG_WRITE, 0x020C, 0x0000078F, NULL,  CAMCMP_D_LEN_WORD},	/* <015> */
	{CAMCMP_REG_WRITE, 0x020E, 0x00000B77, NULL,  CAMCMP_D_LEN_WORD}	/* <015> */
};
/* <014> add-end */

/* <022> add-start */
static camcmp_ius006f_reg_val_type CAMCMP_REG(af_restart)[] =
{
	{CAMCMP_REG_WRITE, 0x0015, 0x00000001, NULL,  CAMCMP_D_LEN_BYTE},
};
/* <022> add-end */

/* <025> add-start */
static camcmp_ius006f_reg_val_type CAMCMP_REG(jpeg_intsts)[] =
{
	{CAMCMP_REG_POLL1, 0x00F8, 0x00000004, NULL,  CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_WRITE, 0x00FC, 0x00000004, NULL,  CAMCMP_D_LEN_BYTE}
};
/* <025> add-end */

static camcmp_ius006f_reg_val_type CAMCMP_REG(otp0l)[] =
{
	{CAMCMP_REG_READ, 0x0250, 0x00000000, NULL, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(otp0m)[] =
{
	{CAMCMP_REG_READ, 0x0254, 0x00000000, NULL, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(otp0h)[] =
{
	{CAMCMP_REG_READ, 0x0258, 0x00000000, NULL, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(otp1l)[] =
{
	{CAMCMP_REG_READ, 0x025C, 0x00000000, NULL, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(otp1m)[] =
{
	{CAMCMP_REG_READ, 0x0260, 0x00000000, NULL, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(otp1h)[] =
{
	{CAMCMP_REG_READ, 0x0264, 0x00000000, NULL, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(intsts)[] =
{
	{CAMCMP_REG_READ, 0x00F8, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

/*<006> start*/
static camcmp_ius006f_reg_val_type CAMCMP_REG(evaluate)[] =
{
	{CAMCMP_REG_READ, 0x6810, 0x00000000, NULL,  CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(lens_pos)[] =
{
	{CAMCMP_REG_READ, 0x6D7A, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(af_step)[] =
{
	{CAMCMP_REG_READ, 0x6D84, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(aescl)[] =
{
	{CAMCMP_REG_READ, 0x027A, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}    /*<010>*/
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(sht_time_h)[] =
{
	{CAMCMP_REG_READ, 0x00F4, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(sht_time_l)[] =
{
	{CAMCMP_REG_READ, 0x00F2, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(agc_scl)[] =
{
	{CAMCMP_REG_READ, 0x0278, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};
/*<006> end*/

/* <009> add start */
static camcmp_ius006f_reg_val_type CAMCMP_REG(af_result)[] =
{
	{CAMCMP_REG_READ, 0x6D77, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(af_state)[] =
{
	{CAMCMP_REG_READ, 0x6D76, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};
/* <009> add end */

/*<018>add start*/
static camcmp_ius006f_reg_val_type CAMCMP_REG(manual_step)[] =
{
	{CAMCMP_REG_READ, 0x486C, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(area_low_type1)[] =
{
	{CAMCMP_REG_READ, 0x4876, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(area_high_type1)[] =
{
	{CAMCMP_REG_READ, 0x4878, 0x00000000, NULL,  CAMCMP_D_LEN_WORD}
};
/*<018>add end*/

/* <025> add-start */
static camcmp_ius006f_reg_val_type CAMCMP_REG(jpg_sts)[] =
{
	{CAMCMP_REG_READ, 0x0200, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

static camcmp_ius006f_reg_val_type CAMCMP_REG(jpg_retry)[] =
{
	{CAMCMP_REG_READ, 0x0209, 0x00000000, NULL,  CAMCMP_D_LEN_BYTE}
};

/* <025> add-end */

static camcmp_data_type CAMCMP_CMD(chk_dev_sts)[]=
{
	{(void*)CAMCMP_REG(chk_dev_sts), CAMCMP_SIZE_REG(CAMCMP_REG(chk_dev_sts))}
};

static camcmp_data_type CAMCMP_CMD(init_set_1)[]=
{
	{(void*)CAMCMP_REG(init_set_1), CAMCMP_SIZE_REG(CAMCMP_REG(init_set_1))}
};

static camcmp_data_type CAMCMP_CMD(init_set_2)[]=
{
	{(void*)CAMCMP_REG(init_set_2),  CAMCMP_SIZE_REG(CAMCMP_REG(init_set_2))}
};

static camcmp_data_type CAMCMP_CMD(init_set_3)[]=
{
	{(void*)CAMCMP_REG(init_set_3),  CAMCMP_SIZE_REG(CAMCMP_REG(init_set_3))}
};

static camcmp_data_type CAMCMP_CMD(rom_correct)[]=
{
	{(void*)CAMCMP_REG(rom_correct), CAMCMP_SIZE_REG(CAMCMP_REG(rom_correct))}
};

static camcmp_data_type CAMCMP_CMD(init_set_4)[]=
{
	{(void*)CAMCMP_REG(init_set_4), CAMCMP_SIZE_REG(CAMCMP_REG(init_set_4))}
};

static camcmp_data_type CAMCMP_CMD(init_set_5)[]=
{
	{(void*)CAMCMP_REG(init_af_drv), CAMCMP_SIZE_REG(CAMCMP_REG(init_af_drv))},
	{(void*)CAMCMP_REG(init_set_5),  CAMCMP_SIZE_REG(CAMCMP_REG(init_set_5))}
};

static camcmp_data_type CAMCMP_CMD(wb_auto)[] =
{
	{(void*)CAMCMP_REG(wb_auto), CAMCMP_SIZE_REG(CAMCMP_REG(wb_auto))}
};

static camcmp_data_type CAMCMP_CMD(wb_inc)[] =
{
	{(void*)CAMCMP_REG(wb_inc), CAMCMP_SIZE_REG(CAMCMP_REG(wb_inc))}
};

static camcmp_data_type CAMCMP_CMD(wb_sunny)[] =
{
	{(void*)CAMCMP_REG(wb_sunny), CAMCMP_SIZE_REG(CAMCMP_REG(wb_sunny))}
};

static camcmp_data_type CAMCMP_CMD(wb_fluor)[] =
{
	{(void*)CAMCMP_REG(wb_fluor), CAMCMP_SIZE_REG(CAMCMP_REG(wb_fluor))}
};

static camcmp_data_type CAMCMP_CMD(wb_cloudy)[] =
{
	{(void*)CAMCMP_REG(wb_cloudy), CAMCMP_SIZE_REG(CAMCMP_REG(wb_cloudy))}
};

static camcmp_data_type CAMCMP_CMD(wb_led)[] =
{
	{(void*)CAMCMP_REG(wb_led), CAMCMP_SIZE_REG(CAMCMP_REG(wb_led))}
};

static camcmp_data_type CAMCMP_CMD(night_off)[] =
{
	{(void*)CAMCMP_REG(night_off), CAMCMP_SIZE_REG(CAMCMP_REG(night_off))}
};

static camcmp_data_type CAMCMP_CMD(night_on)[] =
{
	{(void*)CAMCMP_REG(night_on), CAMCMP_SIZE_REG(CAMCMP_REG(night_on))}
};

static camcmp_data_type CAMCMP_CMD(movie)[] =
{
	{(void*)CAMCMP_REG(movie), CAMCMP_SIZE_REG(CAMCMP_REG(movie))}
};

static camcmp_data_type CAMCMP_CMD(cap_size)[] =
{
	{(void*)CAMCMP_REG(cap_size), CAMCMP_SIZE_REG(CAMCMP_REG(cap_size))}
};

static camcmp_data_type CAMCMP_CMD(moni_size)[] =
{
	{(void*)CAMCMP_REG(moni_size), CAMCMP_SIZE_REG(CAMCMP_REG(moni_size))}
};

static camcmp_data_type CAMCMP_CMD(chg_mode)[] =
{
	{(void*)CAMCMP_REG(chg_mode), CAMCMP_SIZE_REG(CAMCMP_REG(chg_mode))}
};

static camcmp_data_type CAMCMP_CMD(zoom_5m)[] =
{
	{(void*)CAMCMP_REG(zoom), CAMCMP_SIZE_REG(CAMCMP_REG(zoom))}
};

static camcmp_data_type CAMCMP_CMD(zoom_3m)[] =
{
	{(void*)CAMCMP_REG(zoom), CAMCMP_SIZE_REG(CAMCMP_REG(zoom))}
};

static camcmp_data_type CAMCMP_CMD(zoom_2m)[] =
{
	{(void*)CAMCMP_REG(zoom), CAMCMP_SIZE_REG(CAMCMP_REG(zoom))}
};

static camcmp_data_type CAMCMP_CMD(zoom_1m)[] =
{
	{(void*)CAMCMP_REG(zoom), CAMCMP_SIZE_REG(CAMCMP_REG(zoom))}
};

static camcmp_data_type CAMCMP_CMD(zoom_qh)[] =
{
	{(void*)CAMCMP_REG(zoom), CAMCMP_SIZE_REG(CAMCMP_REG(zoom))}
};

static camcmp_data_type CAMCMP_CMD(zoom_ql)[] =
{
	{(void*)CAMCMP_REG(zoom), CAMCMP_SIZE_REG(CAMCMP_REG(zoom))}
};

static camcmp_data_type CAMCMP_CMD(brightness)[] =
{
	{(void*)CAMCMP_REG(brightness), CAMCMP_SIZE_REG(CAMCMP_REG(brightness))}
};

static camcmp_data_type CAMCMP_CMD(effect)[] =
{
	{(void*)CAMCMP_REG(effect), CAMCMP_SIZE_REG(CAMCMP_REG(effect))}
};

static camcmp_data_type CAMCMP_CMD(quality)[] =
{
	{(void*)CAMCMP_REG(quality), CAMCMP_SIZE_REG(CAMCMP_REG(quality))}
};

static camcmp_data_type CAMCMP_CMD(int_clr)[] =
{
	{(void*)CAMCMP_REG(int_clr), CAMCMP_SIZE_REG(CAMCMP_REG(int_clr))}
};

static camcmp_data_type CAMCMP_CMD(af_cancel)[] =
{
	{(void*)CAMCMP_REG(af_cancel), CAMCMP_SIZE_REG(CAMCMP_REG(af_cancel))}
};

static camcmp_data_type CAMCMP_CMD(af_mode)[] =
{
	{(void*)CAMCMP_REG(af_mode), CAMCMP_SIZE_REG(CAMCMP_REG(af_mode))}
};

static camcmp_data_type CAMCMP_CMD(mf_start)[] =
{
	{(void*)CAMCMP_REG(mf_pos),   CAMCMP_SIZE_REG(CAMCMP_REG(mf_pos))},
	{(void*)CAMCMP_REG(mf_start), CAMCMP_SIZE_REG(CAMCMP_REG(mf_start))}
};

static camcmp_data_type CAMCMP_CMD(ae_normal)[] =
{
	{(void*)CAMCMP_REG(ae_normal), CAMCMP_SIZE_REG(CAMCMP_REG(ae_normal))}
};

static camcmp_data_type CAMCMP_CMD(ae_long)[] =
{
	{(void*)CAMCMP_REG(ae_long), CAMCMP_SIZE_REG(CAMCMP_REG(ae_long))}
};

static camcmp_data_type CAMCMP_CMD(ptnchk_off)[] =
{
	{(void*)CAMCMP_REG(ptnchk_off), CAMCMP_SIZE_REG(CAMCMP_REG(ptnchk_off))}
};

static camcmp_data_type CAMCMP_CMD(ptnchk_on)[] =
{
	{(void*)CAMCMP_REG(ptnchk_on), CAMCMP_SIZE_REG(CAMCMP_REG(ptnchk_on))}
};

static camcmp_data_type CAMCMP_CMD(chk_af_lock)[] =
{
	{(void*)CAMCMP_REG(chk_af_lock), CAMCMP_SIZE_REG(CAMCMP_REG(chk_af_lock))}
};

static camcmp_data_type CAMCMP_CMD(thumbnail)[] =
{
	{(void*)CAMCMP_REG(thumbnail), CAMCMP_SIZE_REG(CAMCMP_REG(thumbnail))}
};

/* <014> add-start */
static camcmp_data_type CAMCMP_CMD(jpeg_limit)[] =
{
	{(void*)CAMCMP_REG(jpeg_limit), CAMCMP_SIZE_REG(CAMCMP_REG(jpeg_limit))}
};
/* <014> add-end*/

/* <022> add-start */
static camcmp_data_type CAMCMP_CMD(af_restart)[] =
{
	{(void*)CAMCMP_REG(af_restart), CAMCMP_SIZE_REG(CAMCMP_REG(af_restart))}
};
/* <022> add-end*/

static camcmp_data_type CAMCMP_CMD(otp0l)[] =
{
	{(void*)CAMCMP_REG(otp0l), CAMCMP_SIZE_REG(CAMCMP_REG(otp0l))}
};

static camcmp_data_type CAMCMP_CMD(otp0m)[] =
{
	{(void*)CAMCMP_REG(otp0m), CAMCMP_SIZE_REG(CAMCMP_REG(otp0m))}
};

static camcmp_data_type CAMCMP_CMD(otp0h)[] =
{
	{(void*)CAMCMP_REG(otp0h), CAMCMP_SIZE_REG(CAMCMP_REG(otp0h))}
};

static camcmp_data_type CAMCMP_CMD(otp1l)[] =
{
	{(void*)CAMCMP_REG(otp1l), CAMCMP_SIZE_REG(CAMCMP_REG(otp1l))}
};

static camcmp_data_type CAMCMP_CMD(otp1m)[] =
{
	{(void*)CAMCMP_REG(otp1m), CAMCMP_SIZE_REG(CAMCMP_REG(otp1m))}
};

static camcmp_data_type CAMCMP_CMD(otp1h)[] =
{
	{(void*)CAMCMP_REG(otp1h), CAMCMP_SIZE_REG(CAMCMP_REG(otp1h))}
};

static camcmp_data_type CAMCMP_CMD(otp)[] =
{
	{(void*)CAMCMP_REG(otp0l), CAMCMP_SIZE_REG(CAMCMP_REG(otp0l))},
	{(void*)CAMCMP_REG(otp0m), CAMCMP_SIZE_REG(CAMCMP_REG(otp0m))},
	{(void*)CAMCMP_REG(otp0h), CAMCMP_SIZE_REG(CAMCMP_REG(otp0h))},
	{(void*)CAMCMP_REG(otp1l), CAMCMP_SIZE_REG(CAMCMP_REG(otp1l))},
	{(void*)CAMCMP_REG(otp1m), CAMCMP_SIZE_REG(CAMCMP_REG(otp1m))},
	{(void*)CAMCMP_REG(otp1h), CAMCMP_SIZE_REG(CAMCMP_REG(otp1h))}
};

static camcmp_data_type CAMCMP_CMD(intsts)[] =
{
	{(void*)CAMCMP_REG(intsts), CAMCMP_SIZE_REG(CAMCMP_REG(intsts))}
};


/*<006> start*/
static camcmp_data_type CAMCMP_CMD(evaluate)[] =
{
	{(void*)CAMCMP_REG(evaluate), CAMCMP_SIZE_REG(CAMCMP_REG(evaluate))}
};

static camcmp_data_type CAMCMP_CMD(lens_pos)[] =
{
	{(void*)CAMCMP_REG(lens_pos), CAMCMP_SIZE_REG(CAMCMP_REG(lens_pos))}
};

static camcmp_data_type CAMCMP_CMD(af_step)[] =
{
	{(void*)CAMCMP_REG(af_step), CAMCMP_SIZE_REG(CAMCMP_REG(af_step))}
};

static camcmp_data_type CAMCMP_CMD(aescl)[] =
{
	{(void*)CAMCMP_REG(aescl), CAMCMP_SIZE_REG(CAMCMP_REG(aescl))}
};

static camcmp_data_type CAMCMP_CMD(sht_time_h)[] =
{
	{(void*)CAMCMP_REG(sht_time_h), CAMCMP_SIZE_REG(CAMCMP_REG(sht_time_h))}
};

static camcmp_data_type CAMCMP_CMD(sht_time_l)[] =
{
	{(void*)CAMCMP_REG(sht_time_l), CAMCMP_SIZE_REG(CAMCMP_REG(sht_time_l))}
};

static camcmp_data_type CAMCMP_CMD(agc_scl)[] =
{
	{(void*)CAMCMP_REG(agc_scl), CAMCMP_SIZE_REG(CAMCMP_REG(agc_scl))}
};
/*<006> end*/

/* <009> add start */
static camcmp_data_type CAMCMP_CMD(af_result)[] =
{
	{(void*)CAMCMP_REG(af_result), CAMCMP_SIZE_REG(CAMCMP_REG(af_result))}
};

static camcmp_data_type CAMCMP_CMD(af_state)[] =
{
	{(void*)CAMCMP_REG(af_state), CAMCMP_SIZE_REG(CAMCMP_REG(af_state))}
};
/* <009> add end */

/*<018>add start*/
static camcmp_data_type CAMCMP_CMD(manual_step)[] =
{
	{(void*)CAMCMP_REG(manual_step), CAMCMP_SIZE_REG(CAMCMP_REG(manual_step))}
};

static camcmp_data_type CAMCMP_CMD(area_low_type1)[] =
{
	{(void*)CAMCMP_REG(area_low_type1), CAMCMP_SIZE_REG(CAMCMP_REG(area_low_type1))}
};

static camcmp_data_type CAMCMP_CMD(area_high_type1)[] =
{
	{(void*)CAMCMP_REG(area_high_type1), CAMCMP_SIZE_REG(CAMCMP_REG(area_high_type1))}
};
/*<018>add end*/
/* <025> add-start */
static camcmp_data_type CAMCMP_CMD(jpeg_intsts)[] =
{
	{(void*)CAMCMP_REG(jpeg_intsts), CAMCMP_SIZE_REG(CAMCMP_REG(jpeg_intsts))}
};

static camcmp_data_type CAMCMP_CMD(jpg_sts)[] =
{
	{(void*)CAMCMP_REG(jpg_sts), CAMCMP_SIZE_REG(CAMCMP_REG(jpg_sts))}
};

static camcmp_data_type CAMCMP_CMD(jpg_retry)[] =
{
	{(void*)CAMCMP_REG(jpg_retry), CAMCMP_SIZE_REG(CAMCMP_REG(jpg_retry))}
};
/* <025> add-end */

static camcmp_ius006f_blank_val_type CAMCMP_BLK(cap_size_5m)[] =
{
	{(void*)&(CAMCMP_REG(cap_size)[0].lwdata), 0x00000A20, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(cap_size)[1].lwdata), 0x00000798, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(cap_size_3m)[] =
{
	{(void*)&(CAMCMP_REG(cap_size)[0].lwdata), 0x00000800, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(cap_size)[1].lwdata), 0x00000600, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(cap_size_2m)[] =
{
	{(void*)&(CAMCMP_REG(cap_size)[0].lwdata), 0x00000640, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(cap_size)[1].lwdata), 0x000004B0, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(cap_size_1m)[] =
{
	{(void*)&(CAMCMP_REG(cap_size)[0].lwdata), 0x00000500, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(cap_size)[1].lwdata), 0x000003C0, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(moni_size_vga)[] =
{
	{(void*)&(CAMCMP_REG(moni_size)[0].lwdata), 0x00000280, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(moni_size)[1].lwdata), 0x000001E0, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(moni_size_wvga)[] =
{
	{(void*)&(CAMCMP_REG(moni_size)[0].lwdata), 0x000002D0, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(moni_size)[1].lwdata), 0x000001E0, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(moni_size_qvga)[] =
{
	{(void*)&(CAMCMP_REG(moni_size)[0].lwdata), 0x00000140, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(moni_size)[1].lwdata), 0x000000F0, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(moni_mode)[] =
{
	{(void*)&(CAMCMP_REG(chg_mode)[0].waddr),  0x00000011,  CAMCMP_D_LEN_WORD},
	{(void*)&(CAMCMP_REG(chg_mode)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(moni_ref)[] =
{
	{(void*)&(CAMCMP_REG(chg_mode)[0].waddr),  0x00000012,  CAMCMP_D_LEN_WORD},
	{(void*)&(CAMCMP_REG(chg_mode)[0].lwdata), 0x00000001, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(half_mode)[] =
{
	{(void*)&(CAMCMP_REG(chg_mode)[0].waddr),  0x00000011,  CAMCMP_D_LEN_WORD},
	{(void*)&(CAMCMP_REG(chg_mode)[0].lwdata), 0x00000001, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(cap_mode)[] =
{
	{(void*)&(CAMCMP_REG(chg_mode)[0].waddr),  0x00000011,  CAMCMP_D_LEN_WORD},
	{(void*)&(CAMCMP_REG(chg_mode)[0].lwdata), 0x00000002, CAMCMP_D_LEN_LWORD}
};

/* <011> start */
static camcmp_ius006f_blank_val_type CAMCMP_BLK(zoom_5m)[] =
{
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000100, CAMCMP_D_LEN_LWORD}
};
/* <011> end */

static camcmp_ius006f_blank_val_type CAMCMP_BLK(zoom_3m)[] =
{
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000100, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000106, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000010C, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000113, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000011A, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000122, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000012A, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000132, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000013B, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000144, CAMCMP_D_LEN_LWORD} 
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(zoom_2m)[] =
{
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000100, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000010C, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000118, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000126, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000135, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000145, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000157, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000016C, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000183, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000019E, CAMCMP_D_LEN_LWORD} 
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(zoom_1m)[] =
{
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000100, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000010F, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000121, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000135, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x0000014C, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000166, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000185, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x000001A9, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x000001D4, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000206, CAMCMP_D_LEN_LWORD} 
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(zoom_qh)[] =
{
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000100, CAMCMP_D_LEN_LWORD}         /* <012> */
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(zoom_ql)[] =
{
	{(void*)&(CAMCMP_REG(zoom)[0].lwdata), 0x00000100, CAMCMP_D_LEN_LWORD}         /* <012> */
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(brightness)[] =
{
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x000000FB, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x000000FC, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x000000FD, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x000000FE, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x000000FF, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x00000001, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x00000002, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x00000003, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x00000004, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(brightness)[0].lwdata), 0x00000005, CAMCMP_D_LEN_LWORD} 
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(effect)[] =
{
	{(void*)&(CAMCMP_REG(effect)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(effect)[0].lwdata), 0x00000004, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(effect)[0].lwdata), 0x00000003, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(effect)[0].lwdata), 0x00000002, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(quality)[] =
{
	{(void*)&(CAMCMP_REG(quality)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(quality)[0].lwdata), 0x00000001, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(quality)[0].lwdata), 0x00000002, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(int_clr)[] =
{
	{(void*)&(CAMCMP_REG(int_clr)[0].lwdata), 0x00000002, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(int_clr)[0].lwdata), 0x00000010, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(int_clr)[0].lwdata), 0x00000012, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(af_mode)[] =
{
	{(void*)&(CAMCMP_REG(af_mode)[0].lwdata), 0x00000003, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(af_mode)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(af_mode)[0].lwdata), 0x00000002, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(mf_pos)[] =
{
	{(void*)&(CAMCMP_REG(mf_pos)[0].lwdata), 0x00000101, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(mf_pos)[0].lwdata), 0x00000227, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(mf_pos)[0].lwdata), 0x000000AF, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(mf_pos)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD},
	{(void*)&(CAMCMP_REG(mf_pos)[0].lwdata), 0x00000000, CAMCMP_D_LEN_LWORD}
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(jpeg_limit_5m)[] =
{
	{(void*)&(CAMCMP_REG(jpeg_limit)[0].lwdata), 0x000003A7, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[1].lwdata), 0x0000078F, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[2].lwdata), 0x00000B77, CAMCMP_D_LEN_LWORD}      /* <015> */
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(jpeg_limit_3m)[] =
{
	{(void*)&(CAMCMP_REG(jpeg_limit)[0].lwdata), 0x00000249, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[1].lwdata), 0x000004D3, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[2].lwdata), 0x0000075D, CAMCMP_D_LEN_LWORD}      /* <015> */
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(jpeg_limit_2m)[] =
{
	{(void*)&(CAMCMP_REG(jpeg_limit)[0].lwdata), 0x00000163, CAMCMP_D_LEN_LWORD},     /* <015> <020> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[1].lwdata), 0x00000307, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[2].lwdata), 0x000004AB, CAMCMP_D_LEN_LWORD}      /* <015> <020> */
};

static camcmp_ius006f_blank_val_type CAMCMP_BLK(jpeg_limit_1m)[] =
{
	{(void*)&(CAMCMP_REG(jpeg_limit)[0].lwdata), 0x000000D7, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[1].lwdata), 0x000001EF, CAMCMP_D_LEN_LWORD},     /* <015> */
	{(void*)&(CAMCMP_REG(jpeg_limit)[2].lwdata), 0x00000307, CAMCMP_D_LEN_LWORD}      /* <015> */
};
/* <014> add-end */

static camcmp_data_type CAMCMP_BTBL(cap_size)[] =
{
	{(void*)CAMCMP_BLK(cap_size_5m), CAMCMP_SIZE_BLK(CAMCMP_BLK(cap_size_5m))},
	{(void*)CAMCMP_BLK(cap_size_3m), CAMCMP_SIZE_BLK(CAMCMP_BLK(cap_size_3m))},
	{(void*)CAMCMP_BLK(cap_size_2m), CAMCMP_SIZE_BLK(CAMCMP_BLK(cap_size_2m))},
	{(void*)CAMCMP_BLK(cap_size_1m), CAMCMP_SIZE_BLK(CAMCMP_BLK(cap_size_1m))} 
};

static camcmp_data_type CAMCMP_BTBL(moni_size)[] =
{
	{(void*)CAMCMP_BLK(moni_size_vga),  CAMCMP_SIZE_BLK(CAMCMP_BLK(moni_size_vga))},
	{(void*)CAMCMP_BLK(moni_size_wvga), CAMCMP_SIZE_BLK(CAMCMP_BLK(moni_size_wvga))},
	{(void*)CAMCMP_BLK(moni_size_qvga), CAMCMP_SIZE_BLK(CAMCMP_BLK(moni_size_qvga))}
};

static camcmp_data_type CAMCMP_BTBL(chg_mode)[] =
{
	{(void*)CAMCMP_BLK(moni_mode), CAMCMP_SIZE_BLK(CAMCMP_BLK(moni_mode))},
	{(void*)CAMCMP_BLK(moni_ref),  CAMCMP_SIZE_BLK(CAMCMP_BLK(moni_ref))},
	{(void*)CAMCMP_BLK(half_mode), CAMCMP_SIZE_BLK(CAMCMP_BLK(half_mode))},
	{(void*)CAMCMP_BLK(cap_mode),  CAMCMP_SIZE_BLK(CAMCMP_BLK(cap_mode))}
};

/* <011> start */
static camcmp_data_type CAMCMP_BTBL(zoom_5m)[] =
{
	{(void*)&CAMCMP_BLK(zoom_5m)[0], CAMCAMP_BLK_SIZE_DEF}
};
/* <011> end */

static camcmp_data_type CAMCMP_BTBL(zoom_3m)[] =
{
	{(void*)&CAMCMP_BLK(zoom_3m)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[3], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[4], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[5], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[6], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[7], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[8], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_3m)[9], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(zoom_2m)[] =
{
	{(void*)&CAMCMP_BLK(zoom_2m)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[3], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[4], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[5], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[6], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[7], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[8], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_2m)[9], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(zoom_1m)[] =
{
	{(void*)&CAMCMP_BLK(zoom_1m)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[3], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[4], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[5], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[6], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[7], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[8], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_1m)[9], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(zoom_qh)[] =
{
	{(void*)&CAMCMP_BLK(zoom_qh)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[3], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[4], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[5], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[6], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[7], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[8], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_qh)[9], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(zoom_ql)[] =
{
	{(void*)&CAMCMP_BLK(zoom_ql)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[3], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[4], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[5], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[6], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[7], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[8], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(zoom_ql)[9], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(brightness)[] =
{
	{(void*)&CAMCMP_BLK(brightness)[0],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[1],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[2],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[3],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[4],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[5],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[6],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[7],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[8],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[9],  CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(brightness)[10], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(effect)[] =
{
	{(void*)&CAMCMP_BLK(effect)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(effect)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(effect)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(effect)[3], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(quality)[] =
{
	{(void*)&CAMCMP_BLK(quality)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(quality)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(quality)[2], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(int_clr)[] =
{
	{(void*)&CAMCMP_BLK(int_clr)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(int_clr)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(int_clr)[2], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(af_mode)[] =
{
	{(void*)&CAMCMP_BLK(af_mode)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(af_mode)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(af_mode)[2], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(mf_pos)[] =
{
	{(void*)&CAMCMP_BLK(mf_pos)[0], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(mf_pos)[1], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(mf_pos)[2], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(mf_pos)[3], CAMCAMP_BLK_SIZE_DEF},
	{(void*)&CAMCMP_BLK(mf_pos)[4], CAMCAMP_BLK_SIZE_DEF}
};

static camcmp_data_type CAMCMP_BTBL(jpeg_limit)[] =
{
	{(void*)CAMCMP_BLK(jpeg_limit_5m), CAMCMP_SIZE_BLK(CAMCMP_BLK(jpeg_limit_5m))},
	{(void*)CAMCMP_BLK(jpeg_limit_3m), CAMCMP_SIZE_BLK(CAMCMP_BLK(jpeg_limit_3m))},
	{(void*)CAMCMP_BLK(jpeg_limit_2m), CAMCMP_SIZE_BLK(CAMCMP_BLK(jpeg_limit_2m))},
	{(void*)CAMCMP_BLK(jpeg_limit_1m), CAMCMP_SIZE_BLK(CAMCMP_BLK(jpeg_limit_1m))}
};


camcmp_makecmd_type g_camcmp_ius006f_cmd_list[] =
{
	{CAMCMP_CMD(chk_dev_sts), CAMCMP_SIZE_DAT(CAMCMP_CMD(chk_dev_sts)), NULL,                    0},
	{CAMCMP_CMD(init_set_1),  CAMCMP_SIZE_DAT(CAMCMP_CMD(init_set_1)),  NULL,                    0},
	{CAMCMP_CMD(init_set_2),  CAMCMP_SIZE_DAT(CAMCMP_CMD(init_set_2)),  NULL,                    0},
	{CAMCMP_CMD(init_set_3),  CAMCMP_SIZE_DAT(CAMCMP_CMD(init_set_3)),  NULL,                    0},
	{CAMCMP_CMD(rom_correct), CAMCMP_SIZE_DAT(CAMCMP_CMD(rom_correct)), NULL,                    0},
	{CAMCMP_CMD(init_set_4),  CAMCMP_SIZE_DAT(CAMCMP_CMD(init_set_4)),  NULL,                    0},
	{CAMCMP_CMD(init_set_5),  CAMCMP_SIZE_DAT(CAMCMP_CMD(init_set_5)),  NULL,                    0},
	{CAMCMP_CMD(wb_auto),     CAMCMP_SIZE_DAT(CAMCMP_CMD(wb_auto)),     NULL,                    0},
	{CAMCMP_CMD(wb_inc),      CAMCMP_SIZE_DAT(CAMCMP_CMD(wb_inc)),      NULL,                    0},
	{CAMCMP_CMD(wb_sunny),    CAMCMP_SIZE_DAT(CAMCMP_CMD(wb_sunny)),    NULL,                    0},
	{CAMCMP_CMD(wb_fluor),    CAMCMP_SIZE_DAT(CAMCMP_CMD(wb_fluor)),    NULL,                    0},
	{CAMCMP_CMD(wb_cloudy),   CAMCMP_SIZE_DAT(CAMCMP_CMD(wb_cloudy)),   NULL,                    0},
	{CAMCMP_CMD(wb_led),      CAMCMP_SIZE_DAT(CAMCMP_CMD(wb_led)),      NULL,                    0},
	{CAMCMP_CMD(night_off),   CAMCMP_SIZE_DAT(CAMCMP_CMD(night_off)),   NULL,                    0},
	{CAMCMP_CMD(night_on),    CAMCMP_SIZE_DAT(CAMCMP_CMD(night_on)),    NULL,                    0},
	{CAMCMP_CMD(movie),       CAMCMP_SIZE_DAT(CAMCMP_CMD(movie)),       NULL,                    0},
	{CAMCMP_CMD(cap_size),    CAMCMP_SIZE_DAT(CAMCMP_CMD(cap_size)),    CAMCMP_BTBL(cap_size),   CAMCMP_SIZE_DAT(CAMCMP_BTBL(cap_size))},    
	{CAMCMP_CMD(moni_size),   CAMCMP_SIZE_DAT(CAMCMP_CMD(moni_size)),   CAMCMP_BTBL(moni_size),  CAMCMP_SIZE_DAT(CAMCMP_BTBL(moni_size))},   
	{CAMCMP_CMD(chg_mode),    CAMCMP_SIZE_DAT(CAMCMP_CMD(chg_mode)),    CAMCMP_BTBL(chg_mode),   CAMCMP_SIZE_DAT(CAMCMP_BTBL(chg_mode))},    
	{CAMCMP_CMD(zoom_5m),     CAMCMP_SIZE_DAT(CAMCMP_CMD(zoom_5m)),     CAMCMP_BTBL(zoom_5m),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(zoom_5m))},     /* <011> */
	{CAMCMP_CMD(zoom_3m),     CAMCMP_SIZE_DAT(CAMCMP_CMD(zoom_3m)),     CAMCMP_BTBL(zoom_3m),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(zoom_3m))},     
	{CAMCMP_CMD(zoom_2m),     CAMCMP_SIZE_DAT(CAMCMP_CMD(zoom_2m)),     CAMCMP_BTBL(zoom_2m),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(zoom_2m))},     
	{CAMCMP_CMD(zoom_1m),     CAMCMP_SIZE_DAT(CAMCMP_CMD(zoom_1m)),     CAMCMP_BTBL(zoom_1m),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(zoom_1m))},     
	{CAMCMP_CMD(zoom_qh),     CAMCMP_SIZE_DAT(CAMCMP_CMD(zoom_qh)),     CAMCMP_BTBL(zoom_qh),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(zoom_qh))},     
	{CAMCMP_CMD(zoom_ql),     CAMCMP_SIZE_DAT(CAMCMP_CMD(zoom_ql)),     CAMCMP_BTBL(zoom_ql),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(zoom_ql))},     
	{CAMCMP_CMD(brightness),  CAMCMP_SIZE_DAT(CAMCMP_CMD(brightness)),  CAMCMP_BTBL(brightness), CAMCMP_SIZE_DAT(CAMCMP_BTBL(brightness))},  
	{CAMCMP_CMD(effect),      CAMCMP_SIZE_DAT(CAMCMP_CMD(effect)),      CAMCMP_BTBL(effect),     CAMCMP_SIZE_DAT(CAMCMP_BTBL(effect))},      
	{CAMCMP_CMD(quality),     CAMCMP_SIZE_DAT(CAMCMP_CMD(quality)),     CAMCMP_BTBL(quality),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(quality))},     
	{CAMCMP_CMD(int_clr),     CAMCMP_SIZE_DAT(CAMCMP_CMD(int_clr)),     CAMCMP_BTBL(int_clr),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(int_clr))},     
	{CAMCMP_CMD(af_cancel),   CAMCMP_SIZE_DAT(CAMCMP_CMD(af_cancel)),   NULL,                    0},                                         
	{CAMCMP_CMD(af_mode),     CAMCMP_SIZE_DAT(CAMCMP_CMD(af_mode)),     CAMCMP_BTBL(af_mode),    CAMCMP_SIZE_DAT(CAMCMP_BTBL(af_mode))},     
	{CAMCMP_CMD(mf_start),    CAMCMP_SIZE_DAT(CAMCMP_CMD(mf_start)),    CAMCMP_BTBL(mf_pos),     CAMCMP_SIZE_DAT(CAMCMP_BTBL(mf_pos))},      
	{CAMCMP_CMD(ae_normal),   CAMCMP_SIZE_DAT(CAMCMP_CMD(ae_normal)),   NULL,                    0},                                         
	{CAMCMP_CMD(ae_long),     CAMCMP_SIZE_DAT(CAMCMP_CMD(ae_long)),     NULL,                    0},                                         
	{CAMCMP_CMD(ptnchk_off),  CAMCMP_SIZE_DAT(CAMCMP_CMD(ptnchk_off)),  NULL,                    0},                                         
	{CAMCMP_CMD(ptnchk_on),   CAMCMP_SIZE_DAT(CAMCMP_CMD(ptnchk_on)),   NULL,                    0},                                         
	{CAMCMP_CMD(chk_af_lock), CAMCMP_SIZE_DAT(CAMCMP_CMD(chk_af_lock)), NULL,                    0},                                         
	{CAMCMP_CMD(thumbnail),   CAMCMP_SIZE_DAT(CAMCMP_CMD(thumbnail)),   NULL,                    0},                                         /* <007> */
	{CAMCMP_CMD(jpeg_limit),  CAMCMP_SIZE_DAT(CAMCMP_CMD(jpeg_limit)),  CAMCMP_BTBL(jpeg_limit), CAMCMP_SIZE_DAT(CAMCMP_BTBL(jpeg_limit))},  /* <014> */
	{CAMCMP_CMD(af_restart),  CAMCMP_SIZE_DAT(CAMCMP_CMD(af_restart)),  NULL,                    0},                                         /* <022> */
	{CAMCMP_CMD(jpeg_intsts), CAMCMP_SIZE_DAT(CAMCMP_CMD(jpeg_intsts)), NULL,                    0},                                         /* <025> */

	{CAMCMP_CMD(otp0l),       CAMCMP_SIZE_DAT(CAMCMP_CMD(otp0l)),       NULL,                    0},
	{CAMCMP_CMD(otp0m),       CAMCMP_SIZE_DAT(CAMCMP_CMD(otp0m)),       NULL,                    0},
	{CAMCMP_CMD(otp0h),       CAMCMP_SIZE_DAT(CAMCMP_CMD(otp0h)),       NULL,                    0},
	{CAMCMP_CMD(otp1l),       CAMCMP_SIZE_DAT(CAMCMP_CMD(otp1l)),       NULL,                    0},
	{CAMCMP_CMD(otp1m),       CAMCMP_SIZE_DAT(CAMCMP_CMD(otp1m)),       NULL,                    0},
	{CAMCMP_CMD(otp1h),       CAMCMP_SIZE_DAT(CAMCMP_CMD(otp1h)),       NULL,                    0},
	{CAMCMP_CMD(otp),         CAMCMP_SIZE_DAT(CAMCMP_CMD(otp)),         NULL,                    0},
	{CAMCMP_CMD(intsts),      CAMCMP_SIZE_DAT(CAMCMP_CMD(intsts)),      NULL,                    0},
/*<006> start*/
	{CAMCMP_CMD(evaluate),    CAMCMP_SIZE_DAT(CAMCMP_CMD(evaluate)),    NULL,                    0},
	{CAMCMP_CMD(lens_pos),    CAMCMP_SIZE_DAT(CAMCMP_CMD(lens_pos)),    NULL,                    0},
	{CAMCMP_CMD(af_step),     CAMCMP_SIZE_DAT(CAMCMP_CMD(af_step)),     NULL,                    0},
	{CAMCMP_CMD(aescl),       CAMCMP_SIZE_DAT(CAMCMP_CMD(aescl)),       NULL,                    0},
	{CAMCMP_CMD(sht_time_h),  CAMCMP_SIZE_DAT(CAMCMP_CMD(sht_time_h)),  NULL,                    0},
	{CAMCMP_CMD(sht_time_l),  CAMCMP_SIZE_DAT(CAMCMP_CMD(sht_time_l)),  NULL,                    0},
	{CAMCMP_CMD(agc_scl),     CAMCMP_SIZE_DAT(CAMCMP_CMD(agc_scl)),     NULL,                    0},
/*<006> end*/
/* <009> add start */
	{CAMCMP_CMD(af_result),   CAMCMP_SIZE_DAT(CAMCMP_CMD(af_result)),   NULL,                    0},
	{CAMCMP_CMD(af_state),    CAMCMP_SIZE_DAT(CAMCMP_CMD(af_state)),    NULL,                    0},
/* <009> add end */
/*<018>add start*/
	{CAMCMP_CMD(manual_step), CAMCMP_SIZE_DAT(CAMCMP_CMD(manual_step)), NULL,                    0},
	{CAMCMP_CMD(area_low_type1),  CAMCMP_SIZE_DAT(CAMCMP_CMD(area_low_type1)),  NULL,            0},
	{CAMCMP_CMD(area_high_type1), CAMCMP_SIZE_DAT(CAMCMP_CMD(area_high_type1)), NULL,            0},
/*<018>add end*/
	{CAMCMP_CMD(jpg_sts),     CAMCMP_SIZE_DAT(CAMCMP_CMD(jpg_sts)),     NULL,                    0},                                         /* <025> */
	{CAMCMP_CMD(jpg_retry),   CAMCMP_SIZE_DAT(CAMCMP_CMD(jpg_retry)),   NULL,                    0},                                         /* <025> */

	{NULL,                    0,                                        NULL,                    0}
};

camcmp_ius006f_af_val_type* g_p_camcmp_ius006f_af = &CAMCMP_DAT(af_val);
uint16_t* g_p_camcmp_ius006f_ver = &CAMCMP_DAT(version);


/* <001> add start */
static camcmp_ius006f_reg_val_type CAMCMP_POLL(chk_af_lock)[] =
{
	{CAMCMP_REG_POLL1, 0x00F8, 0x00000010, NULL,                CAMCMP_D_LEN_BYTE},
	{CAMCMP_REG_POLL2, 0x6D76, 0x000000FF, NULL,                CAMCMP_D_LEN_BYTE}
};

camcmp_data_type g_camcmp_ius006f_poll_list[] =
{
	{(void*)CAMCMP_POLL(chk_af_lock), CAMCMP_SIZE_REG(CAMCMP_POLL(chk_af_lock))}
};
/* <001> add end */

