/* drivers/i2c/chips/akm8977.c - akm8977 compass driver
 *
 * Copyright (C) 2007-2008 HTC Corporation.
 * Author: Hou-Kun Chen <houkun.chen@gmail.com>
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


#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/akm8977.h>

#define DEBUG 0
#define MAX_FAILURE_COUNT 10

#define SENSOR_DATA_SIZE 2432  

static struct i2c_client *this_client;

struct akm8977_data {
	struct input_dev *input_dev;
	struct work_struct work;
};

/* Addresses to scan -- protected by sense_data_mutex */
static char sense_data[RBUFF_SIZE + 1];
static struct mutex sense_data_mutex;
static struct mutex sense_msg_mutex;  

static DECLARE_WAIT_QUEUE_HEAD(data_ready_wq);
static DECLARE_WAIT_QUEUE_HEAD(open_wq);
static DECLARE_WAIT_QUEUE_HEAD(msg_ready_wq);  

static char cspec_num;
static atomic_t cspec_frq;

static atomic_t data_ready;
static atomic_t open_count;
static atomic_t open_flag;
static atomic_t reserve_open_flag;

static atomic_t m_flag;
static atomic_t a_flag;
static atomic_t t_flag;
static atomic_t mv_flag;

static atomic_t msg_ready;  

//static int pffd_mode = 0;
static int failure_count = 0;

static short akmd_delay = 0;

static int  akmd_sensor_msg = -1;  
static char akmd_sensor_data[SENSOR_DATA_SIZE];
static spinlock_t sensor_data_spin_lock;

static atomic_t suspend_flag = ATOMIC_INIT(0);

static struct akm8977_platform_data *pdata;
static int revision = -1;
/* AKM HW info */
static ssize_t gsensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "AK8977_%#x\n", revision);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(vendor, 0444, gsensor_vendor_show, NULL);

static struct kobject *android_gsensor_kobj;

static int gsensor_sysfs_init(void)
{
	int ret ;

	android_gsensor_kobj = kobject_create_and_add("android_gsensor", NULL);
	if (android_gsensor_kobj == NULL) {
		printk(KERN_ERR
		       "akm8977 gsensor_sysfs_init:"\
		       "subsystem_register failed\n");
		ret = -ENOMEM;
		goto err;
	}

	ret = sysfs_create_file(android_gsensor_kobj, &dev_attr_vendor.attr);
	if (ret) {
		printk(KERN_ERR
		       "akm8977 gsensor_sysfs_init:"\
		       "sysfs_create_group failed\n");
		goto err4;
	}

	return 0 ;
err4:
	kobject_del(android_gsensor_kobj);
err:
	return ret ;
}

/* following are the sysfs callback functions */

#define config_ctrl_reg(name,address) \
static ssize_t name##_show(struct device *dev, struct device_attribute *attr, \
			   char *buf) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
        return sprintf(buf, "%u\n", i2c_smbus_read_byte_data(client,address)); \
} \
static ssize_t name##_store(struct device *dev, struct device_attribute *attr, \
			    const char *buf,size_t count) \
{ \
	struct i2c_client *client = to_i2c_client(dev); \
	unsigned long val = simple_strtoul(buf, NULL, 10); \
	if (val > 0xff) \
		return -EINVAL; \
	i2c_smbus_write_byte_data(client,address, val); \
        return count; \
} \
static DEVICE_ATTR(name, S_IWUSR | S_IRUGO, name##_show, name##_store)

//config_ctrl_reg(ms1, AKECS_REG_MS1);
//config_ctrl_reg(ms2, AKECS_REG_MS2);
//config_ctrl_reg(ms3, AKECS_REG_MS3);

static int AKI2C_RxData(char *rxData, int length)
{
	struct i2c_msg msgs[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxData,
		 },
		{
		 .addr = this_client->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxData,
		 },
	};
	if (i2c_transfer(this_client->adapter, msgs, 2) < 0) {
		printk(KERN_ERR "akm8977 AKI2C_RxData: transfer error\n");
		msleep(125);  /* 125ms Wait */
		return -EIO;
	} else {
	}
		return 0;
}

