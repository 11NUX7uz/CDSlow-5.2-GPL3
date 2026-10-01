/*
Copyright 2000-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include "log.h"
#include "uniscsi.h"
#include "regfunc.h"
#include "configdata.h"
#include "dlglib.h"
#include "cdlist.h"

#define MAXCOUNT_CD 16

struct CDROM_Desc *CDROM_List=NULL;
unsigned CDROM_Count=0;

static busy_func *BusyNotify=NULL;

static struct REG_ValueUnsigned UnsignedSpeedList[]=
    {
    {"CanGetSpeed",     offsetof(struct CDROM_SpeedList, CanGetSpeed)},
    {"CanSetSpeed",     offsetof(struct CDROM_SpeedList, CanSetSpeed)},
    {"CanGetStreaming", offsetof(struct CDROM_SpeedList, CanGetStreaming)},
    {"CanSetStreaming", offsetof(struct CDROM_SpeedList, CanSetStreaming)},
    {"MethodDetected",  offsetof(struct CDROM_SpeedList, MethodDetected)},
    {"MethodPrefered",  offsetof(struct CDROM_SpeedList, MethodPrefered)},
    {"MaxLevel",        offsetof(struct CDROM_SpeedList, MaxLevel)},
    {"MaxSpeed",        offsetof(struct CDROM_SpeedList, MaxSpeed)}
    };

static struct REG_ValueUnsigned UnsignedSpeedListConfig[]=
    {
    {"Selected",        offsetof(struct CDROM_SpeedList, Selected)}
    };

static struct REG_ValueUnsigned UnsignedCaps[]=
    {
    {"CanClose",       offsetof(struct CDROM_Config, CanClose)},
    {"CanSenseMedium", offsetof(struct CDROM_Config, CanSenseMedium)},
    {"IsDVD",          offsetof(struct CDROM_Config, IsDVD)},
    {"IsBD",           offsetof(struct CDROM_Config, IsBD)}
    };

static struct REG_ValueUnsigned UnsignedConfig[]=
    {
    {"Ignore",         offsetof(struct CDROM_Config, Ignore)},
    {"EventMask",      offsetof(struct CDROM_Config, EventMask)},
    {"OnlyWithDisk",   offsetof(struct CDROM_Config, OnlyWithDisk)},
    {"WhenAudio",      offsetof(struct CDROM_Config, WhenAudio)},
    {"IgnoreSpeed",    offsetof(struct CDROM_Config, IgnoreSpeed)},
    {"ShortMenu",      offsetof(struct CDROM_Config, ShortMenu)},
    {"ForceRead",      offsetof(struct CDROM_Config, ForceRead)},
    {"AutoLock",       offsetof(struct CDROM_Config, AutoLock)},
    {"Timer",          offsetof(struct CDROM_Config, Timer)}
    };

static char *CDListRegName="SpeedListCD";
static char *DVDListRegName="SpeedListDVD";
static char *BDListRegName="SpeedListBD";
static char *RangeRegName="Range";
static char *ShortRegName="ShortName";
static char *HotKeysRegName="HotKeys";
static char *LockRegKey="Lock";
static char *LockRegName="Value";

typedef void SCAN_FUNC(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    );

static void CheckDriveMode(struct CDROM_Desc *cd);
static BOOL UpdateDriveProperties(struct CDROM_Desc *cd);
static void CheckDiskPresent(struct CDROM_Desc *cd);
static void CheckTrayOpen(struct CDROM_Desc *cd);
static void CheckAudioStatus(struct CDROM_Desc *cd);
static BOOL UpdateSpeed(struct CDROM_Desc *cd);
static void CheckMediumLevel(struct CDROM_Desc *cd);
static enum SPEED_METHOD CurrentGetMethod(struct CDROM_Desc *cd);
static enum MEDIUM_LEVEL GetCDMediumLevel(struct CDROM_Desc *cd);
static enum MEDIUM_LEVEL GetDVDMediumLevel(struct CDROM_Desc *cd);
static enum MEDIUM_LEVEL GetBlurayMediumLevel(struct CDROM_Desc *cd);
static void UpdateSpeedModes(struct CDROM_Desc *cd);

static void InsertSpeed(struct CDROM_SpeedList *sl,
    unsigned speed, unsigned command, progress_func *pf, void *param);
static int IsOldSpeed(struct CDROM_SpeedList *sl, unsigned speed, int eps);
static unsigned MeasureSpeed(UNI_ADDR *addr, unsigned capacity, unsigned time_limit);
static void ReadSomeData(struct CDROM_Desc *cd);
static void ScanCDROMSpeed(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    SCAN_FUNC *scan_func
    );
static void ScanFast(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    );
static void ScanFull(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    );
static void ScanRead(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    );

static BOOL GetSpeedList(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    );
static BOOL GetSpeedListConfig(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    );
static void SetSpeedList(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    );
static void SetSpeedListConfig(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    );

static void AddCDROM(UNI_ADDR *addr);

void RegisterBusyNotify(busy_func *f)
    {
    BusyNotify=f;
    }

static BOOL SeizeDrive(struct CDROM_Desc *cd)
    {
    if(cd->State.Busy) return FALSE;
    cd->State.Busy=TRUE;
    if(BusyNotify) BusyNotify(cd->State.Busy);

    return TRUE;
    }

static void ReleaseDrive(struct CDROM_Desc *cd)
    {
    cd->State.Busy=FALSE;
    if(BusyNotify) BusyNotify(cd->State.Busy);
    }

BOOL GetCDROMFullName(struct CDROM_Desc *cd)
    {
    struct SCSI_DevInfo DI;
    char *s;
    WCHAR *w;
    int n;
    BOOL ret;

    if(!SeizeDrive(cd)) return FALSE;

    ret=GetDeviceInfo(&cd->State.Addr, &DI);

    if(!ret)
        wsprintf(cd->Config.FullName, "<unknown>");
    else
        wsprintf(cd->Config.FullName, "%s %s %s",
            DI.Vendor, DI.Product, DI.Revision);

    s=cd->Config.FullName;
    w=cd->Config.AutoShortName;
    while(0!=*s && !isalpha(*s))
        ++s;
    n=CDROM_SHORT_SIZE-1;
    while(n>0 && 0!=*s && isalpha(*s))
        {
        *w++=*s++;
        --n;
        }
    *w=0;

    ReleaseDrive(cd);

    return ret;
    }

void CheckReadCaps(struct CDROM_Desc *cd)
    {
    DebugLog(("CheckReadCaps [%s]\n", cd->Config.FullName));

    if(!SeizeDrive(cd)) return;

    cd->Config.IsDVD=CheckCDROMFeature(&cd->State.Addr, FEATURE_DVD);
    DebugLog(("    CheckCDROMFeature(FEATURE_DVD)=%d\n", cd->Config.IsDVD));
    cd->Config.IsBD=CheckCDROMFeature(&cd->State.Addr, FEATURE_BD);
    DebugLog(("    CheckCDROMFeature(FEATURE_BD)=%d\n", cd->Config.IsBD));

    ReleaseDrive(cd);

    DebugLog(("FINISHED CheckReadCaps\n"));
    }

/* Check CDROM disk status and current speed */
void CheckDriveState(struct CDROM_Desc *cd)
    {
    DebugLog(("CheckDriveState [%s]\n", cd->Config.FullName));

    if(!SeizeDrive(cd)) return;

    if(CheckDriveStateOnly(cd))
        StoreCDROMCaps(cd);

    if(cd->State.DiskPresent
        && ML_MAX>cd->State.SL->MaxLevel
        && !IsAudioPlaying(cd->State.AudioStatus)
        )
        CheckMediumLevel(cd);

    ReleaseDrive(cd);

    if(cd->State.SL->MaxLevel<cd->State.MediumLevel)
        {
        ScanCDROMSpeedAuto(cd, NULL, NULL);
        cd->State.SL->MaxLevel=cd->State.MediumLevel;
        StoreCDROMCaps(cd);
        }

    DebugLog(("FINISHED CheckDriveState\n"));
    }

