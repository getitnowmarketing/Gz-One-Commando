/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

/*
 * Definitions for akm8977 compass chip.
 */
#ifndef akm8977_H
#define akm8977_H

#include <linux/ioctl.h>

/* Compass device dependent definition */
#define AKECS_MODE_POWERDOWN	0x00	/* Power down mode */
#define AKECS_MODE_MEASURE_SNG	0x01	/* Starts single measurement */
#define AKECS_MODE_MEASURE_SEQ	0x02	/* Starts sequential measurement */
#define AKECS_MODE_E2P_READ	    0x04	/* E2P access mode (read). */
#define AKECS_MODE_SELF_TEST	0x08	/* E2P access mode (read). */

#define AKECS_MODE_PFFD		0x01	/* Start pedometer and free fall detect. */

#define AK8977_MS_POWER_DOWN		0x00
#define AK8977_MS_SNG_MEASURE		0x01
#define AK8977_MS_CONT_MEASURE		0x02
#define AK8977_MS_EEPROM_ACCESS		0x04
#define AK8977_MS_SELF_TEST			0x08

/* Default register settings */
#define CSPEC_AINT		0x01	/* Amplification for acceleration sensor */
#define CSPEC_SNG_NUM		0x01	/* Single measurement mode */
#define CSPEC_SEQ_NUM		0x02	/* Sequential measurement mode */
#define CSPEC_SFRQ_32		0x00	/* Measurement frequency: 32Hz */
#define CSPEC_SFRQ_64		0x01	/* Measurement frequency: 64Hz */
#define CSPEC_MCS		0x07	/* Clock frequency */
#define CSPEC_MKS		0x01	/* Clock type: CMOS level */
#define CSPEC_INTEN		0x01	/* Interruption pin enable: Enable */

#define RBUFF_SIZE		2	/* Rx buffer size */
#define MAX_CALI_SIZE	0x1000U	/* calibration buffer size */
#define EEPROM_SIZE			21		//EREF1-ET2

/* AK8977 register address */
#define AKECS_REG_ST			AK8977_REG_ST1
#define AKECS_REG_TMPS			AK8977_REG_TMPS

//
#define AK8977_REG_WIA		0x00
#define AK8977_REG_INT1ST	0x01
#define AK8977_REG_INT2ST	0x02
#define AK8977_REG_ST1		0x03
#define AK8977_REG_HXL		0x04
#define AK8977_REG_HXH		0x05
#define AK8977_REG_HYL		0x06
#define AK8977_REG_HYH		0x07
#define AK8977_REG_HZL		0x08
#define AK8977_REG_HZH		0x09
#define AK8977_REG_TMPS		0x0A
#define AK8977_REG_EMPT		0x0B
#define AK8977_REG_A1XL		0x0C
#define AK8977_REG_A1XH		0x0D
#define AK8977_REG_A1YL		0x0E
#define AK8977_REG_A1YH		0x0F
#define AK8977_REG_A1ZL		0x10
#define AK8977_REG_A1ZH		0x11
#define AK8977_REG_A2XL		0x12
#define AK8977_REG_A2XH		0x13
#define AK8977_REG_A2YL		0x14
#define AK8977_REG_A2YH		0x15
#define AK8977_REG_A2ZL		0x16
#define AK8977_REG_A2ZH		0x17
#define AK8977_REG_A3XL		0x18
#define AK8977_REG_A3XH		0x19
#define AK8977_REG_A3YL		0x1A
#define AK8977_REG_A3YH		0x1B
#define AK8977_REG_A3ZL		0x1C
#define AK8977_REG_A3ZH		0x1D
#define AK8977_REG_A4XL		0x1E
#define AK8977_REG_A4XH		0x1F
#define AK8977_REG_A4YL		0x20
#define AK8977_REG_A4YH		0x21
#define AK8977_REG_A4ZL		0x22
#define AK8977_REG_A4ZH		0x23
#define AK8977_REG_ST2		0x24
#define AK8977_REG_ST3		0x25
#define AK8977_REG_ST4		0x26
#define AK8977_REG_PEWSL	0x27
#define AK8977_REG_PEWSH	0x28
#define AK8977_REG_MEST1	0x29
#define AK8977_REG_MEST2	0x2A
#define AK8977_REG_MEST3	0x2B
#define AK8977_REG_MEST4	0x2C
#define AK8977_REG_MEST5	0x2D
#define AK8977_REG_MEST6	0x2E
#define AK8977_REG_MEST7	0x2F
#define AK8977_REG_MEST8	0x30
#define AK8977_REG_MEST9	0x31
#define AK8977_REG_HLIST	0x32
#define AK8977_REG_HLST		0x33

