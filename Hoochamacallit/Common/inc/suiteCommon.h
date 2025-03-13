/*
 * FILENAME         : suiteCommon.h
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-03-02
 * DESCRIPTION      :
 *   This header file declares common definitions, structures, and function prototypes
 *   shared across the Hoochamacallit project.
 */


#ifndef SUITECOMMON_H
#define SUITECOMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#define IPC_PERMISSIONS 0660
#define TIME_STR_SIZE 32
#define LOG_BUFFER_SIZE 256
#define DC_LOG_PATH "/tmp/dataCreator.log"
#define DR_LOG_PATH "/tmp/dataMonitor.log"
#define DX_LOG_PATH "/tmp/dataCorruptor.log"

#define MSG_KEY 1234                        // Key of the message queue
#define MSG_TYPE_STATUS 1                   // Message Type
#define MAX_DC_ROLES 10

typedef enum StatusCode
{
    OKAY = 0,
    HYDRAULIC_PRESSURE_FAILURE = 1,
    SAFETY_BUTTON_FAILURE = 2,
    NO_RAW_MATERIAL = 3,
    TEMP_OUT_OF_RANGE = 4,
    OPERATOR_ERROR = 5,
    MACHINE_OFFLINE = 6
} StatusCode;

typedef struct
{
    long msgType;
    int clientId;
    int status;
} Message;

typedef struct
{
    pid_t dcProcessID;
    time_t lastTimeHeardFrom; // time_t to store time
} DCInfo;

typedef struct
{
    int msgQueueID;
    int numberOfDCs;
    DCInfo dc[MAX_DC_ROLES];
} MasterList;

extern const char* statusDesc[];

void logError(const char* prefix, const char* logPath);
void logEvent(const char* msg, const char* logPath);

#endif //SUITECOMMON_H
