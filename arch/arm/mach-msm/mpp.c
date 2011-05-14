/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
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


/* Qualcomm PMIC Multi-Purpose Pin Configurations */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/debugfs.h>


#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/timer.h> 
#include <linux/kthread.h> 
#include <linux/spinlock.h>


#include <mach/mpp.h>

#include "proc_comm.h"


#define D_MEAS_STATE_STOP			(0)
#define D_MEAS_STATE_START			(1)

static wait_queue_head_t g_meas_ctrl_queue;
static int g_meas_state = D_MEAS_STATE_STOP;

static int g_open_count = 0;

int mpp_config_digital_out(unsigned mpp, unsigned config)
{
	int err;
	err = msm_proc_comm(PCOM_PM_MPP_CONFIG, &mpp, &config);
	if (err)
		pr_err("%s: msm_proc_comm(PCOM_PM_MPP_CONFIG) failed\n",
		       __func__);
	return err;
}
EXPORT_SYMBOL(mpp_config_digital_out);

int mpp_config_digital_in(unsigned mpp, unsigned config)
{
	int err;
	err = msm_proc_comm(PCOM_PM_MPP_CONFIG_DIGITAL_INPUT, &mpp, &config);
	if (err)
		pr_err("%s: msm_proc_comm(PCOM_PM_MPP_CONFIG) failed\n",
		       __func__);
	return err;
}
EXPORT_SYMBOL(mpp_config_digital_in);


spinlock_t read_lock;
unsigned int g_analog_thermo_data;
wait_queue_head_t dummy_queue;



int mpp_get_analog_in(unsigned channel, unsigned *data)
{
	int err;

	channel |= 0x01000000;
	err = msm_proc_comm(PCOM_PM_MPP_CONFIG_DIGITAL_INPUT, &channel, data);
	if (err)
		printk( KERN_ERR "%s: msm_proc_comm(PCOM_PM_MPP_CONFIG_DIGITAL_INPUT(mpp_get_analog_in)) failed:err(%d)\n",
		       __func__, err);
	return err;
}
EXPORT_SYMBOL(mpp_get_analog_in);


static int thermo_open(struct inode *inode, struct file *file)
{
	spin_lock( &read_lock );
	
	g_open_count++;
	
	if( g_open_count > 0 )
	{
		g_meas_state = D_MEAS_STATE_START;
	}
	spin_unlock( &read_lock );
	
	wake_up( &g_meas_ctrl_queue );
	
	return 0;
}

static int thermo_release(struct inode *inode, struct file *file)
{
	spin_lock( &read_lock );

	g_open_count--;

	if( g_open_count <= 0 )
	{
		g_open_count = 0;
		
		g_meas_state = D_MEAS_STATE_STOP;
	}
	spin_unlock( &read_lock );
	
	wake_up( &g_meas_ctrl_queue );
	
	return 0;
}

static int
thermo_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	int ret = -EINVAL;
	
	return ret;
}


static ssize_t thermo_write(struct file *file, const char __user *data,
                                   size_t len, loff_t *ppos)
{
	unsigned int analog_data = 0;

	mpp_get_analog_in( ADC_MSM_THERM, &analog_data );
	
	printk( KERN_INFO ">>  analog_data: %08X \n", analog_data );
	
	
	return len;
}

static ssize_t thermo_read(struct file *file, char __user *buf, size_t len,
                                  loff_t *ppos)
{
	unsigned int analog_data = 0;

	spin_lock( &read_lock );
	analog_data = g_analog_thermo_data;
	spin_unlock( &read_lock );
	

	
	memcpy( buf, &analog_data, 4 );

	return 4;
}

static struct file_operations thermo_fops = {
	.owner = THIS_MODULE,
	.read		= thermo_read,
	.write		= thermo_write,
	.open = thermo_open,
	.release = thermo_release,
	.ioctl = thermo_ioctl,
};

static struct miscdevice thermo_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "thermo_sensor",
	.fops = &thermo_fops,
};

static int thermo_poll_thread( void* param )
{
	unsigned int analog_data = 0;
	int dummy_wait = 0;
	
	while( 1 )
	{
		wait_event( g_meas_ctrl_queue, g_meas_state == D_MEAS_STATE_START );

		mpp_get_analog_in( ADC_MSM_THERM, &analog_data );
		
		spin_lock( &read_lock );
		g_analog_thermo_data = analog_data;
		spin_unlock( &read_lock );
		
		wait_event_timeout( dummy_queue, dummy_wait != 0, HZ * 5 );
	}
	
	return 0;
}

static int __init thermo_init(void)
{
	int err;
	struct task_struct *th;
	
	spin_lock_init( &read_lock );
	init_waitqueue_head( &dummy_queue );
	init_waitqueue_head( &g_meas_ctrl_queue );

	err = misc_register(&thermo_device);

	if (err)
	{
		printk(KERN_ERR
		       "thermo_init: misc_register() err\n");
		return err;
	}

	th = kthread_create(thermo_poll_thread, NULL, "thermo_sensor_poll");

	if(IS_ERR(th) )
	{
		misc_deregister(&thermo_device);
		
		printk(KERN_ERR
		       "thermo_init: Can't create thread\n");
		return (-1);
	}
	else
	{
		wake_up_process(th);
	}

	
	return err;
}

static void __exit thermo_exit(void)
{
	misc_deregister(&thermo_device);
}

module_init(thermo_init);
module_exit(thermo_exit);


#if defined(CONFIG_DEBUG_FS)
static int test_result;

static int mpp_debug_set(void *data, u64 val)
{
	unsigned mpp = (unsigned) data;

	test_result = mpp_config_digital_out(mpp, (unsigned)val);
	if (test_result) {
		printk(KERN_ERR
			   "%s: mpp_config_digital_out \
			   [mpp(%d) = 0x%x] failed (err=%d)\n",
			   __func__, mpp, (unsigned)val, test_result);
	}
	return 0;
}

static int mpp_debug_get(void *data, u64 *val)
{
	if (!test_result)
		*val = 0;
	else
		*val = 1;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mpp_fops, mpp_debug_get, mpp_debug_set, "%llu\n");

static int __init mpp_debug_init(void)
{
	struct dentry *dent;
	int n;
	char	file_name[16];

	dent = debugfs_create_dir("mpp", 0);
	if (IS_ERR(dent))
		return 0;

	for (n = 0; n < MPPS; n++) {
		snprintf(file_name, sizeof(file_name), "mpp%d", n + 1);
		debugfs_create_file(file_name, 0644, dent,
				    (void *)n, &mpp_fops);
	}

	return 0;
}

device_initcall(mpp_debug_init);
#endif