static int AKI2C_TxData(char *txData, int length)
{

	struct i2c_msg msg[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txData,
		 },
	};

	if (i2c_transfer(this_client->adapter, msg, 1) < 0) {
		printk(KERN_ERR "akm8977 AKI2C_TxData: transfer error\n");
		return -EIO;
	} else
		return 0;
}

static int AKECS_Init(void)
{
	return 1;

}

static void AKECS_Reset(void)
{
	gpio_set_value(pdata->reset, 0);
	udelay(120);
	gpio_set_value(pdata->reset, 1);
	udelay(1000);
}


static int AKECS_StartMeasureSNG(void)
{
	char buffer[2];

	/* Set measure mode */
	buffer[0] = AK8977_REG_MS;
	buffer[1] = AK8977_MS_SNG_MEASURE;

	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

static int AKECS_StartMeasureCONT(void)
{
	char buffer[2];

	/* Set measure mode */
	buffer[0] = AK8977_REG_MS;
	buffer[1] = AK8977_MS_CONT_MEASURE;

	/* Set data */
	return AKI2C_TxData(buffer, 2);
}


static int AKECS_PowerDown(void)
{
	char buffer[2];

	/* Set powerdown mode */
	buffer[0] = AK8977_REG_MS;
	buffer[1] = AK8977_MS_POWER_DOWN;
	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

static int AKECS_StartE2PRead(void)
{
	char buffer[2];

	/* Set measure mode */
	buffer[0] = AK8977_REG_MS;
	buffer[1] = AK8977_MS_EEPROM_ACCESS;
	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

static int AKECS_GetData(void)
{
	char buffer[RBUFF_SIZE + 1];
	int ret;

	memset(buffer, 0, RBUFF_SIZE + 1);
	buffer[0] = AK8977_REG_INT1ST;
	/* ST1-ST4 */
	ret = AKI2C_RxData(buffer, RBUFF_SIZE);
	if (ret < 0)
		return ret;

	if((buffer[0] == 0) && (buffer[1] == 0)) {
		return 0;
	}

	mutex_lock(&sense_data_mutex);
	memcpy(sense_data, buffer, sizeof(buffer));
	atomic_set(&data_ready, 1);
	wake_up(&data_ready_wq);
	mutex_unlock(&sense_data_mutex);

	return 0;
}

static int AKECS_SetMode(char mode)
{
	int ret, status;
	char buffer[1];

	if (mode == AKECS_MODE_MEASURE_SNG) {
		/* Check INT pin before mode setting */
		status = gpio_get_value(pdata->intr);
		if (status) {
			buffer[0] = AKECS_REG_TMPS;
			ret = AKI2C_RxData(buffer, 1);
			if (ret < 0)
				return ret;
			status = gpio_get_value(pdata->intr);
		}
	}

	switch (mode) {
	case AKECS_MODE_POWERDOWN:
		ret = AKECS_PowerDown();
		break;
	case AKECS_MODE_MEASURE_SNG:
		ret = AKECS_StartMeasureSNG();
		break;
	case AKECS_MODE_MEASURE_SEQ:
		ret = AKECS_StartMeasureCONT();
		break;
	case AKECS_MODE_E2P_READ:
		ret = AKECS_StartE2PRead();
		break;
//	case AKECS_MODE_SELF_TEST:
//		break;
	default:
		return -EINVAL;
	}

	/* wait at least 300us after changing mode */
	msleep(1);
	return ret;
}

static int AKECS_TransRBuff(char *rbuf, int size)
{


	wait_event_interruptible( data_ready_wq, atomic_read(&data_ready) );  

	if (!atomic_read(&data_ready)) {
		if (!atomic_read(&suspend_flag)) {
			printk(KERN_ERR
			       "akm8977 AKECS_TransRBUFF: Data not ready\n");
			failure_count++;
			if (failure_count >= MAX_FAILURE_COUNT) {
				printk(KERN_ERR
				       "akm8977 AKECS_TransRBUFF:"\
				       "successive %d failure.\n",
				       failure_count);
				atomic_set(&open_flag, -1);
				wake_up(&open_wq);
				failure_count = 0;
			}
		}
		return -EBUSY;  
	}

	mutex_lock(&sense_data_mutex);
	memcpy(rbuf, sense_data, size);
	atomic_set(&data_ready, 0);
	mutex_unlock(&sense_data_mutex);


	failure_count = 0;
	return 0;
}

static int AKECS_Set_PERST(void)
{
	char buffer[2];

	buffer[0] = AK8977_REG_PESE;
	buffer[1] = 0x03;

	/* Set data */
	return AKI2C_TxData(buffer, 2);
}

static int AKECS_Set_G0RST(void)
{
	return 1;

}

static void AKECS_Report_Value(short *rbuf)
{
	struct akm8977_data *data = i2c_get_clientdata(this_client);
	/* Report magnetic sensor information */
	if (atomic_read(&m_flag)) {
		input_report_abs(data->input_dev, ABS_RX, rbuf[0]);
		input_report_abs(data->input_dev, ABS_RY, rbuf[1]);
		input_report_abs(data->input_dev, ABS_RZ, rbuf[2]);
		input_report_abs(data->input_dev, ABS_RUDDER, rbuf[4]);
	}

	/* Report acceleration sensor information */
	if (atomic_read(&a_flag)) {
		input_report_abs(data->input_dev, ABS_X, rbuf[6]);
		input_report_abs(data->input_dev, ABS_Y, rbuf[7]);
		input_report_abs(data->input_dev, ABS_Z, rbuf[8]);
		input_report_abs(data->input_dev, ABS_WHEEL, rbuf[5]);
	}

	/* Report temperature information */
	if (atomic_read(&t_flag)) {
		input_report_abs(data->input_dev, ABS_THROTTLE, rbuf[3]);
	}

	if (atomic_read(&mv_flag)) {
		input_report_abs(data->input_dev, ABS_HAT0X, rbuf[9]);
		input_report_abs(data->input_dev, ABS_HAT0Y, rbuf[10]);
		input_report_abs(data->input_dev, ABS_BRAKE, rbuf[11]);
	}

	input_sync(data->input_dev);
}

static void AKECS_Report_StepCount(short count)
{
	struct akm8977_data *data = i2c_get_clientdata(this_client);

	/* Report pedometer information */
	input_report_abs(data->input_dev, ABS_GAS, count);
	input_sync(data->input_dev);
}

static int AKECS_GetOpenStatus(void)
{
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) != 0));
	return atomic_read(&open_flag);
}