/* Check CDROM disk status and current speed */
BOOL CheckDriveStateOnly(struct CDROM_Desc *cd)
    {
    BOOL config_changed=FALSE;

    CheckDriveMode(cd);
    config_changed|=UpdateDriveProperties(cd);
    CheckDiskPresent(cd);
    CheckTrayOpen(cd);
    CheckAudioStatus(cd);
    config_changed|=UpdateSpeed(cd);

    return config_changed;
    }

#define MAX_WAIT 30
static void CheckDriveMode(struct CDROM_Desc *cd)
    {
    BOOL CDI_valid;
    enum ATAPI_MEDIUM_TYPE medium;
    enum CDROM_PROFILE profile=PROFILE_ERROR;
    int i;

    DebugLog(("  CheckDriveMode\n"));
    i=0;
    for(;;)
        {
        CDI_valid=GetCDROMInfo(&cd->State.Addr, &cd->State.CDI);
        DebugLog(("    GetCDROMInfo %s\n", CDI_valid ? "OK" : "*FAILED*"));
        medium=cd->State.CDI.MediumType;
        if(IsCDMedium(medium) || IsEmptyMedium(medium))
            {
            DebugLog(("    Medium is CD\n"));
            cd->State.SL=&cd->Config.CDList;
            break;
            }
        profile=GetCDROMProfile(&cd->State.Addr);
        DebugLog(("    GetCDROMProfile %s\n",
                PROFILE_ERROR!=profile ? "OK" : "*FAILED*"));
        if(IsCDProfile(profile))
            {
            DebugLog(("    Profile is CD\n"));
            cd->State.SL=&cd->Config.CDList;
            break;
            }
        else if(IsDVDProfile(profile))
            {
            DebugLog(("    Profile is DVD\n"));
            cd->State.SL=&cd->Config.DVDList;
            break;
            }
        else if(IsBDProfile(profile))
            {
            DebugLog(("    Profile is BD\n"));
            cd->State.SL=&cd->Config.BDList;
            break;
            }
        if(IsMediumReady(medium))
            {
            DebugLog(("    Medium assumed to be CD (0x%02X)\n", medium));
            cd->State.SL=&cd->Config.CDList;
            break;
            }
        if(IsProfileReady(profile))
            {
            DebugLog(("    Profile assumed to be CD (0x%04X)\n", profile));
            cd->State.SL=&cd->Config.CDList;
            break;
            }
        if(!cd->Config.CanSenseMedium)
            {
            DebugLog(("    Can't sense it, let it be CD\n"));
            cd->State.SL=&cd->Config.CDList;
            break;
            }
        if(++i>=MAX_WAIT)
            {
            DebugLog(("    Can't wait anymore, let it be CD\n"));
            LogLastSCSIError("");
            cd->State.SL=&cd->Config.CDList;
            break;
            }
        DebugLog(("    waiting for media [%02d]...\n", i));
        Sleep(500);
        }
    cd->State.Profile=profile;
    DebugLog(("  FINISHED CheckDriveMode\n"));
    }

static BOOL UpdateDriveProperties(struct CDROM_Desc *cd)
    {
    BOOL config_changed=FALSE;

    if(!cd->Config.CanClose && DriveHasTray(&cd->State.CDI))
        {
        cd->Config.CanClose=TRUE;
        config_changed=TRUE;
        }
    if(!cd->Config.CanSenseMedium && ATAPI_UNKNOWN!=cd->State.CDI.MediumType)
        {
        cd->Config.CanSenseMedium=TRUE;
        config_changed=TRUE;
        }
    if(!cd->Config.IsDVD
        && (DriveIsDVD(&cd->State.CDI) || &cd->Config.DVDList==cd->State.SL))
        {
        cd->Config.IsDVD=TRUE;
        config_changed=TRUE;
        }
    if(!cd->Config.IsBD && &cd->Config.BDList==cd->State.SL)
        {
        cd->Config.IsBD=TRUE;
        config_changed=TRUE;
        }
    if(cd->State.SL->MaxSpeed<cd->State.CDI.MaxSpeed)
        {
        cd->State.SL->MaxSpeed=cd->State.CDI.MaxSpeed;
        config_changed=TRUE;
        }
    if(!cd->State.SL->CanGetSpeed && 0!=cd->State.CDI.Speed)
        {
        cd->State.SL->CanGetSpeed=TRUE;
        config_changed=TRUE;
        }

    return config_changed;
    }

static void CheckDiskPresent(struct CDROM_Desc *cd)
    {
    int not_ready;

    if(IsEmptyMedium(cd->State.CDI.MediumType))
        cd->State.DiskPresent=FALSE;
    else if(IsCDMedium(cd->State.CDI.MediumType))
        cd->State.DiskPresent=TRUE;
    else if(&cd->Config.CDList!=cd->State.SL || IsCDProfile(cd->State.Profile))
        cd->State.DiskPresent=TRUE;
    else
        {
        not_ready=TestUnitReady(&cd->State.Addr);
        DebugLog(("    TestUnitReady=%d\n", not_ready));
        cd->State.DiskPresent=!not_ready;
        }

    if(cd->State.DiskPresent)
        cd->State.MediumLevel=ML_UNKNOWN;
    else
        cd->State.MediumLevel=ML_NONE;
    }

