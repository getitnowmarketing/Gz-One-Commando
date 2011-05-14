/*
 */
/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/timer.h> 
#include <linux/kthread.h> 
#include <linux/spinlock.h>

#include <linux/i2c/gp2ap012a00f.h> 


#include <mach/msm_battery.h>



#define DEBUG_PRINT( arg... )

#define DEBUG_PRINT2( arg... )

#define GP2AP012A00F_DRV_NAME   "gp2ap012a00f"
#define DRIVER_VERSION      "2.0.0"

/*
 * Defines
 */


//mode change request
#define D_REQ_MODE_NONE             (D_MODE_ALS - 1)
#define D_REQ_MODE_ALS              (D_MODE_ALS)
#define D_REQ_MODE_PROX             (D_MODE_PROX)
#define D_REQ_MODE_DIAG             (D_MODE_DIAG)

//measurement state
#define D_MEAS_STATE_STOP           (0)
#define D_MEAS_STATE_START          (1)

typedef enum _e_int_mode
{
    E_INT_MODE_LOW_LEVEL = 0,
    E_INT_MODE_HIGH_LEVEL,
    E_INT_MODE_FALLING_EDGE,
    E_INT_MODE_RASING_EDGE,
    E_INT_MODE_MAX
}E_INT_MODE;

typedef enum _e_range_mode
{
    E_RANGE_MODE_1 = 0,
    E_RANGE_MODE_2,
    E_RANGE_MODE_3,
    E_RANGE_MODE_MAX
}E_RANGE_MODE;

#define D_RANGE_MODE_HIGH_TH        (0x3E80)
#define D_RANGE_MODE_LOW_TH         (0x05DC)
#define D_RANGE_MODE_LOW_3_TH       (0x04E2)

/*
 * Structs
 */
struct gp2ap012a00f_data
{
    struct input_dev *input_dev;
    struct work_struct work;
};

/*
 * Global data
 */
static spinlock_t sense_data_spin_lock;

static struct i2c_client *this_client;

static unsigned char g_prox_reg_data[D_ADDR_MAX];
static unsigned char g_ALS_reg_data[D_ADDR_MAX];

static unsigned char g_nv_ALS_data[D_ADDR_MAX];
static unsigned char g_nv_prox_data[D_ADDR_MAX];
static unsigned char g_nv_ALS_filter_ratio;
static unsigned char g_nv_prox_filter_ratio;
static int g_is_nv_set = 0;

static int g_measurement_mode = D_MODE_ALS;
static int g_diag_mode = 0;
static wait_queue_head_t g_mode_change_queue;

static wait_queue_head_t g_mesurement_queue;
static int g_is_measurement_comp = 0;
static int g_measurement_count = 0;
static unsigned long g_measurement_data[5] = { 0, 0, 0, 0, 0 };
static int g_clear_flag = 0;

static int g_mode_change_request = D_REQ_MODE_NONE;

static wait_queue_head_t g_meas_ctrl_queue;
static int g_meas_state = D_MEAS_STATE_STOP;

static int g_open_count = 0;

static int g_RANGE_mode = 0;

static int g_detect_state = D_PROX_DETECT_STATE_NON_DETECT;

/*
 * Management functions
 */
 