static int AKECS_GetCloseStatus(void)
{
	wait_event_interruptible(open_wq, (atomic_read(&open_flag) <= 0));
	return atomic_read(&open_flag);
}

static void AKECS_CloseDone(void)
{
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);
}

static void AKECS_SetSensorData( unsigned long arg )
{
	char* data = (char*)arg;
	
	spin_lock( &sensor_data_spin_lock );
	memcpy( akmd_sensor_data, data, SENSOR_DATA_SIZE );
	spin_unlock( &sensor_data_spin_lock );
	
	return;
}

static void AKECS_GetSensorData( unsigned long arg )
{
	char* data = (char*)arg;
	
	spin_lock( &sensor_data_spin_lock );
	memcpy( data, akmd_sensor_data, SENSOR_DATA_SIZE );
	spin_unlock( &sensor_data_spin_lock );

	return;
}


static void AKECS_SetMsgData( unsigned long arg )
{

	int *data = (int*)arg;

	while(akmd_sensor_msg != -1){
		msleep(10);
	}
	mutex_lock(&sense_msg_mutex);
	akmd_sensor_msg = *data;
	atomic_set(&msg_ready, 1);
	wake_up(&msg_ready_wq);
	mutex_unlock(&sense_msg_mutex);


	return;
}

static void AKECS_GetMsgData( unsigned long arg )
{

	int *data = (int*)arg;

	wait_event_interruptible(msg_ready_wq, atomic_read(&msg_ready));

	mutex_lock(&sense_msg_mutex);
	*data = akmd_sensor_msg;
	akmd_sensor_msg = -1;
	atomic_set(&msg_ready, 0);
	mutex_unlock(&sense_msg_mutex);


	return;
}


