#ifndef _YDA160_SNDAMP_DIAG_H
#define _YDA160_SNDAMP_DIAG_H
/*===========================================================================
 FILENAME : yda160_sndamp_diag.h

 Copyright (C) 2010 NEC Corporation.
===========================================================================*/

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/






/*===========================================================================

                            DATA DECLARATIONS

===========================================================================*/

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


#define SNDAMP_IOCTL_AMPCTL_READ    _IOWR(SOUNDAMP_IOCTL_MAGIC, 0x00, sndamp_diag_ctrl_type)
#define SNDAMP_IOCTL_AMPCTL_WRITE   _IOWR(SOUNDAMP_IOCTL_MAGIC, 0x01, sndamp_diag_ctrl_type)
#define SNDAMP_IOCTL_REGISTER_READ  _IOWR(SOUNDAMP_IOCTL_MAGIC, 0x02, sndamp_diag_ctrl_type)
#define SNDAMP_IOCTL_REGISTER_WRITE _IOW(SOUNDAMP_IOCTL_MAGIC, 0x03, sndamp_diag_ctrl_type)
#define SNDAMP_IOCTL_DRIVE          _IOW(SOUNDAMP_IOCTL_MAGIC, 0x04, sndamp_diag_ctrl_type)



/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


#define SNDAMP_DIAG_VOL_SUB_CMD               0x00
                        
#define SNDAMP_DIAG_REG_SUB_CMD               0x01
                        
#define SNDAMP_DIAG_DRV_SUB_CMD               0x02
                        


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


#define SNDAMP_DIAG_VOL_WRITE                 0x00
                        
#define SNDAMP_DIAG_VOL_READ                  0x01
                        


#define SNDAMP_DIAG_REQ_POS_VOL_SUB_CMD       0
                        
#define SNDAMP_DIAG_REQ_POS_VOL_RES_STATUS    1
                        
#define SNDAMP_DIAG_REQ_POS_VOL_RW            2
                        
#define SNDAMP_DIAG_REQ_POS_VOL_DEV_ID        3
                        
#define SNDAMP_DIAG_REQ_POS_VOL_IN_SVLA       4
                        
#define SNDAMP_DIAG_REQ_POS_VOL_IN_SVRA       5
                        
#define SNDAMP_DIAG_REQ_POS_VOL_OUT_SALA      6
                        
#define SNDAMP_DIAG_REQ_POS_VOL_OUT_SARA      7
                        
#define SNDAMP_DIAG_REQ_POS_VOL_OUT_MNA       8
                        
#define SNDAMP_DIAG_REQ_POS_VOL_NONCLIP_MODE  9
                        
#define SNDAMP_DIAG_REQ_POS_VOL_ATT_REL_TIME  10
                        

#define SNDAMP_DIAG_RES_POS_VOL_SUB_CMD       0
                        
#define SNDAMP_DIAG_RES_POS_VOL_RES_STATUS    1
                        
#define SNDAMP_DIAG_RES_POS_VOL_RW            2
                        
#define SNDAMP_DIAG_RES_POS_VOL_DEV_ID        3
                        
#define SNDAMP_DIAG_RES_POS_VOL_IN_SVLA       4
                        
#define SNDAMP_DIAG_RES_POS_VOL_IN_SVRA       5
                        
#define SNDAMP_DIAG_RES_POS_VOL_OUT_SALA      6
                        
#define SNDAMP_DIAG_RES_POS_VOL_OUT_SARA      7
                        
#define SNDAMP_DIAG_RES_POS_VOL_OUT_MNA       8
                        
#define SNDAMP_DIAG_RES_POS_VOL_NONCLIP_MODE  9
                        
#define SNDAMP_DIAG_RES_POS_VOL_ATT_REL_TIME  10
                        

#define SNDAMP_DIAG_VOL_DATA_SIZE             32
                        


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

#define SNDAMP_DIAG_DRV_POWER_DOWN            0x00
                        
#define SNDAMP_DIAG_DRV_POWER_DOWN_RELEASE    0x01
                        


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


typedef enum {
  SNDAMP_DIAG_STATUS_NORMAL = 0,                     
  SNDAMP_DIAG_STATUS_ERR_PROCESS_MODE,               
  SNDAMP_DIAG_STATUS_ERR_INVALID_ID,                 
} sndamp_diag_status_type;

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


typedef struct {
  unsigned char   data[SNDAMP_DIAG_VOL_DATA_SIZE];
                                
} sndamp_volume_ctrl_type;


typedef struct {
  unsigned char   r_w;          
  unsigned char   reg_addr;     
  unsigned char   reg_data;     
} sndamp_reg_ctrl_type;


typedef struct {
  unsigned char   drv_ctl;
                                
                                
                                
  unsigned char   vol_id;
                                
} sndamp_drive_ctrl_type;

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

typedef union {
  sndamp_volume_ctrl_type     vol;   
  sndamp_reg_ctrl_type        reg;   
  sndamp_drive_ctrl_type      drv;   
} sndamp_diag_ctrl_type;


/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/


extern int sndamp_write_reg_diag(unsigned char addrReg, unsigned char buf);


extern int sndamp_read_reg_diag(unsigned char addrReg, unsigned char* buf);


extern void sndamp_Drive_Control( sndamp_drive_ctrl_type  *ctrl );


extern void sndamp_RW_SoundParam( sndamp_volume_ctrl_type  *ctrl );

#endif /* _YDA160_SNDAMP_DIAG_H */

