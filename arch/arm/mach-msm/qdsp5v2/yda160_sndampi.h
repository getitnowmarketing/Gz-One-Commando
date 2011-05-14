#ifndef _YDA160_SNDAMPI_H
#define _YDA160_SNDAMPI_H
/*===========================================================================
 FILENAME : yda160_sndampi.h

 Copyright (C) 2010 NEC Corporation.
===========================================================================*/

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/






/*===========================================================================

                            DATA DECLARATIONS

===========================================================================*/


#define SNDAMP_VOLUME_OUTPUT_MUTE                   0x00
                                
#define SNDAMP_VOLUME_INPUT_MUTE                    0x00
                                


#define SNDAMP_DCDC_START_DELAY_TIME                4
                                
#define SNDAMP_SPEAKER_MUTE_TO_ATT_DELAY_TIME       16
                                
#define SNDAMP_SPEAKER_ATT_TO_MUTE_DELAY_TIME       12
                                
#define SNDAMP_HEADPHONE_MUTE_TO_ATT_DELAY_TIME     24
                                
#define SNDAMP_HEADPHONE_ATT_TO_MUTE_DELAY_TIME     12
                                
#define SNDAMP_CHARGE_PUMP_WAKE_UP_DELAY_TIME       1
                                
#define SNDAMP_CHARGE_INOUT_DC_CAP_DELAY_TIME       12
                                
#define SNDAMP_SET_VOLUME_DELAY_TIME                50
                                
#define SNDAMP_POWER_DOWN_DELAY_TIME                5000
                                
#define SNDAMP_AUD_AMP_RST_DELAY_TIME               1
                                
#define SNDAMP_KERNEL_TIMER_OFFSET                  1
                                


#define SNDAMP_PM_OBS_SPEAKER_OFF                   0
                                
#define SNDAMP_PM_OBS_SPEAKER_ON                    100
                                


#define SNDAMP_ADDR_BASE                            0x80
                                
#define SNDAMP_ADDR(offset)                         (SNDAMP_ADDR_BASE + offset)
                                
#define SNDAMP_BUF(offset)                          (sndamp_yda160_reg_buffer[offset])
                                
#define SNDAMP_BUFFER_LENGTH                        1
                                
#define SNDAMP_REG_ON                               1
                                
#define SNDAMP_REG_OFF                              0
                                


#define SNDAMP_REG_0_POWER_DOWN1_PDPC               0x01
                    
#define SNDAMP_REG_0_POWER_DOWN1_PDP                0x02
                    
#define SNDAMP_REG_2_POWER_DOWN3_PD_CHP             0x01
                    
#define SNDAMP_REG_2_POWER_DOWN3_PD_REG             0x04
                    
#define SNDAMP_REG_2_POWER_DOWN3_HIZ_HP             0x08
                    
#define SNDAMP_REG_3_POWER_DOWN4_HIZ_SPR            0x04
                    
#define SNDAMP_REG_3_POWER_DOWN4_HIZ_SPL            0x08
                    
#define SNDAMP_REG_3_POWER_DOWN4_PD_REC             0x10
                    
#define SNDAMP_REG_3_POWER_DOWN4_PD_SNT             0x80
                    
#define SNDAMP_REG_4_NON_CLIP2_DALC                 0x07
                    
#define SNDAMP_REG_4_NON_CLIP2_DPLT                 0x70
                    
#define SNDAMP_REG_5_ATTACK_RELEASE_TIME_DATT       0x03
                    
#define SNDAMP_REG_5_ATTACK_RELEASE_TIME_DREL       0x0C
                    
#define SNDAMP_REG_7_MONAURAL_MNX                   0x1F
                    
#define SNDAMP_REG_7_MONAURAL_ZCS_MV                0x80
                    
#define SNDAMP_REG_8_LINE1_LCH_SVLA                 0x1F
                    
#define SNDAMP_REG_8_LINE1_LCH_LAT_VA               0x20
                    
#define SNDAMP_REG_8_LINE1_LCH_ZCS_SVA              0x80
                    
#define SNDAMP_REG_9_LINE1_RCH_SVRA                 0x1F
                    
#define SNDAMP_REG_10_LINE2_LCH_SVLB                0x1F
                    