static int i2c_rx_data(char *rxData, int length)
{
    int ret = 0;
    struct i2c_msg msgs[] = 
    {
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

    DEBUG_PRINT( "i2c_rx_data(): rxData(0x%08X) rxData[0](%08X) rxData[1](%08X) length(%d) slave_addr(0x%02X)\n", (unsigned int)rxData, rxData[0], rxData[1], length, this_client->addr );
    
    if (i2c_transfer(this_client->adapter, msgs, 2) < 0)
    {
        printk(KERN_ERR "GP2AP012A00F gp2ap012a00f_RxData: transfer error\n");
        ret = -EIO;
    }
    
    DEBUG_PRINT( "i2c_rx_data(): rxData[0](%08X) rxData[1](%08X) length(%d) slave_addr(0x%02X)\n", rxData[0], rxData[1], length, this_client->addr );

    return ret;
}

static int i2c_tx_data(char *txData, int length)
{
    int ret = 0;
    struct i2c_msg msg[] =
    {
        {
            .addr = this_client->addr,
            .flags = 0,
            .len = length,
            .buf = txData,
        },
    };

    if (i2c_transfer(this_client->adapter, msg, 1) < 0)
    {
        printk(KERN_ERR "GP2AP012A00F gp2ap012a00f_TxData: transfer error\n");
        ret = -EIO;
    }
    
    return ret;
}

static int get_register( unsigned char addr, unsigned char* pval, int len )
{
    int ret = 0;

    DEBUG_PRINT( "get_register() start: addr(%02X) pval(0x%08X) pval[0](%02X) len(%d)\n", addr, (unsigned int)pval, pval[0], len );

    //register addr
    pval[0] = addr;
    
    ret = i2c_rx_data( pval, len );
    
    DEBUG_PRINT( "get_register() end: addr(%02X) pval(0x%08X) pval[0](%02X) pval[1](%02X) len(%d)\n", addr, (unsigned int)pval, pval[0], pval[1], len );

    return ret;
}


static int set_register( unsigned char addr, unsigned char val )
{
    int ret = 0;

    unsigned char data[2];

    //register addr
    data[0] = addr;
    
    //register data
    data[1] = val;
    
    ret = i2c_tx_data( data, 2 );
    
    if( ret == 0 )
    {
        g_prox_reg_data[addr] = val;
    }
    
    return ret;
}

static int register_ctrl( int mode, unsigned long arg )
{
    int ret = 0;
    T_GP2AP012A00F_IOCTL_REG* ioctl_data = (T_GP2AP012A00F_IOCTL_REG*)arg;
    unsigned char data[2] = {0, 0};

    if( mode == D_IOCTL_SET_REG )
    {
        ret = set_register( ioctl_data->address, (unsigned char)ioctl_data->data );

        if( ret != 0 )
        {
            printk(KERN_ERR "GP2AP012A00F register_ctrl: Can't set register data\n");
        }
        else
        {
            if( (ioctl_data->address != D_ADDR_DATA_LSB) && (ioctl_data->address != D_ADDR_DATA_MSB) )
            {
                g_prox_reg_data[ioctl_data->address] = (unsigned char)ioctl_data->data;
                g_ALS_reg_data[ioctl_data->address] = (unsigned char)ioctl_data->data;
            }
            
            DEBUG_PRINT( "GP2AP012A00F register_ctrl: IOCTL_SET(ADDR=%02lX DATA=0x%02lX) \n", ioctl_data->address, ioctl_data->data);
        }
    }
    else
    {
        ret = get_register( ioctl_data->address, (unsigned char*)data, 2 );
        
        if( ret != 0 )
        {
            printk(KERN_ERR "GP2AP012A00F register_ctrl: Can't get register data\n");
        }
        else
        {
            ioctl_data->data = (((unsigned long)data[0]) & 0x000000FF);
            DEBUG_PRINT( "IOCTL_GET: data[0]=(%02X) data[1]=(%02X)\n", data[0], data[1] );
            DEBUG_PRINT( "GP2AP012A00F register_ctrl: IOCTL_GET(ADDR=%02lX DATA=0x%02lX) \n", ioctl_data->address, ioctl_data->data);

            if( (ioctl_data->address != D_ADDR_DATA_LSB) && (ioctl_data->address != D_ADDR_DATA_MSB) )
            {
                g_prox_reg_data[ioctl_data->address] = (unsigned char)ioctl_data->data;
                g_ALS_reg_data[ioctl_data->address] = (unsigned char)ioctl_data->data;
            }
            
        }
    }
    
    return ret;
}


static int get_sensor_data( unsigned long arg )
{
    int ret = 0;
    T_GP2AP012A00F_IOCTL_SENSOR* sensor_data = (T_GP2AP012A00F_IOCTL_SENSOR*)arg;
    int count = 0;
    
    if( g_is_nv_set == 0 )
    {
        return -EAGAIN;
    }

    if( g_mode_change_request != D_REQ_MODE_NONE )
    {
        
        wait_event( g_mesurement_queue, g_mode_change_request == D_REQ_MODE_NONE );
    }

    spin_lock( &sense_data_spin_lock );

    if( g_is_measurement_comp == 0 )        //1st read
    {
        spin_unlock( &sense_data_spin_lock );
        
        wait_event( g_mesurement_queue, g_is_measurement_comp != 0 );

        spin_lock( &sense_data_spin_lock );
    }
    
    if( g_measurement_mode == D_MODE_ALS )          //als
    {
        sensor_data->data = (((g_ALS_reg_data[D_ADDR_DATA_MSB] << 8 ) | g_ALS_reg_data[D_ADDR_DATA_LSB]) & 0xFFFF);
        sensor_data->sensor_mode = D_MODE_ALS;
        
        for( count = 0; count < D_ADDR_MAX; count++ )
        {
            sensor_data->reg_data[count] = g_ALS_reg_data[count];
        }
    }
    else                                                //prox
    {
        sensor_data->data = (((g_prox_reg_data[D_ADDR_DATA_MSB] << 8 ) | g_prox_reg_data[D_ADDR_DATA_LSB]) & 0xFFFF);
        sensor_data->sensor_mode = D_MODE_PROX;
        sensor_data->detect_state = g_detect_state;

        for( count = 0; count < D_ADDR_MAX; count++ )
        {
            sensor_data->reg_data[count] = g_prox_reg_data[count];
        }
    }
    
    spin_unlock( &sense_data_spin_lock );
    
    return ret;
}

static int als_or_prox( unsigned int mode )
{
    int ret = 0;
    unsigned char data[2];
    int addr = 0;

    DEBUG_PRINT( "als_or_prox: mode(%u)\n", mode );

    if( mode == D_MODE_PROX )                               // prox mode
    {
        //clear data
        memset( g_prox_reg_data, 0x0, D_ADDR_MAX );
        
        

        spin_lock( &sense_data_spin_lock );
        for( addr = 0; addr < D_ADDR_MAX; addr++ )
        {
            if( addr != D_ADDR_DATA_LSB && addr != D_ADDR_DATA_MSB )
            {
                g_prox_reg_data[addr] = g_nv_prox_data[addr];
            }
            else
            {
                g_prox_reg_data[addr] = 0;
            }
        }
        spin_unlock( &sense_data_spin_lock );
        
        //COMMAND1 register(lastest set)
        for( addr = D_ADDR_MAX - 1; addr >= 0; addr-- )
        {
            if( addr != D_ADDR_DATA_LSB && addr != D_ADDR_DATA_MSB )
            {
                data[0] = addr;
                data[1] = g_prox_reg_data[addr];
                
                ret = i2c_tx_data( data, 2 );
                
                if( ret != 0 )
                {
                    printk( KERN_ERR "i2c err!! als_or_prox :prox addr(0x%02X)\n", addr );
                    break;
                }
            }
        }
        
        
        pm_obs_a_sensor(PM_OBS_SENSOR_KINSETSU_ON);
        
    }
    else
    {                                               // ALS mode
        //clear data
        memset( g_ALS_reg_data, 0x0, D_ADDR_MAX );
        

        //ALS
        spin_lock( &sense_data_spin_lock );
        for( addr = 0; addr < D_ADDR_MAX; addr++ )
        {
            if( addr != D_ADDR_DATA_LSB && addr != D_ADDR_DATA_MSB )
            {
                g_ALS_reg_data[addr] = g_nv_ALS_data[addr];
            }
            else
            {
                g_ALS_reg_data[addr] = 0;
            }
        }
        spin_unlock( &sense_data_spin_lock );

        //register set( COMMAND1 register lastest set ).
        for( addr = D_ADDR_MAX - 1; addr >= 0; addr-- )
        {
            if( addr != D_ADDR_DATA_LSB && addr != D_ADDR_DATA_MSB )
            {
                data[0] = addr;
                data[1] = g_ALS_reg_data[addr];
                
                ret = i2c_tx_data( data, 2 );

                if( ret != 0 )
                {
                    printk( KERN_ERR "i2c err!! als_or_prox :ALS addr(0x%02X)\n", addr );
                    break;
                }
            }
        }
        
        
        
        pm_obs_a_sensor(PM_OBS_SENSOR_SYOUDO_ON);
        
    }

    return ret;
}

static int get_measurement_mode( unsigned long arg )
{
    int ret = 0;
    T_GP2AP012A00F_IOCTL_SENSOR* sensor_data = (T_GP2AP012A00F_IOCTL_SENSOR*)arg;

    if( g_is_nv_set == 0 )
    {
        return -EAGAIN;
    }
    
    if( sensor_data != NULL )
    {
        sensor_data->sensor_mode = g_measurement_mode;
    }
    else
    {
        ret = (-1);
    }

    return ret;
}

static int set_measurement_mode( unsigned long arg )
{
    int ret = 0;
    T_GP2AP012A00F_IOCTL_SENSOR* sensor_data = (T_GP2AP012A00F_IOCTL_SENSOR*)arg;
    wait_queue_head_t dummy_queue;
    int dummy_wait = 0;
    init_waitqueue_head( &dummy_queue );
    
    if( sensor_data->sensor_mode == D_MODE_DIAG_OFF )
    {
        DEBUG_PRINT2( KERN_INFO "set_measurement_mode: diag mode(OFF)\n" );
        g_diag_mode = 0;
        wake_up( &g_mode_change_queue );
        return ret;
    }
    
    if( g_measurement_mode != sensor_data->sensor_mode )
    {
        if( sensor_data->sensor_mode == D_MODE_DIAG ||
            sensor_data->sensor_mode == D_MODE_DIAG_ALS || 
            sensor_data->sensor_mode == D_MODE_DIAG_PROX )
        {
            DEBUG_PRINT2( "set_measurement_mode: diag mode(ON)\n" );
            g_diag_mode = 1;
        }
        else
        {
            wait_event( g_mode_change_queue, g_diag_mode == 0 );
        }
        
        while( 1 )
        {
            if( g_diag_mode == 1 )
            {
                if( sensor_data->sensor_mode == D_MODE_DIAG ||
                    sensor_data->sensor_mode == D_MODE_DIAG_ALS || 
                    sensor_data->sensor_mode == D_MODE_DIAG_PROX )
                {
                    sensor_data->sensor_mode &= ~(D_MODE_DIAG_BIT);
                    break;
                }
                
                wait_event_timeout( dummy_queue, dummy_wait != 0, (HZ / 10) );
            }
            else
            {
                break;
            }
        }

        
        if( sensor_data->sensor_mode == D_MODE_PROX )
        {
            DEBUG_PRINT2( "set_measurement_mode: mode(%d) -> (PROX)\n", g_measurement_mode );
            spin_lock( &sense_data_spin_lock );
            g_mode_change_request = D_REQ_MODE_PROX;
            g_is_measurement_comp = 0;
            spin_unlock( &sense_data_spin_lock );
            wake_up( &g_mesurement_queue );

            
            wait_event( g_mesurement_queue, g_mode_change_request == D_REQ_MODE_NONE );
        }
        else if( sensor_data->sensor_mode == D_MODE_ALS )
        {
            DEBUG_PRINT2( "set_measurement_mode: mode(%d) -> (ALS)\n", g_measurement_mode );
            spin_lock( &sense_data_spin_lock );
            g_mode_change_request = D_REQ_MODE_ALS;
            g_is_measurement_comp = 0;
            spin_unlock( &sense_data_spin_lock );
            wake_up( &g_mesurement_queue );
            
            
            wait_event( g_mesurement_queue, g_mode_change_request == D_REQ_MODE_NONE );
        }
        else if( sensor_data->sensor_mode == D_MODE_DIAG )
        {
            DEBUG_PRINT2( "set_measurement_mode: mode(%d) -> (DIAG)\n", g_measurement_mode );
            spin_lock( &sense_data_spin_lock );
            g_mode_change_request = D_REQ_MODE_DIAG;
            g_is_measurement_comp = 0;
            spin_unlock( &sense_data_spin_lock );
            
            
            wait_event( g_mesurement_queue, g_measurement_mode == D_MODE_DIAG );
        }
        else
        {
            ret = (-1);
        }
    }

    return ret;
}

static void clear_sensor_data( void )
{
    spin_lock( &sense_data_spin_lock );
    g_clear_flag = 1;
    
    g_measurement_count = 0;
    memset( g_measurement_data, 0x0, sizeof(unsigned long) * 5 );
    g_is_measurement_comp = 0;
    g_ALS_reg_data[D_ADDR_DATA_LSB] = 0;
    g_ALS_reg_data[D_ADDR_DATA_MSB] = 0;
    g_prox_reg_data[D_ADDR_DATA_LSB] = 0;
    g_prox_reg_data[D_ADDR_DATA_MSB] = 0;
    spin_unlock( &sense_data_spin_lock );

    return;
}

static int set_nv_data( unsigned long arg )
{
    int ret = 0;
    int addr = 0;
    T_GP2AP012A00F_IOCTL_NV* nv_data = (T_GP2AP012A00F_IOCTL_NV*)arg;

    DEBUG_PRINT2( KERN_INFO "%s: start.\n", __func__ );
    
    spin_lock( &sense_data_spin_lock );
    for( addr = 0; addr < D_ADDR_MAX; addr++ )
    {
        if( addr != D_ADDR_DATA_LSB && addr != D_ADDR_DATA_MSB )
        {
            g_nv_ALS_data[addr]  = (unsigned char)(nv_data->ALS_nv_reg[addr]);
            g_nv_prox_data[addr] = (unsigned char)(nv_data->prox_nv_reg[addr]);
        }
    }
    
    g_nv_ALS_filter_ratio    = (unsigned char)(nv_data->ALS_nv_filter_ratio);
    g_nv_prox_filter_ratio   = (unsigned char)(nv_data->prox_nv_filter_ratio);
    
    spin_unlock( &sense_data_spin_lock );
    
    if( g_is_nv_set == 0 )
    {
        als_or_prox( D_MODE_ALS );
        g_measurement_mode = D_MODE_ALS;
        g_is_nv_set = 1;
        wake_up( &g_meas_ctrl_queue );
    }
    
    DEBUG_PRINT2( KERN_INFO "%s: NV data\n"
                    "ALS  reg[COMMAND_1]  : (%02X)  prox reg[COMMAND_1]  : (%02X)\n"
                    "     reg[COMMAND_2]  : (%02X)       reg[COMMAND_2]  : (%02X)\n"
                    "     reg[COMMAND_3]  : (%02X)       reg[COMMAND_3]  : (%02X)\n"
                    "     reg[DATA_LSB]   : (%02X)       reg[DATA_LSB]   : (%02X)\n"
                    "     reg[DATA_MSB]   : (%02X)       reg[DATA_MSB]   : (%02X)\n"
                    "     reg[INT_LT_LSB] : (%02X)       reg[INT_LT_LSB] : (%02X)\n"
                    "     reg[INT_LT_MSB] : (%02X)       reg[INT_LT_MSB] : (%02X)\n"
                    "     reg[INT_HT_LSB] : (%02X)       reg[INT_HT_LSB] : (%02X)\n"
                    "     reg[INT_HT_MSB] : (%02X)       reg[INT_HT_MSB] : (%02X)\n"
                    "     filter_ratio    : (%3d)        filter_ratio    : (%3d) \n"
                    , __func__
                    , g_nv_ALS_data[D_ADDR_COMMAND_1],  g_nv_prox_data[D_ADDR_COMMAND_1]
                    , g_nv_ALS_data[D_ADDR_COMMAND_2],  g_nv_prox_data[D_ADDR_COMMAND_2]
                    , g_nv_ALS_data[D_ADDR_COMMAND_3],  g_nv_prox_data[D_ADDR_COMMAND_3]
                    , g_nv_ALS_data[D_ADDR_DATA_LSB],   g_nv_prox_data[D_ADDR_DATA_LSB]
                    , g_nv_ALS_data[D_ADDR_DATA_MSB],   g_nv_prox_data[D_ADDR_DATA_MSB]
                    , g_nv_ALS_data[D_ADDR_INT_LT_LSB], g_nv_prox_data[D_ADDR_INT_LT_LSB]
                    , g_nv_ALS_data[D_ADDR_INT_LT_MSB], g_nv_prox_data[D_ADDR_INT_LT_MSB]
                    , g_nv_ALS_data[D_ADDR_INT_HT_LSB], g_nv_prox_data[D_ADDR_INT_HT_LSB]
                    , g_nv_ALS_data[D_ADDR_INT_HT_MSB], g_nv_prox_data[D_ADDR_INT_HT_MSB]
                    , g_nv_ALS_filter_ratio, g_nv_prox_filter_ratio
                     );

    DEBUG_PRINT2( KERN_INFO "%s: end.\n", __func__ );
    
    return ret;
}

static int get_nv_data( unsigned long arg )
{
    int ret = 0;
    int addr = 0;
    T_GP2AP012A00F_IOCTL_NV* nv_data = (T_GP2AP012A00F_IOCTL_NV*)arg;

    DEBUG_PRINT2( KERN_INFO "%s: start.\n", __func__ );

    if( g_is_nv_set == 0 )
    {
        return -EAGAIN;
    }

    spin_lock( &sense_data_spin_lock );
    for( addr = 0; addr < D_ADDR_MAX; addr++ )
    {
        nv_data->ALS_nv_reg[addr]  = ((unsigned long)g_nv_ALS_data[addr])  & 0x000000FF;
        nv_data->prox_nv_reg[addr] = ((unsigned long)g_nv_prox_data[addr]) & 0x000000FF;
    }
    
    nv_data->ALS_nv_filter_ratio  = ((unsigned long)g_nv_ALS_filter_ratio)  & 0x000000FF;
    nv_data->prox_nv_filter_ratio = ((unsigned long)g_nv_prox_filter_ratio) & 0x000000FF;
    
    spin_unlock( &sense_data_spin_lock );

    DEBUG_PRINT2( KERN_INFO "%s: NV data\n"
                    "ALS  reg[COMMAND_1]  : (%02X)  prox reg[COMMAND_1]  : (%02X)\n"
                    "     reg[COMMAND_2]  : (%02X)       reg[COMMAND_2]  : (%02X)\n"
                    "     reg[COMMAND_3]  : (%02X)       reg[COMMAND_3]  : (%02X)\n"
                    "     reg[DATA_LSB]   : (%02X)       reg[DATA_LSB]   : (%02X)\n"
                    "     reg[DATA_MSB]   : (%02X)       reg[DATA_MSB]   : (%02X)\n"
                    "     reg[INT_LT_LSB] : (%02X)       reg[INT_LT_LSB] : (%02X)\n"
                    "     reg[INT_LT_MSB] : (%02X)       reg[INT_LT_MSB] : (%02X)\n"
                    "     reg[INT_HT_LSB] : (%02X)       reg[INT_HT_LSB] : (%02X)\n"
                    "     reg[INT_HT_MSB] : (%02X)       reg[INT_HT_MSB] : (%02X)\n"
                    "     filter_ratio    : (%3d)        filter_ratio    : (%3d) \n"
                    , __func__
                    , g_nv_ALS_data[D_ADDR_COMMAND_1],  g_nv_prox_data[D_ADDR_COMMAND_1]
                    , g_nv_ALS_data[D_ADDR_COMMAND_2],  g_nv_prox_data[D_ADDR_COMMAND_2]
                    , g_nv_ALS_data[D_ADDR_COMMAND_3],  g_nv_prox_data[D_ADDR_COMMAND_3]
                    , g_nv_ALS_data[D_ADDR_DATA_LSB],   g_nv_prox_data[D_ADDR_DATA_LSB]
                    , g_nv_ALS_data[D_ADDR_DATA_MSB],   g_nv_prox_data[D_ADDR_DATA_MSB]
                    , g_nv_ALS_data[D_ADDR_INT_LT_LSB], g_nv_prox_data[D_ADDR_INT_LT_LSB]
                    , g_nv_ALS_data[D_ADDR_INT_LT_MSB], g_nv_prox_data[D_ADDR_INT_LT_MSB]
                    , g_nv_ALS_data[D_ADDR_INT_HT_LSB], g_nv_prox_data[D_ADDR_INT_HT_LSB]
                    , g_nv_ALS_data[D_ADDR_INT_HT_MSB], g_nv_prox_data[D_ADDR_INT_HT_MSB]
                    , g_nv_ALS_filter_ratio, g_nv_prox_filter_ratio
                     );

    DEBUG_PRINT2( KERN_INFO "%s: end.\n", __func__ );
    
    return ret;
}

static int gp2ap012a00f_soft_reset(void)
{
    int ret = 0;
    unsigned char data[2];
    
    //register addr
    data[0] = D_ADDR_COMMAND_3;
    
    //soft reset
    data[1] = D_DATA_BIT_RST;
    
    ret = i2c_tx_data( data, 2 );
    
    mdelay( 1 );
    
    return ret;
}

//Init Module
static int gp2ap012a00f_init_module(void)
{
    // software reset
    gp2ap012a00f_soft_reset();
    
    
    
    pm_obs_a_sensor(PM_OBS_SENSOR_OFF);
    
    
    return 0;
}

//ioctl
static int gp2ap012a00f_open(struct inode *inode, struct file *file)
{

    spin_lock( &sense_data_spin_lock );
    
    g_open_count++;
    
    if( g_is_nv_set != 0 )
    {
        if( g_open_count > 0 )
        {
            g_meas_state = D_MEAS_STATE_START;
        }
        spin_unlock( &sense_data_spin_lock );
        
        wake_up( &g_meas_ctrl_queue );
    }
    else
    {
        spin_unlock( &sense_data_spin_lock );
    }
    
    return 0;
}

static int gp2ap012a00f_release(struct inode *inode, struct file *file)
{
    spin_lock( &sense_data_spin_lock );

    g_open_count--;

    if( g_is_nv_set != 0 )
    {
        if( g_open_count <= 0 )
        {
            g_open_count = 0;
            
            g_meas_state = D_MEAS_STATE_STOP;
        }
        spin_unlock( &sense_data_spin_lock );
        
        wake_up( &g_meas_ctrl_queue );
    }
    else
    {
        if( g_open_count > 0 )
        {
            g_meas_state = D_MEAS_STATE_START;
        }
        spin_unlock( &sense_data_spin_lock );
        
        wake_up( &g_meas_ctrl_queue );
    }

    return 0;
}

static int
gp2ap012a00f_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
       unsigned long arg)
{
    int ret = -EINVAL;
    
    switch( cmd )
    {
        case D_IOCTL_SET_REG:
            //fall through
        case D_IOCTL_GET_REG:
            ret = register_ctrl( cmd, arg );
            break;
            
        case D_IOCTL_SET_SENSOR_MODE:
            ret = set_measurement_mode( arg );
            break;

        case D_IOCTL_GET_SENSOR_MODE:
            ret = get_measurement_mode( arg );
            break;
            
        case D_IOCTL_GET_SENSOR_DATA:
            ret = get_sensor_data( arg );
            break;

        case D_IOCTL_CLEAR_SENSOR_DATA:
            clear_sensor_data();
            ret = 0;
            break;

        case D_IOCTL_SET_NV:
            ret = set_nv_data( arg );
            break;

        case D_IOCTL_GET_NV:
            ret = get_nv_data( arg );
            break;

        default:
            break;
    }
    
    return ret;
}

