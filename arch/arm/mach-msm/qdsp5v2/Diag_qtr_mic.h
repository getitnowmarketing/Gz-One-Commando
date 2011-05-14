/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


#ifndef DIAG_QTR_MIC_H_
#define DIAG_QTR_MIC_H_




typedef struct
{
	
	char tx_dev;
	
	
	char rx_dev;
	
	
	char type;
}Diag_Audio_qtr_action_type;



char Diag_Audio_qtr_action(Diag_Audio_qtr_action_type *data);


/** QTRLoopback */

typedef struct
{
	
	char path;

}Diag_Audio_qtr_loopback_type;

char Diag_Audio_qtr_loopback( Diag_Audio_qtr_loopback_type *data );


/** PMIC MIC BIAS */

typedef struct
{
	
	char bias;

	
}Diag_Audio_hsedbias_ctl_type;


char Diag_Audio_hsedbias_ctl( Diag_Audio_hsedbias_ctl_type *data );



#endif	/* DIAG_QTR_MIC_H_ */

