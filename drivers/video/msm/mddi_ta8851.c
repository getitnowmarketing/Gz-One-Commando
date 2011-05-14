/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include "mddi_ta8851.h"

#include <mach/gpio.h>

#include <linux/leds-bd6082gul.h> /* For backlight */


#include <linux/unistd.h>
#include <linux/time.h>


#include <mach/synopex.h>


#include <linux/oemnc_info.h>



#define LCD_BACKLIGHT_OFF (0)
#define LCD_BACKLIGHT_ON  (1)


#define _STOP_CMD  0x0000FFFF

#define MDDI_TA8851_HW_REV_ES1B  0x00000008


/* LCD-Panel Internal Status */
static mddi_ta8851_state_t mddi_ta8851_state = TA8851_NORMAL_MODE;

static struct msm_panel_common_pdata *mddi_ta8851_pdata;


#define MDDI_TA8851_VSYNC_TIMEOUT (HZ * 10)       /* 10sec */

static int   mddi_ta8851_lcd_isr_cnt = 0;
struct timeval mddi_ta8851_timetemp[8];
unsigned int mddi_ta8851_difftemp[5] = {0,};
static wait_queue_head_t mddi_ta8851_vsync_wait;
spinlock_t mddi_ta8851_spin_lock;

/* synopex_init_request() is executed only once at the end of mddi_ta8851_set_disp_on(). */
/* After synopex_init_request() call, lcd2synopex_init_req is update to FALSE. */
static boolean lcd2synopex_init_req = TRUE; /* TRUE:need FALSE:executed */

static DEFINE_SPINLOCK(mddi_ta8851_vsync_spin_lock);


unsigned int mddi_ta8851_standby_ctl_flag = 1;


/* Register Access Packet Settings */
/* Initialize*/
static mddi_ta8851_regset lcd_set_power_on_ES1a[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x00000030,   0x00000000 },  /* set_partial_area */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x0000001F },  /*  */
    {  0x00000033,   0x00000000 },  /* set_scroll_area */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000020 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x00000036,   0x00000000 },  /* set_address_mode */ 
    {  0x00000037,   0x00000000 },  /* set_scroll_start */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x0000003A,   0x00000007 },  /* set_pixel_fomat */
    {  0x00000071,   0x00000000 },  /* Ex_Vsync_en */
    {  0x000000B2,   0x00000001 },  /* VCSEL */            
    {  0x000000B4,   0x00000000 },  /* SETVGMM */
    {  0x000000B5,   0x00000032 },  /* RBIAS1 */
    {  0x000000B6,   0x00000003 },  /* RBIAS2 */
    {  0x000000B7,   0x00000011 },  /* SET_DDVDHP */
    {  0x000000AF,   0x00000033 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000004 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000B8,   0x00000011 },  /* SET_DDVDHM */
    {  0x000000AF,   0x00000033 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000004 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000B9,   0x00000002 },  /* SET_VGH */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000BA,   0x00000002 },  /* SET_VGL */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000BB,   0x00000000 },  /* SET_VCL */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000C1,   0x00000001 },  /* Number of Lines */
    {  0x000000C2,   0x00000008 },  /* Number of FP Lines */
    {  0x000000C3,   0x00000004 },  /* GATESET (1) */
    {  0x000000C4,   0x0000004D },  /* 1H period */
    {  0x000000C5,   0x00000000 },  /* Source Precharge */
    {  0x000000C6,   0x00000003 },  /* Source Precharge timing */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000C7,   0x00000000 },  /* Source level */
    {  0x000000C8,   0x00000002 },  /* Number of BP Lines */
    {  0x000000C9,   0x0000001B },  /* GATESET (2) */
    {  0x000000CA,   0x00000000 },  /* GATESET (3) */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000CB,   0x00000003 },  /* GATESET (4) */
    {  0x000000CC,   0x00000012 },  /* GATESET (5) */
    {  0x000000CD,   0x00000012 },  /* GATESET (6) */
    {  0x000000CE,   0x00000030 },  /* GATESET (7) */
    {  0x000000CF,   0x00000030 },  /* GATESET (8) */
    {  0x000000D0,   0x00000040 },  /* GATESET (9) */
    {  0x000000D1,   0x00000022 },  /* FLHW */
    {  0x000000D2,   0x00000024 },  /* VCKHW */
    {  0x000000D3,   0x00000004 },  /* FLT */
    {  0x000000D4,   0x00000014 },  /* TCTRL */
    {  0x000000D6,   0x00000003 },  /* DOTINV */
    {  0x000000D7,   0x00000000 },  /* ON/OFF sequence period */
    {  0x000000D8,   0x00000001 },  /* PONSEQA */
    {  0x000000AF,   0x00000005 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x0000000D },  /*  */
    {  0x000000AF,   0x00000018 },  /*  */
    {  0x000000AF,   0x00000009 },  /*  */
    {  0x000000AF,   0x00000022 },  /*  */
    {  0x000000AF,   0x00000023 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000D9,   0x00000024 },  /* PONSEQB */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000DE,   0x00000009 },  /* PONSEQC */
    {  0x000000AF,   0x0000000F },  /*  */
    {  0x000000AF,   0x00000021 },  /*  */
    {  0x000000AF,   0x00000012 },  /*  */
    {  0x000000AF,   0x00000004 },  /*  */
    {  0x000000DF,   0x00000002 },  /* POFSEQA */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000E0,   0x00000001 },  /* POFSEQB */
    {  0x00000051,   0x000000FF },  /* Manual Brightness */
    {  0x00000052,   0x00000000 },  /* Minimum Brightness */
    {  0x00000053,   0x00000040 },  /* BackLightControl */
    {  0x000000E2,   0x00000000 },  /* CABC PWM */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000E3,   0x00000003 },  /* CABC */
    {  0x000000E4,   0x00000066 },  /* CABC Brightness */
    {  0x000000AF,   0x0000007B },  /*  */
    {  0x000000AF,   0x00000090 },  /*  */
    {  0x000000AF,   0x000000A5 },  /*  */
    {  0x000000AF,   0x000000BB },  /*  */
    {  0x000000AF,   0x000000C7 },  /*  */
    {  0x000000AF,   0x000000E1 },  /*  */
    {  0x000000AF,   0x000000E5 },  /*  */
    {  0x000000E5,   0x000000C5 },  /* CABC Brightness */
    {  0x000000AF,   0x000000C5 },  /*  */
    {  0x000000AF,   0x000000C9 },  /*  */
    {  0x000000AF,   0x000000C9 },  /*  */
    {  0x000000AF,   0x000000D1 },  /*  */
    {  0x000000AF,   0x000000E1 },  /*  */
    {  0x000000AF,   0x000000F1 },  /*  */
    {  0x000000AF,   0x000000FE },  /*  */
    {  0x000000E7,   0x0000002A },  /* CABC */
    {  0x000000E8,   0x00000000 },  /* BRT_REV */
    {  0x000000EA,   0x00000001 },  /* High Speed RAM */
    {  0x000000EB,   0x00000003 },  /* Gamma setting R (pos) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000EC,   0x00000003 },  /* Gamma setting R (neg) */ 
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000ED,   0x00000003 },  /* Gamma setting G (pos) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000EE,   0x00000003 },  /* Gamma setting G (neg) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000EF,   0x00000003 },  /* Gamma setting B (pos) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000F0,   0x00000003 },  /* Gamma setting B (neg) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x00000044,   0x00000000 },  /* set_tear_scanline */
    {  0x000000AF,   0x00000000 },  /*  */                 
    {  0x000000E9,   0x00000000 },  /* TEFREQ */           
    {  0x00000035,   0x00000000 },  /* set_tear_on */      
    {  _STOP_CMD ,   0x00000000 },  /* Stop Data (Dummy) */
};

