/*
* FILENAME         : DataCorruptor.c
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-02-28
 * DESCRIPTION      :
 *   This file implements the Data Corruptor (DX) which is responsible for
 *   performing random actions to kill DC processes or to delete the message queue
 *   according to the "Wheel of Destruction".
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <time.h>
#include "../inc/DataCorruptorUtils.h"

int main(void)
{
    key_t shmKey = 0;
    int shmId = 0;
    MasterList* masterList = NULL;
    int retries = 0;

    logEvent("DX Launched", DX_LOG_PATH);
    shmKey = ftok(".", 16535);
    if (shmKey == -1)
    {
        logError("ftok failed",DX_LOG_PATH);
        exit(EXIT_FAILURE);
    }

    // Check for the existence of the shared memory
    while ((shmId = shmget(shmKey, sizeof(MasterList), IPC_PERMISSIONS)) == -1)
    {
        // This re-try will continue until 100 retries have taken place
        if (retries >= MAX_RETRIES)
        {
            logEvent("DX: Shared memory not available after maximum retries",
                     DX_LOG_PATH);
            exit(EXIT_FAILURE);
        }
        // Sleep for 10 seconds and try again
        sleep(10);
        retries++;
    }

    // Attach the shm and check error
    masterList = (MasterList*)shmat(shmId, NULL, 0);
    if (masterList == (void*)-1)
    {
        logError("MasterList shmat failed",DX_LOG_PATH);
        exit(EXIT_FAILURE);
    }

    srand(time(0));
    int loopContinue = 1;

    while (loopContinue)
    {
        int msgQueueId = 0;
        // 1. Sleep for a random amount of time (between 10 and 30 seconds)
        int delay = rand() % (MAX_DELAY - MIN_DELAY + 1) + MIN_DELAY;
        sleep(delay);

        // 2.Check for the existence of the message queue
        msgQueueId = msgget(MSG_KEY, IPC_PERMISSIONS);
        // If msgQueue not exists
        if (msgQueueId == -1)
        {
            // Log the event, detach DX from shared memory and exit
            char logBuffer[LOG_BUFFER_SIZE] = {0};
            snprintf(logBuffer, LOG_BUFFER_SIZE,
                     "DX detected that msgQ is gone – assuming DR/DCs done");
            logEvent(logBuffer, DX_LOG_PATH);
            shmdt(masterList);
            loopContinue = 0;
            break;
        }
        wheelOfDestruction(msgQueueId, masterList);
    }
    logEvent("DX Exits", DX_LOG_PATH);
    return 0;
}
