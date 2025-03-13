//
// The server
// Created by zdong3119 on 01/03/25.
//
#include "../inc/DataReaderUtils.h"

int main(void)
{
    int msgQueueId = 0;
    key_t shmKey = 0;
    int shmId = 0;
    int loopContinue = 1;
    MasterList* masterList = NULL;

    logEvent("Data Reader Lanched",DR_LOG_PATH);
    // Check for the existence of msgQueue
    msgQueueId = msgget(MSG_KEY, 0);
    if (msgQueueId == -1)
    {
        // Create Message Queue
        msgQueueId = msgget(MSG_KEY, IPC_CREAT | IPC_PERMISSIONS);
        if (msgQueueId == -1)
        {
            logError("Create Message Queue Failed", DR_LOG_PATH);
            exit(EXIT_FAILURE);
        }
        // log event
        char logBuffer[LOG_BUFFER_SIZE] = {0};
        snprintf(logBuffer, sizeof(logBuffer),
            "Message Queue[%04d] Created", msgQueueId);
        logEvent(logBuffer, DR_LOG_PATH);
    }

    shmKey = ftok(".", 16535);
    if (shmKey == -1)
    {
        logError("Generate shmKey Failed", DR_LOG_PATH);
        exit(EXIT_FAILURE);
    }

    // Check for the existence of the shm for MasterList
    shmId = shmget(shmKey, sizeof(MasterList), 0);
    if (shmId == -1)
    {
        // Allocate shm
        shmId = shmget(shmKey, sizeof(MasterList), IPC_CREAT | IPC_PERMISSIONS);
        if (shmId == -1)
        {
            logError("MasterList shmget Failed", DR_LOG_PATH);
            exit(EXIT_FAILURE);
        }
        // log event
        char logBuffer[LOG_BUFFER_SIZE] = {0};
        snprintf(logBuffer, sizeof(logBuffer),
            "Shared Memory[%04d] (Master List) Created", shmId);
        logEvent(logBuffer, DR_LOG_PATH);
    }
    // Attach the shm and check error
    masterList = (MasterList*)shmat(shmId, NULL, 0);
    if (masterList == (void*)-1)
    {
        logError("MasterList shmat Failed", DR_LOG_PATH);
        exit(EXIT_FAILURE);
    }

    memset(masterList, 0, sizeof(MasterList));
    masterList->msgQueueID = msgQueueId;
    masterList->numberOfDCs = 0;

    // Wait for launching some DC clients and have them begin to feed the DR with messages
    sleep(WAIT_DC_TIME);

    while (loopContinue)
    {
        // Receive message
        Message msg = {0, 0, 0};
        int ret = msgrcv(msgQueueId, &msg, sizeof(Message) - sizeof(long),
                         0, IPC_NOWAIT);
        if (ret != -1)
        {
            processMessage(&msg, masterList);
        }
        else
        {
            if (errno != ENOMSG)
            {
                logError("msgrcv failed", DR_LOG_PATH);
            }
        }
        // 2. Check if a DC has no message for more than 35 seconds
        checkTimeouts(masterList);

        // 3.2 Check if the number of machines registered in the master list reaches zero
        if (msgget(MSG_KEY, 0) == -1 || masterList->numberOfDCs == 0)
        {
            logEvent("All DCs have gone offline or terminated – DR TERMINATING", DR_LOG_PATH);
            // Remove message queue.
            msgctl(msgQueueId, IPC_RMID, NULL);
            // Release and deallocate the shm
            shmdt(masterList);
            shmctl(shmId, IPC_RMID, NULL);
            loopContinue = 0;
            break;
        }
        // 4. sleep for 1.5seconds
        usleep(LOOP_INTERVAL);
    }
    logEvent("DR Terminated", DR_LOG_PATH);
    return 0;
}