static mddi_ta8851_regset lcd_set_power_on_ES1b[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x00000030,   0x00000000 },  /* set_partial_area */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x0000001F },  /*  */
    {  0x00000033,   0x00000000 },  /* set_scroll_area */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000020 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x00000036,   0x00000000 },  /* set_address_mode */ 
    {  0x00000037,   0x00000000 },  /* set_scroll_start */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x0000003A,   0x00000007 },  /* set_pixel_fomat */
    {  0x00000071,   0x00000000 },  /* Ex_Vsync_en */
    {  0x000000B2,   0x00000001 },  /* VCSEL */            
    {  0x000000B4,   0x00000000 },  /* SETVGMM */
    {  0x000000B5,   0x00000032 },  /* RBIAS1 */
    {  0x000000B6,   0x00000003 },  /* RBIAS2 */
    {  0x000000B7,   0x00000001 },  /* SET_DDVDHP */       
    {  0x000000AF,   0x00000033 },  /*  */                 
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000004 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */                 
    {  0x000000B8,   0x0000001A },  /* SET_DDVDHM */
    {  0x000000AF,   0x00000053 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000003 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000004 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000B9,   0x00000002 },  /* SET_VGH */          
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */                 
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */                 
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000BA,   0x00000002 },  /* SET_VGL */          
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */                 
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */                 
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000BB,   0x00000000 },  /* SET_VCL */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000C1,   0x00000001 },  /* Number of Lines */
    {  0x000000C2,   0x00000008 },  /* Number of FP Lines */
    {  0x000000C3,   0x00000004 },  /* GATESET (1) */
    {  0x000000C4,   0x0000004D },  /* 1H period */
    {  0x000000C5,   0x00000000 },  /* Source Precharge */
    {  0x000000C6,   0x00000003 },  /* Source Precharge timing */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000C7,   0x00000000 },  /* Source level */
    {  0x000000C8,   0x00000002 },  /* Number of BP Lines */
    {  0x000000C9,   0x0000001B },  /* GATESET (2) */      
    {  0x000000CA,   0x00000000 },  /* GATESET (3) */
    {  0x000000AF,   0x00000002 },  /*  */
    {  0x000000CB,   0x00000003 },  /* GATESET (4) */
    {  0x000000CC,   0x00000012 },  /* GATESET (5) */
    {  0x000000CD,   0x00000012 },  /* GATESET (6) */
    {  0x000000CE,   0x00000030 },  /* GATESET (7) */
    {  0x000000CF,   0x00000030 },  /* GATESET (8) */
    {  0x000000D0,   0x00000040 },  /* GATESET (9) */
    {  0x000000D1,   0x00000022 },  /* FLHW */
    {  0x000000D2,   0x00000024 },  /* VCKHW */
    {  0x000000D3,   0x00000004 },  /* FLT */
    {  0x000000D4,   0x00000014 },  /* TCTRL */
    {  0x000000D6,   0x00000003 },  /* DOTINV */
    {  0x000000D7,   0x00000000 },  /* ON/OFF sequence period */
    {  0x000000D8,   0x00000001 },  /* PONSEQA */
    {  0x000000AF,   0x00000005 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x0000000A },  /*  */
    {  0x000000AF,   0x00000018 },  /*  */
    {  0x000000AF,   0x0000000E },  /*  */
    {  0x000000AF,   0x00000022 },  /*  */
    {  0x000000AF,   0x00000023 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000D9,   0x00000024 },  /* PONSEQB */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000DE,   0x00000009 },  /* PONSEQC */
    {  0x000000AF,   0x0000000D },  /*  */
    {  0x000000AF,   0x00000021 },  /*  */
    {  0x000000AF,   0x00000017 },  /*  */
    {  0x000000AF,   0x00000004 },  /*  */
    {  0x000000DF,   0x00000002 },  /* POFSEQA */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000001 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000E0,   0x00000001 },  /* POFSEQB */
    {  0x00000051,   0x000000FF },  /* Manual Brightness */
    {  0x00000052,   0x00000000 },  /* Minimum Brightness */
    {  0x00000053,   0x00000040 },  /* BackLightControl */
    {  0x000000E2,   0x00000000 },  /* CABC PWM */
    {  0x000000AF,   0x00000000 },  /*  */
    {  0x000000E3,   0x00000003 },  /* CABC */
    {  0x000000E4,   0x00000066 },  /* CABC Brightness */
    {  0x000000AF,   0x0000007B },  /*  */
    {  0x000000AF,   0x00000090 },  /*  */
    {  0x000000AF,   0x000000A5 },  /*  */
    {  0x000000AF,   0x000000BB },  /*  */
    {  0x000000AF,   0x000000C7 },  /*  */
    {  0x000000AF,   0x000000E1 },  /*  */
    {  0x000000AF,   0x000000E5 },  /*  */
    {  0x000000E5,   0x000000C5 },  /* CABC Brightness */
    {  0x000000AF,   0x000000C5 },  /*  */
    {  0x000000AF,   0x000000C9 },  /*  */
    {  0x000000AF,   0x000000C9 },  /*  */
    {  0x000000AF,   0x000000D1 },  /*  */
    {  0x000000AF,   0x000000E1 },  /*  */
    {  0x000000AF,   0x000000F1 },  /*  */
    {  0x000000AF,   0x000000FE },  /*  */
    {  0x000000E7,   0x0000002A },  /* CABC */
    {  0x000000E8,   0x00000000 },  /* BRT_REV */
    {  0x000000EA,   0x00000001 },  /* High Speed RAM */
    {  0x000000EB,   0x00000003 },  /* Gamma setting R (pos) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */
    {  0x000000AF,   0x0000000D },  /*  */
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000EC,   0x00000003 },  /* Gamma setting R (neg) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000ED,   0x00000003 },  /* Gamma setting G (pos) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000EE,   0x00000003 },  /* Gamma setting G (neg) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000EF,   0x00000003 },  /* Gamma setting B (pos) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x000000F0,   0x00000003 },  /* Gamma setting B (neg) */
    {  0x000000AF,   0x00000039 },  /*  */
    {  0x000000AF,   0x00000006 },  /*  */                      
    {  0x000000AF,   0x0000000D },  /*  */                      
    {  0x000000AF,   0x0000001B },  /*  */                      
    {  0x000000AF,   0x000000A8 },  /*  */                      
    {  0x000000AF,   0x00000096 },  /*  */                      
    {  0x000000AF,   0x0000000E },  /*  */                      
    {  0x00000044,   0x00000000 },  /* set_tear_scanline */
    {  0x000000AF,   0x00000000 },  /*  */                 
    {  0x000000E9,   0x00000000 },  /* TEFREQ */           
    {  0x00000035,   0x00000000 },  /* set_tear_on */      
    {  _STOP_CMD ,   0x00000000 },  /* Stop Data (Dummy) */
};