static void CheckTrayOpen(struct CDROM_Desc *cd)
    {
    if(!IsTrayOpen(cd))
        cd->State.OpenAt=0;
    else if(0==cd->State.OpenAt)
        cd->State.OpenAt=GetTickCount();
    }

static void CheckAudioStatus(struct CDROM_Desc *cd)
    {
    if(cd->State.DiskPresent
        && &cd->Config.CDList==cd->State.SL
        && (!cd->Config.CanSenseMedium
            || IsCDAMedium(cd->State.CDI.MediumType))
        )
        {
        cd->State.AudioStatus=ReadCDROMAudioStatus(&cd->State.Addr);
        DebugLog(("    ReadCDROMAudioStatus=0x%02X\n", cd->State.AudioStatus));
        }
    else
        cd->State.AudioStatus=AUDIO_INVALID_STATUS;
    }

static BOOL UpdateSpeed(struct CDROM_Desc *cd)
    {
    BOOL config_changed=FALSE;

    cd->State.Speed=cd->State.CDI.Speed;

    if(METHOD_STREAMING==CurrentGetMethod(cd))
        {
        cd->State.Speed=GetDVDSpeed(&cd->State.Addr);
        DebugLog(("    GetDVDSpeed=%u\n", cd->State.Speed));
        if(!cd->State.SL->CanGetStreaming && 0!=cd->State.Speed)
            {
            cd->State.SL->CanGetStreaming=TRUE;
            config_changed=TRUE;
            }
        }

    if(cd->State.SL->MaxSpeed<cd->State.Speed)
        {
        cd->State.SL->MaxSpeed=cd->State.Speed;
        config_changed=TRUE;
        }

    return config_changed;
    }

static void CheckMediumLevel(struct CDROM_Desc *cd)
    {
    if(&cd->Config.CDList==cd->State.SL)
        cd->State.MediumLevel=GetCDMediumLevel(cd);
    else if(&cd->Config.DVDList==cd->State.SL)
        cd->State.MediumLevel=GetDVDMediumLevel(cd);
    else if(&cd->Config.BDList==cd->State.SL)
        cd->State.MediumLevel=GetBlurayMediumLevel(cd);
    }

enum SPEED_METHOD ListSetMethod(struct CDROM_SpeedList *sl,
    enum SPEED_METHOD def_method)
    {
    if(METHOD_NONE!=sl->MethodPrefered)
        return sl->MethodPrefered;
    if(METHOD_NONE!=sl->MethodDetected)
        return sl->MethodDetected;
    if(METHOD_SPEED==def_method && sl->CanSetSpeed)
        return def_method;
    if(METHOD_STREAMING==def_method && sl->CanSetStreaming)
        return def_method;
    if(!sl->CanSetSpeed && sl->CanSetStreaming)
        return METHOD_STREAMING;

    return METHOD_SPEED;
    }

static enum SPEED_METHOD CurrentGetMethod(struct CDROM_Desc *cd)
    {
    if(METHOD_NONE!=cd->State.SL->MethodPrefered)
        return cd->State.SL->MethodPrefered;
    if(METHOD_NONE!=cd->State.SL->MethodDetected)
        return cd->State.SL->MethodDetected;
    if(!cd->State.SL->CanGetSpeed)
        return METHOD_STREAMING;

    return METHOD_SPEED;
    }

static enum SPEED_METHOD CurrentSetMethod(struct CDROM_Desc *cd,
    enum SPEED_METHOD def_method)
    {
    return ListSetMethod(cd->State.SL, def_method);
    }

static enum MEDIUM_LEVEL GetCDMediumLevel(struct CDROM_Desc *cd)
    {
    if(!cd->State.DiskPresent)
        return ML_NONE;
    else if(IsCDROMMedium(cd->State.CDI.MediumType))
        return ML_ROM;
    else if(IsCDRMedium(cd->State.CDI.MediumType))
        return ML_R;
    else if(IsCDRWMedium(cd->State.CDI.MediumType))
        return ML_RW;

    return ML_UNKNOWN;
    }

static enum MEDIUM_LEVEL GetDVDMediumLevel(struct CDROM_Desc *cd)
    {
    enum DVD_TYPE dt;

    dt=ReadDVDBookType(&cd->State.Addr);
    DebugLog(("    ReadDVDBookType=0x%X\n", dt));
    switch(dt)
        {
    case DVD_TYPE_ROM:
        return ML_ROM;
    case DVD_TYPE_R:
    case DVD_TYPE_PLUSR:
        return ML_R;
    case DVD_TYPE_RAM:
    case DVD_TYPE_RW:
    case DVD_TYPE_PLUSRW:
        return ML_RW;
    default:
        return ML_UNKNOWN;
        }
    }

static enum MEDIUM_LEVEL GetBlurayMediumLevel(struct CDROM_Desc *cd)
    {
    BD_TYPE bt;

    bt=ReadBlurayDiscType(&cd->State.Addr);
    DebugLog(("    ReadBlurayDiscType='%s'\n", bt.type_str));
    if(0==memcmp(&BD_TYPE_ROM, &bt, sizeof(bt)))
        return ML_ROM;
    else if(0==memcmp(&BD_TYPE_R, &bt, sizeof(bt)))
        return ML_R;
    else if(0==memcmp(&BD_TYPE_RW, &bt, sizeof(bt)))
        return ML_RW;

    return ML_UNKNOWN;
    }

void ScanCDROMSpeedAuto(struct CDROM_Desc *cd, progress_func *pf, void *param)
    {
    DebugLog(("ScanCDROMSpeedAuto [%s]\n", cd->Config.FullName));
    if(!SeizeDrive(cd)) return;
    ScanCDROMSpeed(cd, pf, param, ScanFast);
    ReleaseDrive(cd);
    DebugLog(("FINISHED ScanCDROMSpeedAuto\n"));
    }

/* Detect CDROM speed list */
void ScanCDROMSpeedFast(struct CDROM_Desc *cd, progress_func *pf, void *param)
    {
    DebugLog(("ScanCDROMSpeedFast [%s]\n", cd->Config.FullName));
    ScanCDROMSpeed(cd, pf, param, ScanFast);
    DebugLog(("FINISHED ScanCDROMSpeedFast\n"));
    }

