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


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>

#include "f_timer.h"

/**< Timer handle for TL-BS */
static struct timer_list f_timer_list;

/**< Timer mode */
static int f_timer_mode = EN_MODE_IDLE;

/**
 * @brief Timer control initialize.
 *
 * @param tl_bs_timer_cb Callback function for TL-BS
 * @param tl_bs_timer_value Callback parameter for TS-BS
 * @return 0 only
 */
int f_timer_init(int mode, void (*tl_bs_timer_cb)(unsigned long), unsigned long tl_bs_timer_value)
{
	printk(KERN_DEBUG "%s: mode = %d, value = %ld\n",
		__func__, mode, tl_bs_timer_value);

	if (f_timer_mode != EN_MODE_IDLE) {
		f_timer_end();
	}

	f_timer_mode = mode;
	memset(&f_timer_list, 0, sizeof(f_timer_list));
	setup_timer(&f_timer_list, tl_bs_timer_cb, tl_bs_timer_value);
	printk(KERN_DEBUG "%s: f_timer_mode = %d\n", __func__, f_timer_mode);
	return 0;
}

/**
 * @brief Modify timer control.
 * @return If the timer is registered, it is one 0 if it is not so. 
 */
int f_timer_mod_timer()
{
	int	ret;

	ret = mod_timer(&f_timer_list, jiffies + msecs_to_jiffies(30000));
	printk(KERN_DEBUG "%s: f_timer_mode = %d mod_timer() ret = %d\n", 
		__func__, f_timer_mode, ret);
	return ret;
}

/**
 * @brief Timer control finilize.
 */
void f_timer_end()
{
	printk(KERN_DEBUG "%s: f_timer_mode = %d\n",
		__func__, f_timer_mode);

	del_timer_sync(&f_timer_list);
	printk(KERN_DEBUG "del_timer_sync: f_timer_mode = %d\n", f_timer_mode);
}
