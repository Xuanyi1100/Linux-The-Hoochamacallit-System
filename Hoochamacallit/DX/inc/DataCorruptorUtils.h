/*
 * FILENAME         : DataCorruptorUtils.h
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-03-02
 * DESCRIPTION      :
 *   This header file declares functions and enums for the Data Corruptor (DX).
 */

#ifndef DATACORRUPTORUTILS_H
#define DATACORRUPTORUTILS_H
#include "../../Common/inc/suiteCommon.h"
#include <signal.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define MAX_ACTION 20
#define MAX_RETRIES 100
#define MAX_DELAY 30
#define MIN_DELAY 10

typedef enum
{
    DO_NOTHING_1 = 0,
    KILL_DC_01_A = 1,
    KILL_DC_03_A = 2,
    KILL_DC_02_A = 3,
    KILL_DC_01_B = 4,
    KILL_DC_03_B = 5,
    KILL_DC_02_B = 6,
    KILL_DC_04 = 7,
    DO_NOTHING_2 = 8,
    KILL_DC_05 = 9,
    DELETE_MSG_QUEUE_A = 10,
    KILL_DC_01_C = 11,
    KILL_DC_06 = 12,
    KILL_DC_02_C = 13,
    KILL_DC_07 = 14,
    KILL_DC_03_C = 15,
    KILL_DC_08 = 16,
    DELETE_MSG_QUEUE_B = 17,
    KILL_DC_09 = 18,
    DO_NOTHING_3 = 19,
    KILL_DC_10 = 20
} WodAction;

void wheelOfDestruction(int msgQueueId, const MasterList* masterList);

#endif //DATACORRUPTORUTILS_H
