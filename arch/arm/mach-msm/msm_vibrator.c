/* include/asm/mach-msm/htc_pwrsink.h
 *
 * Copyright (C) 2008 HTC Corporation.
 * Copyright (C) 2007 Google, Inc.
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
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <../../../drivers/staging/android/timed_output.h>
#include <linux/sched.h>

#include <mach/msm_rpcrouter.h>


#include <linux/init.h>



#include <mach/msm_battery.h>


#define PM_LIBPROG      0x30000061

#define PM_LIBVERS      0x00030004

#define ONCRPC_PM_VIB_MOT_SET_VOLT_PROC	22


#define PMIC_VIBRATOR_LEVEL	(3000)

#define __DEBUG_LOG   0

#define DEBUG_INFO( arg... )

#define VIB_WORK_NUM		(5)

struct vib_on_work_struct
{
	struct work_struct work_vibrator_on;
	int vibrator_timer;
};

static struct vib_on_work_struct vib_on_work_vibrator_on[VIB_WORK_NUM];
static struct work_struct work_vibrator_off[VIB_WORK_NUM];

static int work_vibrator_on_pos = 0;
static int work_vibrator_off_pos = 0;
static struct hrtimer vibe_timer;

static void set_pmic_vibrator( int on, struct vib_on_work_struct* work )
{
	static struct msm_rpc_endpoint *vib_endpoint;
	struct set_vib_on_off_req {
		struct rpc_request_hdr hdr;
		uint32_t data;
	} req;

	if (!vib_endpoint) {
		vib_endpoint = msm_rpc_connect(PM_LIBPROG, PM_LIBVERS, 0);
		if (IS_ERR(vib_endpoint)) {
			printk(KERN_ERR "init vib rpc failed!\n");
			vib_endpoint = 0;
			return;
		}
	}


    if (on) {
		req.data = cpu_to_be32(PMIC_VIBRATOR_LEVEL);
        
        pm_obs_a_vibration(PM_OBS_VIBRATION_MODE, TRUE);
        
    }
    else {
		req.data = cpu_to_be32(0);
        
        pm_obs_a_vibration(PM_OBS_VIBRATION_MODE, FALSE);
        
    }
    
	msm_rpc_call(vib_endpoint, ONCRPC_PM_VIB_MOT_SET_VOLT_PROC, &req,
		sizeof(req), 5 * HZ);

	if( on && work != NULL)
	{
		hrtimer_start(&vibe_timer,
			      ktime_set(work->vibrator_timer / 1000, (work->vibrator_timer % 1000) * 1000000),
			      HRTIMER_MODE_REL);
	}

}

static void pmic_vibrator_on(struct work_struct *work)
{
	DEBUG_INFO( "[pmic_vibrator_on]vibrator on(1)\n" );
	set_pmic_vibrator( 1, (struct vib_on_work_struct*)work );
}

static void pmic_vibrator_off(struct work_struct *work)
{
	DEBUG_INFO( "[pmic_vibrator_off]vibrator off(0)\n" );
	set_pmic_vibrator( 0, NULL );

}

static void timed_vibrator_on(struct timed_output_dev *sdev, int timeout_val)
{
	vib_on_work_vibrator_on[work_vibrator_on_pos].vibrator_timer = timeout_val;

	if( schedule_work(&(vib_on_work_vibrator_on[work_vibrator_on_pos].work_vibrator_on)) == 0 )
	{
		DEBUG_INFO( " X== Err schedule_work(): work_vibrator_on \n" );
	}
	else
	{
		work_vibrator_on_pos++;
		
		if( work_vibrator_on_pos >= VIB_WORK_NUM )
		{
			work_vibrator_on_pos = 0;
		}
	}
}


static void timed_vibrator_off(struct timed_output_dev *sdev)
{
	if( schedule_work(&work_vibrator_off[work_vibrator_off_pos]) == 0 )
	{
		DEBUG_INFO( " X== Err schedule_work(): work_vibrator_off \n" );
	}
	else
	{
		work_vibrator_off_pos++;
		
		if( work_vibrator_off_pos >= VIB_WORK_NUM )
		{
			work_vibrator_off_pos = 0;
		}
	}
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
	hrtimer_cancel(&vibe_timer);

	if (value == 0)
	{

		DEBUG_INFO( "vibrator_enable : value(%d) = vib off\n", value );

		timed_vibrator_off(dev);
	}
	else {
		value = (value > 15000 ? 15000 : value);

		DEBUG_INFO( "vibrator_enable : value(%d) = vib on\n", value );
		timed_vibrator_on( dev, value );
	}
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{

	DEBUG_INFO( "<< Timer timeout >> timed_vibrator_off call\n" );

	timed_vibrator_off(NULL);
	return HRTIMER_NORESTART;
}

static struct timed_output_dev pmic_vibrator = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

static int __init msm_init_pmic_vibrator(void)
{
	int count = 0;
	
	for( count = 0; count < VIB_WORK_NUM; count++ )
	{
		INIT_WORK(&(vib_on_work_vibrator_on[count].work_vibrator_on), pmic_vibrator_on);
		INIT_WORK(&work_vibrator_off[count], pmic_vibrator_off);
		vib_on_work_vibrator_on[count].vibrator_timer = 0;
	}
	
	work_vibrator_on_pos = 0;
	work_vibrator_off_pos = 0;


	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

	return timed_output_dev_register(&pmic_vibrator);
}


static void __exit msm_exit_pmic_vibrator(void)
{
	timed_output_dev_unregister(&pmic_vibrator);
}


MODULE_DESCRIPTION("timed output pmic vibrator device");
MODULE_LICENSE("GPL");


module_init(msm_init_pmic_vibrator);
module_exit(msm_exit_pmic_vibrator);