static struct file_operations gp2ap012a00f_fops = {
    .owner = THIS_MODULE,
    .open = gp2ap012a00f_open,
    .release = gp2ap012a00f_release,
    .ioctl = gp2ap012a00f_ioctl,
};

static struct miscdevice gp2ap012a00f_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "prox_sensor",
    .fops = &gp2ap012a00f_fops,
};


/*
 * Initialization function
 */

static int gp2ap012a00f_init_client(struct i2c_client *client)
{
    struct gp2ap012a00f_data *data;
//  int ret;

    data = i2c_get_clientdata(client);

    spin_lock_init( &sense_data_spin_lock );


    return 0;
}

static unsigned long calc_measurement_val( void )
{
    int count = 0;
    unsigned long measurement_data = 0;
    
    unsigned long lowest_data = 0;
    unsigned long highest_data = 0;
    unsigned long lowest_no = 0;
    unsigned long highest_no = 0;
    int first = 0;
    
    //lowenst data
    for( count = 0; count < 5; count++ )
    {
        if( count == 0 )
        {
            lowest_data = g_measurement_data[count];
            lowest_no = count;
        }
        else
        {
            if( lowest_data > g_measurement_data[count] )
            {
                lowest_data = g_measurement_data[count];
                lowest_no = count;
            }
        }
    }

    //highest data
    first = 0;
    for( count = 0; count < 5; count++ )
    {
        if( count == lowest_no )
        {
            continue;
        }
        
        if( first == 0 )
        {
            highest_data = g_measurement_data[count];
            highest_no = count;
            first = 1;
        }
        else
        {
            if( highest_data < g_measurement_data[count] )
            {
                highest_data = g_measurement_data[count];
                highest_no = count;
            }
        }
    }
    
    //calc data
    for( count = 0; count < 5; count++ )
    {
        if( count != highest_no && count != lowest_no )
        {
            measurement_data += g_measurement_data[count];
        }
    }
    
    measurement_data /= 3;
    
    return measurement_data;
}