/* Display Start */
static mddi_ta8851_regset lcd_set_disp_on[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x00000029,   0x00000000 },  /* DISPON */
    {  _STOP_CMD,    0x00000000 },  /* Stop Data (Dummy) */
};

/* Display Stop */
static mddi_ta8851_regset lcd_set_disp_off[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x00000028,   0x00000000 },  /* DISPOFF */
    {  _STOP_CMD,    0x00000000 },  /* Stop Data (Dummy) */
};

/* Enter Sleep Mode */
static mddi_ta8851_regset lcd_set_enter_stb[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x00000010,   0x00000000 },  /* Sleep_In */
    {  _STOP_CMD,    0x00000000 },  /* Stop Data (Dummy) */
};

/* Exit Sleep Mode */
static mddi_ta8851_regset lcd_set_exit_stb[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x00000011,   0x00000000 },  /* Sleep_Out */
    {  _STOP_CMD,    0x00000000 },  /* Stop Data (Dummy) */
};

/* Enter Deep Sleep Mode */
static mddi_ta8851_regset lcd_set_enter_deep_stb[] = {
    /* reg_addr, set_data */    /* Instruction */
    {  0x000000FF,   0x00000055 },  /* Password */
    {  0x000000FF,   0x000000AA },  /*  */    
    {  0x000000FF,   0x00000001 },  /*  */    
    {  0x00000070,   0x00000001 },  /* Deep Standby */
    {  _STOP_CMD,    0x00000000 },  /* Stop Data (Dummy) */
};

