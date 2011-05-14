/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

/*
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/i2c/i2c_sensors.h> 
#include <linux/oemnc_info.h>

#define SENSOR_DEBUG		0

#define DEBUG_PRINT( arg... )

#define sensors_common_DRV_NAME	"sensors_common"
#define DRIVER_VERSION		"1.0.0"

/*
 * Defines
 */

enum _e_sensors_type
{
	SENSORS_TYPE_ORIENTATION = 0,
	SENSORS_TYPE_ACCELERATION,
	SENSORS_TYPE_TEMPERATURE,
	SENSORS_TYPE_MAGNETIC_FIELD,
	SENSORS_TYPE_LIGHT,
	SENSORS_TYPE_PROXIMITY,
	SENSORS_TYPE_NUM_MAX
};

unsigned long g_ActiveSensors = 0;
unsigned long g_CallState = 0;
unsigned long g_DelayTime[SENSORS_TYPE_NUM_MAX];
static wait_queue_head_t g_active_queue;
static spinlock_t time_spin_lock;


static int get_sensors_delay( unsigned long arg )
{
	int ret = 0;
	struct delay_time* delay_time = (struct delay_time *)arg;

	spin_lock( &time_spin_lock );
	
	if( delay_time->type >= SENSORS_TYPE_NUM_MAX )
	{
		ret = -EINVAL;
	}
	else
	{
		delay_time->time = g_DelayTime[delay_time->type];

//		printk( KERN_INFO "[set_sensors_delay] delay_time->type(%ld) g_DelayTime(%ld)\n", delay_time->type, g_DelayTime[delay_time->type] );
	}
	
	spin_unlock( &time_spin_lock );

	return ret;
}

static int set_sensors_delay( unsigned long arg )
{
	int ret = 0;
	struct delay_time* delay_time = (struct delay_time *)arg;
	
	spin_lock( &time_spin_lock );

	if( delay_time->type >= SENSORS_TYPE_NUM_MAX )
	{
		ret = -EINVAL;
	}
	else
	{
		g_DelayTime[delay_time->type] = delay_time->time;
//		printk( KERN_INFO "[set_sensors_delay] delay_time->type(%ld) delay_time->time(%ld)\n", delay_time->type, delay_time->time );
	}

	spin_unlock( &time_spin_lock );
	
	return ret;
}


//ioctl
static int sensors_common_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int sensors_common_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int
sensors_common_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{
	int ret = -EINVAL;
	
	if( arg == 0 )
	{
		return ret;
	}
	
	switch( cmd )
	{
		case D_IOCTL_SET_ACTIVE_SENSOR:
			g_ActiveSensors = *((unsigned long*)arg);
//			wake_up( &g_active_queue );
			wake_up_interruptible( &g_active_queue );
			ret = 0;
			break;
			
		case D_IOCTL_GET_ACTIVE_SENSOR:
			*((unsigned long*)arg) = g_ActiveSensors;
			ret = 0;
			break;

		case D_IOCTL_GET_CALL_STATE:
			*((unsigned long*)arg) = g_CallState;
			ret = 0;
			break;
			
		case D_IOCTL_SET_CALL_STATE:
			g_CallState = *((unsigned long*)arg);
			ret = 0;
			break;

		case D_IOCTL_GET_DELAY_TIME:
			ret = get_sensors_delay( arg );
			break;
			
		case D_IOCTL_SET_DELAY_TIME:
			ret = set_sensors_delay( arg );
			break;

		case D_IOCTL_WAIT_GET_ACTIVE_SENSOR:
//			wait_event( g_active_queue, g_ActiveSensors != 0 );
			wait_event_interruptible( g_active_queue, g_ActiveSensors != 0 );
			*((unsigned long*)arg) = g_ActiveSensors;
			ret = 0;
			break;

		case D_IOCTL_GET_HW_REVISION:
			*((unsigned long*)arg) = (unsigned long)hw_revision_read();
			ret = 0;
			break;

		default:
			break;
	}
	
	return ret;
}

static ssize_t sensors_common_write(struct file *file, const char __user *data,
                                   size_t len, loff_t *ppos)
{
	if( len <= 0 )
	{
		return (-1);
	}
	
	if( data[0] == '0' )
	{
		if( g_CallState != 0 )
		{
			printk(KERN_INFO "sensors_common_write: Now CallState(Active) -> New CallState(Idle)\n" );
		}
		
		g_CallState = 0;
	}
	else
	{
		if( g_CallState == 0 )
		{
			printk(KERN_INFO "sensors_common_write: Now CallState(Idle) -> New CallState(Active)\n" );
		}

		g_CallState = 1;
	}
	
	return len;
}

static struct file_operations sensors_common_fops = {
	.owner = THIS_MODULE,
	.open = sensors_common_open,
	.release = sensors_common_release,
	.ioctl = sensors_common_ioctl,
	.write = sensors_common_write,
};

static struct miscdevice sensors_common_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sensors_common",
	.fops = &sensors_common_fops,
};

static int __init sensors_common_init(void)
{
	printk(KERN_INFO "sensors_common_init: " sensors_common_DRV_NAME " driver ver." DRIVER_VERSION "\n" );
	
	init_waitqueue_head( &g_active_queue );
	spin_lock_init( &time_spin_lock );
	
	return misc_register(&sensors_common_device);
}

static void __exit sensors_common_exit(void)
{
	misc_deregister(&sensors_common_device);
}

MODULE_AUTHOR("NEC Communication Systems Co.,Ltd");
MODULE_DESCRIPTION("Sensors Common driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(sensors_common_init);
module_exit(sensors_common_exit);