static int ALS_measurement_start( void )
{
    int ret = 0;
    unsigned char data[2];

    
    g_ALS_reg_data[D_ADDR_COMMAND_2] &= ~( D_DATA_BIT_RANGE );

    if( g_RANGE_mode == E_RANGE_MODE_2 )
    {
        
        g_ALS_reg_data[D_ADDR_COMMAND_2] |= ( D_DATA_BIT_RANGE3 );
    }
    else if( g_RANGE_mode == E_RANGE_MODE_3 )
    {
        
        g_ALS_reg_data[D_ADDR_COMMAND_2] |= ( D_DATA_BIT_RANGE );
    }
    else        /* E_RANGE_MODE_1 */
    {
        
        g_ALS_reg_data[D_ADDR_COMMAND_2] |= ( D_DATA_BIT_RANGE0 );
    }
    
    data[0] = D_ADDR_COMMAND_2;
    data[1] = g_ALS_reg_data[D_ADDR_COMMAND_2];

    ret = i2c_tx_data( data, 2 );
    
    if( ret != 0 )
    {
        printk( KERN_ERR "i2c err!! ALS_measurement_start :Can't set range reg.\n" );
    }
    
    g_ALS_reg_data[D_ADDR_COMMAND_1] |= ( D_DATA_BIT_OP3 ); 
    
    data[0] = D_ADDR_COMMAND_1;
    data[1] = g_ALS_reg_data[D_ADDR_COMMAND_1];

    ret = i2c_tx_data( data, 2 );
    
    if( ret != 0 )
    {
        printk( KERN_ERR "i2c err!! ALS_measurement_start :Can't start measurement.\n" );
    }

    return ret;
}