/* Exit Deep Sleep Mode */
static mddi_ta8851_regset lcd_set_exit_deep_stb[] = {
    /* reg_addr, set_data */    /* Instruction */
    /* Command None */
    {  _STOP_CMD,    0x00000000 },  /* Stop Data (Dummy) */
};

static unsigned mddi_ta8851_LCD_CS =
  GPIO_CFG(24, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA);

static int mddi_ta8851_on(struct platform_device *pdev);
static int mddi_ta8851_off(struct platform_device *pdev);


static void mddi_ta8851_backlight_ctl(int flg);

static void mddi_ta8851_state_transition( mddi_ta8851_state_t current_value,
                                          mddi_ta8851_state_t set_value )
{
    if (mddi_ta8851_state != current_value)
    {
        /* Error Log */
        MDDI_MSG_ERR("ta8851 state trans. (%d->%d) found %d\n", current_value,
                     set_value, mddi_ta8851_state);
    }

    mddi_ta8851_state = set_value;
    return;
}

static void mddi_ta8851_initial_setup( void )
{
    int cnt = 0;
    
    volatile uint32_t hw_revision = 0x00;
    
    hw_revision = hw_revision_read();
    if ( hw_revision == HW_REVISION_READ_ERR )
    {
        MDDI_MSG_ERR("hw_revision_read error");
    }

    /* ES1a : 0.7 / ES1b : 0.8 */
    /* Since ES1b */
    if ( hw_revision >= MDDI_TA8851_HW_REV_ES1B )
    {
        for ( cnt = 0; lcd_set_power_on_ES1b[cnt].reg_addr != _STOP_CMD; cnt++ )
        {
            /* Send Command */
            mddi_ta8851_write( lcd_set_power_on_ES1b[cnt].reg_addr, lcd_set_power_on_ES1b[cnt].set_data, TRUE );
        }
    }
    /* ES1a former */
    else
    {
        for ( cnt = 0; lcd_set_power_on_ES1a[cnt].reg_addr != _STOP_CMD; cnt++ )
        {
            /* Send Command */
            mddi_ta8851_write( lcd_set_power_on_ES1a[cnt].reg_addr, lcd_set_power_on_ES1a[cnt].set_data, TRUE );
        }
    }
    
    
    return;
}

static void mddi_ta8851_set_disp_on( void )
{
    int cnt = 0;

    for ( cnt = 0; lcd_set_disp_on[cnt].reg_addr != _STOP_CMD; cnt++ )
    {
        /* Send Command */
        mddi_ta8851_write( lcd_set_disp_on[cnt].reg_addr, lcd_set_disp_on[cnt].set_data, TRUE );
    }


    mddi_ta8851_backlight_ctl(LCD_BACKLIGHT_ON);


/* ADD-S 2010/06/16 Synopex TouchPanel ES1 */
    if (lcd2synopex_init_req)
    {
        synopex_init_request();
        lcd2synopex_init_req = FALSE;
    }
/* ADD-E 2010/06/16 Synopex TouchPanel ES1 */

    return;
}

static void mddi_ta8851_set_disp_off( void )
{
    int cnt = 0;


    if( mddi_ta8851_standby_ctl_flag == 0 )
        return;

    

    mddi_ta8851_backlight_ctl(LCD_BACKLIGHT_OFF);


    for ( cnt = 0; lcd_set_disp_off[cnt].reg_addr != _STOP_CMD; cnt++ )
    {
        /* Send Command */
        mddi_ta8851_write( lcd_set_disp_off[cnt].reg_addr, lcd_set_disp_off[cnt].set_data, TRUE );
    }

    /* Wait 55ms */
    mddi_wait(55);

    return;
}

static int mddi_ta8851_enter_standby( void )
{
    int cnt = 0;


    if( mddi_ta8851_standby_ctl_flag == 0 )
        return 0;


    /* Enter Standby Mode */
    for ( cnt = 0; lcd_set_enter_stb[cnt].reg_addr != _STOP_CMD; cnt++ )
    {
        /* Send Command */
        mddi_ta8851_write( lcd_set_enter_stb[cnt].reg_addr, lcd_set_enter_stb[cnt].set_data, TRUE );
    }

    /* Wait 101ms */
    mddi_wait(101);

    return 0;
}

static int mddi_ta8851_exit_standby( void )
{
    int cnt = 0;

    /* Exit Standby Mode */
    for ( cnt = 0; lcd_set_exit_stb[cnt].reg_addr != _STOP_CMD; cnt++ )
    {
        /* Send Command */
        mddi_ta8851_write( lcd_set_exit_stb[cnt].reg_addr, lcd_set_exit_stb[cnt].set_data, TRUE );
    }

    /* Wait 101ms */
    mddi_wait(101);

    return 0;
}