#define SNDAMP_REG_10_LINE2_LCH_LAT_VB              0x20
                    
#define SNDAMP_REG_10_LINE2_LCH_ZCS_SVB             0x80
                    
#define SNDAMP_REG_11_LINE2_RCH_SVRB                0x1F
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_HPL_BMIX      0x01
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_HPL_AMIX      0x02
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_HPL_MMIX      0x04
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_MONO_HP       0x08
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_HPR_BMIX      0x10
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_HPR_AMIX      0x20
                    
#define SNDAMP_REG_12_HEADPHONE_MIXER_HPR_MMIX      0x40
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SPL_BMIX        0x01
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SPL_AMIX        0x02
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SPL_MMIX        0x04
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_MONO_SP         0x08
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SPR_BMIX        0x10
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SPR_AMIX        0x20
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SPR_MMIX        0x40
                    
#define SNDAMP_REG_13_SPEAKER_MIXER_SWAP_SP         0x80
                    
#define SNDAMP_REG_14_SPEAKER_ATT_MNA               0x1F
                    
#define SNDAMP_REG_14_SPEAKER_ATT_SVOL_SP           0x40
                    
#define SNDAMP_REG_14_SPEAKER_ATT_ZCS_SPA           0x80
                    
#define SNDAMP_REG_15_HEADPHONE_ATT_LCH_SALA        0x1F
                    
#define SNDAMP_REG_15_HEADPHONE_ATT_LCH_LAT_HP      0x20
                    
#define SNDAMP_REG_15_HEADPHONE_ATT_LCH_SVOL_HP     0x40
                    
#define SNDAMP_REG_15_HEADPHONE_ATT_LCH_ZCS_HPA     0x80
                    
#define SNDAMP_REG_16_HEADPHONE_ATT_RCH_SARA        0x1F
                    
#define SNDAMP_REG_17_ERROR_FLAG_OTP_ERR            0x40
                    
#define SNDAMP_REG_17_ERROR_FLAG_OCP_ERR            0x80
                    


#define SNDAMP_GPIO_AUD_AMP_RST                     82  /* /RST (GPIO_082) */
#define SNDAMP_PMIC_GPIO_AUD_DDC_EN                 23  /* PMIC GPIO Number 24 */

#define SNDAMP_GPIO_HIGH_VALUE                      1   /* GPIO High       */
#define SNDAMP_GPIO_LOW_VALUE                       0   /* GPIO Low        */




#define SNDAMP_REG_0_POWERDOWN1_INIT_VAL            0x03
                        
#define SNDAMP_REG_1_POWERDOWN2_INIT_VAL            0x07
                        
#define SNDAMP_REG_2_POWERDOWN3_INIT_VAL            0x07
                        
#define SNDAMP_REG_3_POWERDOWN4_INIT_VAL            0x13
                        
#define SNDAMP_REG_4_NONCLIP2_INIT_VAL              0x01
                        
#define SNDAMP_REG_5_ATTACK_RELEASE_TIME_INIT_VAL   0x0D
                        
#define SNDAMP_REG_6_RESERVE_INIT_VALD              0x00
                        
#define SNDAMP_REG_7_MONAURAL_INIT_VAL              0x00
                        
#define SNDAMP_REG_8_LINE1_LCH_INIT_VAL             0xA0
                        
#define SNDAMP_REG_9_LINE1_RCH_INIT_VAL             0x00
                        
#define SNDAMP_REG_10_LINE2_LCH_INIT_VAL            0x00
                        
#define SNDAMP_REG_11_LINE2_RCH_INIT_VAL            0x00
                        
#define SNDAMP_REG_12_HEADPHONE_MIXER_INIT_VAL      0x00
                        
#define SNDAMP_REG_13_SPEAKER_MIXER_INIT_VAL        0x00
                        
#define SNDAMP_REG_14_SPEAKER_ATT_INIT_VAL          0x40
                        
#define SNDAMP_REG_15_HEADPHONE_ATT_LCH_INIT_VAL    0x60
                        
#define SNDAMP_REG_16_HEADPHONE_ATT_RCH_INIT_VAL    0x00
                        
#define SNDAMP_REG_17_ERROR_FLAG_INIT_VAL           0x00
                        


