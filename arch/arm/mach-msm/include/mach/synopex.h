/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#ifndef __ARCH_ARM_MACH_MSM_SYNOPEX_H
#define __ARCH_ARM_MACH_MSM_SYNOPEX_H

#include <linux/input/msm_ts.h>

struct synopex_platform_data {
	int (*setup)(struct device *);
	int (*powerup)(struct device *);
	int (*powerdown)(struct device *);
	void (*teardown)(struct device *);
	struct msm_ts_platform_data *msm_ts_data;
};

void synopex_init_request(void);

#endif // __ARCH_ARM_MACH_MSM_SYNOPEX_H
