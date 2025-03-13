/*
 * FILENAME         : DataReaderUtils.h
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-02-28
 * DESCRIPTION      :
 *   This header file declares definitions and functions for the Data Reader (DR).
 */

#ifndef DATAREADERUTILS_H
#define DATAREADERUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include "../../Common/inc/suiteCommon.h"
#define WAIT_DC_TIME 15         // seconds
#define LOOP_INTERVAL 1500000   // microseconds

void processMessage(const Message* msg, MasterList* masterList);
void checkTimeouts(MasterList* masterList);

#endif //DATAREADERUTILS_H
