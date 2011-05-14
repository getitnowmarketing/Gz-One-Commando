#ifndef _LINUX_OEMNC_SMEM_H
#define _LINUX_OEMNC_SMEM_H

/* ========================================================================
FILE: oemnc_smem.h
All Right Reserved, Copyright(c) NEC Casio Mobile Communications Co.,Ltd.
===========================================================================*/

#define BOOT_TIME_NUM 40
#define AARM_KICK_POINT 4  

void  oemnc_boot_time_probe(unsigned int num);


typedef struct
{
	unsigned int boot_time[BOOT_TIME_NUM];
}boot_time_info;



/* SMEM_ID_VENDOR0 */
typedef struct
{
	boot_time_info time_info;  
}smem_id_vendor0;



#endif /* _LINUX_OEMNC_SMEM_H */

