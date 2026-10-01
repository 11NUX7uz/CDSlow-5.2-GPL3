/*
Copyright 2000-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <dbt.h>
#include <pbt.h>
#include "osinfo.h"
#include "dlglib.h"
#include "cdlist.h"
#include "configdata.h"
#include "regfunc.h"
#include "cdshell.h"
#include "cdscmd.h"
#include "menu.h"
#include "dialogs.h"
#include "w6notify.h"
#include "build.h"
#include "version.h"
#include "msg_ids.h"
#include "hotkeys.h"
#include "log.h"
#include "cmdline.h"
#include "cdslow.h"

#define ICON_MESSAGE (WM_USER+1)
enum
    {
    TIMER_EVENT=1,
    TIMER_CDCLOSE
    };
#define TIMER_TICK 500
#define CHECK_STEP (5000/TIMER_TICK)

static NOTIFYICONDATAW IconData;
static UINT TaskbarCreatedMsg;
static int BusyCount=0;
static unsigned BlinkIcon=0;
static BOOL Hidden=FALSE;

enum ICON_VARIANT
    {
    ICON_16=0,
    ICON_XP,
    ICON_VARIANTS
    };
enum ICON_NAME
    {
    I_TASK,
    I_TASK_RED,
    I_TASK_GREEN,
    I_QUESTION,
    I_WAIT,
    I_EXCLAMATION,
    ICON_COUNT
    };

struct IconH
    {
    HICON h;
    char *name[ICON_VARIANTS];
    };

static struct IconH Icon[ICON_COUNT]=
    {
    {NULL, {"IconTask16", "IconTaskXP"}},
    {NULL, {"IconTask16Red", "IconTaskXPRed"}},
    {NULL, {"IconTask16Green", "IconTaskXPGreen"}},
    {NULL, {"IconQuestion16", "IconQuestionXP"}},
    {NULL, {"IconWait16", "IconWaitXP"}},
    {NULL, {"IconError16", "IconErrorXP"}}
    };

static LRESULT CALLBACK WindowProc(HWND window, UINT uMsg,
    WPARAM wParam, LPARAM lParam);
static BOOL OnCreate(HWND window, CREATESTRUCT FAR *cstruct);
static void OnDestroy(HWND window);
static void OnCommand(HWND window, int id, HWND hwndCtl, UINT codeNotify);
static BOOL CreateNotifyIcon(void);
static void DeleteNotifyIcon(void);
static void BusyNotifyProc(unsigned busy);
static void UpdateNotifyIcon();
static void OnNotifyIcon(HWND window, WPARAM wParam, LPARAM lParam);
static void OnDeviceChange(HWND window, WPARAM wParam, LPARAM lParam);
static BOOL UnitIsCDROM(DWORD devmask);
static void InitApp(HWND window, BOOL list);
static void UpdateTimer(HWND window, unsigned timeout);
static void OnTimer(HWND window, UINT id);
static void OnPowerBroadcast(HWND window, DWORD event, DWORD data);
static void CDROMCommand(HWND window, unsigned cmd, unsigned i, unsigned speed);
static void OnHotKey(HWND window, UINT id, DWORD key);
static void OnTaskbarCreated(HWND window);
static void ClearConfig();
static void SetSpeed(unsigned cd, unsigned speed);
static void DoEject(HWND window, unsigned i, BOOL eject);
static void OnCDTimer(HWND window, unsigned i);
static void UpdateHotKeys(HWND window);
static void RaiseOwned(HWND window);
static BOOL OnQueryEndSession(HWND window);
static void OnEndSession(HWND window, BOOL EndSession);
static void ShowHelp(void);
static void CmdTimer(HWND window);
static void LoadIcons(struct IconH icon[], int n,
    enum ICON_VARIANT ivar, HINSTANCE hInstance);
static void PrepareNotifyIcon(HWND window);

void CDSlowPrepare(void)
    {
    UnicodeInit();
    GetConfigFromRegistry();
    SCSIInit(Config.ApiPrefered);
    GetCDROMList();
    }

void CDSlowFinish(void)
    {
    FreeCDROMList();
    }

void CDSlowOnce(void)
    {
    CDSlowPrepare();

    if(Config.CloseOnRun)
        CloseAllDisks();

    SetAllSpeed(EVENT_ATRUN, 0);

    CDSlowFinish();
    }

static int ParseDopt(
    char **name,
    WCHAR **nameW,
    int *idx,
    int *value,
    struct CMD_DriveOption const *dopt
    )
    {
    *name = "";
    *nameW = L"";
    *idx = -1;

    if(dopt->Name == NULL || dopt->NameW == NULL)
        {
        DebugLog(("    No drive name\n"));
        return FALSE;
        }

    if(value)
        {
        *value = 0;

        if(*dopt->ValueW == L'\0')
            {
            DebugLog(("    Empty value for '%s'\n", dopt->Name));
            return FALSE;
            }

        if(_wcsicmp(dopt->ValueW, L"max") == 0)
            *value = INT_MAX;

        else if(_wcsicmp(dopt->ValueW, L"min") == 0)
            *value = INT_MIN;

        else
            {
            *value = _wtoi(dopt->ValueW);

            if(*value <= 0)
                {
                DebugLog(("    Wrong speed for '%s': %d\n", dopt->Name, *value));
                return FALSE;
                }
            }
        }

    if(*dopt->IndexW != L'\0')
        {
        *idx = _wtoi(dopt->IndexW);

        if(*idx <= 0)
            {
            DebugLog(("    Wrong index for '%s': %d\n", dopt->Name, *idx));
            return FALSE;
            }

        *idx = *idx - 1;
        }

    if(dopt->NameW != dopt->ValueW || !value)
        {
        *nameW = dopt->NameW;
        *name = dopt->Name;
        }

    return TRUE;
    }

void CDSlowSet(size_t count, struct CMD_DriveOption const *dopts)
    {
    size_t i;

    for(i = 0; i != count; ++i)
        {
        int speed = 0;
        int idx = -1;
        char *name = "";
        WCHAR *nameW = L"";

        DebugLog(("CDSlowSet [%d]...\n", (int)i));

        if(ParseDopt(&name, &nameW, &idx, &speed, dopts + i))
            SetSpeedByName(name, nameW, idx, speed);

        DebugLog(("CDSlowSet [%d] DONE\n", (int)i));
        }
    }

void CDSlowEject(size_t count, struct CMD_DriveOption const *dopts)
    {
    size_t i;

    for(i = 0; i != count; ++i)
        {
        int idx = -1;
        char *name = "";
        WCHAR *nameW = L"";

        DebugLog(("CDSlowEject [%d]...\n", (int)i));

        if(ParseDopt(&name, &nameW, &idx, NULL, dopts + i))
            EjectByName(name, nameW, idx, TRUE);

        DebugLog(("CDSlowEject [%d] DONE\n", (int)i));
        }
    }

void CDSlowClose(size_t count, struct CMD_DriveOption const *dopts)
    {
    size_t i;

    for(i = 0; i != count; ++i)
        {
        int idx = -1;
        char *name = "";
        WCHAR *nameW = L"";

        DebugLog(("CDSlowClose [%d]...\n", (int)i));

        if(ParseDopt(&name, &nameW, &idx, NULL, dopts + i))
            EjectByName(name, nameW, idx, FALSE);

        DebugLog(("CDSlowClose [%d] DONE\n", (int)i));
        }
    }

void CDSlow(HINSTANCE hInstance, BOOL hide)
    {
    WNDCLASS cls;
    HWND window;
    MSG wmsg;
    enum ICON_VARIANT icon_variant;

    UnicodeInit();
    Hidden = hide;

    TaskbarCreatedMsg=RegisterWindowMessage("TaskbarCreated");
    
    if(GetShellVersionMajor()>=6 && GetDeviceCaps(GetDC(NULL), BITSPIXEL)>8)
        icon_variant=ICON_XP;
    else
        icon_variant=ICON_16;
    LoadIcons(Icon, sizeof(Icon)/sizeof(*Icon), icon_variant, hInstance);

    /* Create main window */
    cls.style=0;
    cls.lpfnWndProc=WindowProc;
    cls.cbClsExtra=0;
    cls.cbWndExtra=0;
    cls.hInstance=hInstance;
    cls.hIcon=LoadIcon(hInstance, "IconApp");
    cls.hCursor=NULL;
    cls.hbrBackground=NULL;
    cls.lpszMenuName=NULL;
    cls.lpszClassName=AppName;
    if(RegisterClass(&cls)==0)
        {
        AppMessageBox(NULL, STR_CDS_REGCLASS, MB_OK|MB_ICONERROR|MB_TASKMODAL);
        return;
        }
    window=CreateWindow(AppName, AppName, 0,
        0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if(window==NULL)
        {
        AppMessageBox(NULL, STR_CDS_CREATEWINDOW,
            MB_OK|MB_ICONERROR|MB_TASKMODAL);
        return;
        }

    /* Set icon for dialog windows */
    window=CreateWindow(WC_DIALOG, "", 0,
        0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if(window)
        {
        SetClassLong(window, GCL_HICON, (LONG)cls.hIcon);
        DestroyWindow(window);
        }


    while(GetMessage(&wmsg, NULL, 0, 0)==TRUE)
        DispatchMessage(&wmsg);
    DebugLog(("Message loop terminated\n"));

    return;
    }

static void LoadIcons(struct IconH icon[], int n,
    enum ICON_VARIANT ivar, HINSTANCE hInstance)
    {
    int i;

    for(i=0; i<n; ++i)
        icon[i].h=LoadImage(hInstance, icon[i].name[ivar], IMAGE_ICON, 0, 0,
            LR_DEFAULTCOLOR);
    }

static BOOL MessageLog=FALSE;

static LRESULT CALLBACK WindowProc(HWND window, UINT message,
    WPARAM wParam, LPARAM lParam)
    {
    if(MessageLog)
        DebugLog(("MSG: %10d(0x%08X)\n", message, message));

    switch(message)
        {
    HANDLE_MSG(window, WM_CREATE, OnCreate);
    HANDLE_MSG(window, WM_DESTROY, OnDestroy);
    HANDLE_MSG(window, WM_COMMAND, OnCommand);
    HANDLE_MSG(window, WM_TIMER, OnTimer);
    HANDLE_MSG(window, WM_QUERYENDSESSION, OnQueryEndSession);
    HANDLE_MSG(window, WM_ENDSESSION, OnEndSession);
    case ICON_MESSAGE:
        OnNotifyIcon(window, wParam, lParam);
        return 0;
    case WM_DEVICECHANGE:
        OnDeviceChange(window, wParam, lParam);
        break;
    case WM_POWERBROADCAST:
        OnPowerBroadcast(window, wParam, lParam);
        return TRUE;
    case WM_HOTKEY:
        OnHotKey(window, wParam, lParam);
        return 0;
    default:
        if(message==TaskbarCreatedMsg)
            OnTaskbarCreated(window);
        }

    return DefWindowProc(window, message, wParam, lParam);
    }

static BOOL OnCreate(HWND window, CREATESTRUCT FAR *cstruct)
    {
    (void)cstruct; /* Unused */

    PrepareNotifyIcon(window);
    if(!Hidden)
        CreateNotifyIcon();
    RegisterBusyNotify(BusyNotifyProc);
    if(!CreateRMenu())
        {
        AppMessageBox(NULL, STR_CDS_RMENU, MB_OK|MB_ICONERROR|MB_TASKMODAL);
        return FALSE;
        }
    CreateLMenu();
    InitApp(window, TRUE);
    if(Config.CloseOnRun)
        CloseAllDisks();

    return TRUE;
    }

static void InitApp(HWND window, BOOL list)
    {
    BusyNotifyProc(1);
    GetConfigFromRegistry();
    DebugLog(("Executing SCSIInit(%d)\n", Config.ApiPrefered));
    SCSIInit(Config.ApiPrefered);
    if(list)
        {
        DebugLog(("Getting CDROM List\n"));
        GetCDROMList();
        SetAllSpeed(EVENT_ATRUN, 0);
        }
    else
        FreeCDROMList();
    UpdateTimer(window, Config.TimerEnable);
    UpdateHotKeys(window);
    BusyNotifyProc(0);
    }

static void OnDestroy(HWND window)
    {
    if(*Config.SavePath)
        SaveConfigToFile(Config.SavePath);
    ClearHotKeys(window);
    UpdateTimer(window, 0);
    FreeCDROMList();
    DestroyLMenu();
    DestroyRMenu();
    DeleteNotifyIcon();
    PostQuitMessage(0);
    }

static void PrepareNotifyIcon(HWND window)
    {
    IconData.cbSize=sizeof(IconData);
    IconData.hWnd=window;
    IconData.uID=0;
    IconData.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
    IconData.uCallbackMessage=ICON_MESSAGE;
    IconData.hIcon=Icon[I_TASK].h;
    strcpyU(IconData.szTip, AppTitle);
    }

static BOOL CreateNotifyIcon(void)
    {
    BOOL ret;

    ret=Shell_NotifyIconU(NIM_ADD, &IconData);
    DebugLog(("CreateNotifyIcon %s\n", ret ? "OK" : "FAILED!"));

    return ret;
    }

static void DeleteNotifyIcon(void)
    {
    Shell_NotifyIconU(NIM_DELETE, &IconData);
    }

static void BusyNotifyProc(unsigned busy)
    {
    if(busy)
        ++BusyCount;
    else
        --BusyCount;
    UpdateNotifyIcon();
    }

static void UpdateNotifyIcon()
    {
    WCHAR *w;
    int space;
    unsigned i;
    struct CDROM_Desc *cd;
    WCHAR *s;
    int open_count=0, disk_count=0;
    enum ICON_NAME name;
    int rcid;

    if(Hidden)
        return;

    if(CDROM_Count==0)
        strcpyU(IconData.szTip, AppTitle);
    else
        {
        /* Initial position in tip buffer */
        w=IconData.szTip;
        space=sizeof(IconData.szTip)/sizeof(*IconData.szTip)-1;
        /* For each CDROM */
        for(i=0, cd=CDROM_List; i<CDROM_Count; ++i, ++cd)
            {
            if(cd->Config.Ignore)
                continue;
            /* Tray status check for icon */
            if(!cd->State.Busy)
                {
                if(cd->State.DiskPresent)
                    disk_count++;
                if(IsTrayOpen(cd))
                    open_count++;
                }
            /* Delimiter */
            if(IconData.szTip!=w)
                {
                --space;
                if(space<0)
                    continue;
                w=strcpyU(w, GetShellVersionMajor()<5 ? L" " : L"\n");
                }
            /* Drive name and speed */
            if(cd->Config.ShortName[0]!=0)
                s=cd->Config.ShortName;
            else
                s=cd->Config.AutoShortName;
            space-=lstrlenW(s)+1+4+1;
            if(space<0)
                continue;
            w=strcpyU(strcpyU(w, s), L" ");
            w=Num(w, SPEED_DIV(cd->State.Speed, cd->State.SL->x1)%100);
            w=strcpyU(w, L"x");
            /* Medium status */
            if(cd->State.Busy)
                rcid=STR_TRAY_BUSY;
            else if(ATAPI_OPEN==cd->State.CDI.MediumType)
                rcid=STR_TRAY_OPEN;
            else if(!cd->State.DiskPresent)
                rcid=STR_TRAY_EMPTY;
            else if(AUDIO_PLAY==cd->State.AudioStatus)
                rcid=STR_TRAY_PLAY;
            else if(AUDIO_PAUSE==cd->State.AudioStatus)
                rcid=STR_TRAY_PAUSE;
            else if(&cd->Config.CDList==cd->State.SL)
                {
                switch(cd->State.MediumLevel)
                    {
                case ML_ROM: rcid=STR_TRAY_CDROM; break;
                case ML_R: rcid=STR_TRAY_CDR; break;
                case ML_RW: rcid=STR_TRAY_CDRW; break;
                default: rcid=STR_TRAY_CD;
                    }
                }
            else if(&cd->Config.DVDList==cd->State.SL)
                {
                switch(cd->State.MediumLevel)
                    {
                case ML_ROM: rcid=STR_TRAY_DVDROM; break;
                case ML_R: rcid=STR_TRAY_DVDR; break;
                case ML_RW: rcid=STR_TRAY_DVDRW; break;
                default: rcid=STR_TRAY_DVD;
                    }
                }
            else if(&cd->Config.BDList==cd->State.SL)
                {
                switch(cd->State.MediumLevel)
                    {
                case ML_ROM: rcid=STR_TRAY_BDROM; break;
                case ML_R: rcid=STR_TRAY_BDR; break;
                case ML_RW: rcid=STR_TRAY_BDRW; break;
                default: rcid=STR_TRAY_BD;
                    }
                }
            else
                rcid=STR_TRAY_UNKNOWN;
            space-=1+LoadStringU(NULL, rcid, NULL, 0, 0)+1;
            if(space<0)
                continue;
            w=strcpyU(Rcs(strcpyU(w, L"("), rcid), L")");
            }
        }

    if(BusyCount>0)
        name=I_WAIT;
    else if(IsSCSIDriverError())
        name=I_EXCLAMATION;
    else if(CDROM_Count==0)
        name=I_QUESTION;
    else if(BlinkIcon)
        name=I_EXCLAMATION;
    else if(open_count>0 && Config.IconOpen)
        name=I_TASK_RED;
    else if(disk_count>0 && Config.IconDisk)
        name=I_TASK_GREEN;
    else
        name=I_TASK;

    IconData.hIcon=Icon[name].h;

    Shell_NotifyIconU(NIM_MODIFY, &IconData);
    }

static void UpdateTimer(HWND window, unsigned enable)
    {
    static BOOL active=FALSE;

    if(active)
        {
        KillTimer(window, TIMER_EVENT);
        active=FALSE;
        }
    if(enable!=0)
        {
        if(SetTimer(window, TIMER_EVENT, TIMER_TICK, NULL)!=0)
            active=TRUE;
        }
    }

static void UpdateHotKeys(HWND window)
    {
    ClearHotKeys(window);
    SetHotKeys(window);
    }

static void OnNotifyIcon(HWND window, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */

    if(lParam==WM_RBUTTONDOWN)
        {
        if(IsWindowEnabled(window))
            PostMessage(window, WM_COMMAND, CMD_SHOW_RMENU, 0);
        else
            RaiseOwned(window);
        }
    else if(lParam==WM_LBUTTONDOWN)
        PostMessage(window, WM_COMMAND, CMD_SHOW_LMENU, 0);
    }

static void RaiseOwned(HWND window)
    {
    HWND child;

    MessageBeep(MB_OK);
    child=GetWindow(window, GW_HWNDPREV);
    if(child)
        SetForegroundWindow(child);
    }

static void OnDeviceChange(HWND window, WPARAM wParam, LPARAM lParam)
    {
    DEV_BROADCAST_VOLUME *devinfo=(DEV_BROADCAST_VOLUME *)lParam;

    DebugLog(("Got WM_DEVICECHANGE with wParam=0x%X\n", wParam));
    if(wParam==DBT_DEVICEARRIVAL)
        {
        DebugLog(("Processing DBT_DEVICEARRIVAL\n"));
        if(devinfo->dbcv_devicetype==DBT_DEVTYP_VOLUME)
            {
            DebugLog(("New device is DBT_DEVTYP_VOLUME, unitmask=0x%X\n",
                    devinfo->dbcv_unitmask));
            if(devinfo->dbcv_flags==DBTF_MEDIA)
                {
                DebugLog(("New media arrived\n"));
                PostMessage(window, WM_COMMAND,
                    CMD_MEDIA_CHANGED, devinfo->dbcv_unitmask);
                }
            else if(devinfo->dbcv_flags!=DBTF_NET)
                {
                DebugLog(("NOT media, NOT network - maybe CDROM?\n"));
                if(UnitIsCDROM(devinfo->dbcv_unitmask))
                    {
                    DebugLog(("New volume is CDROM - rescanning CDROM's\n"));
                    PostMessage(window, WM_COMMAND, CMD_FIND_CDROM, 0);
                    }
                else
                    {
                    DebugLog(("No, it's not CDROM.\n"));
                    }
                }
            }
        }
    else if(wParam==DBT_DEVICEREMOVECOMPLETE)
        {
        DebugLog(("Processing DBT_DEVICEREMOVECOMPLETE\n"));
        if(devinfo->dbcv_devicetype==DBT_DEVTYP_VOLUME)
            {
            DebugLog(("Removed device is DBT_DEVTYP_VOLUME, unitmask=0x%X\n",
                    devinfo->dbcv_unitmask));
            if(devinfo->dbcv_flags==DBTF_MEDIA)
                {
                DebugLog(("Media removed\n"));
                PostMessage(window, WM_COMMAND, CMD_MEDIA_REMOVED, 0);
                }
            else if(devinfo->dbcv_flags!=DBTF_NET)
                {
                DebugLog(("NOT media, NOT network - rescanning CDROM's\n"));
                PostMessage(window, WM_COMMAND, CMD_FIND_CDROM, 0);
                }
            }
        }
    }

static BOOL UnitIsCDROM(DWORD devmask)
    {
    int letter;
    UINT DriveType;
    char path[]="X:\\";

    letter='A';
    while(devmask && letter<='Z')
        {
        if(devmask&1)
            {
            path[0]=letter;
            DriveType=GetDriveType(path);
            if(DriveType==DRIVE_CDROM)
                return TRUE;
            }
        devmask>>=1;
        letter++;
        }

    return FALSE;
    }

static void OnTimer(HWND window, UINT id)
    {
    if(id==TIMER_EVENT)
        PostMessage(window, WM_COMMAND, CMD_TIMER, 0);
    else if(id>=TIMER_CDCLOSE)
        PostMessage(window, WM_COMMAND,
            MKCDCMD(CMD_CD_TIMER, id-TIMER_CDCLOSE), 0);
    }

static void OnPowerBroadcast(HWND window, DWORD event, DWORD data)
    {
    (void)data; /* Unused */

    if(event==PBT_APMRESUMESUSPEND || event==PBT_APMRESUMECRITICAL)
        PostMessage(window, WM_COMMAND, CMD_POWER_RESUME, 0);
    }

static void OnHotKey(HWND window, UINT id, DWORD key)
    {
    (void)key; /* Unused */

    if(HIBYTE(id)<CDROM_HOTKEYS)
        PostMessage(window, WM_COMMAND,
            MKCDCMD(HotKeyCmd[HIBYTE(id)], LOBYTE(id)), 0);
    }

static void OnTaskbarCreated(HWND window)
    {
    (void)window; /* Unused */

    DebugLog(("Got TaskbarCreated notification\n"));
    if(!Hidden)
        {
        CreateNotifyIcon();
        UpdateNotifyIcon();
        }
    }

static DWORD EndStartAt=0;
static BOOL OnQueryEndSession(HWND window)
    {
    DebugLog(("OnQueryEndSession\n"));

    EndStartAt=GetTickCount();

    if(Config.DNEject)
        {
        DiskInDriveW6(window, 0, 0);
        DebugLog(("    EjectAllDisks\n"));
        EjectAllDisks();
        }
    else if(Config.DNMessage && !IsAllEmpty())
        {
        if(!DiskInDriveW6(window, 0, 0))
            DiskInDriveNotify(window, Config.DNTimer);
        DebugLog(("OnQueryEndSession DONE\n"));
        return TRUE;
        }

    if((Config.DNMessage || Config.DNClose) && !IsAllClosed())
        TrayOpenW6(window, 0, FALSE, 0);

    DebugLog(("OnQueryEndSession DONE\n"));

    return TRUE;
    }

static void OnEndSession(HWND window, BOOL EndSession)
    {
    unsigned interval=5;

    (void)EndSession; /* Unused */

    interval=(GetTickCount()-EndStartAt)/1000;
    if(interval>=5)
        interval=0;
    else
        interval=5-interval;

    DebugLog(("OnEndSession\n"));
    if(Config.DNMessage && !IsAllEmpty() && OsVersion()>=0x010600)
        {
        DiskInDriveW6(window, Config.DNTimer, interval);
        interval=0;
        }

    if(!IsAllClosed())
        {
        if(Config.DNMessage)
            {
            if(OsVersion()<0x010600)
                TrayOpenNotify(window, Config.DNTimer, Config.DNClose);
            else
                {
                TrayOpenW6(window, Config.DNTimer, Config.DNClose, interval);
                interval=0;
                }
            }
        else if(Config.DNClose)
            TrayOpenW6(window, Config.DNTimer, Config.DNClose, 0);
        }

    ShutdownBlockReasonDestroyDll(window);

    DebugLog(("OnEndSession DONE\n"));
    }

static void OnCommand(HWND window, int id, HWND hwndCtl, UINT codeNotify)
    {
    (void)codeNotify; /* Unused */

    switch(id)
        {
    case CMD_SHOW_RMENU: RunRMenu(window);
        break;
    case CMD_SHOW_LMENU:
        DebugLog(("LEFT CLICK\n"));
        CheckAllStatus();
        DebugLog(("CheckAllStatus DONE\n"));
        if(CDROM_Count==0)
            InfoDialog(window, CDROM_List, CDROM_Count);
        else
            RunLMenu(window);
        break;
    case CMD_ABOUT: AboutDialog(window);
        break;
    case CMD_HELP: ShowHelp();
        break;
    case CMD_INFO: InfoDialog(window, CDROM_List, CDROM_Count);
        break;
    case CMD_FIND_CDROM: InitApp(window, TRUE);
        break;
    case CMD_CONFIG:
        {
        int disable_aspi=Config.ApiPrefered&UAPI_DISABLE_ASPI;

        if(EditConfig(window, &Config))
            {
            StoreConfigToRegistry();
            if(*Config.SavePath)
                SaveConfigToFile(Config.SavePath);
            if(0!=(Config.ApiPrefered&UAPI_DISABLE_ASPI) && 0==disable_aspi)
                {
                DebugLog(("ASPI disabled - restarting...\n"));
                ShellExecute(NULL, NULL, GetExePath(), GetCmdOptRestart(),
                    NULL, SW_SHOWDEFAULT);	
                DebugLog(("    DONE\n"));
                }
            else
                InitApp(window, TRUE);
            }
        }
        break;
    case CMD_CLEARCONFIG:
        if((!*Config.Hash || AppQueryPassword(window, Config.Hash))
                && IDOK==AppMessageBox(window,
                STR_CDS_DELCONFIG, MB_ICONQUESTION|MB_OKCANCEL|MB_TASKMODAL)
            )
            {
            ClearConfig();
            InitApp(window, FALSE);
            }
        break;
    case CMD_EXIT: DestroyWindow(window);
        break;
    case CMD_MEDIA_CHANGED: SetAllSpeed(EVENT_ATCHANGE, (ULONG)hwndCtl);
        break;
    case CMD_MEDIA_REMOVED: CheckAllStatus();
        break;
    case CMD_TIMER: CmdTimer(window);
        break;
    case CMD_POWER_RESUME: SetAllSpeed(EVENT_ATRESUME, 0);
        break;
    case CMD_ICON_HIDE:
        if(!Hidden)
            DeleteNotifyIcon();
        Hidden=TRUE;
        break;
    case CMD_ICON_SHOW:
        CreateNotifyIcon();
        UpdateNotifyIcon();
        Hidden=FALSE;
        break;
    default:
        if(ISCDCMD(id))
            CDROMCommand(window, GETCDCMD(id), GETCDNUM(id), GETCDSPEED(id));
        break;
        }
    UpdateNotifyIcon();
    }

static void ShowHelp(void)
    {
    char *help;

    help=GetHelpPath();
    if(NULL!=help)
        {
        ShellExecute(NULL, "open", help, NULL, NULL, SW_SHOWNORMAL);
        free(help);
        }
    }

static void ClearConfig()
    {
    ClearConfigToRegistry();
    DeleteAutoLoad(AppName);
    }

static void CDROMCommand(HWND window, unsigned cmd, unsigned i, unsigned speed)
    {
    if(i>=CDROM_Count || (CDROM_List[i].Config.Ignore && cmd!=CMD_CD_EDIT))
        {
        if(CMD_CD_TIMER==cmd)
            KillTimer(window, TIMER_CDCLOSE+i);
        return;
        }

    DebugLog(("CDROMCommand: cmd=%u, i=%u, speed=%u\n", cmd, i, speed));
    switch(cmd)
        {
    case CMD_CD_EDIT:
        if(EditDriveOptions(window, CDROM_List+i, Config.Hash))
            {
            StoreCDROMCaps(CDROM_List+i);
            StoreCDROMConfig(CDROM_List+i);
            UpdateHotKeys(window);
            if(*Config.SavePath)
                SaveConfigToFile(Config.SavePath);
            }
        CheckDriveState(CDROM_List+i);
        break;
    case CMD_CD_EJECT:
        if(!Config.EjectSingle)
            DoEject(window, i, TRUE);
        else
            DoEject(window, i, EjectOrClose(CDROM_List+i));
        break;
    case CMD_CD_CLOSE:
        DoEject(window, i, FALSE);
        break;
    case CMD_CD_BROWSE:
        DoCDROM(CDROM_List+i, "open");
        break;
    case CMD_CD_EXPLORE:
        DoCDROM(CDROM_List+i, "explore");
        break;
    case CMD_CD_AUTORUN:
        if(!DoAutorunInf(CDROM_List+i))
            DoCDROM(CDROM_List+i, "open");
        break;
    case CMD_CD_SETMAX:
        CheckDriveState(CDROM_List+i);
        if(CDROM_List[i].State.SL->Count>0)
            CommandCDROM(CDROM_List+i,
                CDROM_List[i].State.SL->Range[0].Command);
        break;
    case CMD_CD_SETSEL:
        CheckDriveState(CDROM_List+i);
        if(CDROM_List[i].State.SL->Selected!=0)
            CommandCDROM(CDROM_List+i,
                GetCmd(CDROM_List[i].State.SL,
                    CDROM_List[i].State.SL->Selected));
        break;
    case CMD_CD_SETMIN:
        CheckDriveState(CDROM_List+i);
        if(CDROM_List[i].State.SL->Count>0)
            CommandCDROM(CDROM_List+i, CDROM_List[i].State.SL->
                Range[CDROM_List[i].State.SL->Count-1].Command);
        break;
    case CMD_CD_SPEEDUP:
        CheckDriveState(CDROM_List+i);
        UpperCDROMSpeed(CDROM_List+i);
        break;
    case CMD_CD_SPEEDDOWN:
        CheckDriveState(CDROM_List+i);
        LowerCDROMSpeed(CDROM_List+i);
        break;
    case CMD_CD_TIMER:
        OnCDTimer(window, i);
        break;
    case CMD_SELECT_SPEED:
        SetSpeed(i, speed);
        if(*Config.SavePath)
            SaveConfigToFile(Config.SavePath);
        break;
        }
    }

static void SetSpeed(unsigned cd, unsigned speed)
    {
    CheckDriveState(CDROM_List+cd);
    if(speed<CDROM_List[cd].State.SL->Count)
        {
        CDROM_List[cd].State.SL->Selected=
            CDROM_List[cd].State.SL->Range[speed].Speed;
        StoreCDROMConfig(CDROM_List+cd);
        CommandCDROM(CDROM_List+cd,
            CDROM_List[cd].State.SL->Range[speed].Command);
        }
    }

static void DoEject(HWND window, unsigned i, BOOL eject)
    {
    if(eject && CDROM_List[i].Config.AutoLock && *Config.Hash &&
        !AppQueryPassword(window, Config.Hash))
            return;
    if(!eject)
        RefreshAllDrives();
    EjectCDROM(CDROM_List+i, eject);
    if(eject)
        RefreshAllDrives();
    CheckDriveState(CDROM_List+i);
    if(!eject && (CDROM_List[i].Config.EventMask&EVENT_ATCLOSE) &&
        CDROM_List[i].State.SL->Selected!=0)
        {
        if(CDROM_List[i].State.DiskPresent)
            RestoreCDROMSpeed(CDROM_List+i);
        else
            {
            CDROM_List[i].State.TimerCount=30;
            SetTimer(window, TIMER_CDCLOSE+i, 500, NULL);
            }
        }
    }

static void OnCDTimer(HWND window, unsigned i)
    {
    CheckDriveState(CDROM_List+i);
    if(CDROM_List[i].State.DiskPresent)
        {
        RestoreCDROMSpeed(CDROM_List+i);
        CDROM_List[i].State.TimerCount=0;
        }
    else
        --CDROM_List[i].State.TimerCount;
    if(0==CDROM_List[i].State.TimerCount)
        KillTimer(window, TIMER_CDCLOSE+i);
    }

static void CmdTimer(HWND window)
    {
    int need_blink=0;
    DWORD tick, tick_count;
    unsigned i;
    BOOL force_read;
    int delta;
    unsigned timer_step;

    tick_count=GetTickCount();
    tick=tick_count/TIMER_TICK;
    for(i=0; i<CDROM_Count; ++i)
        {
        if(CDROM_List[i].Config.Ignore || CDROM_List[i].State.Busy)
            continue;
        if(0==tick%CHECK_STEP)
            {
            DebugLog(("Timer: Checking drive [%s]\n",
                    CDROM_List[i].Config.FullName));
            if(CDROM_List[i].Config.AutoLock)
                SetCDROMLock(&CDROM_List[i].State.Addr, TRUE);
            CheckDriveState(CDROM_List+i);
            }
        timer_step=CDROM_List[i].Config.Timer*1000/TIMER_TICK;
        if(0!=timer_step && 0==tick%timer_step)
            {
            DebugLog(("Timer: Restoring speed for [%s]\n",
                CDROM_List[i].Config.FullName));
            force_read=CDROM_List[i].Config.ForceRead;
            CDROM_List[i].Config.ForceRead=FALSE;
            RestoreCDROMSpeed(CDROM_List+i);
            CDROM_List[i].Config.ForceRead=force_read;
            }
        if(Config.EnableAutoclose && Config.TimerAutoclose>=AUTOCLOSE_MIN
            && IsTrayOpen(CDROM_List+i))
            {
            DebugLog(("Timer: Tray open on [%s] at %u\n",
                    CDROM_List[i].Config.FullName, CDROM_List[i].State.OpenAt));
            delta=(int)(Config.TimerAutoclose+CDROM_List[i].State.OpenAt)
                -(int)tick_count;
            DebugLog(("Timer: delta=%d\n", delta));
            if(delta<=0)
                DoEject(window, i, FALSE);
            else if(delta<AUTOCLOSE_MIN)
                ++need_blink;
            }
        }
    if(need_blink)
        BlinkIcon=tick&1;
    else
        BlinkIcon=0;
    }

void ShowIcon(HWND window)
    {
    SendMessage(window, WM_COMMAND, CMD_ICON_SHOW, 0);
    }

void HideIcon(HWND window)
    {
    SendMessage(window, WM_COMMAND, CMD_ICON_HIDE, 0);
    }
