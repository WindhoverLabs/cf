#ifndef PMC_APP_H
#define PMC_APP_H


#include <mixer/MultirotorMixer.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 ** Pragmas
 *************************************************************************/

/************************************************************************
 ** Includes
 *************************************************************************/
#include "cfe.h"

#include "pmc_platform_cfg.h"
#include "pmc_mission_cfg.h"
#include "pmc_private_ids.h"
#include "pmc_private_types.h"
#include "pmc_perfids.h"
#include "pmc_msgids.h"
#include "pmc_msg.h"
#include "pmc_events.h"
#include "pmc_tbldefs.h"
#include "px4_msgs.h"
#include <pwm_limit/pwm_limit.h>


/************************************************************************
 ** Local Defines
 *************************************************************************/

/************************************************************************
 ** Local Structure Definitions
 *************************************************************************/

typedef struct
{
    PX4_ActuatorControlsMsg_t ActuatorControls0;
    PX4_ActuatorArmedMsg_t    ActuatorArmed;
    PX4_RcChannelsMsg_t       RcChannels;
} PMC_CurrentValueTable_t;


/**
 **  \brief PMC Operational Data Structure
 */
class PMC
{
public:
    PMC();
    ~PMC();

    /** \brief CFE Event Table */
    CFE_EVS_BinFilter_t EventTbl[PMC_EVT_CNT];

    /**\brief Scheduling Pipe ID */
    CFE_SB_PipeId_t SchPipeId;

    /** \brief Command Pipe ID */
    CFE_SB_PipeId_t CmdPipeId;

    /** \brief Data Pipe ID */
    CFE_SB_PipeId_t DataPipeId;

    /* Task-related */

    /** \brief Task Run Status */
    uint32 uiRunStatus;

    /* Config table-related */

    /** \brief PWM Config Table Handle */
    CFE_TBL_Handle_t PwmConfigTblHdl;

    /** \brief Mixer Config Table Handle */
    CFE_TBL_Handle_t MixerConfigTblHdl;

    /** \brief PWM Config Table Pointer */
    PMC_PwmConfigTbl_t* PwmConfigTblPtr;

    /** \brief Mixer Config Table Pointer */
    MultirotorMixer_ConfigTable_t* MixerConfigTblPtr;

    /** \brief Output Data published at the end of cycle */
    PX4_ActuatorOutputsMsg_t ActuatorOutputs;

    /** \brief Housekeeping Telemetry for downlink */
    PMC_HkTlm_t HkTlm;

    PMC_CurrentValueTable_t CVT;

    MultirotorMixer MixerObject;

    //MIXER_Data_t  MixerData;
    PwmLimit_Data_t PwmLimit;

    /************************************************************************/
    /** \brief CFS PWM Motor Controller Task (PMC) application entry point
     **
     **  \par Description
     **       CFS PWM Motor Controller Task application entry point.  This function
     **       performs app initialization, then waits for the cFE ES Startup
     **       Sync, then executes the RPR main processing loop.
     **
     **  \par Assumptions, External Events, and Notes:
     **       If there is an unrecoverable failure during initialization the
     **       main loop is never executed and the application will exit.
     **
     *************************************************************************/
    void AppMain(void);

    /************************************************************************/
    /** \brief Initialize the CFS PWM Motor Controller (PMC) application
     **
     **  \par Description
     **       PWM Motor Controller application initialization routine. This
     **       function performs all the required startup steps to
     **       initialize (or restore from CDS) PMC data structures and get
     **       the application registered with the cFE services so it can
     **       begin to receive command messages and send events.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \returns
     **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS    \endcode
     **  \retstmt Return codes from #CFE_ES_RegisterApp          \endcode
     **  \retstmt Return codes from #PMC_InitEvent               \endcode
     **  \retstmt Return codes from #PMC_InitPipe                \endcode
     **  \retstmt Return codes from #PMC_InitData                \endcode
     **  \retstmt Return codes from #PMC_InitConfigTbl           \endcode
     **  \retstmt Return codes from #OS_TaskInstallDeleteHandler \endcode
     **  \endreturns
     **
     *************************************************************************/
    int32 InitApp(void);