static int mddi_ta8851_enter_deep_standby( void )
{
    int cnt = 0;


    if( mddi_ta8851_standby_ctl_flag == 0 )
        return 0;


    /* Enter Deep Standby Mode */
    for ( cnt = 0; lcd_set_enter_deep_stb[cnt].reg_addr != _STOP_CMD; cnt++ )
    {
        /* Send Command */
        mddi_ta8851_write( lcd_set_enter_deep_stb[cnt].reg_addr,
                           lcd_set_enter_deep_stb[cnt].set_data, TRUE );
    }

    /* Wait 2ms */
    mddi_wait(2);

    return 0;
}

static int mddi_ta8851_exit_deep_standby( void )
{
    int cnt = 0;
    int gpio_cnt = 0;
    int ret;

    /* Exit Deep Standby Mode */
    for ( cnt = 0; lcd_set_exit_deep_stb[cnt].reg_addr != _STOP_CMD; cnt++ )
    {
        /* Send Command */
        mddi_ta8851_write( lcd_set_exit_deep_stb[cnt].reg_addr,
                           lcd_set_exit_deep_stb[cnt].set_data, TRUE );
    }

    /* GPIO Setting */
    ret = gpio_tlmm_config(mddi_ta8851_LCD_CS, GPIO_CFG_ENABLE);
    if ( ret )
    {
        MDDI_MSG_ERR("ta8851_set_deep_sleep gpio error");
        return -ENODEV;
    }

    /* LCD_CS = High*/
    gpio_set_value(24, 1);

    for ( gpio_cnt = 0; gpio_cnt < 6; gpio_cnt++ )
    {
        /* LCD_CS = Low*/
        gpio_set_value(24, 0);

        /* LCD_CS = High*/
        gpio_set_value(24, 1);
    }

    /* Wait 11ms */
    mddi_wait(11);

    return 0;
}

int  mddi_ta8851_standby_ctl( int on )
{
    int ret = 0;
    mddi_ta8851_state_t curr_stat;

    /* Status Check */
    curr_stat = mddi_ta8851_state;

    switch ( curr_stat )
    {
        case TA8851_STANDBY :
            if ( !on )
            {
                ret = mddi_ta8851_exit_standby();

                /* after exit standby mode, we must set display on. */
                mddi_ta8851_set_disp_on();

                /* change status */
                mddi_ta8851_state_transition(curr_stat, TA8851_NORMAL_MODE);
            }
            break;

        case TA8851_NORMAL_MODE :
            if ( on )
            {
                /* before enter standby mode, we must set display off. */
                mddi_ta8851_set_disp_off();

                ret = mddi_ta8851_enter_standby();

                /* change status */
                mddi_ta8851_state_transition(curr_stat, TA8851_STANDBY);
            }
            break;

        case TA8851_STATE_OFF :
            /* FALL THROUGH */
        case TA8851_DEEP_STANDBY :
            /* FALL THROUGH */
        case TA8851_READY :
            /* FALL THROUGH */
            /* Error Log */
            MDDI_MSG_ERR("ta8851_standby_ctl error. current:%d\n", curr_stat);
            ret = -EINVAL;
            break;

        default :
            /* Error Log */
            MDDI_MSG_ERR("ta8851_standby_ctl error");
            ret = -ENODEV;
            break;
    }

    return ret;
}

int  mddi_ta8851_deep_standby_ctl( int on )
{
    int ret = 0;
    mddi_ta8851_state_t curr_stat;

    curr_stat = mddi_ta8851_state;

    switch ( curr_stat )
    {
        case TA8851_NORMAL_MODE :
            if ( on )
            {
                /* set display off */
                mddi_ta8851_set_disp_off();

                /* enter standby mode */
                (void)mddi_ta8851_enter_standby();

                /* enter deep standby mode */
                ret = mddi_ta8851_enter_deep_standby();

                /* change internal state */
                mddi_ta8851_state_transition(curr_stat, TA8851_DEEP_STANDBY);
            }
            break;

        case TA8851_DEEP_STANDBY :
            if ( !on )
            {
                /* exit deep standby mode */
                ret = mddi_ta8851_exit_deep_standby();

                /* Initialize setting */
                mddi_ta8851_initial_setup();

                /* exit standby mode */
                (void)mddi_ta8851_exit_standby();

                /* set display on */
                mddi_ta8851_set_disp_on();

                /* change internal state */
                mddi_ta8851_state_transition(curr_stat, TA8851_NORMAL_MODE);
            }
            break;

        case TA8851_STATE_OFF :
            /* FALL THROUGH */
        case TA8851_STANDBY :
            /* FALL THROUGH */
        case TA8851_READY :
            /* Error Log */
            MDDI_MSG_ERR("ta8851_deep_sleep_ctl error. current:%d\n", mddi_ta8851_state);
            ret = -EINVAL;
            break;

        default :
            /* Error Log */
            MDDI_MSG_ERR("ta8851_deep_sleep_ctl error");
            ret = -ENODEV;
            break;
    }

    return ret;
}