static int ALS_measurement( void )
{
    unsigned char data[2];
    int ret = 0;
    int is_measurement_comp = (-1);
    unsigned long get_data = 0;
    unsigned long measurement_data = 0;
    int wait_time = 0;
    unsigned char res = 0;
    int is_RANGE_mode_chg = 0;

    DEBUG_PRINT( "ALS_measurement()\n" );
    
    memset( data, 0x0, 2 );

    
    ALS_measurement_start();

    res = (g_ALS_reg_data[D_ADDR_COMMAND_2] & D_DATA_BIT_RES) >> D_DATA_BIT_SHIFT_NUM_RES0;

    wait_time = 130;
    
    msleep( wait_time );

    while( 1 )
    {
        ret = get_register( D_ADDR_COMMAND_1, data, 2 );
        
        if( ret != 0 )
        {
            return is_measurement_comp;
        }
        
        if( ( data[0] & D_DATA_BIT_OP3 ) == 0 )
        {
            g_ALS_reg_data[D_ADDR_COMMAND_1] = data[0];
            break;
        }
    }
    
    //get LSB data
    ret = get_register( D_ADDR_DATA_LSB, data, 2 );
    
    if( ret != 0 )
    {
        DEBUG_PRINT( "ALS_measurement() : Err LSB get.\n" );
        return is_measurement_comp;
    }
    
    get_data = ( ((unsigned long)data[0]) & 0x000000FF );
    
    memset( data, 0x0, 2 );

    //get MSB data
    ret = get_register( D_ADDR_DATA_MSB, data, 2 );
    
    if( ret != 0 )
    {
        DEBUG_PRINT( "ALS_measurement() : Err MSB get.\n" );
        return is_measurement_comp;
    }

    get_data |= (( (unsigned long)(data[0] << 8) ) & 0x0000FF00);

    spin_lock( &sense_data_spin_lock );

    g_measurement_data[g_measurement_count] = get_data;

    DEBUG_PRINT( "ALS_measurement() : count(%d) data(%lu)\n", g_measurement_count, g_measurement_data[g_measurement_count] );

    g_measurement_count++;

    if( g_measurement_count >= 5 )
    {
        g_measurement_count = 0;
        
        measurement_data = calc_measurement_val();

        if( measurement_data >= D_RANGE_MODE_HIGH_TH )
        {
            if( g_RANGE_mode < E_RANGE_MODE_3 )
            {
                /* reset measurement data */
                g_measurement_count = 0;
                memset( g_measurement_data, 0x0, sizeof(unsigned long) * 5 );
                g_is_measurement_comp = 0;
                DEBUG_PRINT2( KERN_INFO "ALS_measurement() : range mode UP (%d)->(%d) count(%08lX)\n", g_RANGE_mode, g_RANGE_mode+1, measurement_data );

                g_RANGE_mode++;
                is_RANGE_mode_chg = 1;
            }
        }
        else if( ( g_RANGE_mode == E_RANGE_MODE_2 && measurement_data <= D_RANGE_MODE_LOW_TH ) || 
                 ( g_RANGE_mode == E_RANGE_MODE_3 && measurement_data <= D_RANGE_MODE_LOW_3_TH ) )
        {
            /* reset measurement data */
            g_measurement_count = 0;
            memset( g_measurement_data, 0x0, sizeof(unsigned long) * 5 );
            g_is_measurement_comp = 0;
            DEBUG_PRINT2( KERN_INFO "ALS_measurement() : range mode DOWN (%d)->(%d) count(%08lX)\n", g_RANGE_mode, g_RANGE_mode-1, measurement_data );

            g_RANGE_mode--;
            is_RANGE_mode_chg = 1;
        }
        else
        {
            /* fall through */
        }
        
        if( is_RANGE_mode_chg == 0 )
        {
            //set measurement data
            g_ALS_reg_data[D_ADDR_DATA_LSB] = measurement_data & 0x000000FF;
            g_ALS_reg_data[D_ADDR_DATA_MSB] = (( measurement_data >> 8 ) & 0x000000FF);

            is_measurement_comp = 1;
        }
        else
        {
            /* re-measurement */
            is_measurement_comp = 0;
        }

        DEBUG_PRINT( "ALS_measurement() : calc data(%lu)\n", measurement_data );
    }
    else
    {
        is_measurement_comp = 0;
    }

    spin_unlock( &sense_data_spin_lock );

    return is_measurement_comp;
}