    /************************************************************************/
    /** \brief Initialize Event Services and Event tables
     **
     **  \par Description
     **       This function performs the steps required to setup
     **       cFE Event Services for use by the PMC application.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \returns
     **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
     **  \retstmt Return codes from #CFE_EVS_Register  \endcode
     **  \endreturns
     **
     *************************************************************************/
    int32 InitEvent(void);

    /************************************************************************/
    /** \brief Initialize global variables used by PMC application
     **
     **  \par Description
     **       This function performs the steps required to initialize
     **       the PMC application data.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \returns
     **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
     **  \retstmt Return codes from #CFE_EVS_Register  \endcode
     **  \endreturns
     **
     *************************************************************************/
    int32 InitData(void);

    /************************************************************************/
    /** \brief Initialize message pipes
     **
     **  \par Description
     **       This function performs the steps required to setup
     **       initialize the cFE Software Bus message pipes and subscribe to
     **       messages for the PMC application.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \returns
     **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
     **  \retstmt Return codes from #CFE_SB_CreatePipe        \endcode
     **  \retstmt Return codes from #CFE_SB_SubscribeEx       \endcode
     **  \retstmt Return codes from #CFE_SB_Subscribe         \endcode
     **  \endreturns
     **
     *************************************************************************/
    int32 InitPipe(void);

    /************************************************************************/
    /** \brief Receive and process messages from the scheduler pipe.
     **
     **  \par Description
     **       This function receives and processes messages
     **       for the PMC application from the SCH pipe.  This function
     **       will pend for the type defined by iBlocking, allowing
     **       it to wait for messages, i.e. wakeup messages from scheduler.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \param [in]   iBlocking    A #CFE_SB_PEND_FOREVER, #CFE_SB_POLL or
     **                             millisecond timeout
     **
     **  \returns
     **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS \endcode
     **  \retstmt Return codes from #CFE_SB_RcvMsg            \endcode
     **  \endreturns
     **
     *************************************************************************/
    int32 RcvSchPipeMsg(int32 iBlocking);

    /************************************************************************/
    /** \brief PWM Motor Controller Task incoming command processing
     **
     **  \par Description
     **       This function processes incoming commands subscribed
     **       by PMC application
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     *************************************************************************/
    void ProcessNewCmds(void);

    /************************************************************************/
    /** \brief PWM Motor Controller Task application commands
     **
     **  \par Description
     **       This function processes command messages
     **       specific to the PMC application
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \param [in]   MsgPtr       A #CFE_SB_Msg_t pointer that
     **                             references the software bus message
     **
     *************************************************************************/
    void ProcessAppCmds(CFE_SB_Msg_t* MsgPtr);

    /************************************************************************/
    /** \brief Sends PMC housekeeping message
     **
     **  \par Description
     **       This function sends the housekeeping message
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     *************************************************************************/
    void ReportHousekeeping(void);

    /************************************************************************/
    /** \brief Sends the Actuator Output message.
     **
     **  \par Description
     **       This function publishes the actuator output message containing
     **       the commanding values to the motors.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     *************************************************************************/
    void SendActuatorOutputs(void);

    /************************************************************************/
    /** \brief Verify Command Length
     **
     **  \par Description
     **       This function verifies the command message length.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \param [in]   MsgPtr        A #CFE_SB_Msg_t pointer that
     **                              references the software bus message
     **  \param [in]   usExpectedLen The expected length of the message
     **
     **  \returns
     **  TRUE if the message length matches expectations, FALSE if it does not.
     **  \endreturns
     **
     *************************************************************************/
    boolean VerifyCmdLength(CFE_SB_Msg_t* MsgPtr, uint16 usExpectedLen);

private:
    /************************************************************************/
    /** \brief Set the actual motor outputs.
     **
     **  \par Description
     **       This function takes an array of uint16 representing motor
     **       speeds for each of the motors in the array, and sets the motor
     **       to that specific speed.  The range of inputs is configuration
     **       specific.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \param [in]   *PWM          A pointer to an array of uint16.  The
     **                              array size is defined by
     **                              PMC_MAX_MOTOR_OUTPUTS.
     **                              references the software bus message
     **
     *************************************************************************/
    void   SetMotorOutputs(const uint16 *PWM);

