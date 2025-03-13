/*
 * FILENAME         : suiteCommon.c
 * PROJECT          : Hoochamacallit
 * PROGRAMMER       : Deyi Zou, Zhizheng Dong
 * FIRST VERSION    : 2025-03-02
 * DESCRIPTION      :
 *   This file implements common utility functions for the Hoochamacallit project,
 *   primarily providing thread-safe logging functions (logEvent and logError) using file locking.
 */
#include "../inc/suiteCommon.h"
#include <fcntl.h>
#include <unistd.h>

const char* lockfile = "/tmp/loglock.lck";
const int lockWaitInterval = 100000;
const char* statusDesc[] = {
    "Everything is OKAY",
    "Hydraulic Pressure Failure",
    "Safety Button Failure",
    "No Raw Material in the Process",
    "Operating Temperature Out of Range",
    "Operator Error",
    "Machine is Off-line"
};
/*
 * FUNCTION     : logError
 * DESCRIPTION  :
 *    Logs an error message by appending the system error message to the provided prefix.
 * PARAMETERS   :
 *    const char* prefix   : A prefix message indicating the error context.
 *    const char* logPath  : The path to the log file.
 * RETURNS      : void
 */
void logError(const char* prefix, const char* logPath)
{
    char errMsg[LOG_BUFFER_SIZE];
    snprintf(errMsg, LOG_BUFFER_SIZE, "%s: %s", prefix, strerror(errno));
    logEvent(errMsg, logPath);
}

/*
 * FUNCTION     : logEvent
 * DESCRIPTION  : Writes a log entry to the specified log file.
 * PARAMETERS   :
 *    const char* msg      : The log message to record.
 *    const char* filePath : The path to the log file.
 * RETURNS      : void
 */
void logEvent(const char* msg, const char* filePath)
{
    // add a lock for the log file
    int fd = 0;
    while (1)
    {
        fd = open(lockfile, O_WRONLY | O_CREAT | O_EXCL);
        if (fd != -1) break;
        usleep(lockWaitInterval);
    }
    close(fd);

    // Manipulate the log file
    FILE* fp = fopen(filePath, "a");
    if (fp == NULL)
    {
        remove(lockfile);
        return;
    }
    time_t now = time(0);
    struct tm* localTime = localtime(&now);
    char timeStr[TIME_STR_SIZE] = {0};
    // format time 2020-03-06 21:05:07
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localTime);
    fprintf(fp, "[%s] : %s\n", timeStr, msg);
    fclose(fp);
    remove(lockfile);
}

