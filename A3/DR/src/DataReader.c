#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "msg_queue.h"
#include <string.h>
#include <stdio.h>  
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <unistd.h>


int main(void)
{
    // create message queue
    mskey = ftok(PATH_TO_KEY, PROJECT_ID);
    if (mskey == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    int mid = 0;
    if ((mid = msgget(mskey, 0)) == -1)
    { 
        mid = msgget(mskey, IPC_CREAT | 0660);
        if (mid == -1)
        { 
            perror("msgget creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // create enough space in shared memory for the master list
    shmKey = ftok(PATH_TO_SM_KEY, SM_ID);
    if (shmKey == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    int shmid; 
    if ((shmid = shmget (shmKey,  sizeof(MasterList), 0)) == -1) {
        shmid = shmget (shmKey,  sizeof(MasterList), IPC_CREAT | 0644);
        if (shmid == -1) {
            perror("shmget creation failed");
            exit(EXIT_FAILURE);
        }
    }
    // Attach and cast to the correct type
    MasterList * master_list = (MasterList *)shmat(shmid, NULL, 0);
    if (master_list == (MasterList *)-1) {
        perror("shmat failed");
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    memset(master_list, 0, sizeof(MasterList));  // Clear all memory
    master_list->msgQueueID = mid;
    master_list->numberOfDCs = 0;                           


    sleep(15);
    
    bool allDCOffLine = false;
    while(!allDCOffLine) 
    {
        QueueMessage received_msg;

        if(msgrcv(mid, &received_msg, sizeof(QueueMessage) - sizeof(long), 0, 0) == -1) {
            perror("Message receive failed");
            msgctl(mid, IPC_RMID, NULL);  // Clean up
            break;  // Exit loop on error
        }

        printf("From PID: %d\n", received_msg.sender_pid);
        printf("Status: %s\n", received_msg.mtext);
        time_t current_time = time(NULL);
        printf("Timestamp: %s", ctime(&current_time));
        printf("--------------------------\n");

        bool isExisting = false;
        int emptySlot = -1;

        // Search to check if the DC exits
        for (int i = 0; i < MAX_DC_ROLES; i++) {
            // If we find a matching process ID, it's an existing DC
            if (master_list->dc[i].dcProcessID == received_msg.sender_pid) {
                // Update last heard from time
                master_list->dc[i].lastTimeHeardFrom = time(NULL);
                
                printf("Updated existing DC (PID: %d), last active: %s", 
                       received_msg.sender_pid, ctime(&master_list->dc[i].lastTimeHeardFrom));                
                isExisting = true;
                
                // if off-line, remove it
                if(strcmp(received_msg.mtext, "Machine is Off-line") == 0)
                {
                    // Store the PID for logging
                    pid_t removed_pid = master_list->dc[i].dcProcessID;
            
                    // Shift all subsequent elements up by one position
                    for (int j = i; j < master_list->numberOfDCs - 1; j++) {
                        // Copy the next element to the current position
                        master_list->dc[j] = master_list->dc[j + 1];
                    }
            
                    // Clear the last element (now duplicated)
                    memset(&master_list->dc[master_list->numberOfDCs - 1], 0, 
                   sizeof(master_list->dc[0]));
            
                    // Log the removal
                    printf("Removed inactive DC (PID: %d) from slot %d\n", removed_pid, i);
            
                    master_list->numberOfDCs--;
                    if(master_list->numberOfDCs == 0)
                    {
                        allDCOffLine = true;
                    }
                }
            }
            
            // Keep track of the first empty slot for potential new DC
            if (emptySlot == -1 && master_list->dc[i].dcProcessID == 0) {
                emptySlot = i;
                break;
            }            
            
        }

        // If it's a new DC and we have space, add it to the master list
        if (!isExisting) {
            if (emptySlot != -1) {
                // Add this DC to master list in the empty slot
                master_list->dc[emptySlot].dcProcessID = received_msg.sender_pid;
                master_list->dc[emptySlot].lastTimeHeardFrom = time(NULL);
              
                // Increment active DC count if you're tracking it
                master_list->numberOfDCs++;
                
                printf("Added new DC (PID: %d) in slot %d, last active: %s", 
                       received_msg.sender_pid, emptySlot, 
                       ctime(&master_list->dc[emptySlot].lastTimeHeardFrom));
            } else {
                // No empty slots available
                printf("WARNING: Maximum number of DCs reached. Cannot add DC with PID: %d\n", 
                       received_msg.sender_pid);
            }
        }

        // check if any DC beyond 35 seconds, if so, remove it
        for (int i = 0; i < MAX_DC_ROLES; i++) {
            
            printf("DC %d , identifier :%d \n", i,master_list->dc[i].dcProcessID);
            
            // Calculate how long since we last heard from this DC
            time_t elapsed = current_time - master_list->dc[i].lastTimeHeardFrom;
        
            // If it's been more than 35 seconds, remove this DC
            if (master_list->dc[i].dcProcessID != 0 && elapsed > 35) {
                printf("DC with PID %d inactive for %ld seconds - removing\n", 
                master_list->dc[i].dcProcessID, elapsed);
            
                // Store the PID for logging
                pid_t removed_pid = master_list->dc[i].dcProcessID;
            
                // Shift all subsequent elements up by one position
                for (int j = i; j < master_list->numberOfDCs - 1; j++) {
                    // Copy the next element to the current position
                    master_list->dc[j] = master_list->dc[j + 1];
                }
            
                // Clear the last element (now duplicated)
                memset(&master_list->dc[master_list->numberOfDCs - 1], 0, 
                   sizeof(master_list->dc[0]));
            
                // Log the removal
                printf("Removed inactive DC (PID: %d) from slot %d\n", removed_pid, i);
            
                master_list->numberOfDCs--;
                if(master_list->numberOfDCs == 0)
                {
                    allDCOffLine = true;
                }
                // Decrement i to recheck the current position which now contains the next element
                i--;
            }            
            
        }        
        sleep(1.5);
    }

    // clean up 
    if (msgctl(mid, IPC_RMID, NULL) == -1) {
        perror("Message queue removal failed");
    } else {
         printf("Message queue successfully removed\n");
    }

    // First detach from shared memory
    if (shmdt(master_list) == -1) {   
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    // Then mark the segment for removal
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl removal failed");
        exit(EXIT_FAILURE);
    }
    printf("Shared memory segment removed\n");

    return 0;
}