#define SNDAMP_SET_REG_BUF(offset, data, bit, mask) \
  SNDAMP_BUF( offset ) \
    = ( ( SNDAMP_BUF( offset ) & ~mask ) | ( ( data << bit ) & mask) )


#define SNDAMP_SET_REG_0_POWER_DOWN1_PDPC(data) \
  MM_DBG("SET REG0 POWERDOWN1 PDPC[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_0_POWERDOWN1, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_0_POWER_DOWN1_PDPC )


#define SNDAMP_SET_REG_0_POWER_DOWN1_PDP(data) \
  MM_DBG("SET REG0 POWERDOWN1 PDP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_0_POWERDOWN1, data, SNDAMP_REG_BIT_D1, SNDAMP_REG_0_POWER_DOWN1_PDP )


#define SNDAMP_SET_REG_2_POWER_DOWN3_PD_CHP(data) \
  MM_DBG("SET REG2 POWERDOWN3 PD_CHP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_2_POWERDOWN3, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_2_POWER_DOWN3_PD_CHP )


#define SNDAMP_SET_REG_2_POWER_DOWN3_PD_REG(data) \
  MM_DBG("SET REG2 POWERDOWN3 PD_REG[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_2_POWERDOWN3, data, SNDAMP_REG_BIT_D2, SNDAMP_REG_2_POWER_DOWN3_PD_REG )


#define SNDAMP_SET_REG_2_POWER_DOWN3_HIZ_HP(data) \
  MM_DBG("SET REG2 POWERDOWN3 HIZ_HP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_2_POWERDOWN3, data, SNDAMP_REG_BIT_D3, SNDAMP_REG_2_POWER_DOWN3_HIZ_HP )


#define SNDAMP_SET_REG_3_POWER_DOWN4_HIZ_SPR(data) \
  MM_DBG("SET REG3 POWERDOWN4 HIZ_SPR[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_3_POWERDOWN4, data, SNDAMP_REG_BIT_D2, SNDAMP_REG_3_POWER_DOWN4_HIZ_SPR )


#define SNDAMP_SET_REG_3_POWER_DOWN4_HIZ_SPL(data) \
  MM_DBG("SET REG3 POWERDOWN4 HIZ_SPL[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_3_POWERDOWN4, data, SNDAMP_REG_BIT_D3, SNDAMP_REG_3_POWER_DOWN4_HIZ_SPL )


#define SNDAMP_SET_REG_3_POWER_DOWN4_PD_REC(data) \
  MM_DBG("SET REG3 POWERDOWN4 PD_REC[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_3_POWERDOWN4, data, SNDAMP_REG_BIT_D4, SNDAMP_REG_3_POWER_DOWN4_PD_REC )


#define SNDAMP_SET_REG_3_POWER_DOWN4_PD_SNT(data) \
  MM_DBG("SET REG3 POWERDOWN4 PD_SNT[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_3_POWERDOWN4, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_3_POWER_DOWN4_PD_SNT )


#define SNDAMP_SET_REG_4_NON_CLIP2_DALC(data) \
  MM_DBG("SET REG4 NON_CLIP2 DALC[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_4_NONCLIP2, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_4_NON_CLIP2_DALC )


#define SNDAMP_SET_REG_4_NON_CLIP2_DPLT(data) \
  MM_DBG("SET REG4 NON_CLIP2 DPLT[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_4_NONCLIP2, data, SNDAMP_REG_BIT_D4, SNDAMP_REG_4_NON_CLIP2_DPLT )


#define SNDAMP_SET_REG_5_ATTACK_RELEASE_TIME_DATT(data) \
  MM_DBG("SET REG5 ATTACK_RELEASE_TIME DATT[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_5_ATTACK_RELEASE_TIME, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_5_ATTACK_RELEASE_TIME_DATT )


#define SNDAMP_SET_REG_5_ATTACK_RELEASE_TIME_DREL(data) \
  MM_DBG("SET REG5 ATTACK_RELEASE_TIME DREL[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_5_ATTACK_RELEASE_TIME, data, SNDAMP_REG_BIT_D2, SNDAMP_REG_5_ATTACK_RELEASE_TIME_DREL )


#define SNDAMP_SET_REG_7_MONAURAL_MNX(data) \
  MM_DBG("SET REG7 MONAURAL MNX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_7_MONAURAL, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_7_MONAURAL_MNX )


#define SNDAMP_SET_REG_7_MONAURAL_ZCS_MV(data) \
  MM_DBG("SET REG7 MONAURAL ZCS_MV[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_7_MONAURAL, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_7_MONAURAL_ZCS_MV )


#define SNDAMP_SET_REG_8_LINE1_LCH_SVLA(data) \
  MM_DBG("SET REG8 LINE1_LCH SVLA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_8_LINE1_LCH, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_8_LINE1_LCH_SVLA )


#define SNDAMP_SET_REG_8_LINE1_LCH_LAT_VA(data) \
  MM_DBG("SET REG8 LINE1_LCH LAT_VA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_8_LINE1_LCH, data, SNDAMP_REG_BIT_D5, SNDAMP_REG_8_LINE1_LCH_LAT_VA )


#define SNDAMP_SET_REG_8_LINE1_LCH_ZCS_SVA(data) \
  MM_DBG("SET REG8 LINE1_LCH ZCS_SVA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_8_LINE1_LCH, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_8_LINE1_LCH_ZCS_SVA )


#define SNDAMP_SET_REG_9_LINE1_RCH_SVRA(data) \
  MM_DBG("SET REG9 LINE1_RCH SVRA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_9_LINE1_RCH, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_9_LINE1_RCH_SVRA )


#define SNDAMP_SET_REG_10_LINE2_LCH_SVLB(data) \
  MM_DBG("SET REG10 LINE2_LCH LCH_SVLB[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_10_LINE2_LCH, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_10_LINE2_LCH_SVLB )


#define SNDAMP_SET_REG_10_LINE2_LCH_LAT_VB(data) \
  MM_DBG("SET REG10 LINE2_LCH LAT_VB[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_10_LINE2_LCH, data, SNDAMP_REG_BIT_D5, SNDAMP_REG_10_LINE2_LCH_LAT_VB )


#define SNDAMP_SET_REG_10_LINE2_LCH_ZCS_SVA(data) \
  MM_DBG("SET REG10 LINE2_LCH ZCS_SVA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_10_LINE2_LCH, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_10_LINE2_LCH_ZCS_SVB )


#define SNDAMP_SET_REG_11_LINE2_RCH_SVRB(data) \
  MM_DBG("SET REG11 LINE2_RCH SVRB[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_11_LINE2_RCH, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_11_LINE2_RCH_SVRB )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPL_BMIX(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER HPL_BMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_12_HEADPHONE_MIXER_HPL_BMIX )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPL_AMIX(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER HPL_AMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D1, SNDAMP_REG_12_HEADPHONE_MIXER_HPL_AMIX )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPL_MMIX(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER HPL_MMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D2, SNDAMP_REG_12_HEADPHONE_MIXER_HPL_MMIX )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_MONO_HP(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER MONO_HP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D3, SNDAMP_REG_12_HEADPHONE_MIXER_MONO_HP )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_BMIX(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER HPR_BMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D4, SNDAMP_REG_12_HEADPHONE_MIXER_HPR_BMIX )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_AMIX(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER HPR_AMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D5, SNDAMP_REG_12_HEADPHONE_MIXER_HPR_AMIX )


#define SNDAMP_SET_REG_12_HEADPHONE_MIXER_HPR_MMIX(data) \
  MM_DBG("SET REG12 HEADPHONE_MIXER HPR_MMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_12_HEADPHONE_MIXER, data, SNDAMP_REG_BIT_D6, SNDAMP_REG_12_HEADPHONE_MIXER_HPR_MMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SPL_BMIX(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SPL_BMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_13_SPEAKER_MIXER_SPL_BMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SPL_AMIX(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SPL_AMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D1, SNDAMP_REG_13_SPEAKER_MIXER_SPL_AMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SPL_MMIX(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SPL_MMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D2, SNDAMP_REG_13_SPEAKER_MIXER_SPL_MMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_MONO_SP(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER MONO_SP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D3, SNDAMP_REG_13_SPEAKER_MIXER_MONO_SP )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SPR_BMIX(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SPR_BMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D4, SNDAMP_REG_13_SPEAKER_MIXER_SPR_BMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SPR_AMIX(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SPR_AMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D5, SNDAMP_REG_13_SPEAKER_MIXER_SPR_AMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SPR_MMIX(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SPR_MMIX[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D6, SNDAMP_REG_13_SPEAKER_MIXER_SPR_MMIX )


#define SNDAMP_SET_REG_13_SPEAKER_MIXER_SWAP_SP(data) \
  MM_DBG("SET REG13 SPEAKER_MIXER SWAP_SP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_13_SPEAKER_MIXER, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_13_SPEAKER_MIXER_SWAP_SP )


#define SNDAMP_SET_REG_14_SPEAKER_ATT_MNA(data) \
  MM_DBG("SET REG14 SPEAKER_ATT MNA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_14_SPEAKER_ATT, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_14_SPEAKER_ATT_MNA )


#define SNDAMP_SET_REG_14_SPEAKER_ATT_SVOL_SP(data) \
  MM_DBG("SET REG14 SPEAKER_ATT SVOL_SP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_14_SPEAKER_ATT, data, SNDAMP_REG_BIT_D6, SNDAMP_REG_14_SPEAKER_ATT_SVOL_SP )


#define SNDAMP_SET_REG_14_SPEAKER_ATT_ZCS_SPA(data) \
  MM_DBG("SET REG14 SPEAKER_ATT ZCS_SPA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_14_SPEAKER_ATT, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_14_SPEAKER_ATT_ZCS_SPA )


#define SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_SALA(data) \
  MM_DBG("SET REG15 HEADPHONE_ATT_LCH SALA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_15_HEADPHONE_ATT_LCH, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_15_HEADPHONE_ATT_LCH_SALA )


#define SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_LAT_HP(data) \
  MM_DBG("SET REG15 HEADPHONE_ATT_LCH LAT_HP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_15_HEADPHONE_ATT_LCH, data, SNDAMP_REG_BIT_D5, SNDAMP_REG_15_HEADPHONE_ATT_LCH_LAT_HP )


#define SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_SVOL_HP(data) \
  MM_DBG("SET REG15 HEADPHONE_ATT_LCH SVOL_HP[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_15_HEADPHONE_ATT_LCH, data, SNDAMP_REG_BIT_D6, SNDAMP_REG_15_HEADPHONE_ATT_LCH_SVOL_HP )


#define SNDAMP_SET_REG_15_HEADPHONE_ATT_LCH_ZCS_HPA(data) \
  MM_DBG("SET REG15 HEADPHONE_ATT_LCH ZCS_HPA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_15_HEADPHONE_ATT_LCH, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_15_HEADPHONE_ATT_LCH_ZCS_HPA )


#define SNDAMP_SET_REG_16_HEADPHONE_ATT_RCH_SARA(data) \
  MM_DBG("SET REG16 HEADPHONE_ATT_RCH SALA[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_16_HEADPHONE_ATT_RCH, data, SNDAMP_REG_BIT_D0, SNDAMP_REG_16_HEADPHONE_ATT_RCH_SARA )


#define SNDAMP_SET_REG_17_ERROR_FLAG_OTP_ERR(data) \
  MM_DBG("SET REG17 ERROR_FLAG OTP_ERR[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_17_ERROR_FLAG, data, SNDAMP_REG_BIT_D6, SNDAMP_REG_17_ERROR_FLAG_OTP_ERR )


#define SNDAMP_SET_REG_17_ERROR_FLAG_OCP_ERR(data) \
  MM_DBG("SET REG17 ERROR_FLAG OCP_ERR[0x%x]\n", data); \
  SNDAMP_SET_REG_BUF( SNDAMP_REG_17_ERROR_FLAG, data, SNDAMP_REG_BIT_D7, SNDAMP_REG_17_ERROR_FLAG_OCP_ERR )






#define SNDAMP_KERNEL_TIMER_TIMEOUT_VAL(msec) ( ( ( (msec) * (HZ) ) / 1000 ) + SNDAMP_KERNEL_TIMER_OFFSET )


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


typedef enum {
  SNDAMP_REG_0_POWERDOWN1,          
  SNDAMP_REG_1_POWERDOWN2,          
  SNDAMP_REG_2_POWERDOWN3,          
  SNDAMP_REG_3_POWERDOWN4,          
  SNDAMP_REG_4_NONCLIP2,            /* Non-Clip2                           */
  SNDAMP_REG_5_ATTACK_RELEASE_TIME, /* Attack/Release Time                 */
  SNDAMP_REG_6_RESERVED,            /* Reserved                            */
  SNDAMP_REG_7_MONAURAL,            /* Monaural                            */
  SNDAMP_REG_8_LINE1_LCH,           /* Line1 Lch                           */
  SNDAMP_REG_9_LINE1_RCH,           /* Line1 Rch                           */
  SNDAMP_REG_10_LINE2_LCH,          /* Line2 Lch                           */
  SNDAMP_REG_11_LINE2_RCH,          /* Line2 Rch                           */
  SNDAMP_REG_12_HEADPHONE_MIXER,    
  SNDAMP_REG_13_SPEAKER_MIXER,      
  SNDAMP_REG_14_SPEAKER_ATT,        
  SNDAMP_REG_15_HEADPHONE_ATT_LCH,  
  SNDAMP_REG_16_HEADPHONE_ATT_RCH,  
  SNDAMP_REG_17_ERROR_FLAG,         
  SNDAMP_REG_MAX,
} sndamp_yda160_register_type;


typedef enum {
  SNDAMP_REG_BIT_D0,                /* B_DATA D0                           */
  SNDAMP_REG_BIT_D1,                /* B_DATA D1                           */
  SNDAMP_REG_BIT_D2,                /* B_DATA D2                           */
  SNDAMP_REG_BIT_D3,                /* B_DATA D3                           */
  SNDAMP_REG_BIT_D4,                /* B_DATA D4                           */
  SNDAMP_REG_BIT_D5,                /* B_DATA D5                           */
  SNDAMP_REG_BIT_D6,                /* B_DATA D6                           */
  SNDAMP_REG_BIT_D7,                /* B_DATA D7                           */
} sndamp_yda160_reg_bit_type;


typedef enum {
  SNDAMP_DEVICE_HEADSET_STEREO = 0,
                              /* Device : headset_stereo_rx                */
  SNDAMP_DEVICE_SPEAKER_STEREO,
                              /* Device : speaker_stereo_rx                */
  SNDAMP_DEVICE_FMRADIO_SPEAKER,
                              /* Device : fmradio_speaker_rx               */
  SNDAMP_DEVICE_TTY_HEADSET_MONO,
                              /* Device : tty_headset_mono_rx              */
  SNDAMP_DEVICE_HEADSET_STEREO_SPEAKER_STEREO,
                              /* Device : headset_stereo_speaker_stereo_rx */
  SNDAMP_DEVICE_PTT_SPEAKER_STEREO,
                              /* Device : ptt_speaker_stereo_rx            */
  SNDAMP_DEVICE_MAX,          /* Device : MAX(Unknown)                     */
} sndamp_device_type;


typedef enum {
  SNDAMP_PROC_DEVICE_HEADSET = 0,
                                    
  SNDAMP_PROC_DEVICE_SPEAKER,
                                    
  SNDAMP_PROC_DEVICE_TTY_HEADSET,
                                    
  SNDAMP_PROC_DEVICE_HEADSET_SPEAKER,
                                    
  SNDAMP_PROC_DEVICE_MAX,
                                    
  SNDAMP_PROC_DEVICE_INVALID = SNDAMP_DEVICE_MAX,
                                    
} sndamp_proc_device_type;


typedef enum {
  SNDAMP_DCDC_OFF = 0,              
  SNDAMP_DCDC_ON,                   
  SNDAMP_DCDC_MAX,
} sndamp_dcdc_state_type;


typedef enum {
  SNDAMP_SP_OFF = 0,                
  SNDAMP_SP_ON,                     
} sndamp_sp_state_type;


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/


typedef struct{
  unsigned char   in_svla;          
  unsigned char   in_svra;          
  unsigned char   out_sala;         
  unsigned char   out_sara;         
  unsigned char   out_mna;          
  unsigned char   nonclip_mode;     
  unsigned char   att_rel_time;     
} sndamp_cal_type;

#endif /* _YDA160_SNDAMPI_H */
