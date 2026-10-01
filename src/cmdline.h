/*
Copyright 2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <windows.h>
#include <stddef.h>

#define CMD_MAX_SET_COUNT 16

struct CMD_DriveOption
    {
    WCHAR *NameW;
    WCHAR *IndexW;
    WCHAR *ValueW;
    char *Name;
    };

struct CMD_Options
    {
    int Log;
    int Exit;
    int Hide;
    int Restart;
    size_t SetCount;
    struct CMD_DriveOption SetList[CMD_MAX_SET_COUNT];
    size_t EjectCount;
    struct CMD_DriveOption EjectList[CMD_MAX_SET_COUNT];
    size_t CloseCount;
    struct CMD_DriveOption CloseList[CMD_MAX_SET_COUNT];
    };

void ParseCmdLine(struct CMD_Options *opts, WCHAR const *cmdline);
char const *GetCmdOptRestart(void);
int IsCmdOptExit(WCHAR const *cmdline);
int IsCmdOptHide(WCHAR const *cmdline);
WCHAR *AddCmdOptExit(WCHAR *cmdline);
WCHAR *AddCmdOptHide(WCHAR *cmdline);