/* Detect CDROM speed list */
void ScanCDROMSpeedFull(struct CDROM_Desc *cd, progress_func *pf, void *param)
    {
    DebugLog(("ScanCDROMSpeedFull [%s]\n", cd->Config.FullName));
    ScanCDROMSpeed(cd, pf, param, ScanFull);
    DebugLog(("FINISHED ScanCDROMSpeedFull\n"));
    }

/* Detect CDROM speed list */
void ScanCDROMSpeedRead(struct CDROM_Desc *cd, progress_func *pf, void *param)
    {
    DebugLog(("ScanCDROMSpeedRead [%s]\n", cd->Config.FullName));
    ScanCDROMSpeed(cd, pf, param, ScanRead);
    DebugLog(("FINISHED ScanCDROMSpeedRead\n"));
    }

/* Detect CDROM speed list */
static void ScanCDROMSpeed(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    SCAN_FUNC *scan_func
    )
    {
    CheckDriveStateOnly(cd);
    CheckMediumLevel(cd);
    cd->State.SL->MaxLevel=cd->State.MediumLevel;
    DebugLog(("    MaxLevel=%d\n", cd->State.SL->MaxLevel));

    UpdateSpeedModes(cd);
    cd->State.SL->MethodDetected=METHOD_NONE;

    if(METHOD_SPEED==CurrentSetMethod(cd, METHOD_SPEED))
        {
        cd->State.SL->Count=0;
        DebugLog(("    Using SET SPEED...\n"));
        scan_func(cd, pf, param, SetCDROMSpeed, GetCDROMSpeed);
        DebugLog(("    Count=%d\n", cd->State.SL->Count));
        if(cd->State.SL->Count>1)
            cd->State.SL->MethodDetected=METHOD_SPEED;
        }

    if(METHOD_STREAMING==CurrentSetMethod(cd, METHOD_STREAMING))
        {
        cd->State.SL->Count=0;
        DebugLog(("    Using SET STREAMING...\n"));
        scan_func(cd, pf, param, SetDVDSpeed, GetDVDSpeed);
        DebugLog(("    Count=%d\n", cd->State.SL->Count));
        if(cd->State.SL->Count>1)
            cd->State.SL->MethodDetected=METHOD_STREAMING;
        }
 
    /* Restore original CDROM speed */
    if(METHOD_SPEED==CurrentSetMethod(cd, METHOD_NONE))
        SetCDROMSpeed(&cd->State.Addr, cd->State.Speed,
            cd->State.CDI.WriteSpeed);
    else
        SetDVDSpeed(&cd->State.Addr, cd->State.Speed,
            cd->State.CDI.WriteSpeed);
    }

static void UpdateSpeedModes(struct CDROM_Desc *cd)
    {
    BOOL ret;
    unsigned speed;

    /* Check SET/GET SPEED */
    ret=SetCDROMSpeed(&cd->State.Addr, 0xFFFF, cd->State.CDI.WriteSpeed);
    DebugLog(("    SetCDROMSpeed=%d\n", ret));
    cd->State.SL->CanSetSpeed|=ret;
    speed=GetCDROMSpeed(&cd->State.Addr);
    DebugLog(("    GetCDSpeed=%u\n", speed));
    cd->State.SL->CanGetSpeed|=0!=speed;
    if(cd->State.SL->MaxSpeed<speed)
        cd->State.SL->MaxSpeed=speed;

    /* Check SET/GET STREAMING */
    ret=SetDVDSpeed(&cd->State.Addr, 0xFFFF, cd->State.CDI.WriteSpeed);
    DebugLog(("    SetDVDSpeed=%d\n", ret));
    cd->State.SL->CanSetStreaming|=ret;
    speed=GetDVDSpeed(&cd->State.Addr);
    DebugLog(("    GetDVDSpeed=%u\n", speed));
    cd->State.SL->CanGetStreaming|=0!=speed;
    if(cd->State.SL->MaxSpeed<speed)
        cd->State.SL->MaxSpeed=speed;
    }

static void ScanFast(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    )
    {
    unsigned step;
    unsigned speed;
    unsigned cur_speed;
    unsigned new_speed;
    unsigned max;

    step=cd->State.SL->x1;
    max=cd->State.SL->MaxSpeed;
    speed=max;
    DebugLog(("    step=%u, max=%u, speed=%u\n", step, max, speed));
    do
        {
        set_speed(&cd->State.Addr, speed, cd->State.CDI.WriteSpeed);
        cur_speed=get_speed(&cd->State.Addr);
        if(cur_speed==speed)
            {
            InsertSpeed(cd->State.SL, speed, speed, pf, param);
            speed-=step;
            }
        else if(cur_speed<speed)
            {
            speed=cur_speed;
            }
        else
            {
            new_speed=cur_speed;
            if(IsOldSpeed(cd->State.SL, new_speed, 0)<0)
                {
                set_speed(&cd->State.Addr, new_speed, cd->State.CDI.WriteSpeed);
                cur_speed=get_speed(&cd->State.Addr);
                if(cur_speed==new_speed)
                    InsertSpeed(cd->State.SL, new_speed, new_speed, pf, param);
                else
                    InsertSpeed(cd->State.SL, new_speed, speed, pf, param);
                }
            speed-=step;
            }
        if(pf)
            if(!pf(0, max, max-speed, 0, 0, param))
                break;
        }
    while(speed>=step);
    if(pf)
        pf(0, max, max, 0, 0, param);
    }

static void ScanFull(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    )
    {
    unsigned speed;
    unsigned cur_speed;
    unsigned new_speed;
    unsigned max;
    unsigned step;
    DWORD tick;

    max=cd->State.SL->MaxSpeed;
    step=cd->State.SL->x1;
    DebugLog(("    step=%u, max=%u\n", step, max));
    tick=GetTickCount();
    for(speed=max; speed>=step; --speed)
        {
        set_speed(&cd->State.Addr, speed, cd->State.CDI.WriteSpeed);
        new_speed=get_speed(&cd->State.Addr);

        if(0==new_speed)
            DebugLog(("        speed=%u, new_speed=%u!\n", speed, new_speed));

        if(0!=new_speed && IsOldSpeed(cd->State.SL, new_speed, 0)<0)
            {
            set_speed(&cd->State.Addr, new_speed, cd->State.CDI.WriteSpeed);
            cur_speed=get_speed(&cd->State.Addr);
            if(cur_speed==new_speed)
                InsertSpeed(cd->State.SL, new_speed, new_speed, pf, param);
            else
                InsertSpeed(cd->State.SL, new_speed, speed, pf, param);
            }
        if(pf)
            {
            if((unsigned)(GetTickCount()-tick)>250u)
                {
                if(!pf(step, max, max-speed+step, 0, 0, param))
                    break;
                tick=GetTickCount();
                }
            }
        }
    }