#define AK8977_REG_REF1		0x34
#define AK8977_REG_REF2		0x35
#define AK8977_REG_REF3		0x36
#define AK8977_REG_OSC		0x37
#define AK8977_REG_AXGA		0x38
#define AK8977_REG_AYGA		0x39
#define AK8977_REG_AZGA		0x3A
#define AK8977_REG_AXDA		0x3B
#define AK8977_REG_AYDA		0x3C
#define AK8977_REG_AZDA		0x3D
#define AK8977_REG_TMSE1	0x3E
#define AK8977_REG_TMSE2	0x3F
#define AK8977_REG_AXOF		0x40
#define AK8977_REG_AYOF		0x41
#define AK8977_REG_AZOF		0x42
#define AK8977_REG_PESE		0x43
#define AK8977_REG_MESE1	0x44
#define AK8977_REG_MESE2	0x45
#define AK8977_REG_MESE3	0x46
#define AK8977_REG_MESE4	0x47
#define AK8977_REG_MESE5	0x48
#define AK8977_REG_MOSE1	0x49
#define AK8977_REG_MOSE2	0x4A
#define AK8977_REG_MOSE3	0x4B
#define AK8977_REG_MOSE4	0x4C
#define AK8977_REG_TPSE1	0x4D
#define AK8977_REG_TPSE2	0x4E
#define AK8977_REG_TPSE3	0x4F
#define AK8977_REG_TPSE4	0x50
#define AK8977_REG_POSE1	0x51
#define AK8977_REG_POSE2	0x52
#define AK8977_REG_HLXB1	0x53
#define AK8977_REG_HLYB1	0x54
#define AK8977_REG_HLZB1	0x55
#define AK8977_REG_HLSE1	0x56
#define AK8977_REG_HLTH1	0x57
#define AK8977_REG_HLXB2	0x58
#define AK8977_REG_HLYB2	0x59
#define AK8977_REG_HLZB2	0x5A
#define AK8977_REG_HLSE2	0x5B
#define AK8977_REG_HLTH2	0x5C
#define AK8977_REG_PSSE		0x5D
#define AK8977_REG_INT1EN	0x5E
#define AK8977_REG_INT2EN	0x5F
#define AK8977_REG_SLCT1	0x60
#define AK8977_REG_SLCT2	0x61
#define AK8977_REG_MS		0x62
#define AK8977_REG_TST1		0x63
#define AK8977_REG_TST2		0x64
///@}

/*! \name AK8977 eeprom address
\anchor AK8975_EEP
Defines an address of the eeprom of the AK8977.*/
///@{
#define AK8977_EEP_EREF1		0x65
#define AK8977_EEP_EREF2		0x66
#define AK8977_EEP_EREF3		0x67
#define AK8977_EEP_EOSC			0x68
#define AK8977_EEP_EHCX			0x69
#define AK8977_EEP_EHCY			0x6A
#define AK8977_EEP_EHCZ			0x6B
#define AK8977_EEP_EAXGA		0x6C
#define AK8977_EEP_EAYGA		0x6D
#define AK8977_EEP_EAZGA		0x6E
#define AK8977_EEP_EAXDT		0x6F
#define AK8977_EEP_EAYDT		0x70
#define AK8977_EEP_EAZDT		0x71
#define AK8977_EEP_EAXDA		0x72
#define AK8977_EEP_EAYDA		0x73
#define AK8977_EEP_EAZDA		0x74
#define AK8977_EEP_EAXGTD		0x75
#define AK8977_EEP_EAYGTD		0x76
#define AK8977_EEP_EAZGTD		0x77
#define AK8977_EEP_ET1			0x78
#define AK8977_EEP_ET2			0x79
#define AK8977_EEP_WRAL			0x7A
///@}

