/*
Copyright 2005-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "uniscsi.h"
#include "regfunc.h"
#include "dlglib.h"
#include "msg_ids.h"
#include "version.h"
#include "log.h"
#include "osinfo.h"
#include "dialogs.h"


static BOOL Button_OK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Chk_AutoRun(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Chk_CDAutoAll(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_PASS(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_SAVECONFIG(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Chk_TimerEnable(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Chk_DisableASPI(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);

enum
    {
    ID_GRP_ROW=100,
    ID_GRP_COL1,
    ID_GRP_COL2,
    ID_GRP_RUN,
    ID_CHK_AUTORUN,
    ID_CHK_AUTOEXIT,
    ID_CHK_HIDE,
    ID_CHK_CLOSEONRUN,
    ID_GRP_TIMER,
    ID_CHK_TIMERENABLE,
    ID_CHK_AUTOCLOSE,
    ID_GRP_AUTOCLOSE,
    ID_INPUT_ACTIME,
    ID_LABEL_ACTIME,
    ID_SPACER_ACPRE1,
    ID_SPACER_ACPRE2,
    ID_SPACER_ACPOST,
    ID_GRP_SYSTEM,
    ID_CHK_CDAUTOALL,
    ID_CHK_CDAUTORUN,
    ID_CHK_CDAUTOPLAY,
    ID_GRP_MISC,
    ID_CHK_FORCEREFRESH,
    ID_CHK_SAVECONFIG,
    ID_LABEL_PATH,
    ID_GRP_ACCESS,
    ID_CHK_ASPI,
    ID_CHK_SPTI,
    ID_CHK_DISABLEASPI,
    ID_GRP_INTERFACE,
    ID_CHK_SHOWUNKNOWN,
    ID_CHK_NUMNAMES,
    ID_CHK_EJECTCLOSE,
    ID_CHK_TOPNAMES,
    ID_CHK_DEEPMENU,
    ID_CHK_PASSWORD,
    ID_CHK_ICONDISK,
    ID_CHK_ICONOPEN,
    ID_CHK_CMDOPEN,
    ID_CHK_CMDEXPLORE,
    ID_CHK_CMDAUTORUN,
    ID_LABEL_HASH,
    ID_GRP_DISKNOTIFY,
    ID_CHK_DNMESSAGE,
    ID_CHK_DNEJECT,
    ID_CHK_DNCLOSE,
    ID_LABEL_TIMEOUT,
    ID_GRP_TIMEOUT,
    ID_RADIO_TIMEOUTMIN,
    ID_RADIO_TIMEOUT5,
    ID_RADIO_TIMEOUT8,
    ID_RADIO_TIMEOUT10,
    ID_RADIO_TIMEOUTMAX,
    ID_GRP_BUTTONS
    };

static struct DLG_Item Items[]=
    {
    {&CtlGroupH, ID_GRP_ROW, NULL, 0, 0, NULL},
    {&CtlGroupV, ID_GRP_COL1, NULL, 0, ID_GRP_ROW, NULL},
    {&CtlGroupBoxV, ID_GRP_INTERFACE, NULL, STR_CONF_INTERFACE, ID_GRP_COL1, NULL},
    {&CtlCheckBox, ID_CHK_NUMNAMES, NULL, STR_CONF_NUMNAMES, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_EJECTCLOSE, NULL, STR_CONF_EJECTCLOSE, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_TOPNAMES, NULL, STR_CONF_TOPNAMES, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_DEEPMENU, NULL, STR_CONF_DEEPMENU, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_PASSWORD, NULL, STR_CONF_PASSWORD, ID_GRP_INTERFACE, Button_PASS},
    {&CtlCheckBox, ID_CHK_CMDOPEN, NULL, STR_CONF_CMDOPEN, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_CMDEXPLORE, NULL, STR_CONF_CMDEXPLORE, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_CMDAUTORUN, NULL, STR_CONF_CMDAUTORUN, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_ICONDISK, NULL, STR_CONF_ICONDISK, ID_GRP_INTERFACE, NULL},
    {&CtlCheckBox, ID_CHK_ICONOPEN, NULL, STR_CONF_ICONOPEN, ID_GRP_INTERFACE, NULL},
    {&CtlLabel, ID_LABEL_HASH, NULL, 0, ID_CHK_PASSWORD, NULL},
    {&CtlGroupBoxV, ID_GRP_SYSTEM, NULL, STR_CONF_SYSTEM, ID_GRP_COL1, NULL},
    {&CtlCheckBox, ID_CHK_CDAUTOALL, NULL, STR_CONF_CDAUTOALL, ID_GRP_SYSTEM, Chk_CDAutoAll},
    {&CtlCheckBox, ID_CHK_CDAUTORUN, NULL, STR_CONF_CDAUTORUN, ID_GRP_SYSTEM, Chk_CDAutoAll},
    {&CtlCheckBox, ID_CHK_CDAUTOPLAY, NULL, STR_CONF_CDAUTOPLAY, ID_GRP_SYSTEM, NULL},
    {&CtlGroupBoxV, ID_GRP_MISC, NULL, STR_CONF_MISC, ID_GRP_COL1, NULL},
    {&CtlCheckBox, ID_CHK_SHOWUNKNOWN, NULL, STR_CONF_SHOWUNKNOWN, ID_GRP_MISC, NULL},
    {&CtlCheckBox, ID_CHK_FORCEREFRESH, NULL, STR_CONF_FORCEREFRESH, ID_GRP_MISC, NULL},
    {&CtlCheckBox, ID_CHK_SAVECONFIG, NULL, STR_CONF_SAVECONFIG, ID_GRP_MISC, Button_SAVECONFIG},
    {&CtlLabel, ID_LABEL_PATH, NULL, 0, ID_CHK_SAVECONFIG, NULL},
    {&CtlGroupV, ID_GRP_COL2, NULL, 0, ID_GRP_ROW, NULL},
    {&CtlGroupBoxV, ID_GRP_RUN, NULL, STR_CONF_RUN, ID_GRP_COL2, NULL},
    {&CtlCheckBox, ID_CHK_AUTORUN, NULL, STR_CONF_AUTORUN, ID_GRP_RUN, Chk_AutoRun},
    {&CtlCheckBox, ID_CHK_AUTOEXIT, NULL, STR_CONF_AUTOEXIT, ID_GRP_RUN, Chk_AutoRun},
    {&CtlCheckBox, ID_CHK_HIDE, NULL, STR_CONF_HIDE, ID_GRP_RUN, Chk_AutoRun},
    {&CtlCheckBox, ID_CHK_CLOSEONRUN, NULL, STR_CONF_CLOSEONRUN, ID_GRP_RUN, NULL},
    {&CtlGroupBoxV, ID_GRP_DISKNOTIFY, NULL, STR_CONF_DISKNOTIFY, ID_GRP_COL2, NULL},
    {&CtlCheckBox, ID_CHK_DNEJECT, NULL, STR_CONF_DNEJECT, ID_GRP_DISKNOTIFY, NULL},
    {&CtlCheckBox, ID_CHK_DNCLOSE, NULL, STR_CONF_DNCLOSE, ID_GRP_DISKNOTIFY, NULL},
    {&CtlCheckBox, ID_CHK_DNMESSAGE, NULL, STR_CONF_DNMESSAGE, ID_GRP_DISKNOTIFY, NULL},
    {&CtlLabel, ID_LABEL_TIMEOUT, NULL, STR_CONF_DNTIMEOUT, ID_GRP_DISKNOTIFY, NULL},
    {&CtlGroupH, ID_GRP_TIMEOUT, NULL, 0, ID_GRP_DISKNOTIFY, NULL},
    {&CtlRadioBoxFirst, ID_RADIO_TIMEOUTMIN, L"3", 0, ID_GRP_TIMEOUT, NULL},
    {&CtlRadioBox, ID_RADIO_TIMEOUT5, L"5", 0, ID_GRP_TIMEOUT, NULL},
    {&CtlRadioBox, ID_RADIO_TIMEOUT8, L"8", 0, ID_GRP_TIMEOUT, NULL},
    {&CtlRadioBox, ID_RADIO_TIMEOUT10, L"10", 0, ID_GRP_TIMEOUT, NULL},
    {&CtlRadioBox, ID_RADIO_TIMEOUTMAX, L"15", 0, ID_GRP_TIMEOUT, NULL},
    {&CtlGroupBoxV, ID_GRP_TIMER, NULL, STR_CONF_TIMER, ID_GRP_COL2, NULL},
    {&CtlCheckBox, ID_CHK_TIMERENABLE, NULL, STR_CONF_TIMERENABLE, ID_GRP_TIMER, Chk_TimerEnable},
    {&CtlCheckBox, ID_CHK_AUTOCLOSE, NULL, STR_CONF_AUTOCLOSE, ID_GRP_TIMER, Chk_TimerEnable},
    {&CtlGroupH, ID_GRP_AUTOCLOSE, NULL, 0, ID_GRP_TIMER, NULL},
    {&CtlGroupBoxSpacer, ID_SPACER_ACPRE1, NULL, 0, ID_GRP_AUTOCLOSE, NULL},
    {&CtlGroupBoxSpacer, ID_SPACER_ACPRE2, NULL, 0, ID_GRP_AUTOCLOSE, NULL},
    {&CtlEdit, ID_INPUT_ACTIME, NULL, 0, ID_GRP_AUTOCLOSE, NULL},
    {&CtlGroupBoxSpacer, ID_SPACER_ACPOST, NULL, 0, ID_GRP_AUTOCLOSE, NULL},
    {&CtlLabel, ID_LABEL_ACTIME, NULL, STR_CONF_ACTIME, ID_GRP_AUTOCLOSE, NULL},
    {&CtlGroupBoxV, ID_GRP_ACCESS, NULL, STR_CONF_ACCESS, ID_GRP_COL2, NULL},
    {&CtlRadioBoxFirst, ID_CHK_ASPI, NULL, STR_CONF_ASPI, ID_GRP_ACCESS, NULL},
    {&CtlRadioBox, ID_CHK_SPTI, NULL, STR_CONF_SPTI, ID_GRP_ACCESS, NULL},
    {&CtlCheckBox, ID_CHK_DISABLEASPI, NULL, STR_CONF_DISABLEASPI, ID_GRP_ACCESS, Chk_DisableASPI},
    {&CtlGroupBoxH, ID_GRP_BUTTONS, NULL, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_BUTTON_OK, ID_GRP_BUTTONS, Button_OK},
    {&CtlButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_BUTTONS, NULL},
    };
    
BOOL EditConfig(HWND window, struct CDS_Config *cfg)
    {
    return IDOK==DlgRunU(
        window,
        AppTitle,
        STR_MNU_CONFIG,
        WS_BORDER|WS_CAPTION|WS_SYSMENU,
        WS_EX_APPWINDOW,
        Items,
        sizeof(Items)/sizeof(*Items),
        cfg
        );
    }

static void SetCheck(HWND window, WORD id, BOOL state)
    {
    SendDlgItemMessage(window, id, BM_SETCHECK,
        state ? BST_CHECKED : BST_UNCHECKED, 0);
    }

static BOOL IsChecked(HWND window, WORD id)
    {
    return BST_CHECKED==SendDlgItemMessage(window, id, BM_GETCHECK, 0, 0);
    }

static void EnDisAutoPlay(HWND window, int id)
    {
    BOOL state;

    if(OsVersion()>0x010500)
        state=FALSE;
    else if(OsVersion()==0x010500)
        state=IsChecked(window, ID_CHK_CDAUTOALL)&&
            IsChecked(window, ID_CHK_CDAUTORUN);
    else
        state=TRUE;
    EnableWindow(GetDlgItem(window, id), state);
    }

static void EnDisAutoLoad(HWND window)
    {
    EnableWindow(GetDlgItem(window, ID_CHK_AUTOEXIT),
        IsChecked(window, ID_CHK_AUTORUN)&&!IsChecked(window, ID_CHK_HIDE));
    EnableWindow(GetDlgItem(window, ID_CHK_HIDE),
        IsChecked(window, ID_CHK_AUTORUN)&&!IsChecked(window, ID_CHK_AUTOEXIT));
    }

static void EnDisASPI(HWND window)
    {
    BOOL disabled=IsChecked(window, ID_CHK_DISABLEASPI);

    SetCheck(window, ID_CHK_ASPI, UAPI_ASPI==SCSIGetAPI() && !disabled);
    EnableWindow(GetDlgItem(window, ID_CHK_ASPI),
        ASPI_OK==ASPIGetStatus() && !disabled);
    SetCheck(window, ID_CHK_SPTI, UAPI_SPTI==SCSIGetAPI() || disabled);
    EnableWindow(GetDlgItem(window, ID_CHK_SPTI), SPTI_OK==NTGetStatus());
    }

static void EnDisAutoclose(HWND window)
    {
    EnableWindow(GetDlgItem(window, ID_CHK_AUTOCLOSE),
        IsChecked(window, ID_CHK_TIMERENABLE));
    EnableWindow(GetDlgItem(window, ID_INPUT_ACTIME),
        IsChecked(window, ID_CHK_TIMERENABLE) &&
        IsChecked(window, ID_CHK_AUTOCLOSE));
    EnableWindow(GetDlgItem(window, ID_LABEL_ACTIME),
        IsChecked(window, ID_CHK_TIMERENABLE) &&
        IsChecked(window, ID_CHK_AUTOCLOSE));
    }

static void LoadState(HWND window, struct CDS_Config *cfg)
    {
    int i;
    unsigned t;
    unsigned hung=GetHungTimeout()/1000;

    SetCheck(window, ID_CHK_AUTORUN, IsAutoLoadAny(AppNameW));
    SetCheck(window, ID_CHK_AUTOEXIT, IsAutoLoadExit(AppNameW));
    SetCheck(window, ID_CHK_HIDE, IsAutoLoadHide(AppNameW));
    EnDisAutoLoad(window);
    SetCheck(window, ID_CHK_CLOSEONRUN, cfg->CloseOnRun);
    SetCheck(window, ID_CHK_SHOWUNKNOWN, cfg->ShowUnknown);
    SetCheck(window, ID_CHK_NUMNAMES, cfg->NumNames);
    SetCheck(window, ID_CHK_EJECTCLOSE, !cfg->EjectSingle);
    SetCheck(window, ID_CHK_TOPNAMES, cfg->TopNames);
    SetCheck(window, ID_CHK_DEEPMENU, cfg->DeepMenu);
    SetCheck(window, ID_CHK_CDAUTOALL, IsAutoRunSystem());
    EnableWindow(GetDlgItem(window, ID_CHK_CDAUTOALL), CanSetAutoRunSystem());
    SetCheck(window, ID_CHK_CDAUTORUN, IsAutoRunUser());
    EnableWindow(GetDlgItem(window, ID_CHK_CDAUTORUN), CanSetAutoRunUser());
    SetCheck(window, ID_CHK_CDAUTOPLAY, IsAutoPlayOn());
    EnDisAutoPlay(window, ID_CHK_CDAUTOPLAY);
    SetCheck(window, ID_CHK_FORCEREFRESH, cfg->ForceRefresh);
    SetCheck(window, ID_CHK_TIMERENABLE, cfg->TimerEnable);
    SetCheck(window, ID_CHK_AUTOCLOSE, cfg->EnableAutoclose);
    if(cfg->TimerAutoclose<AUTOCLOSE_MIN)
        SetCheck(window, ID_CHK_AUTOCLOSE, FALSE);
    else
        SetDlgItemInt(window, ID_INPUT_ACTIME, cfg->TimerAutoclose/1000, FALSE);
    EnDisAutoclose(window);
    SetCheck(window, ID_CHK_DISABLEASPI, cfg->ApiPrefered&UAPI_DISABLE_ASPI);
    EnDisASPI(window);
    SetCheck(window, ID_CHK_DNEJECT, cfg->DNEject);
    SetCheck(window, ID_CHK_DNCLOSE, cfg->DNClose);
    SetCheck(window, ID_CHK_DNMESSAGE, cfg->DNMessage);
    for(i=ID_RADIO_TIMEOUTMIN; i<=ID_RADIO_TIMEOUTMAX; ++i)
        {
        t=GetDlgItemInt(window, i, NULL, FALSE);
        if(t==cfg->DNTimer)
            SetCheck(window, i, TRUE);
        if(t>hung)
            EnableWindow(GetDlgItem(window, i), FALSE);
        }
    ShowWindow(GetDlgItem(window, ID_LABEL_HASH), SW_HIDE);
    SetDlgItemText(window, ID_LABEL_HASH, cfg->Hash);
    SetCheck(window, ID_CHK_PASSWORD, *cfg->Hash);
    ShowWindow(GetDlgItem(window, ID_LABEL_PATH), SW_HIDE);
    SetDlgItemText(window, ID_LABEL_PATH, cfg->SavePath);
    SetCheck(window, ID_CHK_SAVECONFIG, *cfg->SavePath);
    SetCheck(window, ID_CHK_ICONDISK, cfg->IconDisk);
    SetCheck(window, ID_CHK_ICONOPEN, cfg->IconOpen);
    SetCheck(window, ID_CHK_CMDOPEN, cfg->CmdOpen);
    SetCheck(window, ID_CHK_CMDEXPLORE, cfg->CmdExplore);
    SetCheck(window, ID_CHK_CMDAUTORUN, cfg->CmdAutorun);
    }

static void SaveState(HWND window, struct CDS_Config *cfg)
    {
    int i;
    BOOL ar_system, ar_user;
    BOOL new_ar_system, new_ar_user;
    BOOL autoplay, new_autoplay;

    DebugLog(("SaveState STARTED\n"));

    if(IsChecked(window, ID_CHK_AUTORUN))
        {
        if(IsChecked(window, ID_CHK_HIDE)
            && !IsAutoLoadHide(AppNameW))
            {
            DebugLog(("    AddAutoLoadHide...\n"));
            AddAutoLoadHide(AppNameW);
            DebugLog(("        DONE\n"));
            }
        else if(IsChecked(window, ID_CHK_AUTOEXIT)
            && !IsAutoLoadExit(AppNameW))
            {
            DebugLog(("    AddAutoLoadExit...\n"));
            AddAutoLoadExit(AppNameW);
            DebugLog(("        DONE\n"));
            }
        else if(!IsAutoLoadSimple(AppNameW))
            {
            DebugLog(("    AddAutoLoad...\n"));
            AddAutoLoad(AppNameW);
            DebugLog(("        DONE\n"));
            }
        }
    else if(IsAutoLoadAny(AppNameW))
        {
        DebugLog(("    DeleteAutoLoad...\n"));
        DeleteAutoLoad(AppName);
        DebugLog(("        DONE\n"));
        }

    ar_system=IsAutoRunSystem();
    new_ar_system=IsChecked(window, ID_CHK_CDAUTOALL);
    if(new_ar_system!=ar_system)
        {
        DebugLog(("    SetAutoRunSystem(%d)...\n", new_ar_system));
        SetAutoRunSystem(new_ar_system);
        DebugLog(("        DONE\n"));
        DebugLog(("    IsAutoRunSystem...\n"));
        new_ar_system=IsAutoRunSystem();
        DebugLog(("        DONE(%d)\n", new_ar_system));
        }
    ar_user=IsAutoRunUser();
    new_ar_user=IsChecked(window, ID_CHK_CDAUTORUN);
    if(new_ar_user!=ar_user)
        {
        DebugLog(("    SetAutoRunUser(%d)...\n", new_ar_user));
        SetAutoRunUser(new_ar_user);
        DebugLog(("        DONE\n"));
        DebugLog(("    IsAutoRunUser...\n"));
        new_ar_user=IsAutoRunUser();
        DebugLog(("        DONE(%d)\n", new_ar_user));
        }
    if(new_ar_system!=ar_system || new_ar_user!=ar_user)
        {
        DebugLog(("    GetShellVersionMajor...\n"));
        if(GetShellVersionMajor()<5)
            AppMessageBox(window, STR_CONF_RESTART, MB_OK|MB_ICONINFORMATION);
        DebugLog(("        DONE\n"));
        }

    autoplay=IsAutoPlayOn();
    new_autoplay=IsChecked(window, ID_CHK_CDAUTOPLAY);
    if(new_autoplay!=autoplay)
        {
        DebugLog(("    SetAutoPlay(%d)...\n", new_autoplay));
        SetAutoPlay(new_autoplay);
        DebugLog(("        DONE\n"));
        }

    cfg->CloseOnRun=IsChecked(window, ID_CHK_CLOSEONRUN);
    cfg->ShowUnknown=IsChecked(window, ID_CHK_SHOWUNKNOWN);
    cfg->NumNames=IsChecked(window, ID_CHK_NUMNAMES);
    cfg->EjectSingle=!IsChecked(window, ID_CHK_EJECTCLOSE);
    cfg->TopNames=IsChecked(window, ID_CHK_TOPNAMES);
    cfg->DeepMenu=IsChecked(window, ID_CHK_DEEPMENU);
    cfg->ForceRefresh=IsChecked(window, ID_CHK_FORCEREFRESH);
    cfg->TimerEnable=IsChecked(window, ID_CHK_TIMERENABLE);
    cfg->EnableAutoclose=IsChecked(window, ID_CHK_AUTOCLOSE);
    cfg->TimerAutoclose=GetDlgItemInt(window, ID_INPUT_ACTIME,
        NULL, FALSE)*1000;
    if(cfg->TimerAutoclose<AUTOCLOSE_MIN)
        cfg->EnableAutoclose=FALSE;
    if(IsChecked(window, ID_CHK_ASPI))
        cfg->ApiPrefered=UAPI_ASPI;
    if(IsChecked(window, ID_CHK_SPTI))
        cfg->ApiPrefered=UAPI_SPTI;
    if(IsChecked(window, ID_CHK_DISABLEASPI))
        cfg->ApiPrefered|=UAPI_DISABLE_ASPI;
    cfg->DNEject=IsChecked(window, ID_CHK_DNEJECT);
    cfg->DNClose=IsChecked(window, ID_CHK_DNCLOSE);
    cfg->DNMessage=IsChecked(window, ID_CHK_DNMESSAGE);
    for(i=ID_RADIO_TIMEOUTMIN; i<=ID_RADIO_TIMEOUTMAX; ++i)
        if(IsChecked(window, i))
            {
            cfg->DNTimer=GetDlgItemInt(window, i, NULL, FALSE);
            break;
            }
    GetDlgItemText(window, ID_LABEL_HASH, cfg->Hash, sizeof(cfg->Hash));
    GetDlgItemText(window, ID_LABEL_PATH, cfg->SavePath, sizeof(cfg->SavePath));
    cfg->IconDisk=IsChecked(window, ID_CHK_ICONDISK);
    cfg->IconOpen=IsChecked(window, ID_CHK_ICONOPEN);
    cfg->CmdOpen=IsChecked(window, ID_CHK_CMDOPEN);
    cfg->CmdExplore=IsChecked(window, ID_CHK_CMDEXPLORE);
    cfg->CmdAutorun=IsChecked(window, ID_CHK_CMDAUTORUN);

    DebugLog(("    SaveState FINISHED\n"));
    }

static BOOL Button_OK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    struct DLG_Data *data;
    struct CDS_Config *cfg;

    (void)wParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        data=(struct DLG_Data *)lParam;
        cfg=data->param;
        LoadState(window, cfg);
        return TRUE;
        }
    if(WM_COMMAND==msg)
        {
        data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
        cfg=data->param;
        SaveState(window, cfg);
        EndDialog(window, id);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Chk_AutoRun(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        EnDisAutoLoad(window);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Chk_CDAutoAll(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        EnDisAutoPlay(window, ID_CHK_CDAUTOPLAY);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Chk_TimerEnable(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        EnDisAutoclose(window);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Button_PASS(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        SendDlgItemMessage(window, id, BM_SETSTYLE, BS_CHECKBOX, 0);
        return TRUE;
        }
    else if(WM_COMMAND==msg)
        {
        char hash[CDROM_HASH_SIZE];

        GetDlgItemText(window, ID_LABEL_HASH, hash, sizeof(hash));
        if(0==*hash)
            /* Set password */
            QueryPassword(window, hash, sizeof(hash));
        else
            /* Clear password */
            if(QueryPassword(window, hash, 0))
                *hash=0;
        SetDlgItemText(window, ID_LABEL_HASH, hash);
        SetCheck(window, ID_CHK_PASSWORD, *hash);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Button_SAVECONFIG(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        SendDlgItemMessage(window, id, BM_SETSTYLE, BS_CHECKBOX, 0);
        return TRUE;
        }
    else if(WM_COMMAND==msg)
        {
        char path[MAX_PATH];

        GetDlgItemText(window, ID_LABEL_PATH, path, sizeof(path));
        if(0==*path)
            {
            strcpy(path, ".\\cdslow.reg");
            if(!SaveDialog(window, path, sizeof(path)))
                *path=0;
            }
        else
            *path=0;
        SetDlgItemText(window, ID_LABEL_PATH, path);
        SetCheck(window, ID_CHK_SAVECONFIG, *path);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Chk_DisableASPI(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        EnDisASPI(window);
        return TRUE;
        }
    return FALSE;
    }
