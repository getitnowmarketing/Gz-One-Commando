#ifndef SLEEP_CHK_H
#define SLEEP_CHK_H

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/


/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/


typedef enum {
    SLEEP_CHK_LCD =0,      
    SLEEP_CHK_USB,         
    SLEEP_CHK_NOTVOLATIL,  
    SLEEP_CHK_SOUND,       
    SLEEP_CHK_OoR_SRCH,    
    SLEEP_CHK_MSTIAC,      /* MSTIAC        */
    SLEEP_CHK_EVDO,        /* EVDO          */
    SLEEP_CHK_BACKLIGHT,   
    SLEEP_CHK_KEY,         
    SLEEP_CHK_CHDIAG_GPS,  /* CHDIAG_GPS    */
                           
    SLEEP_CHK_KEYSCN_REC,  
    SLEEP_CHK_BT,          
    SLEEP_CHK_DUMMY0,      
    SLEEP_CHK_DUMMY1,      
    SLEEP_CHK_DUMMY2,      
    SLEEP_CHK_DUMMY3,      
    SLEEP_CHK_DUMMY4,      
    SLEEP_CHK_DUMMY5,      
    SLEEP_CHK_DUMMY6,      
    SLEEP_CHK_DUMMY7,      
                           
    SLEEP_CHK_DUMMY8,      
    SLEEP_CHK_DUMMY9,      
    SLEEP_CHK_DUMMY10,     
    SLEEP_CHK_DUMMY11,     
    SLEEP_CHK_DUMMY12,     
    SLEEP_CHK_DUMMY13,     
    SLEEP_CHK_DUMMY14,     
    SLEEP_CHK_DUMMY15,     
    SLEEP_CHK_DUMMY16,     
    SLEEP_CHK_DEBUG =29,   
    SLEEP_CHK_MAXNUM       
} sleep_chk_base_type;


typedef enum {
    SLEEP_ASSERT =0,       
    SLEEP_NEGATE           
} sleep_okts_flag;


extern void writeSleepChkState(sleep_chk_base_type  base, sleep_okts_flag flag);

#endif /* SLEEP_CHK_H */