    /************************************************************************/
    /** \brief Initialize device
     **
     **  \par Description
     **       This function is defined in the platform specific package and
     **       initializes the device, whatever it is, to allow the application
     **       to set motor speeds.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     **  \returns
     **  Returns 0 if successful.  Returns a negative number if unsuccessful.
     **  Meaning of actual return value is platform specific.
     **  \endreturns
     **
     *************************************************************************/
    int32  InitDevice(void);

    /************************************************************************/
    /** \brief Update motors
     **
     **  \par Description
     **       This function mixes the yaw, pitch, roll, and throttle actuators
     **       and sets each specific motor to result in the commanded attitude
     **       change.  Exactly to which proportion each motor affects each
     **       actuator position is defined in the #MixerConfigTblHdl table.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     *************************************************************************/
    void   UpdateMotors(void);

    /************************************************************************/
    /** \brief Stop motors
     **
     **  \par Description
     **       This function commands the motors to immediately stop.
     **
     **  \par Assumptions, External Events, and Notes:
     **       None
     **
     *************************************************************************/
    void   StopMotors(void);

    /************************************************************************/
    /** \brief Initialize the PMC configuration tables.
    **
    **  \par Description
    **       This function initializes PMC's configuration tables.  This
    **       includes the PWM and the Mixer configuration table.
    **
    **  \par Assumptions, External Events, and Notes:
    **       None
    **
    **  \returns
    **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS  \endcode
    **  \retstmt Return codes from #CFE_TBL_Register          \endcode
    **  \retstmt Return codes from #CFE_TBL_Load              \endcode
    **  \retstmt Return codes from #PMC_AcquireConfigPointers \endcode
    **  \endreturns
    **
    *************************************************************************/
    int32  InitConfigTbl(void);

    /************************************************************************/
    /** \brief Obtain PMC configuration tables data pointers.
    **
    **  \par Description
    **       This function manages the configuration tables
    **       and obtains a pointer to their data.
    **
    **  \par Assumptions, External Events, and Notes:
    **       None
    **
    **  \returns
    **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS  \endcode
    **  \endreturns
    **
    *************************************************************************/
    int32  AcquireConfigPointers(void);

public:
    /************************************************************************/
    /** \brief Validate PMC PWM configuration table
    **
    **  \par Description
    **       This function validates PMC's PWM configuration table
    **
    **  \par Assumptions, External Events, and Notes:
    **       None
    **
    **  \param [in]   ConfigTblPtr    A pointer to the table to validate.
    **
    **  \returns
    **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS  \endcode
    **  \endreturns
    **
    *************************************************************************/
    static int32  ValidatePwmCfgTbl(void*);

    /************************************************************************/
    /** \brief Validate PMC Mixer configuration table
    **
    **  \par Description
    **       This function validates PMC's Mixer configuration table
    **
    **  \par Assumptions, External Events, and Notes:
    **       None
     **
     **  \param [in]   ConfigTblPtr    A pointer to the table to validate.
    **
    **  \returns
    **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS  \endcode
    **  \endreturns
    **
    *************************************************************************/
    static int32  ValidateMixerCfgTbl(void*);

    /************************************************************************/
    /** \brief Return the specified control value
    **
    **  \par Description
    **       This function is called by the Mixer object and returns the
    **       value of the specified control.  This specific function only
    **       supports ControlGroup 0 and will return an error otherwise.
    **
    **  \par Assumptions, External Events, and Notes:
    **       None
     **
     **  \param [in]   ControlGroup    Index of the control group.
     **  \param [in]   ControlIndex    Index of the control.
    **
    **  \returns
    **  \retcode #CFE_SUCCESS  \retdesc \copydoc CFE_SUCCESS  \endcode
    **  \retcode -1  \retdesc Invalid input  \endcode
    **  \endreturns
    **
    *************************************************************************/
    static int32  ControlCallback(cpuaddr Handle,
        uint8 ControlGroup,
        uint8 ControlIndex,
        float &Control);
};

#ifdef __cplusplus
}
#endif 

#endif /* PMC_APP_H */

/************************/
/*  End of File Comment */
/************************/