static int prox_measurement_start( void )
{
    int ret = 0;
    unsigned char data[2];
    
    g_prox_reg_data[D_ADDR_COMMAND_1] |= ( D_DATA_BIT_OP3 );    
    
    data[0] = D_ADDR_COMMAND_1;
    data[1] = g_prox_reg_data[D_ADDR_COMMAND_1];

    ret = i2c_tx_data( data, 2 );
    
    if( ret != 0 )
    {
        printk( KERN_ERR "i2c err!! prox_measurement_start :Can't start measurement.\n" );
    }

    return ret;
}

static int prox_measurement( void )
{
    unsigned char data[2];
    int ret = 0;
    int is_measurement_comp = (-1);
    unsigned long get_data = 0;
    unsigned long measurement_data = 0;
    int wait_time = 0;
    unsigned char res = 0;
    unsigned short high_threshold = 0;
    unsigned short low_threshold = 0;

    memset( data, 0x0, 2 );
    
    
    prox_measurement_start();
    
    
//  mdelay( 6 );

    res = (g_prox_reg_data[D_ADDR_COMMAND_2] & D_DATA_BIT_RES) >> D_DATA_BIT_SHIFT_NUM_RES0;

    if( res == 0 )
    {
        wait_time = 100 * 2;
    }
    else if( res == 1 )
    {
        wait_time = 25 * 2;
    }
    else if( res == 2 )
    {
        wait_time = 6 * 2;
    }
    else if( res == 3 )
    {
        wait_time = 1 * 2;
    }
    else
    {
        wait_time = 0;
    }
    
    if( wait_time != 0 )
    {
        msleep( wait_time );
    }

    while( 1 )
    {
        ret = get_register( D_ADDR_COMMAND_1, data, 2 );
        
        if( ret != 0 )
        {
            return is_measurement_comp;
        }
        
        if( ( data[0] & D_DATA_BIT_OP3 ) == 0 )
        {
            g_prox_reg_data[D_ADDR_COMMAND_1] = data[0];
            break;
        }
    }
    
    //get LSB data
    ret = get_register( D_ADDR_DATA_LSB, data, 2 );
    
    if( ret != 0 )
    {
        return is_measurement_comp;
    }

    get_data = ( ((unsigned long)data[0]) & 0x000000FF );
    
    memset( data, 0x0, 2 );

    //get MSB data
    ret = get_register( D_ADDR_DATA_MSB, data, 2 );
    
    if( ret != 0 )
    {
        return is_measurement_comp;
    }
    
    get_data |= ((unsigned long)( data[0] << 8 ) & 0x0000FF00);

    spin_lock( &sense_data_spin_lock );
    
    g_measurement_data[g_measurement_count] = get_data;

    g_measurement_count++;

    if( g_measurement_count >= 3 )
    {
        g_measurement_data[g_measurement_count] = 0x0000;
        g_measurement_data[g_measurement_count + 1] = 0xFFFF;
        g_measurement_count = 0;
        
        measurement_data = calc_measurement_val();
        
        //set measurement data
        g_prox_reg_data[D_ADDR_DATA_LSB] = measurement_data & 0x000000FF;
        g_prox_reg_data[D_ADDR_DATA_MSB] = (( measurement_data >> 8 ) & 0x000000FF);
         
        low_threshold  = (unsigned short)( g_nv_prox_data[D_ADDR_INT_LT_LSB] | ( g_nv_prox_data[D_ADDR_INT_LT_MSB] << 8 ) );
        high_threshold = (unsigned short)( g_nv_prox_data[D_ADDR_INT_HT_LSB] | ( g_nv_prox_data[D_ADDR_INT_HT_MSB] << 8 ) );
        

        if( (unsigned short)measurement_data < low_threshold )      /* non-detect */
        {
            g_detect_state = D_PROX_DETECT_STATE_NON_DETECT;
        }
        else if( (unsigned short)measurement_data > high_threshold )        /* detect */
        {
            g_detect_state = D_PROX_DETECT_STATE_DETECT;
        }
        else            /* state stay */
        {
            /* no detect state change */
        }

        is_measurement_comp = 1;
    }
    else
    {
        is_measurement_comp = 0;
    }

    spin_unlock( &sense_data_spin_lock );

    return is_measurement_comp;
}

