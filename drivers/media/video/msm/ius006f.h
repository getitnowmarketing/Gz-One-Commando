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


#ifndef IUS006F_H
#define IUS006F_H

#include <linux/types.h>




#define IUS006F_VIFADRDUMP_NULL		(0x00)
#define IUS006F_VIFADRDUMP_END		(0x01)
#define IUS006F_VIFADRDUMP_NEXT		(0x02)
#define IUS006F_YUV_STTCODE			(0x9A)
#define IUS006F_YUV_ENDCODE			(0x9B)
#define IUS006F_YUV_SUMCODE			((IUS006F_YUV_ENDCODE<<8)&IUS006F_YUV_STTCODE)
#define IUS006F_SET_ADRSTTCODE		(0x9C)
#define IUS006F_SET_ADRENDCODE		(0x9D)
#define IUS006F_ADR_SUMCODE			((IUS006F_SET_ADRENDCODE<<8)&IUS006F_SET_ADRSTTCODE)
#define IUS006F_ILCODELEN2			(0x00)
#define IUS006F_ILCODELEN4			(0x01)



#endif 