void mddi_ta8851_set_idle_state( void )
{
    mddi_ta8851_state_t curr_stat;

    /* Status Check */
    curr_stat = mddi_ta8851_state;

    switch ( curr_stat )
    {
        case TA8851_STATE_OFF :
            /* Initialize setting */
            mddi_ta8851_initial_setup();

            /* Exit Standby Mode */
            (void)mddi_ta8851_exit_standby();
            /* FALL THROUGH */

        case TA8851_READY :
            /* set display on */
            mddi_ta8851_set_disp_on();

            break;

        case TA8851_NORMAL_MODE :
            /* nothing to do in this function. */
            break;

        case TA8851_STANDBY :
            /* Exit Standby Mode */
            (void)mddi_ta8851_exit_standby();

            /* set display on */
            mddi_ta8851_set_disp_on();

            break;

        case TA8851_DEEP_STANDBY :
            /* Exit Deep Standby Mode */
            (void)mddi_ta8851_exit_deep_standby();

            /* Initialize setting */
            mddi_ta8851_initial_setup();

            /* Exit Standby Mode */
            (void)mddi_ta8851_exit_standby();

            /* set display on */
            mddi_ta8851_set_disp_on();

            break;

        default :
            /* Error Log */
            MDDI_MSG_ERR("ta8851_set_idle_state error");
            break;
    }

    /* change internal state */
    mddi_ta8851_state_transition(curr_stat, TA8851_NORMAL_MODE);

    return;
}

static void mddi_ta8851_backlight_ctl(int flg)
{
    int ret;
    struct led_backlight_ctrl led_req;

    /* initialize */
    memset(&led_req, 0x00, sizeof(led_req));

    if (flg == LCD_BACKLIGHT_OFF)
    {
        /* LCD backlight OFF */
        led_req.mleden = BD6082GUL_LED_OFF;
    }
    else
    {
        /* LCD backlight ON */
        led_req.mleden = BD6082GUL_LED_ON;
    }
    /* LCD backlight setting */
    ret = bd6082gul_main_lcd_set(&led_req);
    if ((ret != BD6082GUL_LED_SET_OK) && (ret != BD6082GUL_LED_SET_RESERVED))
    {
        MDDI_MSG_ERR("%s: LCD backlight %s failed! (ret=0x%08x)\n",
                __func__, (flg == LCD_BACKLIGHT_OFF ? "OFF" : "ON"), ret);
    }

    return;
}

static void mddi_ta8851_set_backlight(struct msm_fb_data_type *mfd)
{
    int ret = 0;

    if (mddi_ta8851_pdata && mddi_ta8851_pdata->pmic_backlight) 
    {
        ret = mddi_ta8851_pdata->pmic_backlight(mfd->bl_level);

        if (ret)
            MDDI_MSG_ERR("mddi_ta8851_set_backlight error");
    }

    return;
}

static void mddi_ta8851_vsync_set_handler( msm_fb_vsync_handler_type  handler, 
                                           void                       *arg    )
{
    /* Not Use */
    return;
}

static void mddi_ta8851_vsync_detected( boolean detected )
{
    /* Not Use */
    return;
}

static int mddi_ta8851_on( struct platform_device *pdev )
{
    struct msm_fb_data_type *mfd;
    mddi_ta8851_state_t curr_stat;

    mfd = platform_get_drvdata(pdev);
    if (!mfd)
        return -ENODEV;

    if (mfd->key != MFD_KEY)
        return -EINVAL;

    /* Status Check */
    curr_stat = mddi_ta8851_state;

    switch ( curr_stat )
    {
        case TA8851_STATE_OFF :
            /* FALL THROUGH */
        case TA8851_STANDBY :
            /* Initialize */
            mddi_ta8851_initial_setup();

            /* Standby Out */
            (void)mddi_ta8851_exit_standby();
            break;

        case TA8851_DEEP_STANDBY :
            /* Exit Deep Standby Mode */
            (void)mddi_ta8851_exit_deep_standby();

            /* Initialize */
            mddi_ta8851_initial_setup();

            /* Exit Standby Mode */
            (void)mddi_ta8851_exit_standby();
            break;

        case TA8851_READY :
            /* FALL THROUGH */
        case TA8851_NORMAL_MODE :
            /* no action */
            break;

        default :
            /* Error Log */
            MDDI_MSG_ERR("ta8851_set_idle_state error");
            break;
    }

    /* change internal state */
    if( curr_stat != TA8851_NORMAL_MODE)
        mddi_ta8851_state_transition(curr_stat, TA8851_READY);

    return 0;
}

static int mddi_ta8851_off( struct platform_device *pdev )
{
    struct msm_fb_data_type *mfd;
    mddi_ta8851_state_t curr_stat;

    mfd = platform_get_drvdata(pdev);
    if (!mfd)
        return -ENODEV;

    if (mfd->key != MFD_KEY)
        return -EINVAL;

    /* Status Check */
    curr_stat = mddi_ta8851_state;

    switch (curr_stat)
    {
        case TA8851_STATE_OFF :
            /* FALL THROUGH */
        case TA8851_READY :
            /* Enter Standby Mode */
            (void)mddi_ta8851_enter_standby();
            /* Enter Deep Standby Mode */
            (void)mddi_ta8851_enter_deep_standby();
            break;

        case TA8851_NORMAL_MODE :
            /* Display Off */
            mddi_ta8851_set_disp_off();
            /* Enter Standby Mode */
            (void)mddi_ta8851_enter_standby();
            /* Enter Deep Standby Mode */
            (void)mddi_ta8851_enter_deep_standby();
            break;

        case TA8851_STANDBY :
            /* Enter Deep Standby Mode */
            (void)mddi_ta8851_enter_deep_standby();
            break;

        case TA8851_DEEP_STANDBY :
            break;

        default :
            /* Error Log */
            MDDI_MSG_ERR("ta8851_set_idle_state error");
            break;
    }

    /* change internal state */
    mddi_ta8851_state_transition(curr_stat, TA8851_DEEP_STANDBY);

    return 0;
}

