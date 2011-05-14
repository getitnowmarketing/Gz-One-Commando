#ifndef _YDA160_SNDAMP_H
#define _YDA160_SNDAMP_H
/*===========================================================================
 FILENAME : yda160_sndamp.h

 Copyright (C) 2010 NEC Corporation.
===========================================================================*/

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/







/*===========================================================================

                            DATA DECLARATIONS

===========================================================================*/

#define SNDAMP_MAJOR                0
                                      
#define SNDAMP_DEVICECHR_NAME       "yda160amp"
                                      

#define SOUNDAMP_IOCTL_MAGIC        'a'
                         /* all ( Documentation/ioctl/ioctl-number.txt )   */


/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/


extern void sndamp_headset_stereo_power_on( void );


extern void sndamp_speaker_stereo_power_on( void );


extern void sndamp_fmradio_speaker_power_on( void );


extern void sndamp_tty_headset_mono_power_on( void );


extern void sndamp_headset_stereo_speaker_stereo_power_on( void );


extern void sndamp_ptt_speaker_stereo_power_on( void );


extern void sndamp_power_off( void );

#endif /* _YDA160_SNDAMP_H */

