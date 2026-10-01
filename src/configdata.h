/*
Copyright 2000-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __CONFIGDATA_H__
#define __CONFIGDATA_H__

#define AUTOCLOSE_MIN 3000
#define CDROM_HASH_SIZE 35

extern struct CDS_Config
    {
    unsigned ApiPrefered;
    unsigned NumNames;
    unsigned EjectSingle;
    unsigned ShowUnknown;
    unsigned ForceRefresh;
    unsigned DNEject;
    unsigned DNClose;
    unsigned DNMessage;
    unsigned DNTimer;
    unsigned DeepMenu;
    unsigned TopNames;
    unsigned CloseOnRun;
    unsigned IconDisk;
    unsigned IconOpen;
    unsigned CmdOpen;
    unsigned CmdExplore;
    unsigned CmdAutorun;
    unsigned TimerAutoclose;
    unsigned EnableAutoclose;
    unsigned TimerEnable;
    char Hash[CDROM_HASH_SIZE];
    char SavePath[MAX_PATH];
    } Config;

void GetConfigFromRegistry(void);
void StoreConfigToRegistry(void);

#endif /* __CONFIGDATA_H__ */