static int __init mddi_ta8851_probe( struct platform_device *pdev )
{

    if (pdev->id == 0)
    {
        mddi_ta8851_pdata = pdev->dev.platform_data;
        return 0;
    }

    msm_fb_add_device(pdev);

    return 0;
}

static struct platform_driver this_driver = {
    .probe  = mddi_ta8851_probe,
    .driver = {
        .name   = "mddi_ta8851",
    },
};

static struct msm_fb_panel_data mddi_ta8851_panel_data = {
    .on   = mddi_ta8851_on,
    .off  = mddi_ta8851_off,
};

void mddi_ta8851_enable_display( void )
{
    mddi_ta8851_state_t curr_stat = mddi_ta8851_state;

    if( curr_stat == TA8851_READY )
    {
        /* display on */
        mddi_ta8851_set_disp_on();

        /* change internal state */
        mddi_ta8851_state_transition(curr_stat, TA8851_NORMAL_MODE);
    }

    return;
}

boolean mddi_ta8851_displaying_chk( void )
{
    if( mddi_ta8851_state == TA8851_NORMAL_MODE )
        return TRUE;
    else
        return FALSE;
}

int mddi_ta8851_device_register(struct msm_panel_info *pinfo,
                                u32 channel, u32 panel)
{
    struct platform_device *pdev = NULL;
    int ret;

    pdev = platform_device_alloc("mddi_ta8851", (panel << 8)|channel);
    if (!pdev)
        return -ENOMEM;

    mddi_ta8851_panel_data.set_backlight = mddi_ta8851_set_backlight;

    if (pinfo->lcd.vsync_enable)
    {
        
        if (pinfo->lcd.hw_vsync_mode)
        {
            mddi_ta8851_panel_data.set_vsync_notifier = NULL;
            mddi_lcd.vsync_detected = NULL;
        }
        else
        {
            mddi_ta8851_panel_data.set_vsync_notifier = mddi_ta8851_vsync_set_handler;
            mddi_lcd.vsync_detected = mddi_ta8851_vsync_detected;
        }
        
    }

    mddi_ta8851_panel_data.panel_info = *pinfo;

    ret = platform_device_add_data(pdev, &mddi_ta8851_panel_data, sizeof(mddi_ta8851_panel_data));
    if (ret) 
    {
        printk(KERN_ERR
          "%s: platform_device_add_data failed!\n", __func__);
        goto err_device_put;
    }

    ret = platform_device_add(pdev);
    if (ret) 
    {
        printk(KERN_ERR
          "%s: platform_device_register failed!\n", __func__);
        goto err_device_put;
    }

    return 0;

err_device_put:
    platform_device_put(pdev);
    return ret;
}

void mddi_ta8851_register_access_packet( uint32_t reg_addr, uint32_t set_data )
{
    mddi_ta8851_write( reg_addr, set_data, TRUE );
    return;
}

irqreturn_t mddi_ta8851_lcd_vsync_isr( int irq, void *data )
{
    unsigned int   channel_irq;
    wait_queue_head_t *wq = &mddi_ta8851_vsync_wait;

    /* get time */
    do_gettimeofday(&mddi_ta8851_timetemp[mddi_ta8851_lcd_isr_cnt]);

    mddi_ta8851_lcd_isr_cnt++;

    /* complete check */
    if (mddi_ta8851_lcd_isr_cnt > 7)
    {
        channel_irq = MSM_GPIO_TO_INT( 30 );
        disable_irq_nosync( channel_irq );
        wake_up( wq );
    }

    return IRQ_HANDLED;
}

int mddi_ta8851_lcd_vsync_mintomax_average( struct msmfb_vsync_ave *vsync_ave )
{
    int           i, j;
    int           ret = 0;
    uint32        temp_val = 0;
    uint32        total_val = 0;
    uint32        ave_val = 0;
    uint32        integer = 0;
    uint32        decimal = 0;
    unsigned long flags;

    spin_lock_irqsave( &mddi_ta8851_spin_lock, flags);
    /* difference compute */
    for (i = 1; i < 6; i++)
    {
        mddi_ta8851_difftemp[i-1] = mddi_ta8851_timetemp[i+1].tv_usec - mddi_ta8851_timetemp[i].tv_usec; 
    }

    /* min to max sorting */
    for (i = 5; i > 0; i--)
    {
        for (j = 0; j < i; j++)
        {
            if (mddi_ta8851_difftemp[j] > mddi_ta8851_difftemp[j+1])
            {
                temp_val = mddi_ta8851_difftemp[j];
                mddi_ta8851_difftemp[j] = mddi_ta8851_difftemp[j+1];
                mddi_ta8851_difftemp[j+1] = temp_val;
            }
        }
    }

    /* total compute */
    for (i = 1; i < 4; i++)
    {
        total_val += mddi_ta8851_difftemp[i];
    }

    /* average compute */
    if (total_val != 0)
    {
        ave_val = total_val / 3;                             /* usec */
        integer = (1000 * 1000) / ave_val;                   /* Hz */
        decimal = ((1000 * 1000) % ave_val) * 10 / ave_val;  /* Hz */
    }
    spin_unlock_irqrestore( &mddi_ta8851_spin_lock, flags );
    
    vsync_ave->integer = integer;
    vsync_ave->decimal = decimal;
    return ret;
}

