/*===========================================================================

                DIAG APPS CORE AUDIO DRIVER    MODULE

DESCRIPTION
  **********

===========================================================================*/


/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#include <asm/uaccess.h>         /* copy_from_user, copy_to_user */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/fs.h>            /* inode, file, file_operations */
#include <linux/kernel.h>        /* printk()                     */
#include <linux/miscdevice.h>
#include <linux/smp_lock.h>      /* wait queue                   */

#include "Diag_acad.h"
#include "Diag_qtr_mic.h"
#include "Diag_com_appl.h"

static DECLARE_WAIT_QUEUE_HEAD(open_wq);

static atomic_t reserve_open_flag;
static atomic_t open_count;
static atomic_t open_flag;

/*===========================================================================

FUNCTION diag_acad_ioctl

DESCRIPTION
  
DEPENDENCIES
  
RETURN VALUE
  

===========================================================================*/
DEFINE_MUTEX(msm_acad_sem);


static int diag_acad_ioctl(struct inode *inode, struct file *filp,
                           unsigned int cmd, unsigned long arg )
{
    diag_acad_ctrl  acad_ctl;
    void __user *argp = (void __user *)arg;
    int                        ret;

    printk( KERN_INFO "diag_acad I/O Control Start \n");


    if( 
        (cmd == DIAG_ACAD_IOCTL_15)  ||
        (cmd == DIAG_ACAD_IOCTL_16)  ||
        (cmd == DIAG_ACAD_IOCTL_21)  ||
        (cmd == DIAG_ACAD_IOCTL_0201)  ||
        (cmd == DIAG_ACAD_IOCTL_0202) ) {
        if ( copy_from_user( &acad_ctl, argp, sizeof(acad_ctl) ) ) {
            printk( KERN_ERR "diag_acad I/O Control : copy_from_user ERROR \n");
            return -EFAULT;
        }
    }

    
    mutex_lock(&msm_acad_sem);

    switch (cmd) {
    
    case DIAG_ACAD_IOCTL_15:
        printk( KERN_INFO "diag_acad I/O Control : Cmd = DIAG_ACAD_IOCTL_15 \n");

        ret = Diag_Audio_qtr_action( (Diag_Audio_qtr_action_type *)&acad_ctl.act );
        if( ret != 0x00 ) {
            printk( KERN_ERR "diag_acad I/O Control RET error \n");
            mutex_unlock(&msm_acad_sem);
            return -EFAULT;
        }
        break;

    
    case DIAG_ACAD_IOCTL_16:
        printk( KERN_INFO "diag_acad I/O Control : Cmd = DIAG_ACAD_IOCTL_16 \n");

        ret = Diag_Audio_qtr_loopback( (Diag_Audio_qtr_loopback_type *)&acad_ctl.lb );
        if( ret != 0x00 ) {
            printk( KERN_ERR "diag_acad I/O Control RET error \n");
            mutex_unlock(&msm_acad_sem);
            return -EFAULT;
        }
        break;

    
    case DIAG_ACAD_IOCTL_21:
        printk( KERN_INFO "diag_acad I/O Control : Cmd = DIAG_ACAD_IOCTL_21 \n");

        ret = Diag_Audio_hsedbias_ctl( (Diag_Audio_hsedbias_ctl_type *)&acad_ctl.hsed );
        if( ret != 0x00 ) {
            printk( KERN_ERR "diag_acad I/O Control RET error \n");
            mutex_unlock(&msm_acad_sem);
            return -EFAULT;
        }
        break;

    
    case DIAG_ACAD_IOCTL_0201:
        printk( KERN_INFO "diag_acad I/O Control : Cmd = DIAG_ACAD_IOCTL_0201 \n");
        Diag_Audio_vol_limit_wrapper( acad_ctl.req, acad_ctl.rsp );

        
        if ( copy_to_user(argp, &acad_ctl, sizeof(acad_ctl) ) ) {
            printk( KERN_ERR "diag_acad I/O Control : copy_to_user ERROR \n");
            mutex_unlock(&msm_acad_sem);
            return -EFAULT;
        }

        break;

    
    case DIAG_ACAD_IOCTL_0202:
        printk( KERN_INFO "diag_acad I/O Control : Cmd = DIAG_ACAD_IOCTL_0202 \n");
        Diag_Audio_qtr_cal_wrapper( acad_ctl.req, acad_ctl.rsp );

        
        if ( copy_to_user(argp, &acad_ctl, sizeof(acad_ctl) ) ) {
            printk( KERN_ERR "diag_acad I/O Control : copy_to_user ERROR \n");
            mutex_unlock(&msm_acad_sem);
            return -EFAULT;
        }

        break;
    default:
        printk( KERN_ERR "diag_acad I/O Control : Cmd = default \n");
        mutex_unlock(&msm_acad_sem);
        return -EFAULT;
    }
    
    mutex_unlock(&msm_acad_sem);

    printk( KERN_INFO "diag_acad I/O Control End \n");

    return 0;
}

/*===========================================================================

FUNCTION diag_acad_open

DESCRIPTION
  
DEPENDENCIES
  
RETURN VALUE
  

===========================================================================*/
static int diag_acad_open(struct inode *inode, struct file *file)
{
    if (atomic_cmpxchg(&open_count, 0, 1) == 0) {
        if (atomic_cmpxchg(&open_flag, 0, 2) == 0) {
            atomic_set(&reserve_open_flag, 2);
            wake_up(&open_wq);
//            printk( KERN_INFO "diag_acad_OPEN : OK \n");
            return 0;
        }
    }
    printk( KERN_ERR "diag_acad_OPEN : ERROR \n");
    return -1;
}

/*===========================================================================

FUNCTION diag_acad_release

DESCRIPTION
  
DEPENDENCIES
  
RETURN VALUE
  

===========================================================================*/
static int diag_acad_release(struct inode *inode, struct file *file)
{
    atomic_set(&reserve_open_flag, 0);
    atomic_set(&open_flag, 0);
    atomic_set(&open_count, 0);
    wake_up(&open_wq);
//    printk( KERN_INFO "diag_acad_RELEASE : OK \n");
    return 0;
}

static struct file_operations diag_acad_ctl_fops = {
    .owner    = THIS_MODULE,
    .ioctl    = diag_acad_ioctl,
    .open     = diag_acad_open,
    .release  = diag_acad_release,
};

static struct miscdevice diag_acad_dev = {
    .minor = MISC_DYNAMIC_MINOR ,
    .name = DIAG_ACAD_DEVICE_NAME ,
    .fops = &diag_acad_ctl_fops ,
};

static int __init Diag_acad_init(void)
{
    int ret;

    ret = misc_register(&diag_acad_dev);
    if ( ret ) {
        printk( KERN_ERR "Diag_acad_INIT : FAIL to misc_register \n");
        return ret;
    }
    
    mutex_init(&msm_acad_sem);
//    printk( KERN_INFO "Diag_acad_INIT : OK to misc_register \n");

    return 0;
}

static void __exit Diag_acad_exit(void)
{
    
    misc_deregister(&diag_acad_dev);

//    printk( KERN_INFO "Diag_acad_EXIT \n");
    return;
}

module_init(Diag_acad_init);
module_exit(Diag_acad_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Diag Apps Audio core driver");
MODULE_LICENSE("GPL");

