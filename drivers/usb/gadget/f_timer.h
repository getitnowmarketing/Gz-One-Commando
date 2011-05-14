/*
 * Timer control for USB driver.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#ifndef __F_TIMER_H
#define __F_TIMER_H

enum {
        EN_MODE_IDLE = 0,
        EN_MODE_TL_BS,
        EN_MODE_CDROM
};

extern int f_timer_init(int mode, void (*tl_bs_timer_cb)(unsigned long), unsigned long tl_bs_timer_value);
extern int f_timer_mod_timer(void);
extern void f_timer_end(void);

#endif /* __F_TIMER_H */