int mddi_ta8851_mdp_vsync_freq_check( struct msmfb_vsync_ave *vsync_ave )
{
    int            ret = 0;
    long           timeout;
    unsigned int   channel_irq;

    mddi_ta8851_lcd_isr_cnt = 0;

    /* set irq */
    channel_irq = MSM_GPIO_TO_INT( 30 );

    /* Interrupt disable */
    disable_irq_nosync( channel_irq );

    /* set isr */
    ret = gpio_tlmm_config(GPIO_CFG
            (30,
            0,
            GPIO_CFG_INPUT,
            GPIO_CFG_PULL_DOWN,
            GPIO_CFG_2MA),
            GPIO_CFG_ENABLE);

    ret = request_irq( channel_irq,
                       mddi_ta8851_lcd_vsync_isr,
                       IRQF_TRIGGER_RISING,
                       "VSYNC_GPIO",
                       0 );

    /* Interrupt enable */
    enable_irq( channel_irq );

    /* wait queue */
    init_waitqueue_head( &mddi_ta8851_vsync_wait );
    timeout = wait_event_timeout( mddi_ta8851_vsync_wait, mddi_ta8851_lcd_isr_cnt > 7, MDDI_TA8851_VSYNC_TIMEOUT );
    
    /* free isr */
    free_irq( channel_irq, 0 );
    
    /* timeout check */
    if(timeout == 0)
    {
        printk(KERN_ERR "mddi_ta8851_mdp_vsync_freq_check time out! \n");
        ret = -ENODEV;
    } 
    else
    {
        /* calculate */
        ret = mddi_ta8851_lcd_vsync_mintomax_average( vsync_ave );
    }
    

    return ret;
}

void mddi_ta8851_set_vsync_freq( struct msmfb_vsync_parame *vsync_parame, void* data )
{
    unsigned int vsync_freq;
    struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)data;

    if(vsync_parame->set_data == 0x0000FFFF)
    {
        return;
    }

    /* convert value */
    vsync_freq = (vsync_parame->set_data >> 8);
    vsync_freq = (vsync_freq * 100) + (vsync_parame->set_data & 0xFF)*10;

    spin_lock( &mddi_ta8851_vsync_spin_lock );

    /* set value */
    mfd->panel_info.lcd.refx100 = vsync_freq;
    mfd->lcd_ref_usec_time =
        100000000 / mfd->panel_info.lcd.refx100;

    spin_unlock( &mddi_ta8851_vsync_spin_lock );

    
    mdp_hw_vsync_update_config(mfd);
    

    return;
}

static int __init mddi_ta8851_init(void)
{
    return platform_driver_register(&this_driver);
}

int mddi_ta8851_user_request_ctrl( struct msmfb_request_parame *data )
{
    int ret = 0;
    
    switch( data->request )
    {
        case MSM_FB_REQUEST_OVERLAY_ALPHA:
            ret = copy_from_user(&mdp4_overlay_argb_enable, data->data, sizeof(mdp4_overlay_argb_enable));
            break;
        
        default:
            /* Error Log */
            MDDI_MSG_ERR("%s user_request error", __func__);
            break;
    }
    
    return ret;
}

void mddi_ta8851_set_standby_ctl( int on )
{
    mddi_ta8851_standby_ctl_flag = on;
    
    return;
}

void mddi_ta8851_notify_reboot( void )
{
    mddi_ta8851_state_t curr_stat;

    /* Status Check */
    curr_stat = mddi_ta8851_state;

    /* LCD Panel Set "Standby Status" */
    switch ( curr_stat )
    {
        case TA8851_READY :
            /* Enter Standby Mode */
            (void)mddi_ta8851_enter_standby();
            break;

        case TA8851_NORMAL_MODE :
            /* Display Off */
            mddi_ta8851_set_disp_off();
            /* Enter Standby Mode */
            (void)mddi_ta8851_enter_standby();
            break;

        case TA8851_DEEP_STANDBY :

            /* Nothing to Do */

        case TA8851_STANDBY :
            /* FALL THROUGH */
        case TA8851_STATE_OFF :
            /* Nothing to Do */
            break;
        default :
            /* Error Log */
            MDDI_MSG_ERR("mddi_ta8851_notify_reboot status error!!\n");
            break;
    }


    if((curr_stat != TA8851_STATE_OFF ) && (curr_stat != TA8851_DEEP_STANDBY ))

    {
        /* change internal state */
        mddi_ta8851_state_transition(curr_stat, TA8851_STANDBY);
    }

    return;
}


module_init(mddi_ta8851_init);