static int measurement_proc( int mode )
{
    int is_measurement_comp = 0;
    wait_queue_head_t dummy_queue;
    int dummy_wait = 0;
    
    init_waitqueue_head( &dummy_queue );
    
    if( mode == D_MODE_PROX )
    {
        is_measurement_comp = prox_measurement();
    }
    else
    {
        is_measurement_comp = ALS_measurement();
    }
    
    if( is_measurement_comp == (-1) )
    {
        //err case. wait: 1s
        wait_event_timeout( dummy_queue, dummy_wait != 0, HZ );

        is_measurement_comp = 0;
    }
    
    return is_measurement_comp;
}

//polling thread
static int gp2ap012a00f_poll_thread( void* param )
{
    int is_measurement_comp = 0;
    int ret = 0;
    int mode = 0;
    int dummy_wait = 0;
    wait_queue_head_t dummy_queue;
    int mode_change_request = 0;
    int meas_state = D_MEAS_STATE_STOP;
    
    DEBUG_PRINT( "gp2ap012a00f_poll_thread() Start\n" );
    
    init_waitqueue_head( &dummy_queue );
    
    wait_event( g_meas_ctrl_queue, g_is_nv_set != 0 );
    
    while( 1 )
    {
        spin_lock( &sense_data_spin_lock );
        meas_state = g_meas_state;
        
        if( meas_state == D_MEAS_STATE_STOP )
        {
            g_measurement_count = 0;
            memset( g_measurement_data, 0x0, sizeof(unsigned long) * 5 );
            g_is_measurement_comp = 0;
            
            
            
            spin_unlock( &sense_data_spin_lock );
            pm_obs_a_sensor(PM_OBS_SENSOR_OFF);
            
        }
        else
        {
            spin_unlock( &sense_data_spin_lock );
        }
        
        wait_event( g_meas_ctrl_queue, g_meas_state == D_MEAS_STATE_START );
        
        
        if( g_measurement_mode == D_MODE_ALS )
        {
            
            pm_obs_a_sensor(PM_OBS_SENSOR_SYOUDO_ON);
        }
        else if( g_measurement_mode == D_MODE_PROX )
        {
            
            pm_obs_a_sensor(PM_OBS_SENSOR_KINSETSU_ON);
        }
        else
        {
            
        }
        
        
        is_measurement_comp = measurement_proc( g_measurement_mode );
        
        spin_lock( &sense_data_spin_lock );
        mode_change_request = g_mode_change_request;
        spin_unlock( &sense_data_spin_lock );
        
        if( mode_change_request != D_REQ_MODE_NONE )        
        {
            if( mode_change_request == D_REQ_MODE_DIAG )            
            {
                gp2ap012a00f_soft_reset();      //reset
                
                als_or_prox( D_MODE_ALS );  
                
                g_measurement_mode = D_MODE_DIAG;
                
                wake_up( &g_mesurement_queue );
                DEBUG_PRINT2( "wait_event: Enter DIAG\n" );
                wait_event( g_mesurement_queue, g_mode_change_request != D_REQ_MODE_DIAG );
                DEBUG_PRINT2( "wait_event: Exit DIAG\n" );
                
                mode_change_request = g_mode_change_request;
            }

            if( mode_change_request == D_REQ_MODE_PROX )
            {
                mode = D_MODE_PROX;
            }
            else
            {
                mode = D_MODE_ALS;
            }
            
            spin_lock( &sense_data_spin_lock );
            g_measurement_count = 0;
            memset( g_measurement_data, 0x0, sizeof(unsigned long) * 5 );
            g_is_measurement_comp = 0;
            g_RANGE_mode = 0;
            spin_unlock( &sense_data_spin_lock );

            ret = als_or_prox( mode );
            
            if( ret != 0 )
            {
                ret = -EIO;
            }
            else
            {
                spin_lock( &sense_data_spin_lock );
                g_measurement_mode = mode;
                g_mode_change_request = D_REQ_MODE_NONE;
                spin_unlock( &sense_data_spin_lock );
            }
            
            wake_up( &g_mesurement_queue );
        }
        else                                        
        {
            spin_lock( &sense_data_spin_lock );
            if( g_clear_flag == 1 )
            {
                g_clear_flag = 0;
                spin_unlock( &sense_data_spin_lock );
                continue;           
            }
            spin_unlock( &sense_data_spin_lock );
            
            if( is_measurement_comp == 1 )
            {
                spin_lock( &sense_data_spin_lock );
                
                if( g_is_measurement_comp == 0 )
                {
                    if( g_mode_change_request == D_REQ_MODE_NONE )
                    {
                        g_is_measurement_comp = 1;
                    }
                }
                
                spin_unlock( &sense_data_spin_lock );

                wake_up( &g_mesurement_queue );

                if( g_measurement_mode != D_MODE_ALS )          /* PROX */
                {
                    wait_event_timeout( dummy_queue, dummy_wait != 0, (HZ / 10) );
                }
                else                                            /* ALS */
                {
                    //no wait
                }
            }
        }
    }
    
    return 0;
}

