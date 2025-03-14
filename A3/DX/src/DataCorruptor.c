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
#include <signal.h>

void write_log(int WODIndex, int dc_num, id_t sender_pid, int logtype) {
    // Get current time
    time_t current_time = time(NULL);
    
    // Convert to local time
    struct tm* local_time = localtime(&current_time);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    
    // Open log file in append mode
    FILE* log_file = fopen("/tmp/dataCorruptor.log", "a");
    if (log_file != NULL) {
        switch (logtype)
        {
        case 1:
            fprintf(log_file, "[%s]:WOD Action %d - DC-%d [%d] TERMINATED \n", time_str, WODIndex, dc_num, sender_pid);
            break;
        case 2:
            fprintf(log_file, "[%s]:DX deleted the msgQ - the DR/DCs can't talk anymore - exiting \n", time_str);
            break;
        case 3:
            fprintf(log_file, "[%s]:DX detected that msgQ is gone - assuming DR/DCs done \n", time_str);
            break;    
        default:
            fprintf(log_file, "default log \n");
            break;
        }
        
        fclose(log_file);
    }
}

// Function to kill a specific DC
void kill_dc(int wod_index, int dc_num, MasterList* master_list) {
    // Validate DC number 
    if (dc_num < 1 || dc_num > 10) {
        printf("Invalid DC number: %d\n", dc_num);
        return;
    }
    
    // Array index is dc_num - 1 (if DC numbers start at 1)
    int index = dc_num - 1;
    
    // Check if DC exists (has a valid PID)
    if (master_list->dc[index].dcProcessID <= 0) {
        printf("DC%d does not exist or has invalid PID\n", dc_num);
        return;
    }
    
    // Get the process ID
    pid_t target_pid = master_list->dc[index].dcProcessID;
    
    // Send SIGHUP signal to the process

    if (kill(target_pid, SIGHUP) == -1) {
        perror("Failed to send SIGHUP to process");
        return;
    }
    write_log(wod_index, dc_num, target_pid, 1 );  
    
}

// Function to delete the message queue
void delete_message_queue() {
    // Generate the message queue key (same as used by DR and DCs)
    key_t mskey = ftok(PATH_TO_KEY, PROJECT_ID);
    if (mskey == -1) {
        perror("Failed to generate message queue key");
        return;
    }
    
    // Get message queue ID
    int msqid = msgget(mskey, 0);
    if (msqid == -1) {
        perror("Failed to get message queue ID");
        return;
    }
    
    // Delete the message queue
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("Failed to delete message queue");
        return;
    }
    
    printf("Message queue successfully deleted\n");
}

void execute_wod_action(int wod_index, const char* action, MasterList* master_list) {
    // Check for "do nothing" action
    if (strcmp(action, "do nothing") == 0) {
        printf("Wheel says: Do nothing. Continuing normally.\n");
        return;
    }
    
    // Check for "kill DC" action
    if (strncmp(action, "kill DC", 7) == 0) {
        // Extract DC number (assumes format "kill DC X if it exists")
        int dc_num = action[9] - '0';  // Convert character to integer
        if(dc_num == 0)
        {
            dc_num = 10;
        }

        printf("Wheel says: Attempting to kill DC%d if it exists\n", dc_num);

        kill_dc(wod_index, dc_num, master_list);
        
        return;
    }
    
    // Check for "delete the message queue" action
    if (strncmp(action, "delete the message queue", 23) == 0) {
        printf("Wheel says: Deleting the message queue\n");
        delete_message_queue();
        write_log(wod_index, 0, 0, 2 );
        // Detach itself
        if (shmdt(master_list) == -1) {
            perror("shmdt failed");
        }
        exit(1);
    }
    
    // Unknown action
    printf("Unknown action: %s\n", action);
}

int main() {

    // Initialize random seed 
    srand(time(NULL));
    
    // Generate EXACT SAME key as DataReader
    key_t shmKey = ftok(PATH_TO_SM_KEY, SM_ID);
    if (shmKey == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    
    // Get shared memory ID (no creation)
    int shmid = 0;
    int tryCount = 0;
    while( (shmid = shmget(shmKey, sizeof(MasterList), 0))== -1)
    {
        sleep(10);
        tryCount ++;
        if(tryCount == 100)
        {
            perror("fthok failed");
            exit(EXIT_FAILURE);
        }
    }
    
    // Attach to shared memory (read-only is safer)
    MasterList *master_list = (MasterList *)shmat(shmid, NULL, SHM_RDONLY);
    if (master_list == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    mskey = ftok(PATH_TO_KEY, PROJECT_ID);
    int mid = 0;
    int sleeptime;
    while(1){
        
        sleeptime = rand()%21 + 10;
        sleep(sleeptime);
        // check if message queue exists
        if((mid= msgget(mskey, 0)) == -1)
        {
            write_log(0, 0, 0, 3 );
            // Detach itself
            if (shmdt(master_list) == -1) {
                perror("shmdt failed");
            }
            exit(1);
        }

        int wod_index = rand()%WHEEL_OF_DESTRUCTION_SIZE;
        const char* selected_action = WheelOfDestruction[wod_index];

        // Execute the selected action
        execute_wod_action(wod_index,selected_action, master_list);
       
        // List all active DCs
        for (int i = 0; i < master_list->numberOfDCs; i++) {
            printf("DC #%d: PID=%d, Last active: %s", i +1, 
               master_list->dc[i].dcProcessID,
               ctime(&master_list->dc[i].lastTimeHeardFrom));
        }
        printf("-----\n");
        sleep(5);
    }
    
    return 0;
}