#define MAX_CD  52
#define MAX_DVD 24
#define MAX_BD  14
#define DIV 64
static void ScanRead(
    struct CDROM_Desc *cd,
    progress_func *pf,
    void *param,
    BOOL (*set_speed)(UNI_ADDR *, unsigned, unsigned),
    unsigned (*get_speed)(UNI_ADDR *)
    )
    {
    int step;
    int speed;
    unsigned new_speed;
    unsigned LBA;
    unsigned i;
    BOOL ok=TRUE;
    int eps;
    int old;
    unsigned max;
    unsigned speed_up;

    (void)get_speed; /* Unused */

    LBA=ReadCDROMCapacity(&cd->State.Addr);
    DebugLog(("    LBA=%u MB=%u\n", LBA, LBA>>9));
    if(0==LBA)
        return;

    if(&cd->Config.BDList==cd->State.SL)
        max=MAX_BD;
    else if(&cd->Config.DVDList==cd->State.SL)
        max=MAX_DVD;
    else
        max=MAX_CD;

    speed_up=max/4;
    step=cd->State.SL->x1;
    eps=step*2;
    max*=step;

    set_speed(&cd->State.Addr, 0xFFFF, cd->State.CDI.WriteSpeed);
    for(i=1; i<=speed_up && ok; ++i)
        {
        MeasureSpeed(&cd->State.Addr, LBA, 1000);
        if(pf)
            ok=pf(0, (speed_up*step+max-step)/DIV, i*step/DIV, 0, 0, param);
        }

    DebugLog(("    step=%u, max=%u\n", step, max));
    for(speed=max; speed>=step && ok; speed-=step)
        {
        set_speed(&cd->State.Addr, speed, cd->State.CDI.WriteSpeed);
        new_speed=MeasureSpeed(&cd->State.Addr, LBA, 1000);

        if(0==new_speed)
            LogLastSCSIError("ScanRead: MeasureSpeed() failed!");

        old=IsOldSpeed(cd->State.SL, new_speed, eps);
        if(old<0)
            InsertSpeed(cd->State.SL, new_speed, speed, pf, param);
        else if(abs((int)(cd->State.SL->Range[old].Speed)-speed)<
            abs((int)(cd->State.SL->Range[old].Speed)-
                (int)(cd->State.SL->Range[old].Command))
            )
            cd->State.SL->Range[old].Command=speed;
        if(pf)
            ok=pf(0, (speed_up*step+max-step)/DIV,
                (speed_up*step+max-speed)/DIV, 0, 0, param);
        }
    }

static void InsertSpeed(struct CDROM_SpeedList *sl,
    unsigned speed, unsigned command,
    progress_func *pf, void *param
    )
    {
    int i;
    size_t len;

    if(0==speed || 0==command)
        {
        DebugLog(("        InsertSpeed: speed=%u, command=%u!\n",
                speed, command));
        return;
        }

    if(pf)
        pf(0, 0, 0, speed, command, param);

    if(sl->Count>=CDROM_SPEED_SIZE)
        return;

    for(i=sl->Count-1; i>=0; i--)
        {
        if(sl->Range[i].Speed==speed)
            return;
        else if(speed<sl->Range[i].Speed)
            break;
        }
    i++;

    len=sizeof(*sl->Range)*(sl->Count-i);
    if(len>0)
        memmove(sl->Range+i+1, sl->Range+i, len);

    sl->Range[i].Speed=speed;
    sl->Range[i].Command=command;
    sl->Count++;
    }

static int IsOldSpeed(struct CDROM_SpeedList *sl, unsigned speed, int eps)
    {
    int i;
    int delta;

    for(i=sl->Count-1; i>=0; i--)
        {
        delta=sl->Range[i].Speed-speed;
        if(delta<=eps && delta>=-eps)
            return i;
        }

    return -1;
    }

#define SEC_SIZE 2048
#define READ_SINGLE 256
static unsigned MeasureSpeed(
    UNI_ADDR *addr,
    unsigned capacity,
    unsigned time_limit
    )
    {
    char *buf;
    unsigned start_sec, end_sec;
    DWORD start_time, end_time;
    BOOL ret;

    buf=malloc(SEC_SIZE*READ_SINGLE);
    if(buf==NULL)
        return 0;

    capacity-=READ_SINGLE;
    start_sec=capacity-5120-time_limit*75/2; /* capacity-10Mb-seconds*75blk/sec*500x */
    /* Read 1 sector for head positioning */
    ret=ReadCDROM(addr, start_sec, 1, buf);
    if(!ret)
        {
        free(buf);
        return 0;
        }
    start_sec++;

    /* Measure speed */
    end_time=start_time=GetTickCount();
    end_sec=start_sec;
    while(end_time-start_time<time_limit && end_sec<capacity)
        {
        ret=ReadCDROM(addr, end_sec, READ_SINGLE, buf);
        if(ret)
            end_sec+=READ_SINGLE;
        end_time=GetTickCount();
        }

    free(buf);

    if(end_time==start_time)
        return 0;

    return (end_sec-start_sec)*2*176*20/((end_time-start_time)*3);
    }

void RestoreCDROMSpeed(struct CDROM_Desc *cd)
    {
    /* Nothing to do, if no speed selected */
    if(0==cd->State.SL->Selected)
        return;
    /* Skip operation if no disk */
    if(cd->Config.OnlyWithDisk && !cd->State.DiskPresent)
        return;
    if(!cd->Config.WhenAudio && IsAudioPlaying(cd->State.AudioStatus))
        return;
    if(!cd->Config.IgnoreSpeed && cd->State.Speed!=0 &&
        cd->State.Speed<=cd->State.SL->Selected)
        return;

    /* Set speed if permited */
    CommandCDROM(cd, GetCmd(cd->State.SL, cd->State.SL->Selected));
    }

unsigned GetCmd(struct CDROM_SpeedList *sl, unsigned speed)
    {
    unsigned i;
    unsigned command;

    command=speed;
    for(i=0; i<sl->Count; i++)
        if(sl->Range[i].Speed==speed)
            {
            command=sl->Range[i].Command;
            break;
            }

    return command;
    }

void UpperCDROMSpeed(struct CDROM_Desc *cd)
    {
    int i;

    for(i=cd->State.SL->Count-1; i>=0; --i)
        {
        if(cd->State.SL->Range[i].Speed>cd->State.Speed)
            {
            CommandCDROM(cd, cd->State.SL->Range[i].Command);
            break;
            }
        }
    }

