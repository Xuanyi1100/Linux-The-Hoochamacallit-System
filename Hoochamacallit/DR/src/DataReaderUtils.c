/*
 * FILENAME         : DataReaderUtils.c
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-02-28
 * DESCRIPTION      :
 *   This file implements functions for the Data Reader (DR).
 *   It includes the function to process messages from DCs and check for timeouts
 *   to remove non-responsive DCs from the master list.
 */
#include "../inc/DataReaderUtils.h"

/*
 * FUNCTION     : processMessage
 * DESCRIPTION  :
 *    This function examines the incoming message,
 *    adds a new DC to the master list if not present,
 *    updates an existing DC's last-heard timestamp,
 *    handles the case where a DC sends a MACHINE_OFFLINE status and removes it from the master list.
 * PARAMETERS   :
 *    const Message* msg     : Pointer to the incoming message.
 *    MasterList* masterList : Pointer to the master list structure.
 * RETURNS      : void
 */
void processMessage(const Message* msg, MasterList* masterList)
{
    char logBuffer[LOG_BUFFER_SIZE] = {0};
    int isFound = 0;
    int index = -1;

    // 1. Check if the clientId is stored in MasterList
    for (int i = 0; i < masterList->numberOfDCs; i++)
    {
        if (masterList->dc[i].dcProcessID == msg->clientId)
        {
            isFound = 1;
            index = i;
            break;
        }
    }
    if (!isFound)
    {
        // Record a new DC in MasterList
        if (masterList->numberOfDCs < MAX_DC_ROLES)
        {
            index = masterList->numberOfDCs;
            masterList->dc[index].dcProcessID = msg->clientId;
            masterList->dc[index].lastTimeHeardFrom = time(0);
            masterList->numberOfDCs++;
            // Log
            snprintf(logBuffer, sizeof(logBuffer),
                     "DC-%02d [%d] added to master list – NEW DC – Status %d (%s)",
                     index + 1, msg->clientId, msg->status, statusDesc[msg->status]);
            logEvent(logBuffer, DR_LOG_PATH);
        }
    }
    else
    {
        // Update last time heard from the DC
        masterList->dc[index].lastTimeHeardFrom = time(0);
        snprintf(logBuffer, sizeof(logBuffer),
                 "DC-%02d [%d] updated in master list – MSG RECEIVED – Status %d (%s)",
                 index + 1, msg->clientId, msg->status, statusDesc[msg->status]);
        logEvent(logBuffer, DR_LOG_PATH);
    }
    // 3.1 DC Machine is off-line
    if (msg->status == MACHINE_OFFLINE)
    {
        snprintf(logBuffer, sizeof(logBuffer),
                 "DC-%02d [%d] has gone OFFLINE",
                 index + 1, msg->clientId);
        logEvent(logBuffer, DR_LOG_PATH);
        // Remove the DC，shift the following elements one position forward.
        for (int i = index; i < masterList->numberOfDCs - 1; i++)
        {
            masterList->dc[i] = masterList->dc[i + 1];
        }
        masterList->numberOfDCs--;
    }
}
/*
 * FUNCTION     : checkTimeouts
 * DESCRIPTION  :
 *    Scans the master list for DC entries that have not communicated within the
 *    allowed timeout period and removes them.
 * PARAMETERS   :
 *    MasterList* masterList : Pointer to the master list structure.
 * RETURNS      : void
 */
void checkTimeouts(MasterList* masterList)
{
    for (int i = 0; i < masterList->numberOfDCs;)
    {
        if (difftime(time(0), masterList->dc[i].lastTimeHeardFrom) > 35)
        {
            char logBuffer[LOG_BUFFER_SIZE] = {0};
            snprintf(logBuffer, sizeof(logBuffer),
                     "DC-%02d [%d] removed from master list – NON-RESPONSIVE",
                     i + 1, masterList->dc[i].dcProcessID);
            logEvent(logBuffer, DR_LOG_PATH);

            // Remove the DC，shift the following elements one position forward.
            for (int j = i; j < masterList->numberOfDCs - 1; j++)
            {
                masterList->dc[j] = masterList->dc[j + 1];
            }
            masterList->numberOfDCs--;
        }
        else
        {
            i++;
        }
    }
}
