//

#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <stdio.h>  
#include <stdlib.h>
#include "msg_queue.h"
#include <unistd.h>
#include <string.h>
#include <time.h>

void write_log(const char* message, id_t sender_pid, int status_code) {
    // Get current time
    time_t current_time = time(NULL);
    
    // Convert to local time
    struct tm* local_time = localtime(&current_time);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    
    // Open log file in append mode
    FILE* log_file = fopen("/tmp/dataCreator.log", "a");
    if (log_file != NULL) {
        // Write timestamp and message
        fprintf(log_file, "[%s]:DC [%d] - MSG SENT - Status %d (%s) \n", time_str,sender_pid, status_code,message);
        fclose(log_file);
    }
}

int main()
{
    // Each process gets a unique seed based on time + PID
    srand(time(NULL) ^ getpid());

    mskey = ftok(PATH_TO_KEY, PROJECT_ID);
    int message_queue_identifier = 0;
    // check if message queue exits
    while((message_queue_identifier = msgget(mskey, 0)) == -1)
    {
        sleep(10);
    }
    
    QueueMessage msg;
    msg.sender_pid = getpid();  // Set the sender's PID
    msg.mtype = 1;
    
    // send for first time
    msg.status_code = 0;  // Gives a random number between 0 and 6 
    strncpy(msg.mtext, status[0], sizeof(msg.mtext));
    if(msgsnd(message_queue_identifier, &msg, sizeof(msg.sender_pid) + sizeof(msg.mtext) + sizeof(msg.status_code), 0) == -1) 
    {
        exit(EXIT_FAILURE);
    }
    write_log(msg.mtext, msg.sender_pid,msg.status_code);    

    int sleeptime = 0;
    // main processing loop
    while(msg.status_code != 6)
    {        
        sleeptime = rand()%21 + 10;
        sleep(sleeptime);
        msg.status_code = rand() % 7;  // Gives a random number between 0 and 6 
        strncpy(msg.mtext, status[msg.status_code], sizeof(msg.mtext));
        
        if((message_queue_identifier = msgget(mskey, 0)) == -1)
        {
            break;
        }

        // Send message
        if(msgsnd(message_queue_identifier, &msg, sizeof(msg.sender_pid) + sizeof(msg.mtext) + sizeof(msg.status_code), 0) == -1) {
            exit(EXIT_FAILURE);
        }
        write_log(msg.mtext, msg.sender_pid,msg.status_code);   
        
    }
    
    return 0;
}

