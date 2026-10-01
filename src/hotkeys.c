/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "cdscmd.h"
#include "cdlist.h"
#include "hotkeys.h"

unsigned HotKeyCmd[CDROM_HOTKEYS]=
    {
    CMD_CD_EJECT,
    CMD_CD_CLOSE,
    CMD_CD_SETMAX,
    CMD_CD_SPEEDUP,
    CMD_CD_SETSEL,
    CMD_CD_SPEEDDOWN,
    CMD_CD_SETMIN
    };

void SetHotKeys(HWND window)
    {
    unsigned i;
    int key;
    int vk, flag;
    int mod;

    for(i=0; i<CDROM_Count; ++i)
        {
        if(CDROM_List[i].Config.Ignore)
            continue;
        for(key=0; key<CDROM_HOTKEYS; ++key)
            {
            if(CDROM_List[i].Config.HotKeys[key]==0)
                continue;
            vk=LOBYTE(CDROM_List[i].Config.HotKeys[key]);
            flag=HIBYTE(CDROM_List[i].Config.HotKeys[key]);
            mod=0;
            if(flag&HOTKEYF_ALT)
                mod|=MOD_ALT;
            if(flag&HOTKEYF_CONTROL)
                mod|=MOD_CONTROL;
            if(flag&HOTKEYF_SHIFT)
                mod|=MOD_SHIFT;
            if(flag&HOTKEYF_WIN)
                mod|=MOD_WIN;
            RegisterHotKey(window, MAKEWORD(i, key), mod, vk);
            }
        }
    }

void ClearHotKeys(HWND window)
    {
    unsigned i;
    int key;

    for(i=0; i<CDROM_Count; ++i)
        for(key=0; key<CDROM_HOTKEYS; ++key)
            UnregisterHotKey(window, MAKEWORD(i, key));
    }

