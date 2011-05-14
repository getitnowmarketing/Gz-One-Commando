/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#ifndef _DIAG_ACAD_H
#define _DIAG_ACAD_H
/*===========================================================================

                DIAG APPS CORE AUDIO DRIVER    HEADER FILE

DESCRIPTION
  **********

===========================================================================*/


/* Target Specific Defintions           */
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/sizes.h>

#define DIAG_ACAD_MAJOR                0           
#define DIAG_ACAD_DEVICE_NAME        "diagACAD"    


#define DIAG_ACAD_IOCTL_MAGIC         'a'          /* all ( Documentation/ioctl/ioctl-number.txt ) */
#define DIAG_ACAD_IOCTL_15   _IOW (DIAG_ACAD_IOCTL_MAGIC, 0x02, diag_acad_ctrl )
#define DIAG_ACAD_IOCTL_16   _IOW (DIAG_ACAD_IOCTL_MAGIC, 0x03, diag_acad_ctrl )
#define DIAG_ACAD_IOCTL_21   _IOW (DIAG_ACAD_IOCTL_MAGIC, 0x04, diag_acad_ctrl )
#define DIAG_ACAD_IOCTL_0201 _IOW (DIAG_ACAD_IOCTL_MAGIC, 0x05, diag_acad_ctrl )
#define DIAG_ACAD_IOCTL_0202 _IOW (DIAG_ACAD_IOCTL_MAGIC, 0x06, diag_acad_ctrl )

/************************************************/

/************************************************/
typedef  struct {
    unsigned char tx_dev;    
    unsigned char rx_dev;    
    unsigned char type;      
}diag_acad_qtr_action;

/************************************************/

/************************************************/
typedef  struct {
    unsigned char path;    
}diag_acad_qtr_loopback;

/************************************************/

/************************************************/
typedef  struct {
    unsigned char bias;    
}diag_acad_pm_hsedbias;

/************************************************/

/************************************************/
typedef  struct {
    diag_acad_qtr_action       act;         
    diag_acad_qtr_loopback     lb;          
    diag_acad_pm_hsedbias      hsed;        
    unsigned char              req[16];     
    unsigned char              rsp[16];     
}diag_acad_ctrl;




#endif /* _DIAG_ACAD_H */

