/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/err.h>

#include <linux/i2c.h>
#include <linux/gpio.h>

#include <yda160_i2c_driver.h>



/******************************************************************************/

/******************************************************************************/


static int __devinit yda160_probe( struct i2c_client* client , const struct i2c_device_id* id ) ;


static int __devexit yda160_remove( struct i2c_client* client ) ;


static int __init yda160_init( void ) ;


static void __exit yda160_exit( void ) ;



/******************************************************************************/

/******************************************************************************/


struct yda160 yda160_modules ;

static struct i2c_device_id yda160_id_table[ 1 ] = {
	{ YDA160_I2C_DEVICE_NAME , 0x00 }
} ;

static struct i2c_driver yda160_driver = {
	.driver = {
		.owner	= THIS_MODULE ,
		.name	= YDA160_I2C_DEVICE_NAME ,
	} ,
	.id_table	= yda160_id_table ,
	.probe		= yda160_probe ,
	.remove		= __devexit_p( yda160_remove ) ,
} ;



/******************************************************************************/

/******************************************************************************/

#define YDA160_ADDRESS (yda160_modules.client->addr)

MODULE_DESCRIPTION( YDA160_I2C_DEVICE_NAME ) ;
MODULE_ALIAS( "platform:YDA160 i2c driver" ) ;
MODULE_LICENSE( "Dual BSD/GPL" ) ;
MODULE_VERSION( "0.1" ) ;
MODULE_DEVICE_TABLE( i2c , yda160_id_table ) ;



EXPORT_SYMBOL( yda160_read ) ;


EXPORT_SYMBOL( yda160_write ) ;


module_init( yda160_init ) ;


module_exit( yda160_exit ) ;



/******************************************************************************/

/******************************************************************************/
int yda160_read( u8 reg , u8* value )
{
	int ret ;					
	struct i2c_msg msg[ 2 ] = {	
		{	
			.addr = YDA160_ADDRESS ,	
			.flags = 0 ,				/* write:0 / read:I2C_M_RD	*/
			.buf = &reg ,				
			.len = 1 ,					
		} ,
		{	
			.addr = YDA160_ADDRESS ,	
			.flags = I2C_M_RD ,			/* write:0 / read:I2C_M_RD	*/
			.buf = value ,				
			.len = 1 ,					
		}
	} ;

	
	ret = i2c_transfer( yda160_modules.client->adapter , &msg[0] , 1 ) ;
	if ( 0 > ret ) {
		printk( "YDA160 I2C Read(ERROR 01)\n" ) ;
	} else {
		
		(*value) = 0xff ;

		
		ret = i2c_transfer( yda160_modules.client->adapter , &msg[1] , 1 ) ;
		if ( 0 > ret ) {
			printk( "YDA160 I2C Read(ERROR 02)\n" ) ;
		}
	}

	printk( "YDA160 I2C Read / add:0x%02X / reg:0x%02X / value:0x%02X / ret:%d\n" , yda160_modules.client->addr , reg , (*value) , ret ) ;

	return ( ret ) ;
}



/******************************************************************************/

/******************************************************************************/
int yda160_write( u8 reg , u8 value )
{
	int ret ;						
	u8 data[2] ;					
	struct i2c_msg msg = {			
		.addr = YDA160_ADDRESS ,	
		.flags = 0 ,				/* write:0 / read:I2C_M_RD		*/
		.buf = &data[0] ,			
		.len = 2					
	} ;

	
	data[0] = reg ;
	data[1] = value ;

	
	ret = i2c_transfer( yda160_modules.client->adapter , &msg , 1 ) ;
	if ( 0 > ret ) {
		printk( "YDA160 I2C Write(ERROR)\n" ) ;
	}

	printk( "YDA160 I2C Write / add:0x%02X / reg:0x%02X / value:0x%02X / ret:%d\n" , yda160_modules.client->addr , reg , value , ret ) ;

	return ( ret ) ;
}



/******************************************************************************/

/******************************************************************************/
static int __devinit yda160_probe( struct i2c_client* client , const struct i2c_device_id* id )
{
	struct yda160_platform_data* pdata ;	
	struct yda160* yda160 ;					
	int status ;							
	struct platform_device* pdev ;


	
	pdata = client->dev.platform_data ;
	yda160 = &yda160_modules ;
	yda160->client = client ;

	
	if ( NULL == pdata ) {
		printk( "YDA160 I2C Probe(ERROR 01)\n" ) ;

		dev_dbg( &client->dev , "no platform data?\n" ) ;
		status = -EINVAL ;
		return ( status ) ;
	}

	
	if ( 0 == i2c_check_functionality( client->adapter , I2C_FUNC_I2C ) ) {
		printk( "YDA160 I2C Probe(ERROR 02)\n" ) ;

		dev_dbg( &client->dev , "can't talk I2C?\n" ) ;
		status = -EIO ;
		return ( status ) ;
	}

	
	if ( true == yda160->inuse ) {
		printk( "YDA160 I2C Probe(ERROR 03)\n" ) ;

		dev_dbg( &client->dev , "driver already in use\n" ) ;
		status = -EBUSY ;
		return ( status ) ;
	}

	
	if ( NULL == yda160->client ) {
		printk( "YDA160 I2C Probe(ERROR 04)\n" ) ;

		dev_err( &yda160->client->dev , "can't attach client\n" ) ;
		status = -ENOMEM ;
	} else {
		

		strlcpy( yda160->client->name , id->name , sizeof( yda160->client->name ) ) ;

		
		yda160->inuse = true ;

		i2c_set_clientdata( client , pdata ) ;

		pdev = platform_device_alloc( YDA160_I2C_DEVICE_NAME , -1 ) ;
		pdev->dev.parent = &yda160->client->dev ;
		platform_set_drvdata( pdev , yda160 ) ;

		if ( NULL != pdata ) {

			status = platform_device_add_data( pdev , pdata , sizeof( struct yda160_platform_data ) ) ;
//			if ( 0 > status ) {
//				printk( "YDA160 I2C Probe(ERROR 05)\n" ) ;
//				platform_set_drvdata( pdev , NULL ) ;
//				platform_device_put( pdev ) ;
//				return ERR_PTR( status ) ;
//			}

			status = platform_device_add( pdev ) ;
		}

		
		if ( NULL != pdata->yda160_setup ) {
			pdata->yda160_setup() ;
		}

		status = 0 ;

		printk( "YDA160 I2C Probe(OK)\n" ) ;
	}

	return ( status ) ;
}



/******************************************************************************/

/******************************************************************************/
static int __devexit yda160_remove( struct i2c_client* client )
{
	struct yda160_platform_data* pdata ;
	struct yda160* yda160 ;

	yda160 = &yda160_modules ;

	if ( NULL != yda160->client ) {
		i2c_unregister_device( yda160->client ) ;
		yda160->client = NULL ;

		
		pdata = client->dev.platform_data ;
		if ( NULL != pdata->yda160_shutdown ) {
			pdata->yda160_shutdown() ;
		}

		printk( "YDA160 I2C Remove(OK)\n" ) ;
	}

	return ( 0 ) ;
}



/******************************************************************************/

/******************************************************************************/
static int __init yda160_init( void )
{
	printk( "YDA160 I2C Init(OK)\n" ) ;

	return ( i2c_add_driver( &yda160_driver ) ) ;
}



/******************************************************************************/

/******************************************************************************/
static void __exit yda160_exit( void )
{
	i2c_del_driver( &yda160_driver ) ;

	printk( "YDA160 I2C Exit(OK)\n" ) ;
}
