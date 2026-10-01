/*
Copyright 2000-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __CDLIST_H__
#define __CDLIST_H__

#include "uniscsi.h"

#define CD1X 176
#define DVD1X 1385
#define BD1X 4495
#define SPEED_DIV(s, d) (((s)*10/(d)+5)/10)

#define CDROM_NAME_SIZE (8+1+16+1+4+1)
#define CDROM_SPEED_SIZE 128
#define CDROM_SHORT_SIZE 11
#define CDROM_HOTKEYS 7

enum SPEED_METHOD
    {
    METHOD_NONE=0,
    METHOD_SPEED=1,
    METHOD_STREAMING=2
    };

enum MEDIUM_LEVEL
    {
    ML_NONE=0,
    ML_UNKNOWN=1,
    ML_RW=2,
    ML_R=4,
    ML_ROM=8,
    ML_MAX=8
    };

enum EVENT_MASK
    {
    EVENT_NONE=0,
    EVENT_ATRUN=0x01,
    EVENT_ATCHANGE=0x02,
    EVENT_ATRESUME=0x08,
    EVENT_ATCLOSE=0x10
    };

struct CDROM_SpeedPair
    {
    unsigned Speed;
    unsigned Command;
    };

struct CDROM_SpeedList
    {
    unsigned x1;
    unsigned CanGetSpeed;
    unsigned CanSetSpeed;
    unsigned CanGetStreaming;
    unsigned CanSetStreaming;
    unsigned MethodDetected;
    unsigned MethodPrefered;
    enum MEDIUM_LEVEL MaxLevel;
    unsigned MaxSpeed;
    unsigned Selected;
    unsigned Count;
    struct CDROM_SpeedPair Range[CDROM_SPEED_SIZE];
    };

struct CDROM_Config
    {
    char FullName[CDROM_NAME_SIZE];
    WCHAR AutoShortName[CDROM_SHORT_SIZE];
    unsigned Index;
    /* CDROM parameters */
    unsigned CanClose;
    unsigned CanSenseMedium;
    unsigned IsDVD;
    unsigned IsBD;
    /* CDROM speed range */
    struct CDROM_SpeedList CDList;
    struct CDROM_SpeedList DVDList;
    struct CDROM_SpeedList BDList;
    /* Config parameters */
    unsigned Ignore;
    unsigned EventMask;
    unsigned OnlyWithDisk;
    unsigned WhenAudio;
    unsigned IgnoreSpeed;
    unsigned ShortMenu;
    unsigned ForceRead;
    unsigned AutoLock;
    unsigned Timer;
    unsigned HotKeys[CDROM_HOTKEYS];
    WCHAR ShortName[CDROM_SHORT_SIZE];
    };

struct CDROM_State
    {
    UNI_ADDR Addr;
    /* Current drive parameters */
    unsigned Speed;
    unsigned DiskPresent;
    unsigned TrayClosed;
    struct SCSI_CDInfo CDI;
    enum CDROM_PROFILE Profile;
    enum CDROM_AUDIO_STATUS AudioStatus;
    enum MEDIUM_LEVEL MediumLevel;
    struct CDROM_SpeedList *SL;
    /* Current drive variables */
    unsigned Busy;
    unsigned TimerCount;
    unsigned OpenAt;
    };

struct CDROM_Desc
    {
    struct CDROM_Config Config;
    struct CDROM_State State;
    };

extern struct CDROM_Desc *CDROM_List;
extern unsigned CDROM_Count;

typedef BOOL progress_func(
    unsigned min, unsigned max, unsigned pos,
    unsigned speed, unsigned command,
    void *param);
typedef void busy_func(unsigned busy);

void RegisterBusyNotify(busy_func *f);
BOOL GetCDROMFullName(struct CDROM_Desc *cd);
void ScanCDROMSpeedAuto(struct CDROM_Desc *cd, progress_func *pf, void *param);
void ScanCDROMSpeedFast(struct CDROM_Desc *cd, progress_func *pf, void *param);
void ScanCDROMSpeedFull(struct CDROM_Desc *cd, progress_func *pf, void *param);
void ScanCDROMSpeedRead(struct CDROM_Desc *cd, progress_func *pf, void *param);
void CheckDriveState(struct CDROM_Desc *cd);
void RestoreCDROMSpeed(struct CDROM_Desc *cd);
unsigned GetCmd(struct CDROM_SpeedList *sl, unsigned speed);
void UpperCDROMSpeed(struct CDROM_Desc *cd);
void LowerCDROMSpeed(struct CDROM_Desc *cd);
void CommandCDROM(struct CDROM_Desc *cd, unsigned command);
void EjectCDROMNoSeize(struct CDROM_Desc *cd, BOOL eject);
void EjectCDROM(struct CDROM_Desc *cd, BOOL eject);
BOOL EjectOrClose(struct CDROM_Desc *cd);
BOOL IsCDAMedium(unsigned mt);
BOOL IsTrayOpen(struct CDROM_Desc *cd);
void CheckReadCaps(struct CDROM_Desc *cd);
BOOL CheckDriveStateOnly(struct CDROM_Desc *cd);
enum SPEED_METHOD ListSetMethod(struct CDROM_SpeedList *sl,
    enum SPEED_METHOD def_method);

void GetCDROMList(void);
void FreeCDROMList(void);
void CheckAllStatus(void);
void SetAllSpeed(unsigned event, ULONG unit_mask);
void RefreshAllDrives(void);
unsigned EnumAllDisks(BOOL do_eject);
BOOL IsAllEmpty(void);
void EjectAllDisks(void);
BOOL IsAllClosed(void);
void CloseAllDisks(void);
WCHAR *GetAllLabels(void);
void SetSpeedByName(char const *name, WCHAR const *nameW, int idx, int speed);
void EjectByName(char const *name, WCHAR const *nameW, int idx, int eject);

void GetCDROMConfig(struct CDROM_Desc *cd);
void StoreCDROMConfig(struct CDROM_Desc *cd);
BOOL GetCDROMCaps(struct CDROM_Desc *cd);
void StoreCDROMCaps(struct CDROM_Desc *cd);

#endif /* __CDLIST_H__ */
