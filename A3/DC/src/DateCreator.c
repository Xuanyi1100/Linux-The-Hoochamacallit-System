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
        perror("Message send failed");
        exit(EXIT_FAILURE);
    }
    printf("Sent: %s\n", msg.mtext);    

    int sleeptime = 0;
    // main processing loop
    while(msg.status_code != 6)
    { 
        sleeptime = rand()%21 + 10;
        sleep(sleeptime);
        msg.status_code = rand() % 7;  // Gives a random number between 0 and 6 
        strncpy(msg.mtext, status[msg.status_code], sizeof(msg.mtext));
        
        // Send message
        if(msgsnd(message_queue_identifier, &msg, sizeof(msg.sender_pid) + sizeof(msg.mtext) + sizeof(msg.status_code), 0) == -1) {
            perror("Message send failed");
            exit(EXIT_FAILURE);
        }
        //printf("Sent: %s\n", msg.mtext);
    }
    
    return 0;
}

