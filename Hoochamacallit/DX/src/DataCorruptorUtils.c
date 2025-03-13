/*
 * FILENAME         : DataCorruptorUtils.c
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-03-02
 * DESCRIPTION      :
 *   This file implements functions for the Data Corruptor (DX).
 *   It includes the implementation of functions to send signals to terminate DC processes
 *   and to execute the Wheel of Destruction actions.
 */
#include "../inc/DataCorruptorUtils.h"
void killDc(WodAction action, int dcIndex, const MasterList* masterList);

/*
 * FUNCTION     : killDc
 * DESCRIPTION  : Sends a SIGHUP signal to the DC process identified by its index in the master list.
 * PARAMETERS   :
 *    WodAction action        : The specific action from the Wheel of Destruction.
 *    int dcIndex             : Index of the DC in the master list.
 *    const MasterList* masterList : Pointer to the master list containing DC process information.
 * RETURNS      : void
 */
void killDc(WodAction action, int dcIndex, const MasterList* masterList)
{
    char logBuffer[LOG_BUFFER_SIZE] = {0};
    if (masterList->numberOfDCs >= dcIndex + 1)
    {
        pid_t pid = masterList->dc[dcIndex].dcProcessID;
        if (kill(pid, SIGHUP) == -1)
        {
            // if errno == ESRCH，then the process can not be found
            if (errno == ESRCH)
            {
                snprintf(logBuffer, LOG_BUFFER_SIZE,
                         "WOD action %02d - DC-%02d not found (already exited)", action, dcIndex + 1);
                logEvent(logBuffer,DX_LOG_PATH);
            }
            else
            {
                perror("DX: kill failed");
                logError("DX: kill failed", DX_LOG_PATH);
            }
        }
        else
        {
            snprintf(logBuffer, LOG_BUFFER_SIZE,
                     "WOD action %02d - DC-%02d [%d] TERMINATED", action, dcIndex + 1, pid);
            logEvent(logBuffer, DX_LOG_PATH);
        }
    }
}

/*
 * FUNCTION     : wheelOfDestruction
 * DESCRIPTION  :
 *  Performs a randomly selected destructive action
 *  such as killing a DC process or deleting the message queue.
 * PARAMETERS   :
 *    int msgQueueId                : The ID of the message queue used for communication.
 *    const MasterList* masterList  : Pointer to the master list containing DC process information.
 * RETURNS      : void
 */
void wheelOfDestruction(int msgQueueId, const MasterList* masterList)
{
    char logBuffer[LOG_BUFFER_SIZE];
    int dcIndex = -1;
    WodAction action = rand() % (MAX_ACTION + 1);

    switch (action)
    {
    // Do nothing cases:
    case DO_NOTHING_1:
    case DO_NOTHING_2:
    case DO_NOTHING_3:
        snprintf(logBuffer, LOG_BUFFER_SIZE,
                 "WOD Action %02d - do nothing", action);
        logEvent(logBuffer, DX_LOG_PATH);
        break;

    // Delete message queue cases:
    case DELETE_MSG_QUEUE_A:
    case DELETE_MSG_QUEUE_B:
        if (msgctl(msgQueueId, IPC_RMID, NULL) == -1)
        {
            logError("msgctl (delete queue) failed", DX_LOG_PATH);
        }
        else
        {
            snprintf(logBuffer, LOG_BUFFER_SIZE,
                     "WOD Action %02d - deleted the message queue", action);
            logEvent(logBuffer, DX_LOG_PATH);
        }
        break;

    case KILL_DC_01_A:
    case KILL_DC_01_B:
    case KILL_DC_01_C:
        dcIndex = 0;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_02_A:
    case KILL_DC_02_B:
    case KILL_DC_02_C:
        dcIndex = 1;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_03_A:
    case KILL_DC_03_B:
    case KILL_DC_03_C:
        dcIndex = 2;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_04:
        dcIndex = 3;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_05:
        dcIndex = 4;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_06:
        dcIndex = 5;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_07:
        dcIndex = 6;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_08:
        dcIndex = 7;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_09:
        dcIndex = 8;
        killDc(action, dcIndex, masterList);
        break;

    case KILL_DC_10:
        dcIndex = 9;
        killDc(action, dcIndex, masterList);
        break;

    default:
        snprintf(logBuffer, LOG_BUFFER_SIZE,
                 "WOD action %02d - Unrecognized action", action);
        logEvent(logBuffer, DX_LOG_PATH);
        break;
    }
}
