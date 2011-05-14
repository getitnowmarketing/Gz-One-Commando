/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

/*
 * Definitions for gp2ap012a00f chip.
 */
#ifndef GP2AP012A00F_H
#define GP2AP012A00F_H

#include <linux/ioctl.h>

#define D_MODE_DIAG_BIT				(0x00010000)

//sensor mode.
#define D_MODE_ALS					(0)
#define D_MODE_PROX					(1)
#define D_MODE_DIAG					(2)
#define D_MODE_DIAG_ALS				(D_MODE_ALS  | D_MODE_DIAG_BIT)
#define D_MODE_DIAG_PROX			(D_MODE_PROX | D_MODE_DIAG_BIT)
#define D_MODE_DIAG_OFF				(3)

//register addr
#define D_ADDR_COMMAND_1			(0x00)
#define D_ADDR_COMMAND_2			(0x01)
#define D_ADDR_COMMAND_3			(0x02)
#define D_ADDR_DATA_LSB				(0x03)
#define D_ADDR_DATA_MSB				(0x04)
#define D_ADDR_INT_LT_LSB			(0x05)
#define D_ADDR_INT_LT_MSB			(0x06)
#define D_ADDR_INT_HT_LSB			(0x07)
#define D_ADDR_INT_HT_MSB			(0x08)
#define D_ADDR_MAX					(D_ADDR_INT_HT_MSB + 1)

//command data bit.
//COMMAND1(00h).
#define D_DATA_BIT_OP3				(1 << D_DATA_BIT_SHIFT_NUM_OP3)
#define D_DATA_BIT_OP2				(1 << D_DATA_BIT_SHIFT_NUM_OP2)
#define D_DATA_BIT_OP1				(1 << D_DATA_BIT_SHIFT_NUM_OP1)
#define D_DATA_BIT_OP0				(1 << D_DATA_BIT_SHIFT_NUM_OP0)
#define D_DATA_BIT_PROX				(1 << D_DATA_BIT_SHIFT_NUM_PROX)
#define D_DATA_BIT_FLAG				(1 << D_DATA_BIT_SHIFT_NUM_FLAG)
#define D_DATA_BIT_PRST1			(1 << D_DATA_BIT_SHIFT_NUM_PRST1)
#define D_DATA_BIT_PRST0			(1 << D_DATA_BIT_SHIFT_NUM_PRST0)

//COMMAND2(01h).
//none								(1 << 7).
#define D_DATA_BIT_RES2				(1 << D_DATA_BIT_SHIFT_NUM_RES2)
#define D_DATA_BIT_RES1				(1 << D_DATA_BIT_SHIFT_NUM_RES1)
#define D_DATA_BIT_RES0				(1 << D_DATA_BIT_SHIFT_NUM_RES0)
#define D_DATA_BIT_RANGE3			(1 << D_DATA_BIT_SHIFT_NUM_RANGE3)
#define D_DATA_BIT_RANGE2			(1 << D_DATA_BIT_SHIFT_NUM_RANGE2)
#define D_DATA_BIT_RANGE1			(1 << D_DATA_BIT_SHIFT_NUM_RANGE1)
#define D_DATA_BIT_RANGE0			(1 << D_DATA_BIT_SHIFT_NUM_RANGE0)

//COMMAND3(02h).
#define D_DATA_BIT_INTVAL1			(1 << D_DATA_BIT_SHIFT_NUM_INTVAL1)
#define D_DATA_BIT_INTVAL0			(1 << D_DATA_BIT_SHIFT_NUM_INTVAL0)
#define D_DATA_BIT_IS1				(1 << D_DATA_BIT_SHIFT_NUM_IS1)
#define D_DATA_BIT_IS0				(1 << D_DATA_BIT_SHIFT_NUM_IS0)
#define D_DATA_BIT_PIN				(1 << D_DATA_BIT_SHIFT_NUM_PIN)
//none								(1 << 2).
#define D_DATA_BIT_FREQ				(1 << D_DATA_BIT_SHIFT_NUM_FREQ)
#define D_DATA_BIT_RST				(1 << D_DATA_BIT_SHIFT_NUM_RST)

//data bit define
#define D_DATA_BIT_PRST				( D_DATA_BIT_PRST1 | D_DATA_BIT_PRST0 )
#define D_DATA_BIT_RES				( D_DATA_BIT_RES2 | D_DATA_BIT_RES1 | D_DATA_BIT_RES0 )
#define D_DATA_BIT_RANGE			( D_DATA_BIT_RANGE3 | D_DATA_BIT_RANGE2 | D_DATA_BIT_RANGE1 | D_DATA_BIT_RANGE0 )
#define D_DATA_BIT_INTVAL			( D_DATA_BIT_INTVAL1 | D_DATA_BIT_INTVAL0 )
#define D_DATA_BIT_IS				( D_DATA_BIT_IS1 | D_DATA_BIT_IS0 )

