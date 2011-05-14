/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#ifndef _LINUX_OEMNC_INFO_H
#define _LINUX_OEMNC_INFO_H
#include <linux/types.h>


#define HW_REVISION_READ_ERR 0xFFFFFFFF

/* Hardware Revision Specification ver1.4         */


typedef enum {
  HARDWARE_REV_01  = 0x00000001,  /* 0.1   */
  HARDWARE_REV_04  = 0x00000004,  /* 0.4   */
  HARDWARE_REV_07  = 0x00000007,  /* 0.7   */
  HARDWARE_REV_08  = 0x00000008,  /* 0.8   */
  HARDWARE_REV_09  = 0x00000009,  /* 0.9   */
  HARDWARE_REV_095 = 0x0000005F,  /* 0.95  */
  HARDWARE_REV_10  = 0x00010000,  /* 1.0   */
  HARDWARE_REV_11  = 0x00010001,  /* 1.1   */
  HARDWARE_REV_12  = 0x00010002,  /* 1.2   */
  HARDWARE_REV_13  = 0x00010003   /* 1.3   */
} hardware_rev_type;

extern uint32_t hw_revision_read (void);


#endif /* _LINUX_OEMNC_INFO_H */
