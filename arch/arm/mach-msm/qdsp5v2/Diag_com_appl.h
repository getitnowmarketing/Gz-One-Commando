/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

#ifndef DIAG_COM_APPL_H
#define DIAG_COM_APPL_H


#define		CMD_SUCCESS					0	
#define		CMD_FAILURE					1	


typedef struct {
	short result ;	/* result value */
} Diag_Audio_earjack_type;

short Diag_Audio_earjack( Diag_Audio_earjack_type *data );



typedef struct {
    u8      modeRW;
    u8      dev_idx;
    s32     max_nb;
    s32     min_nb;
    s32     max_wb;
    s32     min_wb;
} Diag_Audio_vol_limit_type;


u8 Diag_Audio_vol_limit( Diag_Audio_vol_limit_type *vol_info );


typedef struct {
    u8      modeRW;
    u8      act_idx;
    u8      in_act_idx;
    u32     act_type;
    u32     data;
} Diag_Audio_qtr_cal_type;


u8 Diag_Audio_qtr_cal( Diag_Audio_qtr_cal_type *cal_info );


void Diag_Audio_vol_limit_wrapper(char * req_data, char * rsp_data );

void Diag_Audio_qtr_cal_wrapper(char * req_data, char * rsp_data );

#define SND_CALIB_PKG_LEN			16
#define SND_CALIB_DATA_LOAD_BUF		(16*1024)
#define SND_CALIB_DATA_FILENAME		"/data/snd_calb.dat"

typedef void (* Diag_snd_calib_load_func_type) (void);
extern Diag_snd_calib_load_func_type Diag_snd_calib_data_load_func;


#endif