void LowerCDROMSpeed(struct CDROM_Desc *cd)
    {
    unsigned i;

    for(i=0; i<cd->State.SL->Count; ++i)
        {
        if(cd->State.SL->Range[i].Speed<cd->State.Speed)
            {
            CommandCDROM(cd, cd->State.SL->Range[i].Command);
            break;
            }
        }
    }

void CommandCDROM(struct CDROM_Desc *cd, unsigned command)
    {
    if(!SeizeDrive(cd)) return;

    if(METHOD_SPEED==CurrentSetMethod(cd, METHOD_NONE))
        SetCDROMSpeed(&cd->State.Addr, command, cd->State.CDI.WriteSpeed);
    else
        SetDVDSpeed(&cd->State.Addr, command, cd->State.CDI.WriteSpeed);

    if(cd->Config.ForceRead)
        ReadSomeData(cd);

    ReleaseDrive(cd);

    CheckDriveState(cd);
    }

static void ReadSomeData(struct CDROM_Desc *cd)
    {
    unsigned LBA;
    char buf[2048];

    LBA=ReadCDROMCapacity(&cd->State.Addr);
    if(LBA>0)
        {
        ReadCDROM(&cd->State.Addr, LBA/3, 1, buf);
        ReadCDROM(&cd->State.Addr, LBA/3*2, 1, buf);
        }
    }

void EjectCDROMNoSeize(struct CDROM_Desc *cd, BOOL eject)
    {
    DebugLog(("EjectCDROM [%s] (%s)\n",
            cd->Config.FullName, eject ? "EJECT" : "CLOSE"));

    if(eject)
        SetCDROMLock(&cd->State.Addr, FALSE);

    SetCDROMEject(&cd->State.Addr, eject);
    cd->State.TrayClosed=!eject;

    if(cd->Config.AutoLock)
        SetCDROMLock(&cd->State.Addr, TRUE);

    DebugLog(("FINISHED EjectCDROM\n"));
    }

void EjectCDROM(struct CDROM_Desc *cd, BOOL eject)
    {
    if(!SeizeDrive(cd))
        {
        DebugLog(("EjectCDROM [%s] (%s) - drive busy\n",
                cd->Config.FullName, eject ? "EJECT" : "CLOSE"));
        return;
        }

    EjectCDROMNoSeize(cd, eject);
    ReleaseDrive(cd);
    }

BOOL EjectOrClose(struct CDROM_Desc *cd)
    {
    CheckDriveState(cd);
    if(!cd->Config.CanClose)
        return TRUE;
    return !IsTrayOpen(cd);
    }

BOOL IsTrayOpen(struct CDROM_Desc *cd)
    {
    if(cd->Config.CanSenseMedium)
        return ATAPI_OPEN==cd->State.CDI.MediumType;
    if(cd->State.DiskPresent)
        return FALSE;
    return !cd->State.TrayClosed;
    }

BOOL GetCDROMCaps(struct CDROM_Desc *cd)
    {
    HKEY key;
    BOOL ret;

    key=OpenSubkey(cd->Config.FullName);
    if(key==NULL)
        return FALSE;
    
    ret=GetAllUnsigned(key, &cd->Config, UnsignedCaps,
        sizeof(UnsignedCaps)/sizeof(*UnsignedCaps));
    ret=GetSpeedList(key, &cd->Config.CDList, CDListRegName) && ret;
    ret=GetSpeedList(key, &cd->Config.DVDList, DVDListRegName) && ret;
    ret=GetSpeedList(key, &cd->Config.BDList, BDListRegName) && ret;
    
    RegCloseKey(key);

    return ret;
    }

static BOOL GetSpeedList(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    )
    {
    HKEY key=NULL;
    int valsize;
    BOOL ret;

    RegOpenKeyEx(cdkey, list_name, 0, KEY_READ, &key);
    if(NULL==key)
        return FALSE;

    ret=GetAllUnsigned(key, list, UnsignedSpeedList,
        sizeof(UnsignedSpeedList)/sizeof(*UnsignedSpeedList));
    valsize=GetValArray(key, RangeRegName, list->Range, sizeof(list->Range));
    list->Count=valsize/sizeof(*list->Range);

    RegCloseKey(key);

    return ret;
    }

static BOOL GetSpeedListConfig(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    )
    {
    HKEY key=NULL;
    BOOL ret;

    RegOpenKeyEx(cdkey, list_name, 0, KEY_READ, &key);
    if(NULL==key)
        return FALSE;

    ret=GetAllUnsigned(key, list, UnsignedSpeedListConfig,
        sizeof(UnsignedSpeedListConfig)/sizeof(*UnsignedSpeedListConfig));

    RegCloseKey(key);

    return ret;
    }

void StoreCDROMCaps(struct CDROM_Desc *cd)
    {
    HKEY key;

    key=CreateSubkey(cd->Config.FullName);
    if(key==NULL)
        return;

    SetAllUnsigned(key, &cd->Config, UnsignedCaps,
        sizeof(UnsignedCaps)/sizeof(*UnsignedCaps));
    SetSpeedList(key, &cd->Config.CDList, CDListRegName);
    SetSpeedList(key, &cd->Config.DVDList, DVDListRegName);
    SetSpeedList(key, &cd->Config.BDList, BDListRegName);

    RegCloseKey(key);
    }

static void SetSpeedList(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    )
    {
    HKEY key=NULL;
    int valsize;
    DWORD action;

    RegCreateKeyEx(
        cdkey, list_name, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &key, &action);
    if(NULL==key)
        return;

    SetAllUnsigned(key, list, UnsignedSpeedList,
        sizeof(UnsignedSpeedList)/sizeof(*UnsignedSpeedList));
    valsize=sizeof(*list->Range)*list->Count;
    if(valsize==0) valsize=1;
    SetValArray(key, RangeRegName, list->Range, valsize);

    RegCloseKey(key);
    }

static void SetSpeedListConfig(
    HKEY cdkey,
    struct CDROM_SpeedList *list,
    char *list_name
    )
    {
    HKEY key=NULL;
    DWORD action;

    RegCreateKeyEx(
        cdkey, list_name, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &key, &action);
    if(NULL==key)
        return;

    SetAllUnsigned(key, list, UnsignedSpeedListConfig,
        sizeof(UnsignedSpeedListConfig)/sizeof(*UnsignedSpeedListConfig));

    RegCloseKey(key);
    }

