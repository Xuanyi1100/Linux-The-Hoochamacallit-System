#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#define PATH_TO_KEY "/tmp"
#define PROJECT_ID 'A'
#define PERMISSIONS 0660
#define MAX_DC_ROLES 10

#define PATH_TO_SM_KEY "/tmp"
#define SM_ID 16535

key_t mskey; 
key_t shmKey;

typedef struct {
    long mtype;          // Message type (must be > 0)
    int status_code;
    id_t sender_pid;    // Process ID of the sender
    char mtext[256];     // Message content
} QueueMessage;

const char* status[] = {
    "Everything is OKAY",
    "Hydraulic Pressure Failure", 
    "Safety Button Failure", 
    "No Raw Material in the Process", 
    "Operating Temperature Out of Range", 
    "Operator Error",
    "Machine is Off-line"
};

const char* WheelOfDestruction[] = {
    "do nothing",
    "kill DC-01 (if it exists)",
    "kill DC-03 (if it exists)",
    "kill DC-02 (if it exists)",
    "kill DC-01 (if it exists)",
    "kill DC-03 (if it exists)",
    "kill DC-02 (if it exists)",
    "kill DC-04 (if it exists)",
    "do nothing",
    "kill DC-05 (if it exists)",
    "kill DC-10 (if it exists)",
    "kill DC-01 (if it exists)",
    "kill DC-06 (if it exists)",
    "kill DC-02 (if it exists)",
    "kill DC-07 (if it exists)",
    "kill DC-03 (if it exists)",
    "kill DC-08 (if it exists)",
    "delete the message queue being used between DCs and DR1",
    "kill DC-09 (if it exists)",
    "do nothing",
    "kill DC-10 (if it exists)"
};

#define WHEEL_OF_DESTRUCTION_SIZE 21

typedef struct
{
    pid_t dcProcessID;
    time_t lastTimeHeardFrom;
} DCInfo;

typedef struct
{
    int msgQueueID;
    int numberOfDCs;
    DCInfo dc[MAX_DC_ROLES];
} MasterList;