//command data bit shift num.
#define D_DATA_BIT_SHIFT_NUM_OP3			(7)
#define D_DATA_BIT_SHIFT_NUM_OP2			(6)
#define D_DATA_BIT_SHIFT_NUM_OP1			(5)
#define D_DATA_BIT_SHIFT_NUM_OP0			(4)
#define D_DATA_BIT_SHIFT_NUM_PROX			(3)
#define D_DATA_BIT_SHIFT_NUM_FLAG			(2)
#define D_DATA_BIT_SHIFT_NUM_PRST1			(1)
#define D_DATA_BIT_SHIFT_NUM_PRST0			(0)

//COMMAND2(01h).
//none										(7).
#define D_DATA_BIT_SHIFT_NUM_RES2			(6)
#define D_DATA_BIT_SHIFT_NUM_RES1			(5)
#define D_DATA_BIT_SHIFT_NUM_RES0			(4)
#define D_DATA_BIT_SHIFT_NUM_RANGE3			(3)
#define D_DATA_BIT_SHIFT_NUM_RANGE2			(2)
#define D_DATA_BIT_SHIFT_NUM_RANGE1			(1)
#define D_DATA_BIT_SHIFT_NUM_RANGE0			(0)

//COMMAND3(02h).
#define D_DATA_BIT_SHIFT_NUM_INTVAL1		(7)
#define D_DATA_BIT_SHIFT_NUM_INTVAL0		(6)
#define D_DATA_BIT_SHIFT_NUM_IS1			(5)
#define D_DATA_BIT_SHIFT_NUM_IS0			(4)
#define D_DATA_BIT_SHIFT_NUM_PIN			(3)
//none										(2).
#define D_DATA_BIT_SHIFT_NUM_FREQ			(1)
#define D_DATA_BIT_SHIFT_NUM_RST			(0)

/* prox detect state */
#define D_PROX_DETECT_STATE_NON_DETECT		(0)
#define D_PROX_DETECT_STATE_DETECT			(1)

typedef struct _t_gp2ap012a00f_ioctl_reg
{
	unsigned long address;
	unsigned long data;
}T_GP2AP012A00F_IOCTL_REG;

typedef struct _t_gp2ap012a00f_ioctl_sensor
{
	unsigned long sensor_mode;
	unsigned long data;
	unsigned long reg_data[D_ADDR_MAX];
	unsigned long detect_state;
}T_GP2AP012A00F_IOCTL_SENSOR;

typedef struct _t_gp2ap012a00f_ioctl_nv
{
	unsigned long ALS_nv_reg[D_ADDR_MAX];
	unsigned long ALS_nv_filter_ratio;
	
	unsigned long prox_nv_reg[D_ADDR_MAX];
	unsigned long prox_nv_filter_ratio;
}T_GP2AP012A00F_IOCTL_NV;

struct gp2ap012a00f_platform_data
{
	unsigned long data;
};

#define GP2AP012A00F_IO				'G'

#define D_IOCTL_GET_REG				_IOWR(GP2AP012A00F_IO, 0x01, T_GP2AP012A00F_IOCTL_REG)      /* struct _t_gp2ap012a00f_ioctl_reg */
#define D_IOCTL_SET_REG				_IOWR(GP2AP012A00F_IO, 0x02, T_GP2AP012A00F_IOCTL_REG)     /* struct _t_gp2ap012a00f_ioctl_reg */
#define D_IOCTL_SET_SENSOR_MODE		_IOWR(GP2AP012A00F_IO, 0x03, T_GP2AP012A00F_IOCTL_SENSOR)     /* struct _t_gp2ap012a00f_ioctl_sensor */
#define D_IOCTL_GET_SENSOR_DATA		_IOWR(GP2AP012A00F_IO, 0x04, T_GP2AP012A00F_IOCTL_SENSOR)      /* struct _t_gp2ap012a00f_ioctl_sensor */
#define D_IOCTL_GET_SENSOR_MODE		_IOWR(GP2AP012A00F_IO, 0x05, T_GP2AP012A00F_IOCTL_SENSOR)      /* struct _t_gp2ap012a00f_ioctl_sensor */
#define D_IOCTL_CLEAR_SENSOR_DATA	_IO(GP2AP012A00F_IO, 0x06)       /* NULL */
#define D_IOCTL_SET_NV				_IOWR(GP2AP012A00F_IO, 0x07, T_GP2AP012A00F_IOCTL_NV)      /* struct _t_gp2ap012a00f_ioctl_nv */
#define D_IOCTL_GET_NV				_IOWR(GP2AP012A00F_IO, 0x08, T_GP2AP012A00F_IOCTL_NV)      /* struct _t_gp2ap012a00f_ioctl_nv */

#endif