#define AKMIO				0xA1

/* IOCTLs for AKM library */
#define ECS_IOCTL_INIT                  _IO(AKMIO, 0x01)
#define ECS_IOCTL_WRITE                 _IOW(AKMIO, 0x02, char[5])
#define ECS_IOCTL_READ                  _IOWR(AKMIO, 0x03, char[5])
#define ECS_IOCTL_RESET      	          _IO(AKMIO, 0x04)
#define ECS_IOCTL_INT_STATUS            _IO(AKMIO, 0x05)
#define ECS_IOCTL_FFD_STATUS            _IO(AKMIO, 0x06)
#define ECS_IOCTL_SET_MODE              _IOW(AKMIO, 0x07, short)
#define ECS_IOCTL_GETDATA               _IOR(AKMIO, 0x08, char[RBUFF_SIZE+1])
#define ECS_IOCTL_GET_NUMFRQ            _IOR(AKMIO, 0x09, char[2])
#define ECS_IOCTL_SET_PERST             _IO(AKMIO, 0x0A)
#define ECS_IOCTL_SET_G0RST             _IO(AKMIO, 0x0B)
#define ECS_IOCTL_SET_YPR               _IOW(AKMIO, 0x0C, short[12])
#define ECS_IOCTL_GET_OPEN_STATUS       _IOR(AKMIO, 0x0D, int)
#define ECS_IOCTL_GET_CLOSE_STATUS      _IOR(AKMIO, 0x0E, int)
#define ECS_IOCTL_GET_CALI_DATA         _IOR(AKMIO, 0x0F, char[MAX_CALI_SIZE])
#define ECS_IOCTL_GET_DELAY             _IOR(AKMIO, 0x30, short)
#define ECS_IOCTL_GET_E2PDATA           _IOR(AKMIO, 0x15, char[EEPROM_SIZE])

#define ECS_IOCTL_SET_SENSOR_DATA       _IOW(AKMIO, 0x90, char[200])
#define ECS_IOCTL_GET_SENSOR_DATA       _IOR(AKMIO, 0x91, char[200])

#define ECS_IOCTL_SET_SENSOR_MSG        _IOW(AKMIO, 0x92, int)  
#define ECS_IOCTL_GET_SENSOR_MSG        _IOR(AKMIO, 0x93, int)  

/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_MODE		_IOW(AKMIO, 0x10, short)
#define ECS_IOCTL_APP_SET_MFLAG		_IOW(AKMIO, 0x11, short)
#define ECS_IOCTL_APP_GET_MFLAG		_IOW(AKMIO, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG		_IOW(AKMIO, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG		_IOR(AKMIO, 0x14, short)
#define ECS_IOCTL_APP_SET_TFLAG		_IOR(AKMIO, 0x15, short)
#define ECS_IOCTL_APP_GET_TFLAG		_IOR(AKMIO, 0x16, short)
#define ECS_IOCTL_APP_RESET_PEDOMETER   _IO(AKMIO, 0x17)
#define ECS_IOCTL_APP_SET_DELAY		_IOW(AKMIO, 0x18, short)
#define ECS_IOCTL_APP_GET_DELAY		ECS_IOCTL_GET_DELAY
#define ECS_IOCTL_APP_SET_MVFLAG	_IOW(AKMIO, 0x19, short)	/* Set raw magnetic vector flag */
#define ECS_IOCTL_APP_GET_MVFLAG	_IOR(AKMIO, 0x1A, short)	/* Get raw magnetic vector flag */

/* IOCTLs for pedometer */
#define ECS_IOCTL_SET_STEP_CNT          _IOW(AKMIO, 0x20, short)

/* Default GPIO setting */
#define ECS_RST		84	/*reset */
#define ECS_INTR	161	/*INT1 */
#define ECS_INTR2	162	/*INT2 */

struct akm8977_platform_data {
	int reset;
	int intr;
	int intr2;
};

//extern char *get_akm_cal_ram(void);

#endif

