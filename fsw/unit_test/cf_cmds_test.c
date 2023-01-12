/****************************************************************************
*
*   Copyright (c) 2017 Windhover Labs, L.L.C. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
* 3. Neither the name Windhover Labs nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include "cf_cmds_test.h"
#include "cf_test_utils.h"
#include "cf_custom_hooks.h"

#include "cf_events.h"
#include "cf_version.h"
#include "structures.h"
#include "timer.h"
#include "machine.h"
#include "misc.h"
#include "cf_playback.h"

#include "uttest.h"
#include "ut_osapi_stubs.h"
#include "ut_osfileapi_stubs.h"
#include "ut_cfe_sb_stubs.h"
#include "ut_cfe_sb_hooks.h"
#include "ut_cfe_es_stubs.h"
#include "ut_cfe_es_hooks.h"
#include "ut_cfe_evs_stubs.h"
#include "ut_cfe_evs_hooks.h"
#include "ut_cfe_time_stubs.h"
#include "ut_cfe_psp_memutils_stubs.h"
#include "ut_cfe_psp_timer_stubs.h"
#include "ut_cfe_tbl_stubs.h"
#include "ut_cfe_fs_stubs.h"

#include <string.h>
#include <ctype.h>
#include <unistd.h>


CFE_SB_MsgId_t  SendHkHook_MsgId = 0;
CFE_SB_MsgId_t  SendCfgParamsHook_MsgId = 0;
CFE_SB_MsgId_t  SendTransDiagHook_MsgId = 0;
uint32          SendTransDiagHook_CalledCnt = 0;


static char* CF_Test_ToUpperCase(const char *inStr)
{
    int i;
    int len;
    static char retStr[1024];

    memset((void *)retStr, 0x00, sizeof(retStr));
    len = strlen(inStr);
    for (i = 0; i < len; i++)
    {
        retStr[i] = (char)toupper((int)inStr[i]);
    }

    return(retStr);
}

/**************************************************************************
 * Tests for CF_AppPipe()
 **************************************************************************/
/**
 * Test CF_AppPipe, InvalidCmdMessage
 */
void Test_CF_AppPipe_InvalidCmdMessage(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, 0, sizeof(CmdMsg), TRUE);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Unexpected Msg Received MsgId -- ID = 0x%04X", 0);

    /* Verify results */
    UtAssert_EventSent(CF_MID_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppMain, fail InvalidCmdMessage: Event Sent");
}


/**
 * Test CF_AppPipe, InvalidCmdCode
 */
void Test_CF_AppPipe_InvalidCmdCode(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)100);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
        "Cmd Msg with Invalid command code Rcvd -- ID = 0x%04X, CC = %d",
        CF_CMD_MID, 100);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "Test_CF_AppPipe_InvalidCmdCode");

    UtAssert_EventSent(CF_CC_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "Test_CF_AppPipe_InvalidCmdCode: Event Sent");
}


/**
 * Test CF_AppPipe, NoopCmd
 */
void Test_CF_AppPipe_NoopCmd(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_NOOP_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF No-op command, Version %d.%d.%d.%d",
                      CF_MAJOR_VERSION, CF_MINOR_VERSION,
                      CF_REVISION, CF_MISSION_REV);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, NoopCmd");

    UtAssert_EventSent(CF_NOOP_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, NoopCmd: Event Sent");
}


/**
 * Test CF_AppPipe, NoopCmdInvLen
 */
void Test_CF_AppPipe_NoopCmdInvLen(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_NOOP_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d",
            CF_CMD_MID, CF_NOOP_CC, sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, NoopCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, NoopCmdInvLen: Event Sent");
}


/**
 * Test CF_AppPipe, RstCtrsCmd
 */
void Test_CF_AppPipe_RstCtrsCmd(void)
{
    CF_ResetCtrsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_RESET_CC);
    CmdMsg.Value = 0;
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Reset Counters command received - Value %u",
            CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, RstCtrsCmd");

    UtAssert_EventSent(CF_RESET_CMD_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, RstCtrsCmd: Event Sent");
}


/**
 * Test CF_AppPipe, RstCtrsCmdInvLen
 */
void Test_CF_AppPipe_RstCtrsCmdInvLen(void)
{
    CF_ResetCtrsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_RESET_CC);
    CmdMsg.Value = 0;
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d",
            CF_CMD_MID, CF_RESET_CC, sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, RstCtrsCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, RstCtrsCmdInvLen: Event Sent");
}


/**
 * Test CF_AppPipe, PbFileCmd
 */
