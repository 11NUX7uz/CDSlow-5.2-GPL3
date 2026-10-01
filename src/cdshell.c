/*
Copyright 2006-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "cdlist.h"
#include "log.h"
#include "cdshell.h"

BOOL IsAutorunInf(struct CDROM_Desc *cd)
    {
    char autorun[]="A:\\AUTORUN.INF";

    if(0==cd->State.Addr.Letter)
        return FALSE;
    *autorun=cd->State.Addr.Letter;

    if(!cd->State.DiskPresent || IsCDAMedium(cd->State.CDI.MediumType))
        return FALSE;
    
    return 0xFFFFFFFFu!=GetFileAttributes(autorun);
    }

BOOL DoAutorunInf(struct CDROM_Desc *cd)
    {
    char autorun[]="A:\\AUTORUN.INF";
    char cmd[MAX_PATH]="";
    char *args;

    if(0==cd->State.Addr.Letter)
        return FALSE;
    *autorun=cd->State.Addr.Letter;

    GetPrivateProfileString("AutoRun", "shellexecute", "",
        cmd, sizeof(cmd), autorun);
    if(0==*cmd)
        GetPrivateProfileString("AutoRun", "open", "",
            cmd, sizeof(cmd), autorun);

    if(0==*cmd)
        return FALSE;

    DebugLog(("DoAutorunInf: cmd=%s\n", cmd));
    args=cmd;
    while(0!=*args && !isspace(*args))
        args++;
    while(0!=*args && isspace(*args))
        {
        *args=0;
        args++;
        }
    DebugLog(("DoAutorunInf: cmd=%s args=%s\n", cmd, args));

    autorun[2]=0;
    return (unsigned)ShellExecute(NULL,
        "open", cmd, args, autorun, SW_SHOWNORMAL)>32;
    }

void DoCDROM(struct CDROM_Desc *cd, char *operation)
    {
    char path[]="A:\\";

    if(0==cd->State.Addr.Letter)
        return;
    *path=cd->State.Addr.Letter;

    ShellExecute(NULL, operation, path, NULL, NULL, SW_SHOWNORMAL);
    }
