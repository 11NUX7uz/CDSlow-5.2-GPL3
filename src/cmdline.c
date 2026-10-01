/*
Copyright 2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include "cmdline.h"
#include "dlglib.h" /* TextBufWtoA() */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static WCHAR const *ParamLog = L"/log";
static WCHAR const *ParamExit = L"/exit";
static WCHAR const *ParamHide = L"/hide";
static WCHAR const *ParamRestartW = L"/restart";
static char const *ParamRestart = "/restart";
static WCHAR const *ParamSet = L"/set";
static WCHAR const *ParamEject = L"/eject";
static WCHAR const *ParamClose = L"/close";

static WCHAR const *GetToken(WCHAR *token, WCHAR const *cmdline)
    {
    int quoted = 0;

    /* Skip spaces */
    while(*cmdline && iswspace(*cmdline))
        ++cmdline;

    /* Store token */
    while(*cmdline && (quoted || !iswspace(*cmdline)))
        {
        if(*cmdline == L'"')
            {
            quoted = !quoted;
            }

        else
            {
            *token = *cmdline;
            ++token;
            }

        ++cmdline;
        }

    *token = L'\0';

    return cmdline;
    }

static void ParseDriveOption(struct CMD_DriveOption *dopt, WCHAR const *token)
    {
    WCHAR *pos;

    dopt->Name = NULL;
    dopt->NameW = _wcsdup(token);

    if(dopt->NameW == NULL)
        return;

    pos = wcsrchr(dopt->NameW, L'=');

    if(pos != NULL)
        {
        dopt->ValueW = pos + 1;
        *pos = L'\0';
        }

    else
        dopt->ValueW = dopt->NameW;

    pos = wcsrchr(dopt->NameW, L':');

    if(pos != NULL)
        {
        dopt->IndexW = pos + 1;
        *pos = L'\0';
        }

    else
        dopt->IndexW = dopt->NameW + lstrlenW(dopt->NameW);

    dopt->Name = TextBufWtoA(dopt->NameW);
    }

void ParseCmdLine(struct CMD_Options *opts, WCHAR const *cmdline)
    {
    size_t len = lstrlenW(cmdline);
    WCHAR *token = calloc(len + 1, sizeof(*token));

    opts->Log = 0;
    opts->Exit = 0;
    opts->Hide = 0;
    opts->Restart = 0;
    opts->SetCount = 0;
    opts->EjectCount = 0;
    opts->CloseCount = 0;

    for(;;)
        {
        cmdline = GetToken(token, cmdline);

        if(*token == L'\0')
            break;

        if(_wcsicmp(token, ParamLog) == 0)
            {
            opts->Log = 1;
            }

        else if(_wcsicmp(token, ParamExit) == 0)
            {
            opts->Exit = 1;
            }

        else if(_wcsicmp(token, ParamHide) == 0)
            {
            opts->Hide = 1;
            }

        else if(_wcsicmp(token, ParamRestartW) == 0)
            {
            opts->Restart = 1;
            }

        else if(_wcsicmp(token, ParamSet) == 0)
            {
            cmdline = GetToken(token, cmdline);

            if(*token != L'\0')
                {
                if(opts->SetCount < sizeof(opts->SetList) / sizeof(*opts->SetList))
                    ParseDriveOption(&opts->SetList[opts->SetCount++], token);
                }
            }

        else if(_wcsicmp(token, ParamEject) == 0)
            {
            WCHAR const *new_cmdline = GetToken(token, cmdline);

            if(*token == L'\0' || *token == L'/')
                lstrcpyW(token, L"=");

            else
                cmdline = new_cmdline;

            if(opts->EjectCount < sizeof(opts->EjectList) / sizeof(*opts->EjectList))
                ParseDriveOption(&opts->EjectList[opts->EjectCount++], token);
            }

        else if(_wcsicmp(token, ParamClose) == 0)
            {
            WCHAR const *new_cmdline = GetToken(token, cmdline);

            if(*token == L'\0' || *token == L'/')
                lstrcpyW(token, L"=");

            else
                cmdline = new_cmdline;

            if(opts->CloseCount < sizeof(opts->CloseList) / sizeof(*opts->CloseList))
                ParseDriveOption(&opts->CloseList[opts->CloseCount++], token);
            }
        }

    free(token);
    }

char const *GetCmdOptRestart(void)
    {
    return ParamRestart;
    }

static int MatchParam(WCHAR const *cmdline, WCHAR const *param)
    {
    size_t len = lstrlenW(cmdline);
    WCHAR *token = calloc(len + 1, sizeof(*token));
    int ret = 0;

    for(;;)
        {
        cmdline = GetToken(token, cmdline);

        if(*token == L'\0')
            break;

        if(_wcsicmp(token, param) == 0)
            {
            ret = 1;
            break;
            }
        }

    free(token);

    return ret;
    }

int IsCmdOptExit(WCHAR const *cmdline)
    {
    return MatchParam(cmdline, ParamExit);
    }

int IsCmdOptHide(WCHAR const *cmdline)
    {
    return MatchParam(cmdline, ParamHide);
    }

static WCHAR *AddCmdOpt(WCHAR *cmdline, WCHAR const *opt)
    {
    WCHAR *cmd_opt;

    if(!cmdline)
        return cmdline;

    cmd_opt = realloc(cmdline, (lstrlenW(cmdline) + lstrlenW(opt) + 2) * sizeof(*cmd_opt));

    if(!cmd_opt)
        return cmdline;

    lstrcpyW(cmd_opt, cmdline);
    lstrcatW(cmd_opt, L" ");
    lstrcatW(cmd_opt, opt);

    return cmd_opt;
    }

WCHAR *AddCmdOptExit(WCHAR *cmdline)
    {
    return AddCmdOpt(cmdline, ParamExit);
    }

WCHAR *AddCmdOptHide(WCHAR *cmdline)
    {
    return AddCmdOpt(cmdline, ParamHide);
    }
