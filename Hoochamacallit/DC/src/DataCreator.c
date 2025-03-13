/*
 * FILENAME         : DataCreator.c
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-02-28
 * DESCRIPTION      :
 *   This file implements the Data Creator which is responsible for
 *   generating and sending status messages to the Data Reader (DR) via a message queue.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/msg.h>
#include "../../Common/inc/suiteCommon.h"

#define MAX_MSGSND_INTERVAL 30
#define MIN_MSGSND_INTERVAL 10
#define MIN_STATUS 1
#define MAX_STATUS 6

int main(void)
{
    int msgQueueId = 0;
    Message msg = {0, 0, 0};
    int clientId = getpid();
    char logBuffer[LOG_BUFFER_SIZE] = {0};
    int loopContinue = 1; // a flag for the main process loop
    srand(time(0));

    snprintf(logBuffer, LOG_BUFFER_SIZE, "DC [%d] - Launched", clientId);
    logEvent(logBuffer, DC_LOG_PATH);

    // checks for the existence of the message queue
    // If the message queue does not exist, then sleep 10s
    while ((msgQueueId = msgget(MSG_KEY, IPC_PERMISSIONS)) == -1)
    {
        sleep(10);
    }

    // The first message to be sent is 'Everything is OKAY'
    msg.msgType = MSG_TYPE_STATUS;
    msg.clientId = clientId;
    msg.status = 0;
    if (msgsnd(msgQueueId, &msg, sizeof(Message) - sizeof(long), 0) == -1)
    {
        if (errno == EIDRM)
        {
            char errorBuffer[LOG_BUFFER_SIZE] = {0};
            snprintf(errorBuffer, LOG_BUFFER_SIZE,
                "DC [%d]: msgsnd failed (message queue deleted,)", clientId);
            logError(errorBuffer, DC_LOG_PATH);
        }
        else
        {
            char errorBuffer[LOG_BUFFER_SIZE] = {0};
            snprintf(errorBuffer, LOG_BUFFER_SIZE,"DC [%d]: msgsnd failed", clientId);
            logError(errorBuffer, DC_LOG_PATH);
        }
        exit(EXIT_FAILURE);
    }

    snprintf(logBuffer, LOG_BUFFER_SIZE,
             "DC [%d] - MSG SENT - Status %d (%s)",
             clientId, msg.status, statusDesc[msg.status]);
    logEvent(logBuffer, DC_LOG_PATH);

    // Main process loop
    while (loopContinue)
    {
        // Send a message on a random time basis – between 10 and 30 seconds apart
        int delay = rand() % (MAX_MSGSND_INTERVAL- MIN_MSGSND_INTERVAL + 1) + MIN_MSGSND_INTERVAL;
        sleep(delay);

        // Generate a random status 1-6
        int status = rand() % (MAX_STATUS-MIN_STATUS+1) + MIN_STATUS;
        msg.status = status;
        if (msgsnd(msgQueueId, &msg, sizeof(Message) - sizeof(long), 0) == -1)
        {
            if (errno == EIDRM)
            {
                char errorBuffer[LOG_BUFFER_SIZE] = {0};
                snprintf(errorBuffer, LOG_BUFFER_SIZE,
                    "DC [%d]: msgsnd failed (message queue deleted,)", clientId);
                logError(errorBuffer, DC_LOG_PATH);
            }
            else
            {
                char errorBuffer[LOG_BUFFER_SIZE] = {0};
                snprintf(errorBuffer, LOG_BUFFER_SIZE,"DC [%d]: msgsnd failed", clientId);
                logError(errorBuffer, DC_LOG_PATH);
            }
            exit(EXIT_FAILURE);
        }
        snprintf(logBuffer, LOG_BUFFER_SIZE, "DC [%d] - MSG SENT - Status %d (%s)",
                 clientId, status, statusDesc[status]);
        logEvent(logBuffer,DC_LOG_PATH);

        // Go offline if status is 6
        if (status == MACHINE_OFFLINE)
        {
            loopContinue = 0;
        }
    }
    snprintf(logBuffer, LOG_BUFFER_SIZE, "DC [%d] - Terminated", clientId);
    logEvent(logBuffer, DC_LOG_PATH);

    return 0;
}
