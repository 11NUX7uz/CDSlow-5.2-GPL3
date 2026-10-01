/*
Copyright 2000-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <stddef.h>
#include "uniscsi.h"
#include "regfunc.h"
#include "configdata.h"

struct CDS_Config Config;

static struct CDS_Config DefConfig=
    {
    UAPI_NONE, /* unsigned ApiPrefered; */
    TRUE, /* unsigned NumNames; */
    TRUE, /* unsigned EjectSingle; */
    FALSE, /* unsigned ShowUnknown; */
    FALSE, /* unsigned ForceRefresh; */
    FALSE, /* unsigned DNEject; */
    FALSE, /* unsigned DNClose; */
    FALSE, /* unsigned DNMessage; */
    5, /* unsigned DNTimer; */
    FALSE, /* unsigned DeepMenu; */
    FALSE, /* unsigned TopNames; */
    FALSE, /* unsigned CloseOnRun; */
    TRUE, /* unsigned IconDisk; */
    FALSE, /* unsigned IconOpen; */
    FALSE, /* unsigned CmdOpen; */
    TRUE, /* unsigned CmdExplore; */
    TRUE, /* unsigned CmdAutorun; */
    0, /* unsigned TimerAutoclose; */
    FALSE, /* unsigned EnableAutoclose; */
    FALSE, /* unsigned TimerEnable; */
    "", /* char Hash[CDROM_HASH_SIZE]; */
    "", /* char SavePath[MAX_PATH]; */
    };

static struct REG_ValueUnsigned UnsignedConfig[]=
    {
    {"ApiPrefered",     offsetof(struct CDS_Config, ApiPrefered)},
    {"NumNames",        offsetof(struct CDS_Config, NumNames)},
    {"EjectSingle",     offsetof(struct CDS_Config, EjectSingle)},
    {"ShowUnknown",     offsetof(struct CDS_Config, ShowUnknown)},
    {"ForceRefresh",    offsetof(struct CDS_Config, ForceRefresh)},
    {"DNEject",         offsetof(struct CDS_Config, DNEject)},
    {"DNClose",         offsetof(struct CDS_Config, DNClose)},
    {"DNMessage",       offsetof(struct CDS_Config, DNMessage)},
    {"DNTimer",         offsetof(struct CDS_Config, DNTimer)},
    {"DeepMenu",        offsetof(struct CDS_Config, DeepMenu)},
    {"TopNames",        offsetof(struct CDS_Config, TopNames)},
    {"CloseOnRun",      offsetof(struct CDS_Config, CloseOnRun)},
    {"IconDisk",        offsetof(struct CDS_Config, IconDisk)},
    {"IconOpen",        offsetof(struct CDS_Config, IconOpen)},
    {"CmdOpen",         offsetof(struct CDS_Config, CmdOpen)},
    {"CmdExplore",      offsetof(struct CDS_Config, CmdExplore)},
    {"CmdAutorun",      offsetof(struct CDS_Config, CmdAutorun)},
    {"TimerAutoclose",  offsetof(struct CDS_Config, TimerAutoclose)},
    {"EnableAutoclose", offsetof(struct CDS_Config, EnableAutoclose)},
    {"TimerEnable",     offsetof(struct CDS_Config, TimerEnable)}
    };

static char *HashRegKey="Lock";
static char *HashRegName="Data";
static char *PathRegName="SavePath";

void GetConfigFromRegistry(void)
    {
    HKEY key;
    HKEY hash_key;

    Config=DefConfig;

    key=OpenMainKey();
    if(key==NULL)
        return;

    /* Get all numeric values */
    GetAllUnsigned(key, &Config, UnsignedConfig,
        sizeof(UnsignedConfig)/sizeof(*UnsignedConfig));

    /* Get password hash */
    if(ERROR_SUCCESS==RegOpenKeyEx(key, HashRegKey, 0, KEY_READ, &hash_key))
        {
        GetValString(hash_key, HashRegName,
            Config.Hash, sizeof(Config.Hash));
        RegCloseKey(hash_key);
        }

    /* Get last used path */
    GetValString(key, PathRegName, Config.SavePath, sizeof(Config.SavePath));

    RegCloseKey(key);
    }

void StoreConfigToRegistry(void)
    {
    HKEY key;
    HKEY hash_key;
    DWORD disp;

    key=CreateMainKey();
    if(key==NULL)
        return;

    /* Store all numeric values */
    SetAllUnsigned(key, &Config, UnsignedConfig,
        sizeof(UnsignedConfig)/sizeof(*UnsignedConfig));

    /* Store password hash */
    if(ERROR_SUCCESS==RegCreateKeyEx(key, HashRegKey, 0, NULL, 0,
            KEY_ALL_ACCESS, NULL, &hash_key, &disp))
        {
        SetValString(hash_key, HashRegName, Config.Hash);
        RegCloseKey(hash_key);
        }

    /* Store last used path */
    SetValString(key, PathRegName, Config.SavePath);

    RegCloseKey(key);
    }