static int AKECS_GetE2PData(char *e2pdata)
{
	char buffer[EEPROM_SIZE];
	int ret;

	memset(buffer, 0, EEPROM_SIZE);
	buffer[0] = AK8977_REG_ST1;
	/* ST1-ST4 */
	ret = AKI2C_RxData(buffer, 0x21);
	if (ret < 0)
		return ret;

	mutex_lock(&sense_data_mutex);
	memcpy(e2pdata, buffer, sizeof(buffer));
	mutex_unlock(&sense_data_mutex);

	return 0;
}

static int akm_aot_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
			atomic_set(&reserve_open_flag, 1);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int akm_aot_release(struct inode *inode, struct file *file)
{
	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

static int
akm_aot_ioctl(struct inode *inode, struct file *file,
	      unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
	case ECS_IOCTL_APP_SET_AFLAG:
	case ECS_IOCTL_APP_SET_TFLAG:
	case ECS_IOCTL_APP_SET_MVFLAG:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MFLAG:
		atomic_set(&m_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MFLAG:
		flag = atomic_read(&m_flag);
		break;
	case ECS_IOCTL_APP_SET_AFLAG:
		atomic_set(&a_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_AFLAG:
		flag = atomic_read(&a_flag);
		break;
	case ECS_IOCTL_APP_SET_TFLAG:
		atomic_set(&t_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_TFLAG:
		flag = atomic_read(&t_flag);
		break;
	case ECS_IOCTL_APP_SET_MVFLAG:
		atomic_set(&mv_flag, flag);
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		flag = atomic_read(&mv_flag);
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		akmd_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = akmd_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MFLAG:
	case ECS_IOCTL_APP_GET_AFLAG:
	case ECS_IOCTL_APP_GET_TFLAG:
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static int akm_pffd_open(struct inode *inode, struct file *file)
{
	int ret = -1;
	if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
		if (atomic_cmpxchg(&open_flag, 0, 2) == 0) {
			atomic_set(&reserve_open_flag, 2);
			wake_up(&open_wq);
			ret = 0;
		}
	}
	return ret;
}

static int akm_pffd_release(struct inode *inode, struct file *file)
{
	atomic_set(&reserve_open_flag, 0);
	atomic_set(&open_flag, 0);
	atomic_set(&open_count, 0);
	wake_up(&open_wq);
	return 0;
}

static int
akm_pffd_ioctl(struct inode *inode, struct file *file,
	       unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	short flag;
	int ret;

	switch (cmd) {
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_RESET_PEDOMETER:
		ret = AKECS_Set_PERST();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		akmd_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = akmd_delay;
		break;
	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static int akmd_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static int akmd_release(struct inode *inode, struct file *file)
{
	AKECS_CloseDone();
	return 0;
}

static int
akmd_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
	   unsigned long arg)
{

	void __user *argp = (void __user *)arg;

	char msg[RBUFF_SIZE + 1], rwbuf[0x80], numfrq[2];
	int ret = -1, status;
	short mode, value[12], step_count, delay;
	char *pbuffer = 0;
//	char sensor_data[SENSOR_DATA_SIZE];
	char e2pdata[EEPROM_SIZE];

	switch (cmd) {
	case ECS_IOCTL_READ:
	case ECS_IOCTL_WRITE:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_MODE:
		if (copy_from_user(&mode, argp, sizeof(mode)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_YPR:
		if (copy_from_user(&value, argp, sizeof(value)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_STEP_CNT:
		if (copy_from_user(&step_count, argp, sizeof(step_count)))
			return -EFAULT;
		break;

//	case ECS_IOCTL_SET_SENSOR_DATA:
//	case ECS_IOCTL_GET_SENSOR_DATA:
//		if (copy_from_user(&sensor_data, argp, sizeof(sensor_data)))
//			return -EFAULT;
//		break;

	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_INIT:
		ret = AKECS_Init();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_RESET:
		AKECS_Reset();
		break;
	case ECS_IOCTL_READ:
		if (rwbuf[0] < 1)
			return -EINVAL;
		ret = AKI2C_RxData(&rwbuf[1], rwbuf[0]);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_WRITE:
		if (rwbuf[0] < 2)
			return -EINVAL;
		ret = AKI2C_TxData(&rwbuf[1], rwbuf[0]);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_MODE:
		ret = AKECS_SetMode((char)mode);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_GETDATA:
		ret = AKECS_TransRBuff(msg, RBUFF_SIZE);
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_GET_NUMFRQ:
		numfrq[0] = cspec_num;
		numfrq[1] = atomic_read(&cspec_frq);
		break;
	case ECS_IOCTL_SET_PERST:
		ret = AKECS_Set_PERST();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_G0RST:
		ret = AKECS_Set_G0RST();
		if (ret < 0)
			return ret;
		break;
	case ECS_IOCTL_SET_YPR:
		AKECS_Report_Value(value);
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
		status = AKECS_GetOpenStatus();
		break;
	case ECS_IOCTL_GET_CLOSE_STATUS:
		status = AKECS_GetCloseStatus();
		break;
	case ECS_IOCTL_SET_STEP_CNT:
		AKECS_Report_StepCount(step_count);
		break;
	case ECS_IOCTL_GET_CALI_DATA:
//		pbuffer = get_akm_cal_ram();
		break;
	case ECS_IOCTL_GET_DELAY:
		delay = akmd_delay;
		break;
	case ECS_IOCTL_SET_SENSOR_DATA:
		AKECS_SetSensorData( arg );
		break;
	case ECS_IOCTL_GET_SENSOR_DATA:
		AKECS_GetSensorData( arg );
		break;
	case ECS_IOCTL_GET_E2PDATA:
		AKECS_GetE2PData(e2pdata);
		break;

	case ECS_IOCTL_SET_SENSOR_MSG:
		AKECS_SetMsgData( arg );
		break;
	case ECS_IOCTL_GET_SENSOR_MSG:
		AKECS_GetMsgData( arg );
		break;

	default:
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if (copy_to_user(argp, &rwbuf, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GETDATA:
		if (copy_to_user(argp, &msg, sizeof(msg)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_NUMFRQ:
		if (copy_to_user(argp, &numfrq, sizeof(numfrq)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
	case ECS_IOCTL_GET_CLOSE_STATUS:
		if (copy_to_user(argp, &status, sizeof(status)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_CALI_DATA:
		if (copy_to_user(argp, pbuffer, MAX_CALI_SIZE))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_DELAY:
		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;
//	case ECS_IOCTL_GET_SENSOR_DATA:
//		if (copy_to_user(argp, &sensor_data, sizeof(sensor_data)))
//			return -EFAULT;
//		break;
	case ECS_IOCTL_GET_E2PDATA:
		if (copy_to_user(argp, e2pdata, EEPROM_SIZE))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static void akm_work_func(struct work_struct *work)
{
	if (AKECS_GetData() < 0)
		printk(KERN_ERR "akm8977 akm_work_func: Get data failed\n");
	enable_irq(this_client->irq);
}

static irqreturn_t akm8977_interrupt(int irq, void *dev_id)
{
	struct akm8977_data *data = dev_id;
	disable_irq_nosync(this_client->irq);
	schedule_work(&data->work);
	return IRQ_HANDLED;
}

static int akm8977_init_client(struct i2c_client *client)
{
	struct akm8977_data *data;
	int ret;

	data = i2c_get_clientdata(client);

	mutex_init(&sense_data_mutex);
	mutex_init(&sense_msg_mutex);  

	ret = request_irq(client->irq, akm8977_interrupt, IRQF_TRIGGER_HIGH,
			  "akm8977", data);

	if (ret < 0) {
		printk(KERN_ERR "akm8977_init_client: request irq failed\n");
		goto err;
	}

	pdata = client->dev.platform_data;
	if (pdata == NULL) {
		pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
		if (pdata == NULL) {
			ret = -ENOMEM;
			goto err_alloc_data_failed;
		} else {
			pdata->reset = ECS_RST;
			pdata->intr = ECS_INTR;
			pdata->intr2 = ECS_INTR2;
		}
	}

	ret = gpio_request(pdata->reset, "akm8977");
	if (ret < 0) {
		printk(KERN_ERR
		       "akm8977_init_client: request reset gpio failed\n");
		goto err_free_irq;
	}
	ret = gpio_direction_output(pdata->reset, 1);
	if (ret < 0) {
		printk(KERN_ERR
		       "akm8977_init_client: request reset gpio failed\n");
		goto err_free_gpio;
	}

	udelay(1000);



	init_waitqueue_head(&data_ready_wq);
	init_waitqueue_head(&open_wq);
	init_waitqueue_head(&msg_ready_wq);  

	/* As default, report all information */
	atomic_set(&m_flag, 1);
	atomic_set(&a_flag, 1);
	atomic_set(&t_flag, 1);
	atomic_set(&mv_flag, 1);

	return 0;

err_free_gpio:
	gpio_free(pdata->reset);
err_free_irq:
	free_irq(client->irq, 0);
err_alloc_data_failed:
err:
	return ret;
}

static struct file_operations akmd_fops = {
	.owner = THIS_MODULE,
	.open = akmd_open,
	.release = akmd_release,
	.ioctl = akmd_ioctl,
};

static struct file_operations akm_aot_fops = {
	.owner = THIS_MODULE,
	.open = akm_aot_open,
	.release = akm_aot_release,
	.ioctl = akm_aot_ioctl,
};

static struct file_operations akm_pffd_fops = {
	.owner = THIS_MODULE,
	.open = akm_pffd_open,
	.release = akm_pffd_release,
	.ioctl = akm_pffd_ioctl,
};

static struct miscdevice akm_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8977_aot",
	.fops = &akm_aot_fops,
};

static struct miscdevice akm_pffd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8977_pffd",
	.fops = &akm_pffd_fops,
};

static struct miscdevice akmd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8977_daemon",
	.fops = &akmd_fops,
};

static int akm8977_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct akm8977_data *akm;
	int err;
	char rxData[2];

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	akm = kzalloc(sizeof(struct akm8977_data), GFP_KERNEL);
	if (!akm) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	INIT_WORK(&akm->work, akm_work_func);
	i2c_set_clientdata(client, akm);
	akm8977_init_client(client);
	this_client = client;
	
	spin_lock_init( &sensor_data_spin_lock );

	/* Set EEPROM access mode */
	err = AKECS_StartE2PRead();
	if (err < 0)
		goto exit_input_dev_alloc_failed;
	/* Read ETS from EEPROM */
	rxData[0] = 0x42;
	err = AKI2C_RxData(rxData, 1);
	if (err < 0)
		goto exit_input_dev_alloc_failed;
	revision = (0x03 & (rxData[0] >> 6));

	/* Set Power down mode */
	err = AKECS_PowerDown();
	if (err < 0)
		goto exit_input_dev_alloc_failed;

	akm->input_dev = input_allocate_device();

	if (!akm->input_dev) {
		err = -ENOMEM;
		printk(KERN_ERR
		       "akm8977_probe: Failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	set_bit(EV_ABS, akm->input_dev->evbit);
	/* yaw */
	input_set_abs_params(akm->input_dev, ABS_RX, 0, 360, 0, 0);
	/* pitch */
	input_set_abs_params(akm->input_dev, ABS_RY, -180, 180, 0, 0);
	/* roll */
	input_set_abs_params(akm->input_dev, ABS_RZ, -90, 90, 0, 0);
	/* x-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_X, -1872, 1872, 0, 0);
	/* y-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_Y, -1872, 1872, 0, 0);
	/* z-axis acceleration */
	input_set_abs_params(akm->input_dev, ABS_Z, -1872, 1872, 0, 0);
	/* temparature */
	input_set_abs_params(akm->input_dev, ABS_THROTTLE, -30, 85, 0, 0);
	/* status of magnetic sensor */
	input_set_abs_params(akm->input_dev, ABS_RUDDER, -32768, 3, 0, 0);
	/* status of acceleration sensor */
	input_set_abs_params(akm->input_dev, ABS_WHEEL, -32768, 3, 0, 0);
	/* step count */
	input_set_abs_params(akm->input_dev, ABS_GAS, 0, 65535, 0, 0);
	/* x-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0X, -2048, 2032, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0Y, -2048, 2032, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_BRAKE, -2048, 2032, 0, 0);

	akm->input_dev->name = "compass";

	err = input_register_device(akm->input_dev);

	if (err) {
		printk(KERN_ERR
		       "akm8977_probe: Unable to register input device: %s\n",
		       akm->input_dev->name);
		goto exit_input_register_device_failed;
	}

	err = misc_register(&akmd_device);
	if (err) {
		printk(KERN_ERR
		       "akm8977_probe: akmd_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&akm_aot_device);
	if (err) {
		printk(KERN_ERR
		       "akm8977_probe: akm_aot_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&akm_pffd_device);
	if (err) {
		printk(KERN_ERR
		       "akm8977_probe: akm_pffd_device register failed\n");
		goto exit_misc_device_register_failed;
	}

//	err = device_create_file(&client->dev, &dev_attr_ms1);
//	err = device_create_file(&client->dev, &dev_attr_ms2);
//	err = device_create_file(&client->dev, &dev_attr_ms3);

	gsensor_sysfs_init();

	return 0;

exit_misc_device_register_failed:
exit_input_register_device_failed:
	input_free_device(akm->input_dev);
exit_input_dev_alloc_failed:
	kfree(akm);
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int akm8977_remove(struct i2c_client *client)
{
	struct akm8977_data *akm = i2c_get_clientdata(client);
	free_irq(client->irq, akm);
	input_unregister_device(akm->input_dev);
	i2c_release_client(client);
	kfree(akm);
	return 0;
}

static int akm8977_suspend(struct i2c_client *client, pm_message_t mesg)
{
	atomic_set(&suspend_flag, 1);
	if (atomic_read(&open_flag) == 2)
		AKECS_SetMode(AKECS_MODE_POWERDOWN);

	atomic_set(&reserve_open_flag, atomic_read(&open_flag));
	atomic_set(&open_flag, 0);
	wake_up(&open_wq);
	disable_irq(this_client->irq);
	return 0;
}

static int akm8977_resume(struct i2c_client *client)
{
	enable_irq(this_client->irq);
	if (atomic_read(&open_flag) == 2)
		AKECS_SetMode(AKECS_MODE_PFFD);
	atomic_set(&suspend_flag, 0);
	atomic_set(&open_flag, atomic_read(&reserve_open_flag));
	wake_up(&open_wq);
	return 0;
}

static const struct i2c_device_id akm8977_id[] = {
	{ "akm8977", 0 },
	{ }
};

static struct i2c_driver akm8977_driver = {
	.probe = akm8977_probe,
	.remove = akm8977_remove,
	.suspend	= akm8977_suspend,
	.resume		= akm8977_resume,
	.id_table = akm8977_id,
	.driver = {
		   .name = "akm8977",
		   },
};

static int __init akm8977_init(void)
{
	printk(KERN_INFO "akm8977 compass driver: init\n");
	return i2c_add_driver(&akm8977_driver);
}

static void __exit akm8977_exit(void)
{
	i2c_del_driver(&akm8977_driver);
}

module_init(akm8977_init);
module_exit(akm8977_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("akm8977 compass driver");
MODULE_LICENSE("GPL");