void GetCDROMConfig(struct CDROM_Desc *cd)
    {
    HKEY key;
    HKEY hash_key;
    DWORD dw;

    key=OpenIndexKey(cd->Config.FullName, cd->Config.Index);
    if(key==NULL)
        return;

    GetAllUnsigned(key, &cd->Config, UnsignedConfig,
        sizeof(UnsignedConfig)/sizeof(*UnsignedConfig));
    GetValStringW(key, ShortRegName, cd->Config.ShortName,
        sizeof(cd->Config.ShortName));
    GetValArray(key, HotKeysRegName, cd->Config.HotKeys,
        sizeof(cd->Config.HotKeys));

    GetSpeedListConfig(key, &cd->Config.CDList, CDListRegName);
    GetSpeedListConfig(key, &cd->Config.DVDList, DVDListRegName);
    GetSpeedListConfig(key, &cd->Config.BDList, BDListRegName);

    if(ERROR_SUCCESS==RegOpenKeyEx(key, LockRegKey, 0, KEY_READ, &hash_key))
        {
        if(GetValDWORD(hash_key, LockRegName, &dw))
            cd->Config.AutoLock=dw;
        RegCloseKey(hash_key);
        }
    
    RegCloseKey(key);
    }

void StoreCDROMConfig(struct CDROM_Desc *cd)
    {
    HKEY key;
    HKEY hash_key;
    DWORD disp;

    key=CreateIndexKey(cd->Config.FullName, cd->Config.Index);
    if(key==NULL)
        return;
    
    SetAllUnsigned(key, &cd->Config, UnsignedConfig,
        sizeof(UnsignedConfig)/sizeof(*UnsignedConfig));
    SetValStringW(key, ShortRegName, cd->Config.ShortName);
    SetValArray(key, HotKeysRegName, cd->Config.HotKeys,
        sizeof(cd->Config.HotKeys));

    SetSpeedListConfig(key, &cd->Config.CDList, CDListRegName);
    SetSpeedListConfig(key, &cd->Config.DVDList, DVDListRegName);
    SetSpeedListConfig(key, &cd->Config.BDList, BDListRegName);

    if(ERROR_SUCCESS==RegCreateKeyEx(key, LockRegKey, 0, NULL, 0,
            KEY_ALL_ACCESS, NULL, &hash_key, &disp))
        {
        SetValDWORD(hash_key, LockRegName, cd->Config.AutoLock);
        RegCloseKey(hash_key);
        }
    
    RegCloseKey(key);
    }

void GetCDROMList(void)
    {
    UNI_ADDR addr;
    
    FreeCDROMList();

    if(GetFirstCDROM(&addr)==FALSE)
        return;
    do
        {
        AddCDROM(&addr);
        }
    while(GetNextCDROM(&addr));
    }

void FreeCDROMList(void)
    {
    CDROM_Count=0;
    if(CDROM_List!=NULL)
        {
        free(CDROM_List);
        CDROM_List=NULL;
        }
    }

static void AddCDROM(UNI_ADDR *addr)
    {
    int i;

    if(CDROM_Count>=MAXCOUNT_CD)
        return;

    /* Allocate memory for new list item */
    if(CDROM_List==NULL)
        {
        CDROM_List=malloc(sizeof(*CDROM_List));
        if(CDROM_List==NULL)
            return;
        }
    else
        {
        void *p=realloc(CDROM_List, sizeof(*CDROM_List)*(CDROM_Count+1));
        if(p==NULL)
            return;
        else
            CDROM_List=p;
        }

    /* Initialize structure to 0 */
    ZeroMemory(CDROM_List+CDROM_Count, sizeof(*CDROM_List));

    /* Store CDROM SCSI address */
    CDROM_List[CDROM_Count].State.Addr=*addr;

    /* Get CDROM name and index */
    if(GetCDROMFullName(CDROM_List+CDROM_Count)==FALSE &&
        Config.ShowUnknown==FALSE)
        return;
    CDROM_List[CDROM_Count].Config.Index=0;
    for(i=CDROM_Count-1; i>=0; i--)
        if(lstrcmp(
                CDROM_List[CDROM_Count].Config.FullName,
                CDROM_List[i].Config.FullName
                )==0)
            {
            CDROM_List[CDROM_Count].Config.Index=CDROM_List[i].Config.Index+1;
            break;
            }

    /* Init constants */
    CDROM_List[CDROM_Count].Config.CDList.x1=CD1X;
    CDROM_List[CDROM_Count].Config.DVDList.x1=DVD1X;
    CDROM_List[CDROM_Count].Config.BDList.x1=BD1X;

    /* Init state */
    CDROM_List[CDROM_Count].State.SL=&CDROM_List[CDROM_Count].Config.CDList;
    CDROM_List[CDROM_Count].State.TrayClosed=TRUE;
    
    /* Get CDROM capabilities (from registry or detect) */
    if(!GetCDROMCaps(CDROM_List+CDROM_Count))
        {
        CheckReadCaps(CDROM_List+CDROM_Count);
        ScanCDROMSpeedAuto(CDROM_List+CDROM_Count, NULL, NULL);
        StoreCDROMCaps(CDROM_List+CDROM_Count);
        }

    /* Init config */
    CDROM_List[CDROM_Count].Config.EventMask=
        EVENT_ATRUN|EVENT_ATCHANGE|EVENT_ATRESUME;
    CDROM_List[CDROM_Count].Config.ShortMenu=
        (CDROM_List[CDROM_Count].Config.CDList.Count>8);
    CDROM_List[CDROM_Count].Config.ForceRead=TRUE;

    /* Get CDROM configuration from registry */
    GetCDROMConfig(CDROM_List+CDROM_Count);

    CDROM_Count++;
    }

void CheckAllStatus(void)
    {
    unsigned i;

    for(i=0; i<CDROM_Count; i++)
        if(!CDROM_List[i].Config.Ignore)
            CheckDriveState(CDROM_List+i);
    }

void SetAllSpeed(unsigned event, ULONG unit_mask)
    {
    unsigned i;
    int letter_mask;

    for(i=0; i<CDROM_Count; i++)
        {
        if(CDROM_List[i].Config.Ignore)
            continue;
        if(CDROM_List[i].Config.AutoLock)
            SetCDROMLock(&CDROM_List[i].State.Addr, TRUE);
        if(CDROM_List[i].State.Addr.Letter!=0)
            letter_mask=1<<(CDROM_List[i].State.Addr.Letter-'A');
        else
            letter_mask=0;
        CheckDriveState(CDROM_List+i);
        if(CDROM_List[i].Config.EventMask&event &&
            (unit_mask==0 || letter_mask==0 || (unit_mask&letter_mask)!=0))
            RestoreCDROMSpeed(CDROM_List+i);
        }
    }

