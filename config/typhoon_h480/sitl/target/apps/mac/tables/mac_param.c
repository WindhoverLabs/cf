/*
** Pragmas
*/

/*
** Include Files
*/
//#include <mixer/MixerTables.h>
#include "cfe_tbl_filedef.h"
#include "mac_tbldefs.h"

/*
** Local Defines
*/

/*
** Local Structure Declarations
*/

/*
** External Global Variables
*/

/*
** Global Variables
*/

/* Default MAC config table data */
MAC_ParamTbl_t MAC_ParamTbl =
{
		6.0,		/* MC_ROLL_P 	   */
		0.15,		/* MC_ROLLRATE_P   */
		0.05,		/* MC_ROLLRATE_I   */
		0.30,		/* MC_RR_INT_LIM   */
		0.003,		/* MC_ROLLRATE_D   */
		0.0,		/* MC_ROLLRATE_FF  */
		6.5,		/* MC_PITCH_P      */
		0.15,		/* MC_PITCHRATE_P  */
		0.05,		/* MC_PITCHRATE_I  */
		0.30,		/* MC_PR_INT_LIM   */
		0.003,		/* MC_PITCHRATE_D  */
		0.0,		/* MC_PITCHRATE_FF */
		1.0,		/* MC_TPA_BREAK_P  */
		1.0,		/* MC_TPA_BREAK_I  */
		1.0,		/* MC_TPA_BREAK_D  */
		0.0,		/* MC_TPA_RATE_P   */
		0.0,		/* MC_TPA_RATE_I   */
		0.0,		/* MC_TPA_RATE_D   */
		2.8,		/* MC_YAW_P        */
		0.2,		/* MC_YAWRATE_P	   */
		0.1,		/* MC_YAWRATE_I    */
		0.30,       /* MC_YR_INT_LIM   */
		0.0,		/* MC_YAWRATE_D    */
		0.0,		/* MC_YAWRATE_FF   */
		0.5,		/* MC_YAW_FF       */
		220.0,		/* MC_ROLLRATE_MAX */
		220.0,		/* MC_PITCHRATE_MAX */
		200.0,		/* MC_YAWRATE_MAX   */
		45.0,		/* MC_YAWRAUTO_MAX  */
		360.0,      /* MC_ACRO_R_MAX    */
		360.0,		/* MC_ACRO_P_MAX    */
        360.0,		/* MC_ACRO_Y_MAX    */
		0.8,		/* MC_RATT_TH	    */
		0,			/* VT_TYPE          */
		0.2,		/* MC_ROLL_TC		*/
		0.2,		/* MC_PITCH_TC      */
		0,			/* VT_OPT_RECOV_EN  */
		0.15,		/* VT_WV_YAWR_SCL   */
		0,			/* MC_BAT_SCALE_EN  */
		0,			/* SENS_BOARD_ROT   */
		0.0,		/* SENS_BOARD_X_OFF */
		0.0,		/* SENS_BOARD_Y_OFF */
		0.0 		/* SENS_BOARD_Z_OFF */
};

/*
** Local Variables
*/

/*
** Function Prototypes
*/

/*
** Function Definitions
*/

CFE_TBL_FILEDEF(MAC_ParamTbl, MAC.PARAM_TBL, MAC default param table, mac_param.tbl )

/*=======================================================================================
** End of file ci_config.c
**=====================================================================================*/
    