static void gp2ap012a00f_work_func(struct work_struct *work)
{
    //Not Implement.
}

/*
 * I2C init/probing/exit functions
 */

static int __devinit gp2ap012a00f_probe(struct i2c_client *client,
                   const struct i2c_device_id *id)
{
    struct gp2ap012a00f_data *data;
    int err;
    struct task_struct *th;

    data = kzalloc(sizeof(struct gp2ap012a00f_data), GFP_KERNEL);
    if (!data) {
        err = -ENOMEM;
        goto exit;
    }

    INIT_WORK(&data->work, gp2ap012a00f_work_func);
    i2c_set_clientdata(client, data);
    /* Initialize the GP2AP012A00F chip */
    err = gp2ap012a00f_init_client(client);
    if (err)
        goto exit_kfree;
    this_client = client;

    gp2ap012a00f_init_module();

    err = misc_register(&gp2ap012a00f_device);
    
    if (err)
    {
        printk(KERN_ERR
               "gp2ap012a00f_probe: gp2ap012a00f register failed\n");
        goto exit_kfree;
    }

    th = kthread_create(gp2ap012a00f_poll_thread, NULL, "sensor_poll");

    if(IS_ERR(th) )
    {
        misc_deregister(&gp2ap012a00f_device);
        
        printk(KERN_ERR
               "gp2ap012a00f_probe: Can't create thread\n");
        goto exit_kfree;
    }
    else
    {
        wake_up_process(th);
    }

    return 0;

exit_kfree:
    kfree(data);
exit:
    return err;
}

static int __devexit gp2ap012a00f_remove(struct i2c_client *client)
{
    misc_deregister(&gp2ap012a00f_device);
    i2c_release_client(client);

    kfree(i2c_get_clientdata(client));
    
    return 0;
}

static int gp2ap012a00f_suspend(struct i2c_client *client, pm_message_t mesg)
{
    return 0;
}

static int gp2ap012a00f_resume(struct i2c_client *client)
{
    return 0;
}

static const struct i2c_device_id gp2ap012a00f_id[] = {
    { "gp2ap012a00f", 0 },
    { }
};

static struct i2c_driver gp2ap012a00f_driver = {
    .probe = gp2ap012a00f_probe,
    .remove = gp2ap012a00f_remove,
    .suspend    = gp2ap012a00f_suspend,
    .resume     = gp2ap012a00f_resume,
    .id_table = gp2ap012a00f_id,
    .driver = {
           .name = "gp2ap012a00f",
           },
};

static int __init gp2ap012a00f_init(void)
{
    init_waitqueue_head( &g_mesurement_queue );
    init_waitqueue_head( &g_mode_change_queue );
    init_waitqueue_head( &g_meas_ctrl_queue );
    
    printk(KERN_INFO "gp2ap012a00f_init: " GP2AP012A00F_DRV_NAME " driver ver." DRIVER_VERSION "\n" );
    
    return i2c_add_driver(&gp2ap012a00f_driver);
}

static void __exit gp2ap012a00f_exit(void)
{
    i2c_del_driver(&gp2ap012a00f_driver);
}

MODULE_AUTHOR("NEC Communication Systems Co.,Ltd");
MODULE_DESCRIPTION("GP2AP012A00F driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);

module_init(gp2ap012a00f_init);
module_exit(gp2ap012a00f_exit);