static int MatchCDROMName(
    struct CDROM_Desc const *cd,
    char const *name,
    WCHAR const *nameW
    )
    {
    if(*name == '\0')
        return TRUE;

    if(_stricmp(name, cd->Config.FullName) == 0)
        return TRUE;

    if(_wcsicmp(nameW, cd->Config.ShortName) == 0)
        return TRUE;

    if(_wcsicmp(nameW, cd->Config.AutoShortName) == 0)
        return TRUE;

    if((unsigned)name[0] == cd->State.Addr.Letter && name[1] == '\0')
        return TRUE;

    if((unsigned)name[0] == cd->State.Addr.Letter + 0x20 && name[1] == '\0')
        return TRUE;

    return FALSE;
    }

static int MatchCDROMIndex(struct CDROM_Desc const *cd, int idx)
    {
    return idx < 0 || (unsigned)idx == cd->Config.Index;
    }

static unsigned MatchXCmd(struct CDROM_SpeedList *sl, int xspeed)
    {
    int speed;
    unsigned match;
    int delta;
    unsigned i;

    if(xspeed == INT_MAX)
        {
        if(sl->Count == 0)
            return 0xFFFF;

        else
            return sl->Range[0].Command;
        }

    if(xspeed == INT_MIN)
        {
        if(sl->Count == 0)
            return sl->x1;

        else
            return sl->Range[sl->Count - 1].Command;
        }

    speed = xspeed * sl->x1;

    if(sl->Count == 0)
        return speed;

    match = 0;
    delta = abs(speed - (int)(sl->Range[match].Speed));

    for(i = 1; i < sl->Count; i++)
        {
        int di = abs(speed - (int)(sl->Range[i].Speed));

        if(delta > di)
            {
            match = i;
            delta = di;
            }
        }

    return sl->Range[match].Command;
    }

void SetSpeedByName(char const *name, WCHAR const *nameW, int idx, int speed)
    {
    unsigned i;

    for(i = 0; i < CDROM_Count; i++)
        {
        struct CDROM_Desc *cd = CDROM_List + i;

        if(cd->Config.Ignore)
            continue;

        if(MatchCDROMName(cd, name, nameW) && MatchCDROMIndex(cd, idx))
            {
            unsigned command;

            CheckDriveState(cd);
            command = MatchXCmd(cd->State.SL, speed);
            DebugLog(("Set speed: '%s' %d %d: '%s' %u\n",
                name, idx, speed, cd->Config.FullName, command));
            CommandCDROM(cd, command);
            }
        }
    }

void EjectByName(char const *name, WCHAR const *nameW, int idx, int eject)
    {
    unsigned i;

    for(i = 0; i < CDROM_Count; i++)
        {
        struct CDROM_Desc *cd = CDROM_List + i;

        if(cd->Config.Ignore)
            continue;

        if(MatchCDROMName(cd, name, nameW) && MatchCDROMIndex(cd, idx))
            {
            DebugLog(("Eject: '%s' %d %d: '%s'\n", name, idx, eject, cd->Config.FullName));
            EjectCDROM(cd, eject);
            }
        }
    }

void RefreshAllDrives(void)
    {
    DWORD devmask;
    int letter;
    UINT DriveType;
    char path[]="X:\\";


    if(!Config.ForceRefresh)
        return;

    devmask=GetLogicalDrives();
    letter='A';
    while(devmask)
        {
        if(devmask&1)
            {
            path[0]=letter;
            DriveType=GetDriveType(path);
            if(DriveType==DRIVE_CDROM)
                {
                GetVolumeInformationA(path, NULL, 0, NULL, NULL, NULL, NULL,
                    0);
                }
            }
        devmask>>=1;
        letter++;
        }
    }

unsigned EnumAllDisks(BOOL do_eject)
    {
    unsigned i;
    struct CDROM_Desc *cd;
    unsigned ret=0;

    CheckAllStatus();
    for(i=0, cd=CDROM_List; i<CDROM_Count; ++i, ++cd)
        {
        if(!cd->Config.Ignore && !(cd->Config.AutoLock && *Config.Hash) &&
            cd->State.DiskPresent
            )
            {
            if(do_eject)
                EjectCDROM(cd, TRUE);
            if(cd->State.Addr.Letter)
                ret|=1<<(cd->State.Addr.Letter-'A');
            ret|=0x80000000u;
            }
        }

    return ret;
    }

void EjectAllDisks(void)
    {
    EnumAllDisks(TRUE);
    }

BOOL IsAllEmpty(void)
    {
    return 0==EnumAllDisks(FALSE);
    }

void CloseAllDisks(void)
    {
    unsigned i;
    struct CDROM_Desc *cd;

    for(i=0, cd=CDROM_List; i<CDROM_Count; ++i, ++cd)
        if(!cd->Config.Ignore && cd->Config.CanClose)
            EjectCDROM(cd, FALSE);
    }

BOOL IsAllClosed(void)
    {
    unsigned i;
    struct CDROM_Desc *cd;

    DebugLog(("IsAllClosed\n"));
    for(i=0, cd=CDROM_List; i<CDROM_Count; ++i, ++cd)
        {
        DebugLog(("  [%s]\n", cd->Config.FullName));
        if(!cd->Config.Ignore && cd->Config.CanClose)
            {
            CheckDriveState(cd);
            if(IsTrayOpen(cd))
                {
                DebugLog(("  FALSE\n"));
                return FALSE;
                }
            }
        DebugLog(("    OK\n"));
        }
    DebugLog(("  TRUE\n"));

    return TRUE;
    }

WCHAR *GetAllLabels(void)
    {
    unsigned mask;
    WCHAR path[]=L"A:\\";
    WCHAR buf[MAX_PATH];
    WCHAR *text;
    WCHAR *tmp;
    int len;

    text=malloc(sizeof(*text));
    if(NULL==text)
        return NULL;
    *text=0;

    for(mask=EnumAllDisks(FALSE)&0x7FFFFFFF; mask!=0; mask>>=1, path[0]++)
        {
        if(mask&1 && GetVolumeLabelU(path, buf, sizeof(buf)/sizeof(*buf)))
            {
            len=lstrlenW(text);
            tmp=realloc(text, (len+1+3+lstrlenW(buf)+2+1)*sizeof(*text));
            if(NULL==tmp)
                break;
            text=tmp;
            text[len]=*path;
            strcpyU(strcpyU(strcpyU(text+len+1, L": \""), buf), L"\"\n");
            }
        }

    return text;
    }