void Test_CF_AppPipe_PbFileCmd(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    QEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(expEvent,
        "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,Peer %s,File %s",
         PbFileCmdMsg.Class, PbFileCmdMsg.Channel, PbFileCmdMsg.Priority,
         PbFileCmdMsg.Preserve, TestPbPeerEntityId, PbFileCmdMsg.SrcFilename);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PbFileCmd");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, PbFileCmd: Q EntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, PbFileCmd: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdNoMem
 */
void Test_CF_AppPipe_PbFileCmdNoMem(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Execute a playback file command so that one queue entry is added
       to the pending queue */
    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_FDGETINFO_INDEX,
                               OS_FS_ERR_INVALID_FD, 1);

    Ut_CFE_ES_SetReturnCode(UT_CFE_ES_GETPOOLBUF_INDEX,
                            CFE_ES_ERR_MEM_HANDLE, 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "PB File %s Cmd Ignored,Error Allocating Queue Node.",
            PbFileCmdMsg.SrcFilename);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdNoMem");

    UtAssert_EventSent(CF_QDIR_NOMEM1_EID, CFE_EVS_ERROR, expEvent,
                   "CF_AppPipe, PbFileCmdNoMem: AllocQ Error Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdInvLen
 */
void Test_CF_AppPipe_PbFileCmdInvLen(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_PLAYBACK_FILE_CC,
            sizeof(CF_PlaybackFileCmd_t), sizeof(CF_PlaybackFileCmd_t) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdParamErr
 */
void Test_CF_AppPipe_PbFileCmdParamErr(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_2 + 1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "Playback File Cmd Parameter error, class %d, chan %d",
            PbFileCmdMsg.Class, PbFileCmdMsg.Channel);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdParamErr");

    UtAssert_EventSent(CF_PB_FILE_ERR1_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdParamErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdChanNotInUse
 */
void Test_CF_AppPipe_PbFileCmdChanNotInUse(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 1;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.Tbl->OuCh[PbFileCmdMsg.Channel].EntryInUse =
                                                  CF_ENTRY_UNUSED;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "CF:Playback File Cmd Parameter Error, Chan %u is not in use.",
            PbFileCmdMsg.Channel);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdChanNotInUse");

    UtAssert_EventSent(CF_PB_FILE_ERR2_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdChanNotInUse: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdInvSrcFilename
 */
void Test_CF_AppPipe_PbFileCmdInvSrcFilename(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, " pbfile1.dat");
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "Filename in %s must be terminated and have no spaces",
            "PlaybackFileCmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdInvSrcFilename");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdInvSrcFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdInvDstFilename
 */
void Test_CF_AppPipe_PbFileCmdInvDstFilename(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    /* dest filename not terminated */
    CFE_PSP_MemSet(PbFileCmdMsg.DstFilename, 0xFF, OS_MAX_PATH_LEN);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "Filename in %s must be terminated and have no spaces",
            "PlaybackFileCmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdInvDstFilename");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdInvDstFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdPendQFull
 */
void Test_CF_AppPipe_PbFileCmdPendQFull(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt =
                 CF_AppData.Tbl->OuCh[PbFileCmdMsg.Channel].PendingQDepth;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
      "CF:Playback File Cmd Error, Chan %u Pending Queue is full %u.",
      PbFileCmdMsg.Channel,
      (unsigned int)CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdPendQFull");

    UtAssert_EventSent(CF_PB_FILE_ERR3_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdPendQFull: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdInvPeerId
 */
void Test_CF_AppPipe_PbFileCmdInvPeerId(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, "2555.255");
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:PB File Cmd Err, PeerEntityId %s must be "
            "2 byte,dotted decimal fmt.ex 0.24",
            PbFileCmdMsg.PeerEntityId);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdInvPeerId");

    UtAssert_EventSent(CF_PB_FILE_ERR6_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdInvPeerId: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdFileOpen
 */
void Test_CF_AppPipe_PbFileCmdFileOpen(void)
{
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    CFE_SB_InitMsg((void*)&PbFileCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackFileCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbFileCmdMsg,
                      (uint16)CF_PLAYBACK_FILE_CC);
    PbFileCmdMsg.Class = CF_CLASS_1;
    PbFileCmdMsg.Channel = 0;
    PbFileCmdMsg.Priority = 0;
    PbFileCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbFileCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbFileCmdMsg.SrcFilename, TestPbDir);
    strcat(PbFileCmdMsg.SrcFilename, TestPbFile1);
    strcpy(PbFileCmdMsg.DstFilename, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbFileCmdMsg, sizeof(PbFileCmdMsg));

    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_FDGETINFO_INDEX,
                                 (void*)&OS_FDGetInfoHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:Playback File Cmd Error, File is Open:%s",
            PbFileCmdMsg.SrcFilename);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdFileOpen");

    UtAssert_EventSent(CF_PB_FILE_ERR4_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdFileOpen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbFileCmdFileOnQ
 */
void Test_CF_AppPipe_PbFileCmdFileOnQ(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    /* Add same command */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbFileCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(FullSrcFileName, "%s%s", TestPbDir, TestPbFile1);
    sprintf(expEvent, "CF:Playback File Cmd Error, File is Already "
            "Pending or Active:%s", FullSrcFileName);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbFileCmdFileOnQ");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, PbFileCmdFileOnQ: Q EntryCnt");

    UtAssert_EventSent(CF_PB_FILE_ERR5_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbFileCmdFileOnQ: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdNoFileSuccess
 */
void Test_CF_AppPipe_PbDirCmdNoFileSuccess(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Force OS_opendir to return success, instead of default NULL */
    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, 5, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                          UT_OSFILEAPI_OPENDIR_INDEX);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Playback Dir Cmd Rcvd,Ch %d,Cl %d,Pri %d,Pre %d,"
            "Peer %s, Src %s,Dst %s",
            PbDirCmdMsg.Chan, PbDirCmdMsg.Class, PbDirCmdMsg.Priority,
            PbDirCmdMsg.Preserve, PbDirCmdMsg.PeerEntityId,
            TestPbDir, TestDstDir);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PbDirCmdNoFileSuccess");

    UtAssert_EventSent(CF_PLAYBACK_DIR_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, PbDirCmdNoFileSuccess: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdOpenErr
 */
void Test_CF_AppPipe_PbDirCmdOpenErr(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, (int32)NULL, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                           UT_OSFILEAPI_OPENDIR_INDEX);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Playback Dir Error %d,cannot open directory %s",
            (int)NULL, TestPbDir);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdOpenErr");

    UtAssert_EventSent(CF_OPEN_DIR_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdOpenErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdInvLen
 */
void Test_CF_AppPipe_PbDirCmdInvLen(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d",
            CF_CMD_MID, CF_PLAYBACK_DIR_CC, sizeof(CF_PlaybackDirCmd_t),
            sizeof(CF_PlaybackDirCmd_t) + 5);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdParamErr
 */
void Test_CF_AppPipe_PbDirCmdParamErr(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_2 + 1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Playback Dir Cmd Parameter error,class %d,"
            "chan %d,preserve %d",
            PbDirCmdMsg.Class, PbDirCmdMsg.Chan, PbDirCmdMsg.Preserve);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdParamErr");

    UtAssert_EventSent(CF_PB_DIR_ERR1_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdParamErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdChanNotInUse
 */
void Test_CF_AppPipe_PbDirCmdChanNotInUse(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 1;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.Tbl->OuCh[PbDirCmdMsg.Chan].EntryInUse = CF_ENTRY_UNUSED;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "CF:Playback Dir Cmd Parameter Error, Chan %u is not in use.",
            PbDirCmdMsg.Chan);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdChanNotInUse");

    UtAssert_EventSent(CF_PB_DIR_ERR2_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdChanNotInUse: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdInvSrcPath
 */
void Test_CF_AppPipe_PbDirCmdInvSrcPath(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    /* The last char should be forward slash */
    strcpy(PbDirCmdMsg.SrcPath, "/cf/pbfile1.txt");
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "SrcPath in PB Dir Cmd must be terminated,"
            "have no spaces,slash at end");

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdInvSrcPath");

    UtAssert_EventSent(CF_PB_DIR_ERR3_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdInvSrcPath: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdInvDstPath
 */
void Test_CF_AppPipe_PbDirCmdInvDstPath(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    /* dest filename not terminated */
    CFE_PSP_MemSet(PbDirCmdMsg.DstPath, 0xFF, OS_MAX_PATH_LEN);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "DstPath in PB Dir Cmd must be terminated "
            "and have no spaces");

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdInvDstPath");

    UtAssert_EventSent(CF_PB_DIR_ERR4_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdInvDstPath: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdInvPeerId
 */
void Test_CF_AppPipe_PbDirCmdInvPeerId(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, "2555.255");
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:PB Dir Cmd Err,PeerEntityId %s must be 2 byte,"
            "dotted decimal fmt.ex 0.24", PbDirCmdMsg.PeerEntityId);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdInvPeerId");

    UtAssert_EventSent(CF_PB_DIR_ERR5_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdInvPeerId: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdQFull
 */
void Test_CF_AppPipe_PbDirCmdQFull(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Force OS_opendir to return success, instead of default NULL */
    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, 5, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                    UT_OSFILEAPI_OPENDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSEDIR_INDEX,
                               OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                   UT_OSFILEAPI_CLOSEDIR_INDEX);

    ReaddirHookCallCnt = 0;
    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_READDIR_INDEX,
                                 (void*)&OS_readdirHook);

    /* Execute the function being tested */
    CF_AppInit();

    /* makes the pending queue appear full */
    CF_AppData.Chan[PbDirCmdMsg.Chan].PbQ[CF_PB_PENDINGQ].EntryCnt =
                    CF_AppData.Tbl->OuCh[PbDirCmdMsg.Chan].PendingQDepth;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Queue Dir %s Aborted,Ch %d Pending Queue is Full,"
        "%u Entries", TestPbDir, PbDirCmdMsg.Chan,
        (unsigned int)CF_AppData.Chan[PbDirCmdMsg.Chan].PbQ[CF_PB_PENDINGQ].EntryCnt);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdQFull");

    UtAssert_EventSent(CF_QDIR_PQFUL_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdQFull: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdNoMem
 */
void Test_CF_AppPipe_PbDirCmdNoMem(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, 5, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                               UT_OSFILEAPI_OPENDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSEDIR_INDEX,
                               OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                               UT_OSFILEAPI_CLOSEDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_FDGETINFO_INDEX,
                               OS_FS_ERR_INVALID_FD, 1);

    Ut_CFE_ES_SetReturnCode(UT_CFE_ES_GETPOOLBUF_INDEX,
                            CFE_ES_ERR_MEM_HANDLE, 1);

    ReaddirHookCallCnt = 0;
    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_READDIR_INDEX,
                                 (void*)&OS_readdirHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "PB Dir %s Aborted,Error Allocating Queue Node.",
            TestPbDir);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PbDirCmdNoMem");

    UtAssert_EventSent(CF_QDIR_NOMEM2_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PbDirCmdNoMem: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdFileOnQ
 */
void Test_CF_AppPipe_PbDirCmdFileOnQ(void)
{
    uint32               QEntryCnt;
    CF_PlaybackFileCmd_t PbFileCmdMsg;
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, 5, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                     UT_OSFILEAPI_OPENDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSEDIR_INDEX,
                                               OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                    UT_OSFILEAPI_CLOSEDIR_INDEX);

    ReaddirHookCallCnt = 0;
    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_READDIR_INDEX,
                                 (void*)&OS_readdirHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[PbDirCmdMsg.Chan].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(expEvent, "File %s not queued because it's active or pending",
            PbFileCmdMsg.SrcFilename);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PbDirCmdFileOnQ");

    UtAssert_True(QEntryCnt == 3,
                  "CF_AppPipe, PbDirCmdFileOnQ: Q EntryCnt");

    UtAssert_EventSent(CF_QDIR_ACTIVEFILE_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, PbDirCmdFileOnQ: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdFileOpen
 */
void Test_CF_AppPipe_PbDirCmdFileOpen(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, 5, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                     UT_OSFILEAPI_OPENDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSEDIR_INDEX,
                                               OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                    UT_OSFILEAPI_CLOSEDIR_INDEX);

    ReaddirHookCallCnt = 0;
    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_READDIR_INDEX,
                                 (void*)&OS_readdirHook);

    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_FDGETINFO_INDEX,
                                 (void*)&OS_FDGetInfoHook);

    CFE_ES_GetPoolBufHookCallCnt = 0;
    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_GETPOOLBUF_INDEX,
                              (void*)&CFE_ES_GetPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(FullSrcFileName, "%s%s", TestPbDir, TestPbFile1);
    sprintf(expEvent, "File %s not queued because it's open",
            FullSrcFileName);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PbDirCmdFileOpen");

    UtAssert_EventSent(CF_QDIR_OPENFILE_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, PbDirCmdFileOpen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PbDirCmdSuccess
 */
void Test_CF_AppPipe_PbDirCmdSuccess(void)
{
    CF_PlaybackDirCmd_t  PbDirCmdMsg;
    char  expEventTerm[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventLen[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* reset the transactions seq number used by the engine */
    misc__set_trans_seq_num(1);

    /* Setup Inputs */
    CFE_SB_InitMsg((void*)&PbDirCmdMsg, CF_CMD_MID,
                   sizeof(CF_PlaybackDirCmd_t), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PbDirCmdMsg,
                      (uint16)CF_PLAYBACK_DIR_CC);
    PbDirCmdMsg.Class = CF_CLASS_1;
    PbDirCmdMsg.Chan = 0;
    PbDirCmdMsg.Priority = 0;
    PbDirCmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(PbDirCmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(PbDirCmdMsg.SrcPath, TestPbDir);
    strcpy(PbDirCmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&PbDirCmdMsg, sizeof(PbDirCmdMsg));

    /* Force OS_opendir to return success, instead of default NULL */
    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_OPENDIR_INDEX, 5, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                          UT_OSFILEAPI_OPENDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSEDIR_INDEX,
                                                    OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                                         UT_OSFILEAPI_CLOSEDIR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_FDGETINFO_INDEX,
                               OS_FS_ERR_INVALID_FD, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(
                               UT_OSFILEAPI_FDGETINFO_INDEX);

    ReaddirHookCallCnt = 0;
    Ut_OSFILEAPI_SetFunctionHook(UT_OSFILEAPI_READDIR_INDEX,
                                 (void*)&OS_readdirHook);

    /* force the GetPoolBuf call for the queue entry to return
       something valid */
    CFE_ES_GetPoolBufHookCallCnt = 0;
    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_GETPOOLBUF_INDEX,
                              (void*)&CFE_ES_GetPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PbDirCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventTerm,
            "File not queued from %s,Filename not terminated or too long",
            TestPbDir);
    sprintf(expEventLen,
            "File not queued from %s,sum of Pathname,Filename too long",
            TestPbDir);
    sprintf(expEvent, "Playback Dir Cmd Rcvd,Ch %d,Cl %d,Pri %d,Pre %d,"
            "Peer %s, Src %s,Dst %s",
            PbDirCmdMsg.Chan, PbDirCmdMsg.Class, PbDirCmdMsg.Priority,
            PbDirCmdMsg.Preserve, PbDirCmdMsg.PeerEntityId,
            TestPbDir, TestDstDir);

    /* Verify Outputs */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PbDirCmdSuccess");

    UtAssert_EventSent(CF_QDIR_INV_NAME1_EID, CFE_EVS_ERROR, expEventTerm,
             "CF_AppPipe, PbDirCmdSuccess: Termination Error Event Sent");

    UtAssert_EventSent(CF_QDIR_INV_NAME2_EID, CFE_EVS_ERROR, expEventLen,
             "CF_AppPipe, PbDirCmdSuccess: Length Error Event Sent");

    UtAssert_EventSent(CF_PLAYBACK_DIR_EID, CFE_EVS_DEBUG, expEvent,
             "CF_AppPipe, PbDirCmdSuccess: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Hook to support: Test CF_AppPipe, HousekeepingCmd
 */
int32 Test_CF_AppPipe_HousekeepingCmd_SendMsgHook(CFE_SB_Msg_t *MsgPtr)
{
    unsigned char       *pBuff = NULL;
    uint16              msgLen = 0;
    int                 i = 0;
    CFE_SB_MsgId_t      MsgId = 0;
    time_t              localTime;
    struct tm           *loc_time;
    CFE_TIME_SysTime_t  TimeFromMsg;
    CF_HkPacket_t       HkTlm;

    msgLen = CFE_SB_GetTotalMsgLength(MsgPtr);
    MsgId = CFE_SB_GetMsgId(MsgPtr);

    pBuff = (unsigned char *)MsgPtr;
    printf("###HousekeepingCmd_SendMsgHook(msgLen %u): ", msgLen);
    for (i = 0; i < msgLen; i++)
    {
        printf("0x%02x ", *pBuff);
        pBuff++;
    }
    printf("\n");

    TimeFromMsg = CFE_SB_GetMsgTime(MsgPtr);
    localTime = CF_Test_GetTimeFromMsg(TimeFromMsg);
    loc_time = localtime(&localTime);
    printf("TimeFromMessage: %s", asctime(loc_time));

    switch(MsgId)
    {
        case CF_HK_TLM_MID:
        {
            SendHkHook_MsgId = CF_HK_TLM_MID;
            printf("Sent CF_HK_TLM_MID\n");
            memcpy((void *)&HkTlm, (void *)MsgPtr, sizeof(HkTlm));

            printf("CmdCounter: %u\n", HkTlm.CmdCounter);
            printf("ErrCounter: %u\n", HkTlm.ErrCounter);

            printf("App.WakeupForFileProc: %lu\n",
                    HkTlm.App.WakeupForFileProc);
            printf("App.EngineCycleCount: %lu\n",
                    HkTlm.App.EngineCycleCount);
            printf("App.MemInUse: %lu\n", HkTlm.App.MemInUse);
            printf("App.PeakMemInUse: %lu\n", HkTlm.App.PeakMemInUse);
            printf("App.LowMemoryMark: %lu\n", HkTlm.App.LowMemoryMark);
            printf("App.MaxMemNeeded: %lu\n", HkTlm.App.MaxMemNeeded);
            printf("App.MemAllocated: %lu\n", HkTlm.App.MemAllocated);
            printf("App.BufferPoolHandle: %lu\n",
                    HkTlm.App.BufferPoolHandle);
            printf("App.QNodesAllocated: %lu\n", HkTlm.App.QNodesAllocated);
            printf("App.QNodesDeallocated: %lu\n",
                    HkTlm.App.QNodesDeallocated);
            printf("App.PDUsReceived: %lu\n", HkTlm.App.PDUsReceived);
            printf("App.PDUsRejected: %lu\n", HkTlm.App.PDUsRejected);
            printf("App.TotalInProgTrans: %lu\n",
                    HkTlm.App.TotalInProgTrans);
            printf("App.TotalFailedTrans: %lu\n",
                    HkTlm.App.TotalFailedTrans);
            printf("App.TotalAbandonTrans: %lu\n",
                    HkTlm.App.TotalAbandonTrans);
            printf("App.TotalSuccessTrans: %lu\n",
                    HkTlm.App.TotalSuccessTrans);
            printf("App.TotalCompletedTrans: %lu\n",
                    HkTlm.App.TotalCompletedTrans);
            printf("App.LastFailedTrans: %s\n", HkTlm.App.LastFailedTrans);

            printf("AutoSuspend.EnFlag: %lu\n", HkTlm.AutoSuspend.EnFlag);
            printf("AutoSuspend.LowFreeMark: %lu\n",
                    HkTlm.AutoSuspend.LowFreeMark);

            printf("Cond.PosAckNum: %u\n", HkTlm.Cond.PosAckNum);
            printf("Cond.FileStoreRejNum: %u\n", HkTlm.Cond.FileStoreRejNum);
            printf("Cond.FileChecksumNum: %u\n", HkTlm.Cond.FileChecksumNum);
            printf("Cond.FileSizeNum: %u\n", HkTlm.Cond.FileSizeNum);
            printf("Cond.NakLimitNum: %u\n", HkTlm.Cond.NakLimitNum);
            printf("Cond.InactiveNum: %u\n", HkTlm.Cond.InactiveNum);
            printf("Cond.SuspendNum: %u\n", HkTlm.Cond.SuspendNum);
            printf("Cond.CancelNum: %u\n", HkTlm.Cond.CancelNum);

            printf("Eng.FlightEngineEntityId: %s\n",
                    HkTlm.Eng.FlightEngineEntityId);
            printf("Eng.Flags: %lu\n", HkTlm.Eng.Flags);
            printf("Eng.MachinesAllocated: %lu\n",
                    HkTlm.Eng.MachinesAllocated);
            printf("Eng.MachinesDeallocated: %lu\n",
                    HkTlm.Eng.MachinesDeallocated);
            printf("Eng.are_any_partners_frozen: %u\n",
                    HkTlm.Eng.are_any_partners_frozen);
            printf("Eng.how_many_senders: %lu\n",
                    HkTlm.Eng.how_many_senders);
            printf("Eng.how_many_receivers: %lu\n",
                    HkTlm.Eng.how_many_receivers);
            printf("Eng.how_many_frozen: %lu\n",
                    HkTlm.Eng.how_many_frozen);
            printf("Eng.how_many_suspended: %lu\n",
                    HkTlm.Eng.how_many_suspended);
            printf("Eng.total_files_sent: %lu\n",
                    HkTlm.Eng.total_files_sent);
            printf("Eng.total_files_received: %lu\n",
                    HkTlm.Eng.total_files_received);
            printf("Eng.total_unsuccessful_senders: %lu\n",
                    HkTlm.Eng.total_unsuccessful_senders);
            printf("Eng.total_unsuccessful_receivers: %lu\n",
                    HkTlm.Eng.total_unsuccessful_receivers);

            printf("Up.MetaCount: %lu\n", HkTlm.Up.MetaCount);
            printf("Up.UplinkActiveQFileCnt: %lu\n",
                    HkTlm.Up.UplinkActiveQFileCnt);
            printf("Up.SuccessCounter: %lu\n", HkTlm.Up.SuccessCounter);
            printf("Up.FailedCounter: %lu\n", HkTlm.Up.FailedCounter);
            printf("Up.LastFileUplinked: %s\n", HkTlm.Up.LastFileUplinked);

            for (i = 0; i < CF_MAX_PLAYBACK_CHANNELS; i++)
            {
                printf("Chan[%d].PDUsSent: %lu\n",
                        i, HkTlm.Chan[i].PDUsSent);
                printf("Chan[%d].FilesSent: %lu\n",
                        i, HkTlm.Chan[i].FilesSent);
                printf("Chan[%d].SuccessCounter: %lu\n",
                        i, HkTlm.Chan[i].SuccessCounter);
                printf("Chan[%d].FailedCounter: %lu\n",
                        i, HkTlm.Chan[i].FailedCounter);
                printf("Chan[%d].PendingQFileCnt: %lu\n",
                        i, HkTlm.Chan[i].PendingQFileCnt);
                printf("Chan[%d].ActiveQFileCnt: %lu\n",
                        i, HkTlm.Chan[i].ActiveQFileCnt);
                printf("Chan[%d].HistoryQFileCnt: %lu\n",
                        i, HkTlm.Chan[i].HistoryQFileCnt);
                printf("Chan[%d].Flags: %lu\n", i, HkTlm.Chan[i].Flags);
                printf("Chan[%d].RedLightCntr: %lu\n",
                        i, HkTlm.Chan[i].RedLightCntr);
                printf("Chan[%d].GreenLightCntr: %lu\n",
                        i, HkTlm.Chan[i].GreenLightCntr);
                printf("Chan[%d].PollDirsChecked: %lu\n",
                        i, HkTlm.Chan[i].PollDirsChecked);
                printf("Chan[%d].PendingQChecked: %lu\n",
                        i, HkTlm.Chan[i].PendingQChecked);
                printf("Chan[%d].SemValue: %lu\n",
                        i, HkTlm.Chan[i].SemValue);
            }

            break;
        }
        default:
        {
            printf("Sent MID(0x%04X)\n", MsgId);
            break;
        }
    }

    return CFE_SUCCESS;
}


/**
 * Test CF_AppPipe, HousekeepingCmd
 */
void Test_CF_AppPipe_HousekeepingCmd(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    CF_NoArgsCmd_t  InPDUMsg;
    char  FullInFileName[OS_MAX_PATH_LEN];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_SEND_HK_MID, sizeof(CmdMsg), TRUE);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    CFE_SB_InitMsg((void*)&InPDUMsg, CF_CMD_MID,
                   sizeof(InPDUMsg), TRUE);

    /* Used to verify HK was transmitted correctly. */
    SendHkHook_MsgId = 0;
    Ut_CFE_SB_SetFunctionHook(UT_CFE_SB_SENDMSG_INDEX,
                    (void *)&Test_CF_AppPipe_HousekeepingCmd_SendMsgHook);

    /* Execute the function being tested */
#if 0
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&InPDUMsg;
    CF_TstUtil_CreateOneUpHistoryQueueEntry();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);
#endif

    sprintf(FullInFileName, "%s%s", TestInDir, TestInFile1);

    /* Verify results */
#if 0
    UtAssert_True(CF_AppData.Hk.ErrCounter == 0,
                  "CF_AppPipe, HousekeepingCmd: No ErrCount");

    UtAssert_True(SendHkHook_MsgId == CF_HK_TLM_MID,
                  "CF_AppPipe, HousekeepingCmd: Sent HK Telemetry");

    UtAssert_True(CF_AppData.Hk.App.QNodesAllocated == 1,
                  "CF_AppPipe, HousekeepingCmd: Hk.App.QNodesAllocated");

    UtAssert_True(CF_AppData.Hk.App.TotalSuccessTrans == 1,
                "CF_AppPipe, HousekeepingCmd: Hk.App.TotalSuccessTrans");

    UtAssert_True(CF_AppData.Hk.Up.UplinkActiveQFileCnt == 0,
              "CF_AppPipe, HousekeepingCmd: Hk.Up.UplinkActiveQFileCnt");

    UtAssert_True(strcmp(FullInFileName, CF_AppData.Hk.Up.LastFileUplinked)
                  == 0,
                  "CF_AppPipe, HousekeepingCmd: Hk.Up.LastFileUplinked");
#endif

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, FreezeCmd
 */
void Test_CF_AppPipe_FreezeCmd(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_FREEZE_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "Freeze command received.");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, FreezeCmd");

    UtAssert_True(CF_AppData.Hk.Eng.Flags == 1,
                  "CF_AppPipe, FreezeCmd: Flags set");

    UtAssert_EventSent(CF_FREEZE_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, FreezeCmd: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, FreezeCmdInvLen
 */
void Test_CF_AppPipe_FreezeCmdInvLen(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_FREEZE_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d",
            CF_CMD_MID, CF_FREEZE_CC, sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, FreezeCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, FreezeCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, ThawCmd
 */
void Test_CF_AppPipe_ThawCmd(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_THAW_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.Hk.Eng.Flags = 1;
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "Thaw command received.");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, ThawCmd");

    UtAssert_True(CF_AppData.Hk.Eng.Flags == 0,
                  "CF_AppPipe, ThawCmd: Flags cleared");

    UtAssert_EventSent(CF_THAW_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, ThawCmd: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, ThawCmdInvLen
 */
void Test_CF_AppPipe_ThawCmdInvLen(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_THAW_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d",
            CF_CMD_MID, CF_THAW_CC, sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, ThawCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, ThawCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SuspendCmdTransId
 */
void Test_CF_AppPipe_SuspendCmdTransId(void)
{
    uint32                SuspendedQEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_CARSCmd_t          SuspendCmdMsg;
    TRANSACTION           trans;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  expEventInd[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbActiveQueueEntry(&PbFileCmdMsg);

    strcpy(SuspendCmdMsg.Trans, CF_AppData.Tbl->FlightEntityId);
    strcat(SuspendCmdMsg.Trans, "_1");
    CF_Test_PrintCmdMsg((void*)&SuspendCmdMsg, sizeof(SuspendCmdMsg));

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    SuspendedQEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    cfdp_trans_from_string(SuspendCmdMsg.Trans, &trans);
    strcpy(FullSrcFileName, PbFileCmdMsg.SrcFilename);
    sprintf(expEventInd, "Transaction Susupended %d.%d_%d,%s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName);

    sprintf(expEvent, "%s command received.%s", "Suspend",
            SuspendCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SuspendCmdTransId");

    UtAssert_True(SuspendedQEntryCnt == 1,
                  "CF_AppPipe, SuspendCmdTransId: Suspended QEntryCnt");

    UtAssert_EventSent(CF_IND_XACT_SUS_EID, CFE_EVS_INFORMATION, expEventInd,
                  "CF_AppPipe, SuspendCmdTransId: Ind Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SuspendCmdTransId: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SuspendCmdTransIdInvLen
 */
void Test_CF_AppPipe_SuspendCmdTransIdInvLen(void)
{
    CF_CARSCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_SUSPEND_CC);

    /* Execute the function being tested */
    CF_AppInit();

    strcpy(CmdMsg.Trans, CF_AppData.Tbl->FlightEntityId);
    strcat(CmdMsg.Trans, "_1");
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d",
            CF_CMD_MID, CF_SUSPEND_CC, sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SuspendCmdTransIdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SuspendCmdTransIdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SuspendCmdFilename
 */
void Test_CF_AppPipe_SuspendCmdFilename(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_CARSCmd_t          SuspendCmdMsg;
    TRANSACTION           trans;
    char  FullTransString[OS_MAX_PATH_LEN];
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  expEventInd[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    strcpy(SuspendCmdMsg.Trans, TestPbDir);
    strcat(SuspendCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&SuspendCmdMsg, sizeof(SuspendCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbActiveQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
         CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    sprintf(FullTransString, "%s%s", CF_AppData.Tbl->FlightEntityId, "_1");
    cfdp_trans_from_string(FullTransString, &trans);
    strcpy(FullSrcFileName, PbFileCmdMsg.SrcFilename);
    sprintf(expEventInd, "Transaction Susupended %d.%d_%d,%s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName);

    sprintf(expEvent, "%s command received.%s", "Suspend",
            SuspendCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SuspendCmdFilename");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, SuspendCmdFilename: QEntryCnt");

    UtAssert_EventSent(CF_IND_XACT_SUS_EID, CFE_EVS_INFORMATION, expEventInd,
                  "CF_AppPipe, SuspendCmdFilename: Ind Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SuspendCmdFilename: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SuspendCmdInvFilename
 */
void Test_CF_AppPipe_SuspendCmdInvFilename(void)
{
    CF_CARSCmd_t          SuspendCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    strcpy(SuspendCmdMsg.Trans, TestPbDir);
    strcat(SuspendCmdMsg.Trans, " file.txt");

    CF_Test_PrintCmdMsg((void*)&SuspendCmdMsg, sizeof(SuspendCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Filename in %s must be terminated and have no spaces",
            "Suspend Cmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SuspendCmdInvFilename");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SuspendCmdInvFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SuspendCmdUntermTrans
 */
void Test_CF_AppPipe_SuspendCmdUntermTrans(void)
{
    CF_CARSCmd_t          SuspendCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    CFE_PSP_MemSet(SuspendCmdMsg.Trans, 0xFF, OS_MAX_PATH_LEN);

    CF_Test_PrintCmdMsg((void*)&SuspendCmdMsg, sizeof(SuspendCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Unterminated string found in %s", "Suspend Cmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SuspendCmdUntermTrans");

    UtAssert_EventSent(CF_NO_TERM_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SuspendCmdUntermTrans: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SuspendCmdAll
 */
void Test_CF_AppPipe_SuspendCmdAll(void)
{
    uint32                SuspendedQEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg1;
    CF_PlaybackFileCmd_t  PbFileCmdMsg2;
    CF_CARSCmd_t          SuspendCmdMsg;
    TRANSACTION           trans;
    char  FullTransString1[OS_MAX_PATH_LEN];
    char  FullTransString2[OS_MAX_PATH_LEN];
    char  FullSrcFileName1[OS_MAX_PATH_LEN];
    char  FullSrcFileName2[OS_MAX_PATH_LEN];
    char  expEventTr1[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventTr2[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    strcpy(SuspendCmdMsg.Trans, "All");

    CF_Test_PrintCmdMsg((void*)&SuspendCmdMsg, sizeof(SuspendCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateTwoPbActiveQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);
    CF_ShowQs();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    SuspendedQEntryCnt =
         CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    sprintf(FullTransString1, "%s%s", CF_AppData.Tbl->FlightEntityId, "_1");
    cfdp_trans_from_string(FullTransString1, &trans);
    strcpy(FullSrcFileName1, PbFileCmdMsg1.SrcFilename);
    sprintf(expEventTr1, "Transaction Susupended %d.%d_%d,%s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName1);

    sprintf(FullTransString2, "%s%s", CF_AppData.Tbl->FlightEntityId, "_2");
    cfdp_trans_from_string(FullTransString2, &trans);
    strcpy(FullSrcFileName2, PbFileCmdMsg2.SrcFilename);
    sprintf(expEventTr2, "Transaction Susupended %d.%d_%d,%s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName2);

    sprintf(expEvent, "%s command received.%s", "Suspend", "All");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SuspendCmdAll");

    UtAssert_True(SuspendedQEntryCnt == 2,
                  "CF_AppPipe, SuspendCmdAll: Suspended QEntryCnt");

    UtAssert_EventSent(CF_IND_XACT_SUS_EID, CFE_EVS_INFORMATION, expEventTr1,
                       "CF_AppPipe, SuspendCmdAll: Trans #1 Event Sent");

    UtAssert_EventSent(CF_IND_XACT_SUS_EID, CFE_EVS_INFORMATION, expEventTr2,
                       "CF_AppPipe, SuspendCmdAll: Trans #2 Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, SuspendCmdAll: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, ResumeCmdNoTransId
 */
void Test_CF_AppPipe_ResumeCmdNoTransId(void)
{
    CF_CARSCmd_t  CmdMsg;
    char  FullTransString[OS_MAX_PATH_LEN];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg, (uint16)CF_RESUME_CC);
    strcpy(CmdMsg.Trans, "0.24_56");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    strcpy(FullTransString, CmdMsg.Trans);
    sprintf(expEvent, "cfdp_engine: ignoring User-Request that references "
            "unknown transaction (%s).", FullTransString);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, ResumeCmdNoTransId");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, ResumeCmdNoTransId: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, ResumeCmdPbFilename
 */
void Test_CF_AppPipe_ResumeCmdPbFilename(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_CARSCmd_t          SuspendCmdMsg;
    CF_CARSCmd_t          ResumeCmdMsg;
    char  expEventSuspend[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* Build Suspend Command Msg */
    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    strcpy(SuspendCmdMsg.Trans, TestPbDir);
    strcat(SuspendCmdMsg.Trans, TestPbFile1);

    /* Build Resume Command Msg */
    CFE_SB_InitMsg((void*)&ResumeCmdMsg, CF_CMD_MID,
                   sizeof(ResumeCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&ResumeCmdMsg,
                      (uint16)CF_RESUME_CC);
    strcpy(ResumeCmdMsg.Trans, TestPbDir);
    strcat(ResumeCmdMsg.Trans, TestPbFile1);
    CF_Test_PrintCmdMsg((void*)&ResumeCmdMsg, sizeof(ResumeCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbActiveQueueEntry(&PbFileCmdMsg);

    /* Suspend */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    /* Resume */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&ResumeCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    sprintf(expEventSuspend, "%s command received.%s",
            "Suspend", SuspendCmdMsg.Trans);

    sprintf(expEvent, "%s command received.%s",
            "Resume", ResumeCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, ResumeCmdPbFilename");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, ResumeCmdPbFilename: QEntryCnt");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEventSuspend,
                  "CF_AppPipe, ResumeCmdPbFilename: Suspend Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, ResumeCmdPbFilename: Resume Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, ResumeCmdUpTransId
 */
void Test_CF_AppPipe_ResumeCmdUpTransId(void)
{
    uint32              QEntryCnt;
    CF_Test_InPDUMsg_t  InPDUMsg;
    CF_CARSCmd_t        SuspendCmdMsg;
    CF_CARSCmd_t        ResumeCmdMsg;
    char  expEventSuspend[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventResume[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* Build SuspendCmdMsg */
    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    sprintf(SuspendCmdMsg.Trans, "%s%s", TestInSrcEntityId1, "_500");

    /* Build ResumeCmdMsg */
    CFE_SB_InitMsg((void*)&ResumeCmdMsg, CF_CMD_MID,
                   sizeof(ResumeCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&ResumeCmdMsg, (uint16)CF_RESUME_CC);
    strcpy(ResumeCmdMsg.Trans, SuspendCmdMsg.Trans);
    CF_Test_PrintCmdMsg((void*)&ResumeCmdMsg, sizeof(ResumeCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOneUpActiveQueueEntry(&InPDUMsg);
    CF_ShowQs();
    QEntryCnt = CF_AppData.UpQ[CF_UP_ACTIVEQ].EntryCnt;

    /* Suspend */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    /* Resume */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&ResumeCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventSuspend, "%s command received.%s", "Suspend",
            SuspendCmdMsg.Trans);

    sprintf(expEventResume, "%s command received.%s", "Resume",
            ResumeCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, ResumeCmdUpTransId");

    UtAssert_True((CF_AppData.Hk.App.PDUsReceived == 1) &&
                  (CF_AppData.Hk.App.PDUsRejected == 0),
                  "CF_AppPipe, ResumeCmdUpTransId: PDUsReceived cnt");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, ResumeCmdUpTransId: QEntryCnt");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEventSuspend,
                       "CF_AppPipe, ResumeCmdUpTransId: Suspend Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEventResume,
                       "CF_AppPipe, ResumeCmdUpTransId: Resume Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, ResumeCmdAll
 */
void Test_CF_AppPipe_ResumeCmdAll(void)
{
    CF_Test_InPDUMsg_t    InPDUMsg;
    CF_PlaybackFileCmd_t  PbFileCmdMsg1;
    CF_PlaybackFileCmd_t  PbFileCmdMsg2;
    CF_CARSCmd_t          SuspendCmdMsg;
    CF_CARSCmd_t          ResumeCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SuspendCmdMsg, CF_CMD_MID,
                   sizeof(SuspendCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SuspendCmdMsg,
                      (uint16)CF_SUSPEND_CC);
    strcpy(SuspendCmdMsg.Trans, "All");

    CFE_SB_InitMsg((void*)&ResumeCmdMsg, CF_CMD_MID,
                   sizeof(ResumeCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&ResumeCmdMsg, (uint16)CF_RESUME_CC);
    strcpy(ResumeCmdMsg.Trans, "All");
    CF_Test_PrintCmdMsg((void*)&ResumeCmdMsg, sizeof(ResumeCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    /* create two playback chan 0, active queue entry */
    CF_TstUtil_CreateTwoPbActiveQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);

    /* create one uplink active queue entry */
    CF_TstUtil_CreateOneUpActiveQueueEntry(&InPDUMsg);

    CF_ShowQs();

    /* Suspend */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SuspendCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    /* Resume */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&ResumeCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    /* Verify results */

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdSaveIncompleteFiles
 */
void Test_CF_AppPipe_SetMibCmdSaveIncompleteFiles(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "save_incomplete_files");
    strcpy(CmdMsg.Value, "yes");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdSaveIncompleteFiles");

    UtAssert_True(strcmp(CF_AppData.Tbl->SaveIncompleteFiles, CmdMsg.Value)
           == 0,
           "CF_AppPipe, SetMibCmdSaveIncompleteFiles: Config Tbl updated");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SetMibCmdSaveIncompleteFiles: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdSaveIncompleteFilesInvLen
 */
void Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesInvLen(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "save_incomplete_files");
    strcpy(CmdMsg.Value, "yes");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_SET_MIB_PARAM_CC,
            sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdSaveIncompleteFilesInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, SetMibCmdSaveIncompleteFilesInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdSaveIncompleteFilesUntermParam
 */
void Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesUntermParam(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    CFE_PSP_MemSet(CmdMsg.Param, 0xFF, CF_MAX_CFG_PARAM_CHARS);
    strcpy(CmdMsg.Value, "yes");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s",
            "Unterminated string found in SetMib Cmd, Param parameter");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdSaveIncompleteFilesUntermParam");

    UtAssert_EventSent(CF_NO_TERM_ERR_EID, CFE_EVS_ERROR, expEvent,
        "CF_AppPipe, SetMibCmdSaveIncompleteFilesUntermParam: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdSaveIncompleteFilesUntermValue
 */
void Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesUntermValue(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "save_incomplete_files");
    CFE_PSP_MemSet(CmdMsg.Value, 0xFF, CF_MAX_CFG_VALUE_CHARS);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s",
            "Unterminated string found in SetMib Cmd, Value parameter");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdSaveIncompleteFilesUntermValue");

    UtAssert_EventSent(CF_NO_TERM_ERR_EID, CFE_EVS_ERROR, expEvent,
       "CF_AppPipe, SetMibCmdSaveIncompleteFilesUntermValue: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdAckLimit
 */
void Test_CF_AppPipe_SetMibCmdAckLimit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "ack_limit");
    strcpy(CmdMsg.Value, "4");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: 'ack_limit' set to '%lu'.",
            (uint32)atoi(CmdMsg.Value));
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdAckLimit");

    UtAssert_True(strcmp(CF_AppData.Tbl->AckLimit, CmdMsg.Value) == 0,
                  "CF_AppPipe, SetMibCmdAckLimit: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                       "CF_AppPipe, SetMibCmdAckLimit: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, SetMibCmdAckLimit: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdAckLimitNonDigit
 */
void Test_CF_AppPipe_SetMibCmdAckLimitNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "ack_limit");
    strcpy(CmdMsg.Value, "four");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: can't set 'ack_limit'; "
            "value (%s) must be numeric.", CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdAckLimitNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
                  "CF_AppPipe, SetMibCmdAckLimitNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdAckTimeout
 */
void Test_CF_AppPipe_SetMibCmdAckTimeout(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "ACK_TIMEOUT");
    strcpy(CmdMsg.Value, "25");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: 'ack_timeout' set to '%lu'.",
            (uint32)atoi(CmdMsg.Value));
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdAckTimeout");

    UtAssert_True(strcmp(CF_AppData.Tbl->AckTimeout, CmdMsg.Value) == 0,
                  "CF_AppPipe, SetMibCmdAckTimeout: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                       "CF_AppPipe, SetMibCmdAckTimeout: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SetMibCmdAckTimeout: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdAckTimeoutNonDigit
 */
void Test_CF_AppPipe_SetMibCmdAckTimeoutNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "ACK_TIMEOUT");
    strcpy(CmdMsg.Value, "ab");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: can't set 'ack_timeout'; "
            "value (%s) must be numeric.", CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdAckTimeoutNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
             "CF_AppPipe, SetMibCmdAckTimeoutNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdInactTimeout
 */
void Test_CF_AppPipe_SetMibCmdInactTimeout(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "INACTIVITY_TIMEOUT");
    strcpy(CmdMsg.Value, "40");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: 'inactivity_timeout' set to '%lu'.",
            (uint32)atoi(CmdMsg.Value));
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdInactTimeout");

    UtAssert_True(strcmp(CF_AppData.Tbl->InactivityTimeout, CmdMsg.Value)
                  == 0,
                  "CF_AppPipe, SetMibCmdInactTimeout: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                       "CF_AppPipe, SetMibCmdInactTimeout: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SetMibCmdInactTimeout: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdInactTimeoutNonDigit
 */
void Test_CF_AppPipe_SetMibCmdInactTimeoutNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "INACTIVITY_TIMEOUT");
    strcpy(CmdMsg.Value, "cde");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: can't set 'inactivity_timeout'; "
            "value (%s) must be numeric.", CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdInactTimeoutNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
              "CF_AppPipe, SetMibCmdInactTimeoutNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdNakLimit
 */
void Test_CF_AppPipe_SetMibCmdNakLimit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "nak_limit");
    strcpy(CmdMsg.Value, "5");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: 'nak_limit' set to '%lu'.",
            (uint32)atoi(CmdMsg.Value));
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdNakLimit");

    UtAssert_True(strcmp(CF_AppData.Tbl->NakLimit, CmdMsg.Value) == 0,
                  "CF_AppPipe, SetMibCmdNakLimit: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                       "CF_AppPipe, SetMibCmdNakLimit: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, SetMibCmdNakLimit: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdNakLimitNonDigit
 */
void Test_CF_AppPipe_SetMibCmdNakLimitNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "nak_limit");
    strcpy(CmdMsg.Value, "FIVE");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: can't set 'nak_limit'; "
            "value (%s) must be numeric.", CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdNakLimitNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
                 "CF_AppPipe, SetMibCmdNakLimitNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdNakTimeout
 */
void Test_CF_AppPipe_SetMibCmdNakTimeout(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "NAK_TIMEOUT");
    strcpy(CmdMsg.Value, "2");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: 'nak_timeout' set to '%lu'.",
            (uint32)atoi(CmdMsg.Value));
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdNakTimeout");

    UtAssert_True(strcmp(CF_AppData.Tbl->NakTimeout, CmdMsg.Value) == 0,
                  "CF_AppPipe, SetMibCmdNakTimeout: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                       "CF_AppPipe, SetMibCmdNakTimeout: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SetMibCmdNakTimeout: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdNakTimeoutNonDigit
 */
void Test_CF_AppPipe_SetMibCmdNakTimeoutNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "NAK_TIMEOUT");
    strcpy(CmdMsg.Value, "hg");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: can't set 'nak_timeout'; "
            "value (%s) must be numeric.", CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdNakTimeoutNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
               "CF_AppPipe, SetMibCmdNakTimeoutNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdFileChunkSize
 */
void Test_CF_AppPipe_SetMibCmdFileChunkSize(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "OUTGOING_FILE_CHUNK_SIZE");
    strcpy(CmdMsg.Value, "250");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: 'outgoing_file_chunk_size' "
            "set to '%lu'.", (uint32)atoi(CmdMsg.Value));
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdFileChunkSize");

    UtAssert_True(strcmp(CF_AppData.Tbl->OutgoingFileChunkSize, CmdMsg.Value)
                  == 0,
                  "CF_AppPipe, SetMibCmdFileChunkSize: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                      "CF_AppPipe, SetMibCmdFileChunkSize: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, SetMibCmdFileChunkSize: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdFileChunkSizeNonDigit
 */
void Test_CF_AppPipe_SetMibCmdFileChunkSizeNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "OUTGOING_FILE_CHUNK_SIZE");
    strcpy(CmdMsg.Value, "cde");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: can't set "
            "'outgoing_file_chunk_size'; value (%s) must be numeric.",
            CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdFileChunkSizeNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
            "CF_AppPipe, SetMibCmdFileChunkSizeNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdFileChunkOverLimit
 */
void Test_CF_AppPipe_SetMibCmdFileChunkOverLimit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "OUTGOING_FILE_CHUNK_SIZE");
    strcpy(CmdMsg.Value, "10000");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cannot set OUTGOING_FILE_CHUNK_SIZE(%d) > "
            "CF_MAX_OUTGOING_CHUNK_SIZE(%d)",
            atoi(CmdMsg.Value), CF_MAX_OUTGOING_CHUNK_SIZE);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdFileChunkOverLimit");

    UtAssert_EventSent(CF_SET_MIB_CMD_ERR1_EID, CFE_EVS_ERROR, expEvent,
            "CF_AppPipe, SetMibCmdFileChunkOverLimit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdMyId
 */
void Test_CF_AppPipe_SetMibCmdMyId(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "MY_ID");
    strcpy(CmdMsg.Value, "25.29");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEventCfdp, "cfdp_engine: entity-id set to '%s'.",
            CmdMsg.Value);
    sprintf(expEvent, "Set MIB command received.Param %s Value %s",
            CmdMsg.Param, CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetMibCmdMyId");

    UtAssert_True(strcmp(CF_AppData.Tbl->FlightEntityId, CmdMsg.Value)
                  == 0,
                  "CF_AppPipe, SetMibCmdMyId: Config Tbl updated");

    UtAssert_EventSent(CF_CFDP_ENGINE_INFO_EID, CFE_EVS_DEBUG, expEventCfdp,
                       "CF_AppPipe, SetMibCmdMyId: CFDP Event Sent");

    UtAssert_EventSent(CF_SET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, SetMibCmdMyId: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdMyIdInvalid
 */
void Test_CF_AppPipe_SetMibCmdMyIdInvalid(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "MY_ID");
    strcpy(CmdMsg.Value, "10000");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cannot set Flight Entity Id to %s, must be 2 byte, "
            "dotted decimal fmt", CmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdMyIdInvalid");

    UtAssert_EventSent(CF_SET_MIB_CMD_ERR2_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetMibCmdMyIdInvalid: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetMibCmdMyIdNonDigit
 */
void Test_CF_AppPipe_SetMibCmdMyIdNonDigit(void)
{
    CF_SetMibParam_t  CmdMsg;
    char  expEventCfdp[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  upperValue[CF_MAX_CFG_VALUE_CHARS];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(CmdMsg.Param, "MY_ID");
    strcpy(CmdMsg.Value, "ab.10");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    strcpy(upperValue, CF_Test_ToUpperCase(CmdMsg.Value));

    sprintf(expEventCfdp, "cfdp_engine: can't set entity-id to illegal "
            "value (%s).", upperValue);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetMibCmdMyIdNonDigit");

    UtAssert_EventSent(CF_CFDP_ENGINE_ERR_EID, CFE_EVS_ERROR, expEventCfdp,
                  "CF_AppPipe, SetMibCmdMyIdNonDigit: CFDP Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdSaveIncompleteFiles
 */
void Test_CF_AppPipe_GetMibCmdSaveIncompleteFiles(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "save_incomplete_files");
    strcpy(SetMibCmdMsg.Value, "YES");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "save_incomplete_files");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdSaveIncompleteFiles");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                 "CF_AppPipe, GetMibCmdSaveIncompleteFiles: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdSaveIncompleteFilesInvLen
 */
void Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesInvLen(void)
{
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "save_incomplete_files");

    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_GET_MIB_PARAM_CC,
            sizeof(GetMibCmdMsg), sizeof(GetMibCmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, GetMibCmdSaveIncompleteFilesInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, GetMibCmdSaveIncompleteFilesInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdSaveIncompleteFilesUntermParam
 */
void Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesUntermParam(void)
{
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    CFE_PSP_MemSet(GetMibCmdMsg.Param, 0xFF, CF_MAX_CFG_PARAM_CHARS);

    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Unterminated string found in %s",
            "GetMib Cmd, Param parameter");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                   "CF_AppPipe, GetMibCmdSaveIncompleteFilesUntermParam");

    UtAssert_EventSent(CF_NO_TERM_ERR_EID, CFE_EVS_ERROR, expEvent,
        "CF_AppPipe, GetMibCmdSaveIncompleteFilesUntermParam: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdSaveIncompleteFilesInvParam
 */
void Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesInvParam(void)
{
    CF_GetMibParam_t  GetMibCmdMsg;
    char  strUpperParam[CF_MAX_CFG_PARAM_CHARS];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "save_the_bay");

    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    strcpy(strUpperParam, CF_Test_ToUpperCase(GetMibCmdMsg.Param));

    sprintf(expEvent, "cfdp_engine: can't get value of unrecognized MIB "
            "parameter (%s).", strUpperParam);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, GetMibCmdSaveIncompleteFilesInvParam");

    UtAssert_EventSent(CF_CFDP_ENGINE_WARN_EID, CFE_EVS_INFORMATION,
          expEvent,
          "CF_AppPipe, GetMibCmdSaveIncompleteFilesInvParam: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdAckLimit
 */
void Test_CF_AppPipe_GetMibCmdAckLimit(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "ack_limit");
    strcpy(SetMibCmdMsg.Value, "3");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "ack_limit");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdAckLimit");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdAckLimit: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdAckTimeout
 */
void Test_CF_AppPipe_GetMibCmdAckTimeout(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "ack_timeout");
    strcpy(SetMibCmdMsg.Value, "25");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "ack_timeout");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdAckTimeout");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdAckTimeout: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdInactTimeout
 */
void Test_CF_AppPipe_GetMibCmdInactTimeout(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "INACTIVITY_TIMEOUT");
    strcpy(SetMibCmdMsg.Value, "40");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "INACTIVITY_TIMEOUT");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdInactTimeout");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdInactTimeout: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdNakLimit
 */
void Test_CF_AppPipe_GetMibCmdNakLimit(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "nak_limit");
    strcpy(SetMibCmdMsg.Value, "5");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "nak_limit");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdNakLimit");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdNakLimit: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdNakTimeout
 */
void Test_CF_AppPipe_GetMibCmdNakTimeout(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "NAK_TIMEOUT");
    strcpy(SetMibCmdMsg.Value, "2");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "NAK_TIMEOUT");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdNakTimeout");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdNakTimeout: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdFileChunkSize
 */
void Test_CF_AppPipe_GetMibCmdFileChunkSize(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "OUTGOING_FILE_CHUNK_SIZE");
    strcpy(SetMibCmdMsg.Value, "250");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "OUTGOING_FILE_CHUNK_SIZE");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdFileChunkSize");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdFileChunkSize: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, GetMibCmdMyId
 */
void Test_CF_AppPipe_GetMibCmdMyId(void)
{
    CF_SetMibParam_t  SetMibCmdMsg;
    CF_GetMibParam_t  GetMibCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SetMibCmdMsg, CF_CMD_MID,
                   sizeof(SetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SetMibCmdMsg,
                      (uint16)CF_SET_MIB_PARAM_CC);
    strcpy(SetMibCmdMsg.Param, "MY_ID");
    strcpy(SetMibCmdMsg.Value, "25.29");
    CF_Test_PrintCmdMsg((void*)&SetMibCmdMsg, sizeof(SetMibCmdMsg));

    CFE_SB_InitMsg((void*)&GetMibCmdMsg, CF_CMD_MID,
                   sizeof(GetMibCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&GetMibCmdMsg,
                      (uint16)CF_GET_MIB_PARAM_CC);
    strcpy(GetMibCmdMsg.Param, "MY_ID");
    CF_Test_PrintCmdMsg((void*)&GetMibCmdMsg, sizeof(GetMibCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&GetMibCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Get MIB command received.Param %s Value %s",
            GetMibCmdMsg.Param, SetMibCmdMsg.Value);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, GetMibCmdMyId");

    UtAssert_True(strcmp(CF_AppData.Tbl->FlightEntityId, SetMibCmdMsg.Value)
                  == 0,
                  "CF_AppPipe, GetMibCmdMyId: Config Tbl updated");

    UtAssert_EventSent(CF_GET_MIB_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, GetMibCmdMyId: Event Sent");

    CF_ResetEngine();
}


/**
 * Hook to support: Test CF_AppPipe, SendCfgParamsCmd
 */
int32 Test_CF_AppPipe_SendCfgParamsCmd_SendMsgHook(CFE_SB_Msg_t *MsgPtr)
{
    unsigned char       *pBuff = NULL;
    uint16              msgLen = 0;
    int                 i = 0;
    CFE_SB_MsgId_t      MsgId = 0;
    time_t              localTime;
    struct tm           *loc_time;
    CFE_TIME_SysTime_t  TimeFromMsg;
    CF_ConfigPacket_t   CfgParamsMsg;

    msgLen = CFE_SB_GetTotalMsgLength(MsgPtr);
    MsgId = CFE_SB_GetMsgId(MsgPtr);

    pBuff = (unsigned char *)MsgPtr;
    printf("###SendCfgParamsCmd_SendMsgHook(msgLen %u): ", msgLen);
    for (i = 0; i < msgLen; i++)
    {
        printf("0x%02x ", *pBuff);
        pBuff++;
    }
    printf("\n");

    TimeFromMsg = CFE_SB_GetMsgTime(MsgPtr);
    localTime = CF_Test_GetTimeFromMsg(TimeFromMsg);
    loc_time = localtime(&localTime);
    printf("TimeFromMessage: %s", asctime(loc_time));

    switch(MsgId)
    {
        case CF_CONFIG_TLM_MID:
        {
            SendCfgParamsHook_MsgId = CF_CONFIG_TLM_MID;
            printf("Sent CF_CONFIG_TLM_MID\n");
            memcpy((void *)&CfgParamsMsg, (void *)MsgPtr,
                   sizeof(CfgParamsMsg));

            printf("EngCycPerWakeup: %lu\n", CfgParamsMsg.EngCycPerWakeup);
            printf("AckLimit: %lu\n", CfgParamsMsg.AckLimit);
            printf("AckTimeout: %lu\n", CfgParamsMsg.AckTimeout);
            printf("NakLimit: %lu\n", CfgParamsMsg.NakLimit);
            printf("NakTimeout: %lu\n", CfgParamsMsg.NakTimeout);
            printf("InactTimeout: %lu\n", CfgParamsMsg.InactTimeout);
            printf("DefOutgoingChunkSize: %lu\n",
                    CfgParamsMsg.DefOutgoingChunkSize);
            printf("PipeDepth: %lu\n", CfgParamsMsg.PipeDepth);
            printf("MaxSimultaneousTrans: %lu\n",
                    CfgParamsMsg.MaxSimultaneousTrans);
            printf("IncomingPduBufSize: %lu\n",
                    CfgParamsMsg.IncomingPduBufSize);
            printf("OutgoingPduBufSize: %lu\n",
                    CfgParamsMsg.OutgoingPduBufSize);
            printf("NumInputChannels: %lu\n",
                    CfgParamsMsg.NumInputChannels);
            printf("MaxPlaybackChans: %lu\n", CfgParamsMsg.MaxPlaybackChans);
            printf("MaxPollingDirsPerChan: %lu\n",
                    CfgParamsMsg.MaxPollingDirsPerChan);
            printf("MemPoolBytes: %lu\n", CfgParamsMsg.MemPoolBytes);
            printf("DebugCompiledIn: %lu\n", CfgParamsMsg.DebugCompiledIn);
            printf("SaveIncompleteFiles: %s\n",
                    CfgParamsMsg.SaveIncompleteFiles);
            printf("PipeName: %s\n", CfgParamsMsg.PipeName);
            printf("TmpFilePrefix: %s\n", CfgParamsMsg.TmpFilePrefix);
            printf("CfgTblName: %s\n", CfgParamsMsg.CfgTblName);
            printf("CfgTbleFilename: %s\n", CfgParamsMsg.CfgTbleFilename);
            printf("DefQInfoFilename: %s\n", CfgParamsMsg.DefQInfoFilename);
            break;
        }
        default:
        {
            printf("Sent MID(0x%04X)\n", MsgId);
            break;
        }
    }

    return CFE_SUCCESS;
}


/**
 * Test CF_AppPipe, SendCfgParamsCmd
 */
void Test_CF_AppPipe_SendCfgParamsCmd(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SEND_CFG_PARAMS_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    SendCfgParamsHook_MsgId = 0;
    Ut_CFE_SB_SetFunctionHook(UT_CFE_SB_SENDMSG_INDEX,
                    (void *)&Test_CF_AppPipe_SendCfgParamsCmd_SendMsgHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "CF:Sending Configuration Pkt");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SendCfgParamsCmd");

    UtAssert_True(SendCfgParamsHook_MsgId == CF_CONFIG_TLM_MID,
                  "CF_AppPipe, SendCfgParamsCmd: Sent CF_CONFIG_TLM_MID");

    UtAssert_EventSent(CF_SND_CFG_CMD_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, SendCfgParamsCmd: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendCfgParamsCmdInvLen
 */
void Test_CF_AppPipe_SendCfgParamsCmdInvLen(void)
{
    CF_NoArgsCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SEND_CFG_PARAMS_CC);
    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_SEND_CFG_PARAMS_CC,
            sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SendCfgParamsCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SendCfgParamsCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdCreatErr
 */
void Test_CF_AppPipe_WriteQueueCmdCreatErr(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX,
                               OS_FS_ERR_INVALID_POINTER, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "WriteQueueCmd:Error creating file %s, stat=0x%x",
            CF_DEFAULT_QUEUE_INFO_FILENAME, OS_FS_ERR_INVALID_POINTER);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdCreatErr");

    UtAssert_EventSent(CF_SND_QUE_ERR1_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdCreatErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdInvLen
 */
void Test_CF_AppPipe_WriteQueueCmdInvLen(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_WRITE_QUEUE_INFO_CC,
            sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdUpQValueErr
 */
void Test_CF_AppPipe_WriteQueueCmdUpQValueErr(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_UPLINK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PENDINGQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "Invalid Queue Param %u in WriteQueueInfoCmd 1=active,2=history",
            CmdMsg.Queue);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdUpQValueErr");

    UtAssert_EventSent(CF_WR_CMD_ERR1_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdUpQValueErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdUpDefFilename
 */
void Test_CF_AppPipe_WriteQueueCmdUpDefFilename(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_UPLINK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_ACTIVEQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s written:Size=%d,Entries=%d",
            CF_DEFAULT_QUEUE_INFO_FILENAME, sizeof(CFE_FS_Header_t), 0);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteQueueCmdUpDefFilename");

    UtAssert_EventSent(CF_SND_Q_INFO_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, WriteQueueCmdUpDefFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdUpCustomFilename
 */
void Test_CF_AppPipe_WriteQueueCmdUpCustomFilename(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_UPLINK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_HISTORYQ;
    strcpy(CmdMsg.Filename, TestInDir);
    strcat(CmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s written:Size=%d,Entries=%d",
            CmdMsg.Filename, sizeof(CFE_FS_Header_t), 0);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteQueueCmdUpCustomFilename");

    UtAssert_EventSent(CF_SND_Q_INFO_EID, CFE_EVS_DEBUG, expEvent,
                 "CF_AppPipe, WriteQueueCmdUpCustomFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdOutQValueErr
 */
void Test_CF_AppPipe_WriteQueueCmdOutQValueErr(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = 4;  /* 0=pending,1=active,2=history */
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:Write Queue Info Error, Queue Value %u > max %u",
            CmdMsg.Queue, 2);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdOutQValueErr");

    UtAssert_EventSent(CF_WR_CMD_ERR2_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdOutQValueErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdOutQTypeErr
 */
void Test_CF_AppPipe_WriteQueueCmdOutQTypeErr(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = 3;  /* (up=1/down=2) */
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:Write Queue Info Error, Type Num %u not valid.",
            CmdMsg.Type);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdOutQTypeErr");

    UtAssert_EventSent(CF_WR_CMD_ERR3_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdOutQTypeErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdOutChanErr
 */
void Test_CF_AppPipe_WriteQueueCmdOutChanErr(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 16;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:Write Queue Info Error, Channel Value %u > max %u",
            CmdMsg.Chan, CF_MAX_PLAYBACK_CHANNELS - 1);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdOutChanErr");

    UtAssert_EventSent(CF_WR_CMD_ERR4_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdOutChanErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdWriteHdrErr
 */
void Test_CF_AppPipe_WriteQueueCmdWriteHdrErr(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, TestPbDir);
    strcat(CmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX, 0, 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "File write,byte cnt err,file %s,request=%d,actual=%d",
            CmdMsg.Filename, sizeof(CFE_FS_Header_t), 0);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdWriteHdrErr");

    UtAssert_EventSent(CF_FILEWRITE_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteQueueCmdWriteHdrErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdEntryWriteErr
 */
void Test_CF_AppPipe_WriteQueueCmdEntryWriteErr(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_WriteQueueCmd_t    WrQCmdMsg;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventWrQ[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrQCmdMsg, CF_CMD_MID, sizeof(WrQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrQCmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    WrQCmdMsg.Type = CF_PLAYBACK;
    WrQCmdMsg.Chan = 0;
    WrQCmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(WrQCmdMsg.Filename, TestPbDir);
    strcat(WrQCmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&WrQCmdMsg, sizeof(WrQCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t) - 4, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[WrQCmdMsg.Chan].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(FullSrcFileName, "%s%s", TestPbDir, TestPbFile1);
    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,"
            "Peer %s,File %s", PbFileCmdMsg.Class, PbFileCmdMsg.Channel,
            PbFileCmdMsg.Priority, PbFileCmdMsg.Preserve,
            PbFileCmdMsg.PeerEntityId, FullSrcFileName);
    sprintf(expEventWrQ,
            "File write,byte cnt err,file %s,request=%d,actual=%d",
            WrQCmdMsg.Filename, sizeof(CF_QueueInfoFileEntry_t),
            sizeof(CF_QueueInfoFileEntry_t) - 4);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdEntryWriteErr");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, WriteQueueCmdEntryWriteErr: QEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
        "CF_AppPipe, WriteQueueCmdEntryWriteErr: Event Sent");

    UtAssert_EventSent(CF_FILEWRITE_ERR_EID, CFE_EVS_ERROR, expEventWrQ,
        "CF_AppPipe, WriteQueueCmdEntryWriteErr: Write Queue Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdInvFilenameErr
 */
void Test_CF_AppPipe_WriteQueueCmdInvFilenameErr(void)
{
    CF_WriteQueueCmd_t    WrQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrQCmdMsg, CF_CMD_MID, sizeof(WrQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrQCmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    WrQCmdMsg.Type = CF_PLAYBACK;
    WrQCmdMsg.Chan = 0;
    WrQCmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(WrQCmdMsg.Filename, TestPbDir);
    strcat(WrQCmdMsg.Filename, "qinf ofile1.dat");

    CF_Test_PrintCmdMsg((void*)&WrQCmdMsg, sizeof(WrQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "Filename in %s must be terminated and have no spaces",
            "WriteQueueCmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteQueueCmdInvFilenameErr");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, WriteQueueCmdInvFilenameErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdOutDefFilename
 */
void Test_CF_AppPipe_WriteQueueCmdOutDefFilename(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s written:Size=%d,Entries=%d",
            CF_DEFAULT_QUEUE_INFO_FILENAME, sizeof(CFE_FS_Header_t), 0);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteQueueCmdOutDefFilename");

    UtAssert_EventSent(CF_SND_Q_INFO_EID, CFE_EVS_DEBUG, expEvent,
                  "CF_AppPipe, WriteQueueCmdOutDefFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdOutCustomFilename
 */
void Test_CF_AppPipe_WriteQueueCmdOutCustomFilename(void)
{
    CF_WriteQueueCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    CmdMsg.Type = CF_PLAYBACK;
    CmdMsg.Chan = 0;
    CmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(CmdMsg.Filename, TestPbDir);
    strcat(CmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s written:Size=%d,Entries=%d",
            CmdMsg.Filename, sizeof(CFE_FS_Header_t), 0);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteQueueCmdOutCustomFilename");

    UtAssert_EventSent(CF_SND_Q_INFO_EID, CFE_EVS_DEBUG, expEvent,
                  "CF_AppPipe, WriteQueueCmdOutCustomFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteQueueCmdOneEntry
 */
void Test_CF_AppPipe_WriteQueueCmdOneEntry(void)
{
    uint32                QEntryCnt;
    int                   TotalEntrySize;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_WriteQueueCmd_t    WrQCmdMsg;
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventWrQ[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrQCmdMsg, CF_CMD_MID, sizeof(WrQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrQCmdMsg,
                      (uint16)CF_WRITE_QUEUE_INFO_CC);
    WrQCmdMsg.Type = CF_PLAYBACK;
    WrQCmdMsg.Chan = 0;
    WrQCmdMsg.Queue = CF_PB_PENDINGQ;
    strcpy(WrQCmdMsg.Filename, TestPbDir);
    strcat(WrQCmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&WrQCmdMsg, sizeof(WrQCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[WrQCmdMsg.Chan].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d"
            ",Peer %s,File %s", PbFileCmdMsg.Class, PbFileCmdMsg.Channel,
            PbFileCmdMsg.Priority, PbFileCmdMsg.Preserve, TestPbPeerEntityId,
            PbFileCmdMsg.SrcFilename);

    TotalEntrySize = sizeof(CFE_FS_Header_t) +
                     (1 * sizeof(CF_QueueInfoFileEntry_t));
    sprintf(expEventWrQ, "%s written:Size=%d,Entries=%d",
            WrQCmdMsg.Filename, TotalEntrySize, 1);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteQueueCmdOneEntry");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, WriteQueueCmdOneEntry: QEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
             "CF_AppPipe, WriteQueueCmdOneEntry: PlaybackFile Event Sent");

    UtAssert_EventSent(CF_SND_Q_INFO_EID, CFE_EVS_DEBUG, expEventWrQ,
             "CF_AppPipe, WriteQueueCmdOneEntry: Write Queue Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteActiveTransCmdInvLen
 */
void Test_CF_AppPipe_WriteActiveTransCmdInvLen(void)
{
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = 0;  /* all=0/up=1/down=2 */
    strcpy(WrActTrCmdMsg.Filename, "");

    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_WR_ACTIVE_TRANS_CC,
            sizeof(WrActTrCmdMsg), sizeof(WrActTrCmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteActiveTransCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, WriteActiveTransCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteActiveTransCmdInvFilename
 */
void Test_CF_AppPipe_WriteActiveTransCmdInvFilename(void)
{
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = CF_PLAYBACK;
    strcpy(WrActTrCmdMsg.Filename, TestQInfoDir);
    strcat(WrActTrCmdMsg.Filename, " qinfofile1.dat");

    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "Filename in %s must be terminated and have no spaces",
            "WriteActiveTransCmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteActiveTransCmdInvFilename");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, WriteActiveTransCmdInvFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteActiveTransCmdCreatFail
 */
void Test_CF_AppPipe_WriteActiveTransCmdCreatFail(void)
{
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = CF_PLAYBACK;
    strcpy(WrActTrCmdMsg.Filename, TestQInfoDir);
    strcat(WrActTrCmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX,
                               OS_FS_ERR_INVALID_POINTER, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent,
            "SendActiveTransCmd:Error creating file %s, stat=0x%x",
            WrActTrCmdMsg.Filename, OS_FS_ERR_INVALID_POINTER);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteActiveTransCmdCreatFail");

    UtAssert_EventSent(CF_WRACT_ERR2_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, WriteActiveTransCmdCreatFail: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteActiveTransCmdWrHdrFail
 */
void Test_CF_AppPipe_WriteActiveTransCmdWrHdrFail(void)
{
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = CF_PLAYBACK;
    strcpy(WrActTrCmdMsg.Filename, TestQInfoDir);
    strcat(WrActTrCmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t) - 5, 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "File write,byte cnt err,file %s,request=%d,actual=%d",
            WrActTrCmdMsg.Filename, sizeof(CFE_FS_Header_t),
            sizeof(CFE_FS_Header_t) - 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteActiveTransCmdWrHdrFail");

    UtAssert_EventSent(CF_FILEWRITE_ERR_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, WriteActiveTransCmdWrHdrFail: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteActiveTransCmdInvWhichQs
 */
void Test_CF_AppPipe_WriteActiveTransCmdInvWhichQs(void)
{
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = 3;  /* all=0/up=1/down=2 */
    strcpy(WrActTrCmdMsg.Filename, TestQInfoDir);
    strcat(WrActTrCmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "CF:Write Active Cmd Error,Type Value %d > max %d",
            WrActTrCmdMsg.Type, CF_PLAYBACK);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteActiveTransCmdInvWhichQs");

    UtAssert_EventSent(CF_WRACT_ERR1_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, WriteActiveTransCmdInvWhichQs: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, WriteActiveTransCmdEntryWriteErr
 */
void Test_CF_AppPipe_WriteActiveTransCmdEntryWriteErr(void)
{
    uint32                    QEntryCnt;
    CF_PlaybackFileCmd_t      PbFileCmdMsg;
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = CF_PLAYBACK;
    strcpy(WrActTrCmdMsg.Filename, TestQInfoDir);
    strcat(WrActTrCmdMsg.Filename, TestQInfoFile1);

    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t) - 4, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbActiveQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    sprintf(expEvent, "File write,byte cnt err,file %s,request=%d,actual=%d",
            WrActTrCmdMsg.Filename, sizeof(CF_QueueInfoFileEntry_t),
            sizeof(CF_QueueInfoFileEntry_t) - 4);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, WriteActiveTransCmdEntryWriteErr");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, WriteActiveTransCmdEntryWriteErr: QEntryCnt");

    UtAssert_EventSent(CF_FILEWRITE_ERR_EID, CFE_EVS_ERROR, expEvent,
                 "CF_AppPipe, WriteActiveTransCmdEntryWriteErr: Event Sent");

    CF_ResetEngine();
}


#if 0
/**
 * Test CF_AppPipe, WriteActiveTransCmdDefaultFilename
 */
void Test_CF_AppPipe_WriteActiveTransCmdDefaultFilename(void)
{
    uint32                    TotalQEntryCnt;
    int                       WrittenBytes;
    CF_Test_InPDUMsg_t        InPDUMsg;
    CF_PlaybackFileCmd_t      PbFileCmdMsg1;
    CF_PlaybackFileCmd_t      PbFileCmdMsg2;
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  FullSrcFileName1[OS_MAX_PATH_LEN];
    char  FullSrcFileName2[OS_MAX_PATH_LEN];
    char  expEventPb1[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventPb2[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventWr[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* build cmd to write all active entries to a file */
    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = CF_ALL;
    strcpy(WrActTrCmdMsg.Filename, "");
    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

//    CFE_SB_InitMsg((void*)&InPDUMsg, CF_CMD_MID, sizeof(InPDUMsg), TRUE);

    /* force the file create to return a valid file descriptor (5) */
    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    /* create two playback chan 0, active queue entry */
    CF_TstUtil_CreateTwoPbActiveQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);

    /* create one uplink active queue entry */
    CF_TstUtil_CreateOneUpActiveQueueEntry(&InPDUMsg);

    CF_ShowQs();

    TotalQEntryCnt = CF_AppData.UpQ[CF_UP_ACTIVEQ].EntryCnt +
          CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    strcpy(FullSrcFileName1, PbFileCmdMsg1.SrcFilename);
    sprintf(expEventPb1, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,"
            "Peer %s,File %s", PbFileCmdMsg1.Class, PbFileCmdMsg1.Channel,
            PbFileCmdMsg1.Priority, PbFileCmdMsg1.Preserve,
            PbFileCmdMsg1.PeerEntityId, FullSrcFileName1);

    strcpy(FullSrcFileName2, PbFileCmdMsg2.SrcFilename);
    sprintf(expEventPb2, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,"
            "Peer %s,File %s", PbFileCmdMsg2.Class, PbFileCmdMsg2.Channel,
            PbFileCmdMsg2.Priority, PbFileCmdMsg2.Preserve,
            PbFileCmdMsg2.PeerEntityId, FullSrcFileName2);

    /* 1 header size + 3 entry size(uplink/downlink) */
    WrittenBytes = sizeof(CFE_FS_Header_t) +
                   (3 * sizeof(CF_QueueInfoFileEntry_t));
    sprintf(expEventWr, "%s written:Size=%d,Entries=%d",
            CF_DEFAULT_QUEUE_INFO_FILENAME, WrittenBytes, 3);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteActiveTransCmdDefaultFilename");

    UtAssert_True(TotalQEntryCnt == 3,
          "CF_AppPipe, WriteActiveTransCmdDefaultFilename: TotalQEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb1,
      "CF_AppPipe, WriteActiveTransCmdDefaultFilename: PbFile#1 Event Sent");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb1,
      "CF_AppPipe, WriteActiveTransCmdDefaultFilename: PbFile#2 Event Sent");

    UtAssert_EventSent(CF_WRACT_TRANS_EID, CFE_EVS_DEBUG, expEventWr,
         "CF_AppPipe, WriteActiveTransCmdDefaultFilename: Write Event Sent");

    CF_ResetEngine();
}
#endif


#if 0
/**
 * Test CF_AppPipe, WriteActiveTransCmdCustFilename
 */
void Test_CF_AppPipe_WriteActiveTransCmdCustFilename(void)
{
    uint32                    TotalQEntryCnt;
    int                       WrittenBytes;
    CF_NoArgsCmd_t            InPDUMsg;
    CF_PlaybackFileCmd_t      PbFileCmdMsg1;
    CF_PlaybackFileCmd_t      PbFileCmdMsg2;
    CF_WriteActiveTransCmd_t  WrActTrCmdMsg;
    char  FullSrcFileName1[OS_MAX_PATH_LEN];
    char  FullSrcFileName2[OS_MAX_PATH_LEN];
    char  expEventPb1[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventPb2[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventWr[CFE_EVS_MAX_MESSAGE_LENGTH];

    /* build cmd to write all active entries to a file */
    CFE_SB_InitMsg((void*)&WrActTrCmdMsg, CF_CMD_MID,
                   sizeof(WrActTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&WrActTrCmdMsg,
                      (uint16)CF_WR_ACTIVE_TRANS_CC);
    WrActTrCmdMsg.Type = CF_ALL;
    strcpy(WrActTrCmdMsg.Filename, TestQInfoDir);
    strcat(WrActTrCmdMsg.Filename, TestQInfoFile1);
    CF_Test_PrintCmdMsg((void*)&WrActTrCmdMsg, sizeof(WrActTrCmdMsg));

    CFE_SB_InitMsg((void*)&InPDUMsg, CF_CMD_MID, sizeof(InPDUMsg), TRUE);

    /* force the file create to return a valid file descriptor (5) */
    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CREAT_INDEX, 5, 1);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_CLOSE_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_CLOSE_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_WRITE_INDEX,
                               sizeof(CF_QueueInfoFileEntry_t), 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_WRITE_INDEX);

    Ut_CFE_FS_SetReturnCode(UT_CFE_FS_WRITEHDR_INDEX,
                            sizeof(CFE_FS_Header_t), 1);

    /* Execute the function being tested */
    CF_AppInit();

    /* create two playback chan 0, active queue entry */
    CF_TstUtil_CreateTwoPbActiveQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);

    /* create one uplink active queue entry */
    CF_TstUtil_CreateOneUpActiveQueueEntry((CFE_SB_MsgPtr_t)&InPDUMsg);

    CF_ShowQs();

    TotalQEntryCnt = CF_AppData.UpQ[CF_UP_ACTIVEQ].EntryCnt +
          CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&WrActTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    strcpy(FullSrcFileName1, PbFileCmdMsg1.SrcFilename);
    sprintf(expEventPb1, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,"
            "Peer %s,File %s", PbFileCmdMsg1.Class, PbFileCmdMsg1.Channel,
            PbFileCmdMsg1.Priority, PbFileCmdMsg1.Preserve,
            PbFileCmdMsg1.PeerEntityId, FullSrcFileName1);

    strcpy(FullSrcFileName2, PbFileCmdMsg2.SrcFilename);
    sprintf(expEventPb2, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,"
            "Peer %s,File %s", PbFileCmdMsg2.Class, PbFileCmdMsg2.Channel,
            PbFileCmdMsg2.Priority, PbFileCmdMsg2.Preserve,
            PbFileCmdMsg2.PeerEntityId, FullSrcFileName2);

    /* 1 header size + 3 entry size(uplink/downlink) */
    WrittenBytes = sizeof(CFE_FS_Header_t) +
                   (3 * sizeof(CF_QueueInfoFileEntry_t));
    sprintf(expEventWr, "%s written:Size=%d,Entries=%d",
            WrActTrCmdMsg.Filename, WrittenBytes, 3);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, WriteActiveTransCmdCustFilename");

    UtAssert_True(TotalQEntryCnt == 3,
             "CF_AppPipe, WriteActiveTransCmdCustFilename: TotalQEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb1,
      "CF_AppPipe, WriteActiveTransCmdCustFilename: PbFile#1 Event Sent");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb2,
      "CF_AppPipe, WriteActiveTransCmdCustFilename: PbFile#2 Event Sent");

    UtAssert_EventSent(CF_WRACT_TRANS_EID, CFE_EVS_DEBUG, expEventWr,
         "CF_AppPipe, WriteActiveTransCmdCustFilename: Write Event Sent");

    CF_ResetEngine();
}
#endif


/**
 * Test CF_AppPipe, QuickStatusCmdFilenameNotFound
 */
void Test_CF_AppPipe_QuickStatusCmdFilenameNotFound(void)
{
    CF_QuickStatCmd_t     QSCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&QSCmdMsg, CF_CMD_MID, sizeof(QSCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&QSCmdMsg,
                      (uint16)CF_QUICKSTATUS_CC);
    strcpy(QSCmdMsg.Trans, TestPbDir);
    strcat(QSCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&QSCmdMsg, sizeof(QSCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&QSCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Quick Status Cmd Error,Trans %s Not Found",
            QSCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, QuickStatusCmdFilenameNotFound");

    UtAssert_EventSent(CF_QUICK_ERR1_EID, CFE_EVS_ERROR, expEvent,
                "CF_AppPipe, QuickStatusCmdFilenameNotFound: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, QuickStatusCmdFilename
 */
void Test_CF_AppPipe_QuickStatusCmdFilename(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_QuickStatCmd_t     QSCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&QSCmdMsg, CF_CMD_MID, sizeof(QSCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&QSCmdMsg,
                      (uint16)CF_QUICKSTATUS_CC);
    strcpy(QSCmdMsg.Trans, TestPbDir);
    strcat(QSCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&QSCmdMsg, sizeof(QSCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&QSCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(expEvent, "Trans %s_%u %s Stat=%s,CondCode=%s",
      CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].HeadPtr->SrcEntityId,
      (unsigned int)CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].HeadPtr->TransNum,
      CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].HeadPtr->SrcFile,
      "PENDING", "NO_ERR");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, QuickStatusCmdFilename");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, QuickStatusCmdFilename: QEntryCnt");

    UtAssert_EventSent(CF_QUICK_CMD_EID, CFE_EVS_INFORMATION, expEvent,
                       "CF_AppPipe, QuickStatusCmdFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendTransDiagCmdFileNotFound
 */
void Test_CF_AppPipe_SendTransDiagCmdFileNotFound(void)
{
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);
    strcpy(SndTrCmdMsg.Trans, TestPbDir);
    strcat(SndTrCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Send Trans Cmd Error, %s not found.",
            SndTrCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SendTransDiagCmdFileNotFound");

    UtAssert_EventSent(CF_SND_TRANS_ERR_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, SendTransDiagCmdFileNotFound: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendTransDiagCmdTransNotFound
 */
void Test_CF_AppPipe_SendTransDiagCmdTransNotFound(void)
{
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);
    strcpy(SndTrCmdMsg.Trans, TestInSrcEntityId1);
    strcat(SndTrCmdMsg.Trans, "_5");

    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Send Trans Cmd Error, %s not found.", SndTrCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SendTransDiagCmdTransNotFound");

    UtAssert_EventSent(CF_SND_TRANS_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SendTransDiagCmdTransNotFound: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendTransDiagCmdInvLen
 */
void Test_CF_AppPipe_SendTransDiagCmdInvLen(void)
{
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);
    strcpy(SndTrCmdMsg.Trans, TestPbDir);
    strcat(SndTrCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_SEND_TRANS_DIAG_DATA_CC,
            sizeof(SndTrCmdMsg), sizeof(SndTrCmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SendTransDiagCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SendTransDiagCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendTransDiagCmdUntermString
 */
void Test_CF_AppPipe_SendTransDiagCmdUntermString(void)
{
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);
    CFE_PSP_MemSet(SndTrCmdMsg.Trans, 0xFF, OS_MAX_PATH_LEN);

    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Unterminated string found in %s",
            "SendTransData Cmd, Trans parameter");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SendTransDiagCmdUntermString");

    UtAssert_EventSent(CF_NO_TERM_ERR_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, SendTransDiagCmdUntermString: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendTransDiagCmdInvFilename
 */
void Test_CF_AppPipe_SendTransDiagCmdInvFilename(void)
{
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);
    strcpy(SndTrCmdMsg.Trans, TestPbDir);
    strcat(SndTrCmdMsg.Trans, "This name has spaces");

    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Filename in %s must be terminated and have no spaces",
            "SendTransDataCmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SendTransDiagCmdInvFilename");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SendTransDiagCmdInvFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Hook to support: Test CF_AppPipe, SendTransDiagCmd
 */
int32 Test_CF_AppPipe_SendTransDiagCmd_SendMsgHook(CFE_SB_Msg_t *MsgPtr)
{
    unsigned char       *pBuff = NULL;
    uint16              msgLen = 0;
    int                 i = 0;
    CFE_SB_MsgId_t      MsgId = 0;
    time_t              localTime;
    struct tm           *loc_time;
    CFE_TIME_SysTime_t  TimeFromMsg;
    CF_TransPacket_t    TrPktMsg;

    SendTransDiagHook_CalledCnt ++;

    msgLen = CFE_SB_GetTotalMsgLength(MsgPtr);
    MsgId = CFE_SB_GetMsgId(MsgPtr);

    pBuff = (unsigned char *)MsgPtr;
    printf("###SendTransDiagCmd_SendMsgHook(msgLen %u): ", msgLen);
    for (i = 0; i < msgLen; i++)
    {
        printf("0x%02x ", *pBuff);
        pBuff++;
    }
    printf("\n");

    TimeFromMsg = CFE_SB_GetMsgTime(MsgPtr);
    localTime = CF_Test_GetTimeFromMsg(TimeFromMsg);
    loc_time = localtime(&localTime);
    printf("TimeFromMessage: %s", asctime(loc_time));

    switch(MsgId)
    {
        case CF_TRANS_TLM_MID:
        {
            SendTransDiagHook_MsgId = CF_TRANS_TLM_MID;
            printf("Sent CF_TRANS_TLM_MID\n");
            memcpy((void *)&TrPktMsg, (void *)MsgPtr, sizeof(TrPktMsg));

            printf("Eng.TransLen: %u\n", TrPktMsg.Eng.TransLen);
            printf("Eng.TransVal: %u\n", TrPktMsg.Eng.TransVal);
            printf("Eng.Naks: %u\n", TrPktMsg.Eng.Naks);
            printf("Eng.PartLen: %u\n", TrPktMsg.Eng.PartLen);
            printf("Eng.PartVal: %u\n", TrPktMsg.Eng.PartVal);
            printf("Eng.Phase: %u\n", TrPktMsg.Eng.Phase);
            printf("Eng.Flags: %lu\n", TrPktMsg.Eng.Flags);
            printf("Eng.TransNum: %lu\n", TrPktMsg.Eng.TransNum);
            printf("Eng.Attempts: %lu\n", TrPktMsg.Eng.Attempts);
            printf("Eng.CondCode: %lu\n", TrPktMsg.Eng.CondCode);
            printf("Eng.DeliCode: %lu\n", TrPktMsg.Eng.DeliCode);
            printf("Eng.FdOffset: %lu\n", TrPktMsg.Eng.FdOffset);
            printf("Eng.FdLength: %lu\n", TrPktMsg.Eng.FdLength);
            printf("Eng.Checksum: %lu\n", TrPktMsg.Eng.Checksum);
            printf("Eng.FinalStat: %lu\n", TrPktMsg.Eng.FinalStat);
            printf("Eng.FileSize: %lu\n", TrPktMsg.Eng.FileSize);
            printf("Eng.RcvdFileSize: %lu\n", TrPktMsg.Eng.RcvdFileSize);
            printf("Eng.Role: %lu\n", TrPktMsg.Eng.Role);
            printf("Eng.State: %lu\n", TrPktMsg.Eng.State);
            printf("Eng.StartTime: %lu\n", TrPktMsg.Eng.StartTime);
            printf("Eng.SrcFile: %s\n", TrPktMsg.Eng.SrcFile);
            printf("Eng.DstFile: %s\n", TrPktMsg.Eng.DstFile);
            printf("Eng.TmpFile: %s\n", TrPktMsg.Eng.TmpFile);

            printf("App.Status: %lu\n", TrPktMsg.App.Status);
            printf("App.CondCode: %lu\n", TrPktMsg.App.CondCode);
            printf("App.Priority: %lu\n", TrPktMsg.App.Priority);
            printf("App.Class: %lu\n", TrPktMsg.App.Class);
            printf("App.ChanNum: %lu\n", TrPktMsg.App.ChanNum);
            printf("App.Source: %lu\n", TrPktMsg.App.Source);
            printf("App.NodeType: %lu\n", TrPktMsg.App.NodeType);
            printf("App.TransNum: %lu\n", TrPktMsg.App.TransNum);
            printf("App.SrcEntityId: %s\n", TrPktMsg.App.SrcEntityId);
            printf("App.SrcFile: %s\n", TrPktMsg.App.SrcFile);
            printf("App.DstFile: %s\n", TrPktMsg.App.DstFile);
            break;
        }
        default:
        {
            printf("Sent MID(0x%04X)\n", MsgId);
            break;
        }
    }

    return CFE_SUCCESS;
}


/**
 * Test CF_AppPipe, SendTransDiagCmdFilename
 */
void Test_CF_AppPipe_SendTransDiagCmdFilename(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg;
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);
    strcpy(SndTrCmdMsg.Trans, TestPbDir);
    strcat(SndTrCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    SendTransDiagHook_MsgId = 0;
    SendTransDiagHook_CalledCnt = 0;
    Ut_CFE_SB_SetFunctionHook(UT_CFE_SB_SENDMSG_INDEX,
                    (void *)&Test_CF_AppPipe_SendTransDiagCmd_SendMsgHook);

    Ut_CFE_TIME_SetFunctionHook(UT_CFE_TIME_GETTIME_INDEX,
                                (void*)&Test_CF_GetCFETimeHook);

    Ut_CFE_SB_SetFunctionHook(UT_CFE_SB_TIMESTAMPMSG_INDEX,
                              (void*)&Test_CF_SBTimeStampMsgHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(expEvent, "CF:Sending Transaction Pkt %s", SndTrCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SendTransDiagCmdFilename");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, SendTransDiagCmdFilename: QEntryCnt");

    UtAssert_True(SendTransDiagHook_CalledCnt == 1,
                  "CF_AppPipe, SendTransDiagCmdFilename: Hook CalledCnt");

    UtAssert_True(SendTransDiagHook_MsgId == CF_TRANS_TLM_MID,
             "CF_AppPipe, SendTransDiagCmdFilename: Sent CF_TRANS_TLM_MID");

    UtAssert_True(CF_AppData.Trans.App.Source == CF_PLAYBACKFILECMD,
             "CF_AppPipe, SendTransDiagCmdFilename: Source PLAYBACKFILE");

    UtAssert_True(CF_AppData.Trans.App.NodeType == CF_PLAYBACK,
                  "CF_AppPipe, SendTransDiagCmdFilename: PLAYBACK Node");

    UtAssert_True(strcmp(CF_AppData.Trans.App.SrcEntityId,
                         CF_AppData.Tbl->FlightEntityId) == 0,
                  "CF_AppPipe, SendTransDiagCmdFilename: SrcEntityId");

    UtAssert_True(strcmp(CF_AppData.Trans.App.SrcFile, SndTrCmdMsg.Trans)
                  == 0, "CF_AppPipe, SendTransDiagCmdFilename: SrcFile");

    UtAssert_EventSent(CF_SND_TRANS_CMD_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, SendTransDiagCmdFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SendTransDiagCmdTransId
 */
void Test_CF_AppPipe_SendTransDiagCmdTransId(void)
{
    uint32                QEntryCnt;
    CF_PlaybackFileCmd_t  PbFileCmdMsg1;
    CF_PlaybackFileCmd_t  PbFileCmdMsg2;
    CF_SendTransCmd_t     SndTrCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&SndTrCmdMsg, CF_CMD_MID,
                   sizeof(SndTrCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&SndTrCmdMsg,
                      (uint16)CF_SEND_TRANS_DIAG_DATA_CC);

    SendTransDiagHook_MsgId = 0;
    SendTransDiagHook_CalledCnt = 0;
    Ut_CFE_SB_SetFunctionHook(UT_CFE_SB_SENDMSG_INDEX,
                    (void *)&Test_CF_AppPipe_SendTransDiagCmd_SendMsgHook);

    Ut_CFE_TIME_SetFunctionHook(UT_CFE_TIME_GETTIME_INDEX,
                                (void*)&Test_CF_GetCFETimeHook);

    Ut_CFE_SB_SetFunctionHook(UT_CFE_SB_TIMESTAMPMSG_INDEX,
                              (void*)&Test_CF_SBTimeStampMsgHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateTwoPbPendingQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);

    sprintf(SndTrCmdMsg.Trans, "%s%s", CF_AppData.Tbl->FlightEntityId, "_0");
    CF_Test_PrintCmdMsg((void*)&SndTrCmdMsg, sizeof(SndTrCmdMsg));

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&SndTrCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    CF_ShowQs();

    QEntryCnt =
          CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(expEvent, "CF:Sending Transaction Pkt %s", SndTrCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SendTransDiagCmdTransId");

    UtAssert_True(QEntryCnt == 2,
                  "CF_AppPipe, SendTransDiagCmdTransId: QEntryCnt");

    UtAssert_True(SendTransDiagHook_CalledCnt == 2,
                  "CF_AppPipe, SendTransDiagCmdTransId: Hook CalledCnt");

#if 0
    UtAssert_True(SendTransDiagHook_MsgId == CF_TRANS_TLM_MID,
             "CF_AppPipe, SendTransDiagCmdTransId: Sent CF_TRANS_TLM_MID");

    UtAssert_True(CF_AppData.Trans.App.Source == CF_PLAYBACKFILECMD,
             "CF_AppPipe, SendTransDiagCmdTransId: Source PLAYBACKFILE");

    UtAssert_True(CF_AppData.Trans.App.NodeType == CF_PLAYBACK,
                  "CF_AppPipe, SendTransDiagCmdTransId: PLAYBACK Node");

    UtAssert_True(strcmp(CF_AppData.Trans.App.SrcEntityId,
                         CF_AppData.Tbl->FlightEntityId) == 0,
                  "CF_AppPipe, SendTransDiagCmdTransId: SrcEntityId");

    UtAssert_True(strcmp(CF_AppData.Trans.App.SrcFile,
                         PbFileCmdMsg1.SrcFilename) == 0,
                  "CF_AppPipe, SendTransDiagCmdTransId: SrcFile");
#endif

    UtAssert_EventSent(CF_SND_TRANS_CMD_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, SendTransDiagCmdTransId: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvLen
 */
void Test_CF_AppPipe_SetPollParamCmdInvLen(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_SET_POLL_PARAM_CC,
            sizeof(CmdMsg), sizeof(CmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvChan
 */
void Test_CF_AppPipe_SetPollParamCmdInvChan(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 130;  /* 0 to (CF_MAX_PLAYBACK_CHANNELS - 1) */
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Chan Param %u in SetPollParamCmd,Max %u",
            CmdMsg.Chan, CF_MAX_PLAYBACK_CHANNELS - 1);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvChan");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR1_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvChan: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvDir
 */
void Test_CF_AppPipe_SetPollParamCmdInvDir(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = CF_MAX_POLLING_DIRS_PER_CHAN;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid PollDir Param %u in SetPollParamCmd,Max %u",
            CmdMsg.Dir, CF_MAX_POLLING_DIRS_PER_CHAN-1);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvDir");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR2_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvDir: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvClass
 */
void Test_CF_AppPipe_SetPollParamCmdInvClass(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = 0;  /* 1=class 1, 2=class 2 */
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Class Param %u in "
            "SetPollParamCmd,Min %u,Max %u",
            CmdMsg.Class, CF_CLASS_1, CF_CLASS_2);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvClass");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR3_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvClass: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvPreserve
 */
void Test_CF_AppPipe_SetPollParamCmdInvPreserve(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = 2;  /* 0=delete, 1=keep */
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Preserve Param %u in SetPollParamCmd,Max %u",
            CmdMsg.Preserve, CF_KEEP_FILE);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvPreserve");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR4_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, SetPollParamCmdInvPreserve: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvSrc
 */
void Test_CF_AppPipe_SetPollParamCmdInvSrc(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, "/cf /");
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "SrcPath in SetPollParam Cmd must be terminated,"
            "have no spaces,slash at end");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvSrc");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR5_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvSrc: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvDst
 */
void Test_CF_AppPipe_SetPollParamCmdInvDst(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, "gnd path");

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s", "DstPath in SetPollParam Cmd must be "
            "terminated and have no spaces");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvDst");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR6_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvDst: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmdInvId
 */
void Test_CF_AppPipe_SetPollParamCmdInvId(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, "234200");
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "PeerEntityId %s in SetPollParam Cmd must "
            "be 2 byte,dotted decimal fmt.ex 0.24", CmdMsg.PeerEntityId);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, SetPollParamCmdInvId");

    UtAssert_EventSent(CF_SET_POLL_PARAM_ERR7_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, SetPollParamCmdInvId: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, SetPollParamCmd
 */
void Test_CF_AppPipe_SetPollParamCmd(void)
{
    CF_SetPollParamCmd_t  CmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&CmdMsg, CF_CMD_MID, sizeof(CmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&CmdMsg,
                      (uint16)CF_SET_POLL_PARAM_CC);
    CmdMsg.Chan = 0;
    CmdMsg.Dir = 1;
    CmdMsg.Class = CF_CLASS_1;
    CmdMsg.Priority = 1;
    CmdMsg.Preserve = CF_KEEP_FILE;
    strcpy(CmdMsg.PeerEntityId, TestPbPeerEntityId);
    strcpy(CmdMsg.SrcPath, TestPbDir);
    strcpy(CmdMsg.DstPath, TestDstDir);

    CF_Test_PrintCmdMsg((void*)&CmdMsg, sizeof(CmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&CmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "SetPollParam Cmd Rcvd,Ch=%u,Dir=%u,Cl=%u,Pri=%u,Pre=%u",
            CmdMsg.Chan, CmdMsg.Dir, CmdMsg.Class,
            CmdMsg.Priority, CmdMsg.Preserve);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, SetPollParamCmd");

    UtAssert_True(CF_AppData.Tbl->OuCh[CmdMsg.Chan].PollDir[CmdMsg.Dir].Preserve
                  == CF_KEEP_FILE,
                  "CF_AppPipe, SetPollParamCmd: Config Tbl Polling Preserve");

    UtAssert_True(strcmp(
             CF_AppData.Tbl->OuCh[CmdMsg.Chan].PollDir[CmdMsg.Dir].PeerEntityId,
             CmdMsg.PeerEntityId) == 0,
             "CF_AppPipe, SetPollParamCmd: Config Tbl Polling PeerEntityId");

    UtAssert_True(strcmp(
             CF_AppData.Tbl->OuCh[CmdMsg.Chan].PollDir[CmdMsg.Dir].SrcPath,
             CmdMsg.SrcPath) == 0,
             "CF_AppPipe, SetPollParamCmd: Config Tbl Polling SrcPath");

    UtAssert_True(strcmp(
             CF_AppData.Tbl->OuCh[CmdMsg.Chan].PollDir[CmdMsg.Dir].DstPath,
             CmdMsg.DstPath) == 0,
             "CF_AppPipe, SetPollParamCmd: Config Tbl Polling DstPath");

    UtAssert_EventSent(CF_SET_POLL_PARAM1_EID, CFE_EVS_DEBUG, expEvent,
                       "CF_AppPipe, SetPollParamCmd: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdInvLen
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdInvLen(void)
{
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID,
                   sizeof(DeQCmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestInSrcEntityId1);
    strcat(DeQCmdMsg.Trans, "_209");

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_DELETE_QUEUE_NODE_CC,
            sizeof(DeQCmdMsg), sizeof(DeQCmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, DeleteQueueNodeCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdTransUnterm
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdTransUnterm(void)
{
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    CFE_PSP_MemSet(DeQCmdMsg.Trans, 0xFF, OS_MAX_PATH_LEN);

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Unterminated string found in %s",
            "DequeueNode Cmd, Trans parameter");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdTransUnterm");

    UtAssert_EventSent(CF_NO_TERM_ERR_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdTransUnterm: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdInvFilename
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdInvFilename(void)
{
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestPbDir);
    strcat(DeQCmdMsg.Trans, "p bfile1.dat");

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Filename in %s must be terminated and have no spaces",
            "DequeueNodeCmd");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdInvFilename");

    UtAssert_EventSent(CF_INV_FILENAME_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, DeleteQueueNodeCmdInvFilename: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdFileNotFound
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdFileNotFound(void)
{
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestPbDir);
    strcat(DeQCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Dequeue Node Cmd Error, %s not found.",
            DeQCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdFileNotFound");

    UtAssert_EventSent(CF_DEQ_NODE_ERR1_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdFileNotFound: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdIdNotFound
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdIdNotFound(void)
{
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestInSrcEntityId1);
    strcat(DeQCmdMsg.Trans, "_209");

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Dequeue Node Cmd Error, %s not found.",
            DeQCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdIdNotFound");

    UtAssert_EventSent(CF_DEQ_NODE_ERR1_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdIdNotFound: Event Sent");

    CF_ResetEngine();
}


#if 0
/**
 * Test CF_AppPipe, DeleteQueueNodeCmdUpActive
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdUpActive(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    CF_NoArgsCmd_t       InPDUMsg;
    char  FullTransString[OS_MAX_PATH_LEN];
    char  expEventWarn[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestInSrcEntityId1);
    strcat(DeQCmdMsg.Trans, "_500");
    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    CFE_SB_InitMsg((void*)&InPDUMsg, CF_CMD_MID, sizeof(InPDUMsg), TRUE);

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOneUpActiveQueueEntry((CFE_SB_MsgPtr_t)&InPDUMsg);
    QEntryCntBefore = CF_AppData.UpQ[CF_UP_ACTIVEQ].EntryCnt;

    CF_ShowQs();

    /* This first dequeue command will produce the warning */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    /* This identical second cmd will dequeue without warning */
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter = CF_AppData.UpQ[CF_UP_ACTIVEQ].EntryCnt;

    sprintf(FullTransString, "%s", DeQCmdMsg.Trans);
    sprintf(expEventWarn, "DequeueNodeCmd:Trans %s is ACTIVE! Must send "
            "cmd again to remove", FullTransString);
    sprintf(expEvent, "DequeueNodeCmd %s Removed from %s Queue,Stat %d",
            DeQCmdMsg.Trans, "Incoming Active", CF_SUCCESS);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdUpActive");

    UtAssert_True((QEntryCntBefore == 1) && (QEntryCntAfter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdUpActive: QEntryCnt");

    UtAssert_EventSent(CF_DEQ_NODE_ERR2_EID, CFE_EVS_CRITICAL, expEventWarn,
             "CF_AppPipe, DeleteQueueNodeCmdUpActive: Warning Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE1_EID, CFE_EVS_DEBUG, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdUpActive: Success Event Sent");

    CF_ResetEngine();
}
#endif


#if 0
/**
 * Test CF_AppPipe, DeleteQueueNodeCmdUpHist
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdUpHist(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    CF_NoArgsCmd_t       InPDUMsg;
    TRANSACTION          trans;
    char  FullDstFilename[OS_MAX_PATH_LEN];
    char  expEventInTrans[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestInSrcEntityId1);
    strcat(DeQCmdMsg.Trans, "_500");
    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    CFE_SB_InitMsg((void*)&InPDUMsg, CF_CMD_MID, sizeof(InPDUMsg), TRUE);

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOneUpHistoryQueueEntry((CFE_SB_MsgPtr_t)&InPDUMsg);
    QEntryCntBefore = CF_AppData.UpQ[CF_UP_HISTORYQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter = CF_AppData.UpQ[CF_UP_HISTORYQ].EntryCnt;

    cfdp_trans_from_string(DeQCmdMsg.Trans, &trans);
    sprintf(FullDstFilename, "%s%s", TestInDir, TestInFile1);
    sprintf(expEventInTrans, "Incoming trans success %d.%d_%d,dest %s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullDstFilename);
    sprintf(expEvent, "DequeueNodeCmd %s Removed from %s Queue,Stat %d",
            DeQCmdMsg.Trans, "Incoming History", CF_SUCCESS);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdUpHist");

    UtAssert_True((QEntryCntBefore == 1) && (QEntryCntAfter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdUpHist: QEntryCnt");

    UtAssert_EventSent(CF_IN_TRANS_OK_EID, CFE_EVS_INFORMATION,
             expEventInTrans,
             "CF_AppPipe, DeleteQueueNodeCmdUpHist: InTrans Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE1_EID, CFE_EVS_DEBUG, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdUpHist: Success Event Sent");

    CF_ResetEngine();
}
#endif


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdPbPend
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdPbPend(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_PlaybackFileCmd_t PbFileCmdMsg;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  FullTransFileName[OS_MAX_PATH_LEN];
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestPbDir);
    strcat(DeQCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    QEntryCntBefore =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(FullTransFileName, "%s%s", TestPbDir, TestPbFile1);
    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,"
            "Pre %d,Peer %s,File %s", PbFileCmdMsg.Class,
            PbFileCmdMsg.Channel, PbFileCmdMsg.Priority,
            PbFileCmdMsg.Preserve, PbFileCmdMsg.PeerEntityId,
            FullTransFileName);
    sprintf(expEvent,
            "DequeueNodeCmd %s Removed from Chan %u,%s Queue,Stat %d",
            FullTransFileName, PbFileCmdMsg.Channel,
            "Outgoing Pending", CF_SUCCESS);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdPbPend");

    UtAssert_True((QEntryCntBefore == 1) && (QEntryCntAfter == 0),
                "CF_AppPipe, DeleteQueueNodeCmdPbPend: PbPendQ EntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
             "CF_AppPipe, DeleteQueueNodeCmdPbPend: PbFile Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE2_EID, CFE_EVS_DEBUG, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdPbPend: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdPbActive
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdPbActive(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_PlaybackFileCmd_t PbFileCmdMsg;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    TRANSACTION          trans;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  FullTransString[OS_MAX_PATH_LEN];
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventPbTrans[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventWr[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbActiveQueueEntry(&PbFileCmdMsg);
    QEntryCntBefore =
         CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    strcpy(DeQCmdMsg.Trans, CF_AppData.Tbl->FlightEntityId);
    strcat(DeQCmdMsg.Trans, "_1");
    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    /* This first dequeue command will produce the warning */
    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    /* This identical second cmd will dequeue without warning */
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter =
         CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_ACTIVEQ].EntryCnt;

    strcpy(FullSrcFileName, PbFileCmdMsg.SrcFilename);
    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,"
            "Pre %d,Peer %s,File %s", PbFileCmdMsg.Class,
            PbFileCmdMsg.Channel, PbFileCmdMsg.Priority,
            PbFileCmdMsg.Preserve, PbFileCmdMsg.PeerEntityId,
            FullSrcFileName);

    cfdp_trans_from_string(DeQCmdMsg.Trans, &trans);
    sprintf(expEventPbTrans, "Outgoing trans started %d.%d_%d,src %s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName);

    strcpy(FullTransString, DeQCmdMsg.Trans);
    sprintf(expEventWr, "DequeueNodeCmd:Trans %s is ACTIVE! "
            "Must send cmd again to remove", FullTransString);

    sprintf(expEvent, "DequeueNodeCmd %s Removed from Chan %u,%s "
            "Queue,Stat %d", FullTransString, PbFileCmdMsg.Channel,
            "Outgoing Active", CF_SUCCESS);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdPbActive");

    UtAssert_True((QEntryCntBefore == 1) && (QEntryCntAfter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdPbActive: QEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
             "CF_AppPipe, DeleteQueueNodeCmdPbActive: PbFile Event Sent");

    UtAssert_EventSent(CF_OUT_TRANS_START_EID, CFE_EVS_INFORMATION,
              expEventPbTrans,
              "CF_AppPipe, DeleteQueueNodeCmdPbActive: Trans Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE_ERR3_EID, CFE_EVS_CRITICAL, expEventWr,
             "CF_AppPipe, DeleteQueueNodeCmdPbActive: Warning Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE2_EID, CFE_EVS_DEBUG, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdPbActive: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdPbHist
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdPbHist(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_PlaybackFileCmd_t PbFileCmdMsg;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    TRANSACTION          trans;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  FullTransString[OS_MAX_PATH_LEN];
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventPbTrans[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventAb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);

    Ut_CFE_SB_SetReturnCode(UT_CFE_SB_ZEROCOPYGETPTR_INDEX, (int32)NULL, 1);
    Ut_CFE_SB_ContinueReturnCodeAfterCountZero(
                                          UT_CFE_SB_ZEROCOPYGETPTR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_READ_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_READ_INDEX);

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbHistoryQueueEntry(&PbFileCmdMsg);
    QEntryCntBefore =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_HISTORYQ].EntryCnt;

    strcpy(DeQCmdMsg.Trans, CF_AppData.Tbl->FlightEntityId);
    strcat(DeQCmdMsg.Trans, "_1");
    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_HISTORYQ].EntryCnt;

    strcpy(FullSrcFileName, PbFileCmdMsg.SrcFilename);
    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,"
            "Pre %d,Peer %s,File %s", PbFileCmdMsg.Class,
            PbFileCmdMsg.Channel, PbFileCmdMsg.Priority,
            PbFileCmdMsg.Preserve, PbFileCmdMsg.PeerEntityId,
            FullSrcFileName);

    cfdp_trans_from_string(DeQCmdMsg.Trans, &trans);
    sprintf(expEventPbTrans, "Outgoing trans started %d.%d_%d,src %s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName);

    sprintf(expEventAb, "%s command received.%s", "Abandon", "All");

    strcpy(FullTransString, DeQCmdMsg.Trans);
    sprintf(expEvent, "DequeueNodeCmd %s Removed from Chan %u,%s "
            "Queue,Stat %d", FullTransString, PbFileCmdMsg.Channel,
            "Outgoing History", CF_SUCCESS);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdPbHist");

    UtAssert_True((QEntryCntBefore == 1) && (QEntryCntAfter == 0),
                  "CF_AppPipe, DeleteQueueNodeCmdPbHist: QEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
             "CF_AppPipe, DeleteQueueNodeCmdPbHist: PbFile Event Sent");

    UtAssert_EventSent(CF_OUT_TRANS_START_EID, CFE_EVS_INFORMATION,
             expEventPbTrans,
             "CF_AppPipe, DeleteQueueNodeCmdPbHist: Trans Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEventAb,
             "CF_AppPipe, DeleteQueueNodeCmdPbHist: Abandon Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE2_EID, CFE_EVS_DEBUG, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdPbHist: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdPutFail
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdPutFail(void)
{
    uint32               QEntryCnt;
    CF_PlaybackFileCmd_t PbFileCmdMsg;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    TRANSACTION          trans;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  FullTransString[OS_MAX_PATH_LEN];
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventPbTrans[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventAb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, "0.24_1");
    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    Ut_CFE_SB_SetReturnCode(UT_CFE_SB_ZEROCOPYGETPTR_INDEX, (int32)NULL, 1);
    Ut_CFE_SB_ContinueReturnCodeAfterCountZero(
                                          UT_CFE_SB_ZEROCOPYGETPTR_INDEX);

    Ut_OSFILEAPI_SetReturnCode(UT_OSFILEAPI_READ_INDEX, OS_FS_SUCCESS, 1);
    Ut_OSFILEAPI_ContinueReturnCodeAfterCountZero(UT_OSFILEAPI_READ_INDEX);

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbHistoryQueueEntry(&PbFileCmdMsg);

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_HISTORYQ].EntryCnt;

    strcpy(FullSrcFileName, PbFileCmdMsg.SrcFilename);
    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,"
            "Pre %d,Peer %s,File %s", PbFileCmdMsg.Class,
            PbFileCmdMsg.Channel, PbFileCmdMsg.Priority,
            PbFileCmdMsg.Preserve, PbFileCmdMsg.PeerEntityId,
            FullSrcFileName);

    strcpy(FullTransString, CF_AppData.Tbl->FlightEntityId);
    strcat(FullTransString, "_1");
    cfdp_trans_from_string(FullTransString, &trans);
    sprintf(expEventPbTrans, "Outgoing trans started %d.%d_%d,src %s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullSrcFileName);

    sprintf(expEventAb, "%s command received.%s", "Abandon", "All");

    sprintf(expEvent, "Dequeue Node Cmd Error, %s not found.",
            DeQCmdMsg.Trans);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 2) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdPutFail");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, DeleteQueueNodeCmdPutFail: QEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
             "CF_AppPipe, DeleteQueueNodeCmdPutFail: PbFile Event Sent");

    UtAssert_EventSent(CF_OUT_TRANS_START_EID, CFE_EVS_INFORMATION,
             expEventPbTrans,
             "CF_AppPipe, DeleteQueueNodeCmdPutFail: Trans Event Sent");

    UtAssert_EventSent(CF_CARS_CMD_EID, CFE_EVS_INFORMATION, expEventAb,
             "CF_AppPipe, DeleteQueueNodeCmdPutFail: Abandon Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE_ERR1_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdPutFail: Fail Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, DeleteQueueNodeCmdInvType
 */
void Test_CF_AppPipe_DeleteQueueNodeCmdInvType(void)
{
    uint32               QEntryCnt;
    CF_QueueEntry_t      *pQ;
    CF_PlaybackFileCmd_t PbFileCmdMsg;
    CF_DequeueNodeCmd_t  DeQCmdMsg;
    char  FullSrcFileName[OS_MAX_PATH_LEN];
    char  expEventPb[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&DeQCmdMsg, CF_CMD_MID, sizeof(DeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&DeQCmdMsg,
                      (uint16)CF_DELETE_QUEUE_NODE_CC);
    strcpy(DeQCmdMsg.Trans, TestPbDir);
    strcat(DeQCmdMsg.Trans, TestPbFile1);

    CF_Test_PrintCmdMsg((void*)&DeQCmdMsg, sizeof(DeQCmdMsg));

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateOnePbPendingQueueEntry(&PbFileCmdMsg);

    pQ = CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].HeadPtr;
    pQ->NodeType = 55;   /* 1 = uplink, 2 = downlink */

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&DeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCnt =
        CF_AppData.Chan[PbFileCmdMsg.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    strcpy(FullSrcFileName, PbFileCmdMsg.SrcFilename);
    sprintf(expEventPb, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,Pre %d,"
            "Peer %s,File %s", PbFileCmdMsg.Class, PbFileCmdMsg.Channel,
            PbFileCmdMsg.Priority, PbFileCmdMsg.Preserve,
            PbFileCmdMsg.PeerEntityId, FullSrcFileName);

    sprintf(expEvent, "Unexpected NodeType %d in Queue node", pQ->NodeType);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, DeleteQueueNodeCmdInvType");

    UtAssert_True(QEntryCnt == 1,
                  "CF_AppPipe, DeleteQueueNodeCmdInvType: QEntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb,
             "CF_AppPipe, DeleteQueueNodeCmdInvType: PbFile Event Sent");

    UtAssert_EventSent(CF_DEQ_NODE_ERR4_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, DeleteQueueNodeCmdInvType: Fail Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdInvLen
 */
void Test_CF_AppPipe_PurgeQueueCmdInvLen(void)
{
    CF_PurgeQueueCmd_t  PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg) + 5, TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Cmd Msg with Bad length Rcvd: ID = 0x%X, CC = %d, "
            "Exp Len = %d, Len = %d", CF_CMD_MID, CF_PURGE_QUEUE_CC,
            sizeof(PurgeQCmdMsg), sizeof(PurgeQCmdMsg) + 5);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdInvLen");

    UtAssert_EventSent(CF_CMD_LEN_ERR_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PurgeQueueCmdInvLen: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdUplinkActiveErr
 */
void Test_CF_AppPipe_PurgeQueueCmdUplinkActiveErr(void)
{
    CF_PurgeQueueCmd_t  PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_INCOMING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = CF_ACTIVEQ;

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s",
            "PurgeQueueCmd Err:Cannot purge Incoming ACTIVE Queue");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdUplinkActiveErr");

    UtAssert_EventSent(CF_PURGEQ_ERR1_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, PurgeQueueCmdUplinkActiveErr: Event Sent");

    CF_ResetEngine();
}


#if 0
/**
 * Test CF_AppPipe, PurgeQueueCmdUpHistory
 */
void Test_CF_AppPipe_PurgeQueueCmdUpHistory(void)
{
    uint32              QEntryCntBefore;
    uint32              QEntryCntAfter;
    CF_PurgeQueueCmd_t  PurgeQCmdMsg;
    CF_NoArgsCmd_t      InPDUMsg1;
    CF_NoArgsCmd_t      InPDUMsg2;
    TRANSACTION         trans;
    char  FullTransString1[OS_MAX_PATH_LEN];
    char  FullDstFileName1[OS_MAX_PATH_LEN];
    char  FullTransString2[OS_MAX_PATH_LEN];
    char  FullDstFileName2[OS_MAX_PATH_LEN];
    char  expEventIn1[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventIn2[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_INCOMING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = CF_HISTORYQ;

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    CFE_SB_InitMsg((void*)&InPDUMsg1, CF_CMD_MID, sizeof(InPDUMsg1), TRUE);
    CFE_SB_InitMsg((void*)&InPDUMsg2, CF_SEND_HK_MID, sizeof(InPDUMsg2), TRUE);

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateTwoUpHistoryQueueEntry((CFE_SB_MsgPtr_t)&InPDUMsg1,
                                            (CFE_SB_MsgPtr_t)&InPDUMsg2);
    QEntryCntBefore = CF_AppData.UpQ[CF_UP_HISTORYQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter = CF_AppData.UpQ[CF_UP_HISTORYQ].EntryCnt;

    sprintf(FullTransString1, "%s%s", TestInSrcEntityId1, "_500");
    cfdp_trans_from_string(FullTransString1, &trans);
    sprintf(FullDstFileName1, "%s%s", TestInDir, TestInFile1);
    sprintf(expEventIn1, "Incoming trans success %d.%d_%d,dest %s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullDstFileName1);

    sprintf(FullTransString2, "%s%s", TestInSrcEntityId2, "_700");
    cfdp_trans_from_string(FullTransString2, &trans);
    sprintf(FullDstFileName2, "%s%s", TestInDir, TestInFile2);
    sprintf(expEventIn2, "Incoming trans success %d.%d_%d,dest %s",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, FullDstFileName2);

    sprintf(expEvent, "PurgeQueueCmd Removed %u Nodes from "
            "Uplink History Queue", 2);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 1) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PurgeQueueCmdUpHistory");

    UtAssert_True((QEntryCntBefore == 2) && (QEntryCntAfter == 0),
                  "CF_AppPipe, PurgeQueueCmdUpHistory: QEntryCnt");

    UtAssert_EventSent(CF_IN_TRANS_OK_EID, CFE_EVS_INFORMATION, expEventIn1,
                  "CF_AppPipe, PurgeQueueCmdUpHistory: Trans#1 Event Sent");

    UtAssert_EventSent(CF_IN_TRANS_OK_EID, CFE_EVS_INFORMATION, expEventIn2,
                  "CF_AppPipe, PurgeQueueCmdUpHistory: Trans#2 Event Sent");

    UtAssert_EventSent(CF_PURGEQ1_EID, CFE_EVS_INFORMATION, expEvent,
                  "CF_AppPipe, PurgeQueueCmdUpHistory: Success Event Sent");

    CF_ResetEngine();
}
#endif


/**
 * Test CF_AppPipe, PurgeQueueCmdUpInvalidQ
 */
void Test_CF_AppPipe_PurgeQueueCmdUpInvalidQ(void)
{
    CF_PurgeQueueCmd_t  PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_INCOMING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = 3;  /* 0=pending,1=active,2=history */

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Queue Param %u in PurgeQueueCmd",
            PurgeQCmdMsg.Queue);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdUpInvalidQ");

    UtAssert_EventSent(CF_PURGEQ_ERR2_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, PurgeQueueCmdUpInvalidQ: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdPbActiveQErr
 */
void Test_CF_AppPipe_PurgeQueueCmdPbActiveQErr(void)
{
    CF_PurgeQueueCmd_t  PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_OUTGOING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = CF_PB_ACTIVEQ;

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "%s",
            "PurgeQueueCmd Err:Cannot purge Outgoing ACTIVE Queue");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdPbActiveQErr");

    UtAssert_EventSent(CF_PURGEQ_ERR3_EID, CFE_EVS_ERROR, expEvent,
             "CF_AppPipe, PurgeQueueCmdPbActiveQErr: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdPbPendingQ
 */
void Test_CF_AppPipe_PurgeQueueCmdPbPendingQ(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_PlaybackFileCmd_t PbFileCmdMsg1;
    CF_PlaybackFileCmd_t PbFileCmdMsg2;
    CF_PurgeQueueCmd_t   PurgeQCmdMsg;
    char  FullSrcFileName1[OS_MAX_PATH_LEN];
    char  FullSrcFileName2[OS_MAX_PATH_LEN];
    char  expEventPb1[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventPb2[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_OUTGOING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = CF_PB_PENDINGQ;

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateTwoPbPendingQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);

    QEntryCntBefore =
        CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter =
        CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_PENDINGQ].EntryCnt;

    sprintf(FullSrcFileName1, "%s%s", TestPbDir, TestPbFile1);
    sprintf(expEventPb1, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,"
            "Pre %d,Peer %s,File %s", PbFileCmdMsg1.Class,
            PbFileCmdMsg1.Channel, PbFileCmdMsg1.Priority,
            PbFileCmdMsg1.Preserve, PbFileCmdMsg1.PeerEntityId,
            FullSrcFileName1);

    sprintf(FullSrcFileName2, "%s%s", TestPbDir, TestPbFile2);
    sprintf(expEventPb2, "Playback File Cmd Rcvd,Cl %d,Ch %d,Pri %d,"
            "Pre %d,Peer %s,File %s", PbFileCmdMsg2.Class,
            PbFileCmdMsg2.Channel, PbFileCmdMsg2.Priority,
            PbFileCmdMsg2.Preserve, PbFileCmdMsg2.PeerEntityId,
            FullSrcFileName2);

    sprintf(expEvent,
            "PurgeQueueCmd Removed %u Nodes from Chan %u,%s Queue",
            2, PurgeQCmdMsg.Chan, "Pending");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 3) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PurgeQueueCmdPbPendingQ");

    UtAssert_True((QEntryCntBefore == 2) && (QEntryCntAfter == 0),
             "CF_AppPipe, PurgeQueueCmdPbPendingQ: PbPendQ EntryCnt");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb1,
             "CF_AppPipe, PurgeQueueCmdPbPendingQ: PbFile1 Event Sent");

    UtAssert_EventSent(CF_PLAYBACK_FILE_EID, CFE_EVS_DEBUG, expEventPb2,
             "CF_AppPipe, PurgeQueueCmdPbPendingQ: PbFile2 Event Sent");

    UtAssert_EventSent(CF_PURGEQ2_EID, CFE_EVS_INFORMATION, expEvent,
             "CF_AppPipe, PurgeQueueCmdPbPendingQ: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdPbHistQ
 */
void Test_CF_AppPipe_PurgeQueueCmdPbHistQ(void)
{
    uint32               QEntryCntBefore;
    uint32               QEntryCntAfter;
    CF_PlaybackFileCmd_t PbFileCmdMsg1;
    CF_PlaybackFileCmd_t PbFileCmdMsg2;
    CF_PurgeQueueCmd_t   PurgeQCmdMsg;
    TRANSACTION          trans;
    char  FullTransString1[OS_MAX_PATH_LEN];
    char  FullTransString2[OS_MAX_PATH_LEN];
    char  FullSrcFileName1[OS_MAX_PATH_LEN];
    char  FullSrcFileName2[OS_MAX_PATH_LEN];
    char  expEventTr1[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEventTr2[CFE_EVS_MAX_MESSAGE_LENGTH];
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_OUTGOING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = CF_PB_HISTORYQ;

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    Ut_CFE_ES_SetFunctionHook(UT_CFE_ES_PUTPOOLBUF_INDEX,
                              (void*)&CFE_ES_PutPoolBufHook);

    /* Execute the function being tested */
    CF_AppInit();

    CF_TstUtil_CreateTwoPbHistoryQueueEntry(&PbFileCmdMsg1, &PbFileCmdMsg2);
    QEntryCntBefore =
        CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_HISTORYQ].EntryCnt;

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    QEntryCntAfter =
        CF_AppData.Chan[PbFileCmdMsg1.Channel].PbQ[CF_PB_HISTORYQ].EntryCnt;

    sprintf(FullTransString1, "%s%s",
            CF_AppData.Tbl->FlightEntityId, "_1");
    cfdp_trans_from_string(FullTransString1, &trans);
    sprintf(FullSrcFileName1, "%s%s", TestPbDir, TestPbFile1);
    sprintf(expEventTr1,
            "Outgoing trans %d.%d_%d %s,CondCode %s,Src %s,Ch %d ",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, "ABANDONED", "NO_ERR", FullSrcFileName1,
            PbFileCmdMsg1.Channel);

    sprintf(FullTransString2, "%s%s",
            CF_AppData.Tbl->FlightEntityId, "_2");
    cfdp_trans_from_string(FullTransString2, &trans);
    sprintf(FullSrcFileName2, "%s%s", TestPbDir, TestPbFile2);
    sprintf(expEventTr2,
            "Outgoing trans %d.%d_%d %s,CondCode %s,Src %s,Ch %d ",
            trans.source_id.value[0], trans.source_id.value[1],
            (int)trans.number, "ABANDONED", "NO_ERR", FullSrcFileName2,
            PbFileCmdMsg2.Channel);

    sprintf(expEvent, "PurgeQueueCmd Removed %u Nodes from "
            "Chan %u,%s Queue", 2, PurgeQCmdMsg.Chan, "History");

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 4) &&
                  (CF_AppData.Hk.ErrCounter == 0),
                  "CF_AppPipe, PurgeQueueCmdPbHistQ");

    UtAssert_True((QEntryCntBefore == 2) && (QEntryCntAfter == 0),
                  "CF_AppPipe, PurgeQueueCmdPbHistQ: QEntryCnt");

    UtAssert_EventSent(CF_OUT_TRANS_FAILED_EID, CFE_EVS_ERROR, expEventTr1,
                  "CF_AppPipe, PurgeQueueCmdPbHistQ: Trans#1 Event Sent");

    UtAssert_EventSent(CF_OUT_TRANS_FAILED_EID, CFE_EVS_ERROR, expEventTr2,
                  "CF_AppPipe, PurgeQueueCmdPbHistQ: Trans#2 Event Sent");

    UtAssert_EventSent(CF_PURGEQ2_EID, CFE_EVS_INFORMATION, expEvent,
             "CF_AppPipe, PurgeQueueCmdPbHistQ: Success Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdPbInvalidQ
 */
void Test_CF_AppPipe_PurgeQueueCmdPbInvalidQ(void)
{
    CF_PurgeQueueCmd_t   PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_OUTGOING;
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = 3;  /* 0=pending,1=active,2=history */

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Queue Param %u in PurgeQueueCmd,Max %u",
            PurgeQCmdMsg.Queue, CF_PB_HISTORYQ);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdPbInvalidQ");

    UtAssert_EventSent(CF_PURGEQ_ERR4_EID, CFE_EVS_ERROR, expEvent,
                       "CF_AppPipe, PurgeQueueCmdPbInvalidQ: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdPbInvalidChan
 */
void Test_CF_AppPipe_PurgeQueueCmdPbInvalidChan(void)
{
    CF_PurgeQueueCmd_t   PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = CF_OUTGOING;
    PurgeQCmdMsg.Chan = 47;
    PurgeQCmdMsg.Queue = CF_PB_PENDINGQ;

    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Chan Param %u in PurgeQueueCmd,Max %u",
            PurgeQCmdMsg.Chan, CF_MAX_PLAYBACK_CHANNELS - 1);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdPbInvalidChan");

    UtAssert_EventSent(CF_PURGEQ_ERR5_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, PurgeQueueCmdPbInvalidChan: Event Sent");

    CF_ResetEngine();
}


/**
 * Test CF_AppPipe, PurgeQueueCmdInvalidType
 */
void Test_CF_AppPipe_PurgeQueueCmdInvalidType(void)
{
    CF_PurgeQueueCmd_t   PurgeQCmdMsg;
    char  expEvent[CFE_EVS_MAX_MESSAGE_LENGTH];

    CFE_SB_InitMsg((void*)&PurgeQCmdMsg, CF_CMD_MID,
                   sizeof(PurgeQCmdMsg), TRUE);
    CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&PurgeQCmdMsg,
                      (uint16)CF_PURGE_QUEUE_CC);
    PurgeQCmdMsg.Type = 3;  /* up=1, down=2 */
    PurgeQCmdMsg.Chan = 0;
    PurgeQCmdMsg.Queue = CF_PB_PENDINGQ;
    CF_Test_PrintCmdMsg((void*)&PurgeQCmdMsg, sizeof(PurgeQCmdMsg));

    /* Execute the function being tested */
    CF_AppInit();

    CF_AppData.MsgPtr = (CFE_SB_MsgPtr_t)&PurgeQCmdMsg;
    CF_AppPipe(CF_AppData.MsgPtr);

    sprintf(expEvent, "Invalid Type Param %u in PurgeQueueCmd,"
            "must be uplink %u or playback %u", PurgeQCmdMsg.Type,
            CF_UPLINK, CF_PLAYBACK);

    /* Verify results */
    UtAssert_True((CF_AppData.Hk.CmdCounter == 0) &&
                  (CF_AppData.Hk.ErrCounter == 1),
                  "CF_AppPipe, PurgeQueueCmdInvalidType");

    UtAssert_EventSent(CF_PURGEQ_ERR6_EID, CFE_EVS_ERROR, expEvent,
                  "CF_AppPipe, PurgeQueueCmdInvalidType: Event Sent");

    CF_ResetEngine();
}



/**************************************************************************
 * Rollup Test Cases
 **************************************************************************/
void CF_Cmds_Test_AddTestCases(void)
{
    UtTest_Add(Test_CF_AppPipe_InvalidCmdMessage,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_InvalidCmdMessage");
    UtTest_Add(Test_CF_AppPipe_InvalidCmdCode,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_InvalidCmdCode");

    UtTest_Add(Test_CF_AppPipe_NoopCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_NoopCmd");
    UtTest_Add(Test_CF_AppPipe_NoopCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_NoopCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_RstCtrsCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_RstCtrsCmd");
    UtTest_Add(Test_CF_AppPipe_RstCtrsCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_RstCtrsCmdInvLen");

    UtTest_Add(Test_CF_AppPipe_PbFileCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmd");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdNoMem,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdNoMem");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdParamErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdParamErr");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdChanNotInUse,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdChanNotInUse");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdInvSrcFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdInvSrcFilename");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdInvDstFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdInvDstFilename");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdPendQFull,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdPendQFull");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdInvPeerId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdInvPeerId");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdFileOpen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdFileOpen");
    UtTest_Add(Test_CF_AppPipe_PbFileCmdFileOnQ,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbFileCmdFileOnQ");

    UtTest_Add(Test_CF_AppPipe_PbDirCmdNoFileSuccess,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdNoFileSuccess");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdOpenErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdOpenErr");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdParamErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdParamErr");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdChanNotInUse,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdChanNotInUse");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdInvSrcPath,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdInvSrcPath");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdInvDstPath,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdInvDstPath");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdInvPeerId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdInvPeerId");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdQFull,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdQFull");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdNoMem,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdNoMem");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdFileOnQ,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdFileOnQ");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdFileOpen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdFileOpen");
    UtTest_Add(Test_CF_AppPipe_PbDirCmdSuccess,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PbDirCmdSuccess");

    UtTest_Add(Test_CF_AppPipe_HousekeepingCmd,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_HousekeepingCmd");

    UtTest_Add(Test_CF_AppPipe_FreezeCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_FreezeCmd");
    UtTest_Add(Test_CF_AppPipe_FreezeCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_FreezeCmdInvLen");

    UtTest_Add(Test_CF_AppPipe_ThawCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_ThawCmd");
    UtTest_Add(Test_CF_AppPipe_ThawCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_ThawCmdInvLen");

    UtTest_Add(Test_CF_AppPipe_SuspendCmdTransId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SuspendCmdTransId");
    UtTest_Add(Test_CF_AppPipe_SuspendCmdTransIdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SuspendCmdTransIdInvLen");
    UtTest_Add(Test_CF_AppPipe_SuspendCmdFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SuspendCmdFilename");
    UtTest_Add(Test_CF_AppPipe_SuspendCmdInvFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SuspendCmdInvFilename");
    UtTest_Add(Test_CF_AppPipe_SuspendCmdUntermTrans,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SuspendCmdUntermTrans");
    UtTest_Add(Test_CF_AppPipe_SuspendCmdAll,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SuspendCmdAll");

    UtTest_Add(Test_CF_AppPipe_ResumeCmdNoTransId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_ResumeCmdNoTransId");
    UtTest_Add(Test_CF_AppPipe_ResumeCmdPbFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_ResumeCmdPbFilename");
    UtTest_Add(Test_CF_AppPipe_ResumeCmdUpTransId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_ResumeCmdUpTransId");
    UtTest_Add(Test_CF_AppPipe_ResumeCmdAll,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_ResumeCmdAll");

    UtTest_Add(Test_CF_AppPipe_SetMibCmdSaveIncompleteFiles,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdSaveIncompleteFiles");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesInvLen");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesUntermParam,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesUntermParam");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesUntermValue,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdSaveIncompleteFilesUntermValue");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdAckLimit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdAckLimit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdAckLimitNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdAckLimitNonDigit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdAckTimeout,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdAckTimeout");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdAckTimeoutNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdAckTimeoutNonDigit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdInactTimeout,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdInactTimeout");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdInactTimeoutNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdInactTimeoutNonDigit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdNakLimit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdNakLimit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdNakLimitNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdNakLimitNonDigit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdNakTimeout,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdNakTimeout");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdNakTimeoutNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdNakTimeoutNonDigit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdFileChunkSize,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdFileChunkSize");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdFileChunkSizeNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdFileChunkSizeNonDigit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdFileChunkOverLimit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdFileChunkOverLimit");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdMyId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdMyId");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdMyIdInvalid,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdMyIdInvalid");
    UtTest_Add(Test_CF_AppPipe_SetMibCmdMyIdNonDigit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetMibCmdMyIdNonDigit");

    UtTest_Add(Test_CF_AppPipe_GetMibCmdSaveIncompleteFiles,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdSaveIncompleteFiles");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesInvLen");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesUntermParam,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesUntermParam");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesInvParam,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdSaveIncompleteFilesInvParam");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdAckLimit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdAckLimit");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdAckTimeout,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdAckTimeout");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdInactTimeout,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdInactTimeout");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdNakLimit,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdNakLimit");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdNakTimeout,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdNakTimeout");
    UtTest_Add(Test_CF_AppPipe_GetMibCmdFileChunkSize,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdFileChunkSize");
#if 0  // core dump(#299)
    UtTest_Add(Test_CF_AppPipe_GetMibCmdMyId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_GetMibCmdMyId");
#endif

    UtTest_Add(Test_CF_AppPipe_SendCfgParamsCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendCfgParamsCmd");
    UtTest_Add(Test_CF_AppPipe_SendCfgParamsCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendCfgParamsCmdInvLen");

    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdCreatErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdCreatErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdUpQValueErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdUpQValueErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdUpDefFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdUpDefFilename");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdUpCustomFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdUpCustomFilename");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdOutQValueErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdOutQValueErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdOutQTypeErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdOutQTypeErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdOutChanErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdOutChanErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdWriteHdrErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdWriteHdrErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdEntryWriteErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdEntryWriteErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdInvFilenameErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdInvFilenameErr");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdOutDefFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdOutDefFilename");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdOutCustomFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdOutCustomFilename");
    UtTest_Add(Test_CF_AppPipe_WriteQueueCmdOneEntry,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteQueueCmdOneEntry");

    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdInvFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdInvFilename");
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdCreatFail,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdCreatFail");
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdWrHdrFail,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdWrHdrFail");
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdInvWhichQs,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdInvWhichQs");
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdEntryWriteErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdEntryWriteErr");
#if 0
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdDefaultFilename,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdDefaultFilename");
    UtTest_Add(Test_CF_AppPipe_WriteActiveTransCmdCustFilename,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_WriteActiveTransCmdCustFilename");
#endif

    UtTest_Add(Test_CF_AppPipe_QuickStatusCmdFilenameNotFound,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_QuickStatusCmdFilenameNotFound");
    UtTest_Add(Test_CF_AppPipe_QuickStatusCmdFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_QuickStatusCmdFilename");

    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdFileNotFound,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdFileNotFound");
    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdTransNotFound,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdTransNotFound");
    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdUntermString,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdUntermString");
    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdInvFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdInvFilename");
    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdFilename");
    UtTest_Add(Test_CF_AppPipe_SendTransDiagCmdTransId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SendTransDiagCmdTransId");

    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvChan,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvChan");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvDir,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvDir");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvClass,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvClass");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvPreserve,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvPreserve");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvSrc,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvSrc");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvDst,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvDst");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmdInvId,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmdInvId");
    UtTest_Add(Test_CF_AppPipe_SetPollParamCmd,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_SetPollParamCmd");

    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdTransUnterm,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdTransUnterm");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdInvFilename,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdInvFilename");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdFileNotFound,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdFileNotFound");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdIdNotFound,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdIdNotFound");
#if 0
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdUpActive,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdUpActive");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdUpHist,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdUpHist");
#endif
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdPbPend,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdPbPend");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdPbActive,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdPbActive");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdPbHist,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdPbHist");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdPutFail,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdPutFail");
    UtTest_Add(Test_CF_AppPipe_DeleteQueueNodeCmdInvType,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_DeleteQueueNodeCmdInvType");

    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdInvLen,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdInvLen");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdUplinkActiveErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdUplinkActiveErr");
#if 0
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdUpHistory,
               CF_Test_SetupUnitTest, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdUpHistory");
#endif
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdUpInvalidQ,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdUpInvalidQ");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdPbActiveQErr,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdPbActiveQErr");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdPbPendingQ,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdPbPendingQ");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdPbHistQ,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdPbHistQ");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdPbInvalidQ,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdPbInvalidQ");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdPbInvalidChan,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdPbInvalidChan");
    UtTest_Add(Test_CF_AppPipe_PurgeQueueCmdInvalidType,
               CF_Test_Setup, CF_Test_TearDown,
               "Test_CF_AppPipe_PurgeQueueCmdInvalidType");
}
