/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/



#ifndef __MSM_BATTERY_H__
#define __MSM_BATTERY_H__

#define AC_CHG     0x00000001
#define USB_CHG    0x00000002

struct msm_psy_batt_pdata {
	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 avail_chg_sources;
	u32 batt_technology;
	u32 (*calculate_capacity)(u32 voltage);
};

typedef enum {
	PM_OBS_OFF = 0,				

	PM_OBS_1X_MODE = 10,		
	PM_OBS_EVDO_MODE,			
	PM_OBS_CAMERA_MODE,			
	PM_OBS_PLAIED_MODE,			
	PM_OBS_CONNECT_OFF,			
	PM_OBS_CONNECT_ON,			
	PM_OBS_CAMERALIGHT_MODE, 	
	PM_OBS_MOBILELIGHT_MODE,		
	PM_OBS_LCDBACKLIGHT_MODE,	
	PM_OBS_KEYBACKLIGHT_MODE,	
	PM_OBS_VIBRATION_MODE,		
	PM_OBS_SPEAKER_MODE,		
	PM_OBS_BT_MODE,				
	PM_OBS_WLAN_MODE,			
	PM_OBS_SENSOR_MODE,			
} pm_obs_mode_type;

typedef enum {
	PM_OBS_SENSOR_OFF,			
	PM_OBS_SENSOR_KINSETSU_ON,	
	PM_OBS_SENSOR_SYOUDO_ON		
} pm_obs_mode_sensor_type;

typedef enum {
	FALSE = 0,
	TRUE = 1
} boolean;

typedef enum {
	PM_OBS_WLAN_OFF_MODE,        
	PM_OBS_WLAN_SLEEP_MODE,      
	PM_OBS_WLAN_TR_ON_MODE       
} pm_obs_mode_wlan_type;


extern void pm_obs_a_camera(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_camlight(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_lcdbacklight(u32 level);
extern void pm_obs_a_keybacklight(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_vibration(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_speaker(u32 value);
extern void pm_obs_a_bluetooth(pm_obs_mode_type mode, boolean sw);
extern void pm_obs_a_wlan(pm_obs_mode_wlan_type mode);
extern void pm_obs_a_sensor(pm_obs_mode_sensor_type mode);

#endif
