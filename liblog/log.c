/* ===================================================================
 * Copyright (c) 2005-2019 Vadim Druzhin (cdslow@mail.ru).
 * 
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ===================================================================
 */

#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include "log.h"

static char *LogPrefix="cdslow";
static char *LogSuffix=".log";
static char *LogName;
static FILE *Log=NULL;

void WriteLog(char *format, ...)
    {
    va_list va;
    SYSTEMTIME st;

    if(NULL==Log)
        return;

    GetLocalTime(&st);
    fprintf(Log, "%02u:%02u:%02u.%03u ",
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    va_start(va, format);
    vfprintf(Log, format, va);
    va_end(va);

    fflush(Log);
    }

int InitLog(char *prefix, char *suffix)
    {
    int i;
    int fd=-1;

    if(NULL!=LogName)
        {
        free(LogName);
        LogName=NULL;
        }
    if(NULL!=Log)
        {
        fclose(Log);
        Log=NULL;
        }

    if(NULL==prefix)
        prefix=LogPrefix;
    if(NULL==suffix)
        suffix=LogSuffix;

    LogName=malloc(strlen(prefix)+strlen(suffix)+10+1); /* 10 for 32bit int */
    if(NULL==LogName)
        return 0;

    for(i=0; i<INT_MAX; ++i)
        {
        sprintf(LogName, "%s%02u%s", prefix, i, suffix);
        fd=_open(LogName, O_WRONLY|O_CREAT|O_EXCL, S_IREAD|S_IWRITE);
        if(fd>=0)
            break;
        }
    if(fd>=0)
        Log=_fdopen(fd, "w");

    if(NULL==Log)
        {
        free(LogName);
        LogName=NULL;
        return 0;
        }

    return 1;
    }

void InitConsoleLog(void)
    {
    if(NULL!=LogName)
        {
        free(LogName);
        LogName=NULL;
        }
    if(NULL!=Log)
        {
        fclose(Log);
        Log=NULL;
        }
    LogName=_strdup("console");
    Log=_fdopen(2, "w");
    }

char *GetLogName(void)
    {
    return LogName;
    }
