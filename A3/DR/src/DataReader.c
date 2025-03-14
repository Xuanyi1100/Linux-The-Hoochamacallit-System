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
#include <errno.h>

void write_log(const char* message,int DCindex, id_t sender_pid, int status_code, int logtype) {
    // Get current time
    time_t current_time = time(NULL);
    
    // Convert to local time
    struct tm* local_time = localtime(&current_time);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    
    // Open log file in append mode
    FILE* log_file = fopen("/tmp/dataMonitor.log", "a");
    if (log_file != NULL) {
        switch (logtype)
        {
        case 1:
            fprintf(log_file, "[%s]:DC-%d [%d] removed from master list - NON-RESPONSIVE \n", time_str,DCindex, sender_pid);
            break;
        case 2:
            fprintf(log_file, "[%s]:DC-%d [%d] added to the master list - NEW DC - Status 0 (Everything is OKAY) \n", time_str,DCindex, sender_pid);
            break;
        case 3:
            fprintf(log_file, "[%s]:DC-%d [%d] updated in the master list - MSG RECEIVED - Status %d (%s) \n", time_str,DCindex, sender_pid, status_code,message);
            break;
        case 4:
            fprintf(log_file, "[%s]:All DCs have gone offline or terminated - DR TERMINATING \n", time_str);
            break;
        case 5:
            fprintf(log_file, "[%s]:DC-%d [%d] has gone OFFLINE - removing from master-list \n", time_str,DCindex, sender_pid);
            break;
        case 6:
            fprintf(log_file, "[%s]:Message queue deleted - DR TERMINATING \n", time_str);
            break;    
        default:
            fprintf(log_file, "default log \n");
            break;
        }
        
        fclose(log_file);
    }
}

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
    QueueMessage received_msg;
    bool allDCOffLine = false;

    while(!allDCOffLine) 
    {
        
        if((mid = msgget(mskey, 0)) == -1)
        {
            write_log( "", 0 , 0 , 0 , 6);
            break;
        }

        msgrcv(mid, &received_msg, sizeof(QueueMessage) - sizeof(long), 0, 0);

        time_t current_time = time(NULL);

        bool isExisting = false;
        int emptySlot = -1;

        // Search to check if the DC exits
        for (int i = 0; i < MAX_DC_ROLES; i++) {
            // If we find a matching process ID, it's an existing DC
            if (master_list->dc[i].dcProcessID == received_msg.sender_pid) {
                // Update last heard from time
                master_list->dc[i].lastTimeHeardFrom = time(NULL);
                
                // Log the update
                if(received_msg.status_code != 6  )
                {
                    write_log( received_msg.mtext,i +1 , received_msg.sender_pid , received_msg.status_code , 3);               
                }
                
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
                    write_log( received_msg.mtext,i+1 , removed_pid , 0 , 5);
            
                    master_list->numberOfDCs--;
                    if(master_list->numberOfDCs == 0)
                    {
                        allDCOffLine = true;
                        // Log the terminate
                        write_log( "", 1, removed_pid , 0 , 4);
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
                
                // Log the new DC
                write_log( "", emptySlot + 1 , received_msg.sender_pid , 0 , 2);
            } 
            else 
            {
                // No empty slots available
                printf("WARNING: Maximum number of DCs reached. Cannot add DC with PID: %d\n", 
                       received_msg.sender_pid);
            }
        }

        // check if any DC beyond 35 seconds, if so, remove it
        for (int i = 0; i < MAX_DC_ROLES; i++) {
                        
            // Calculate how long since we last heard from this DC
            time_t elapsed = current_time - master_list->dc[i].lastTimeHeardFrom;
        
            // If it's been more than 35 seconds, remove this DC
            if (master_list->dc[i].dcProcessID != 0 && elapsed > 35) {
            
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
                write_log( "", i +1, removed_pid , 0 , 1);
            
                master_list->numberOfDCs--;
                if(master_list->numberOfDCs == 0)
                {
                    allDCOffLine = true;
                    // Log the terminate
                    write_log( "", 1, removed_pid , 0 , 4);
                }
                // Decrement i to recheck the current position which now contains the next element
                i--;
            }            
            
        }        
        sleep(1.5);
    }

    // clean up 
    msgctl(mid, IPC_RMID, NULL);

    // First detach from shared memory
    if (shmdt(master_list) == -1) {   
        exit(EXIT_FAILURE);
    }

    // Then mark the segment for removal
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        exit(EXIT_FAILURE);
    }

    return 0;
}