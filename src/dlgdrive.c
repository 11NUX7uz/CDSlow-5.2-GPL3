/*
Copyright 2005-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "cdlist.h"
#include "dlglib.h"
#include "msg_ids.h"
#include "version.h"
#include "log.h"
#include "osinfo.h"
#include "hotkeys.h"
#include "dialogs.h"

static BOOL Button_OK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_EDIT_ADD(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_EDIT_DEL(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_EDIT_DELALL(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL ListProc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL InputProc1(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL InputProc2(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL InputProc3(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL InputProc4(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_FAST(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_FULL(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_MEASURE(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Radio_CDDVD(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Button_LOCK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Follow_2(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Check_TIMER(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Label_FONT(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);

enum
    {
    ID_GROUP_DRIVENAME=101,
    ID_LABEL_DRIVENAME,
    ID_CHK_IGNORE,
    ID_GRP_COLUMNS,
    ID_GRP_COLUMN1,
    ID_GRP_SPEEDOPTIONS,
    ID_GRP_SO_COLS,
    ID_GRP_SO_COL1,
    ID_CHK_ATRUN,
    ID_CHK_ATCHANGE,
    ID_CHK_ATCLOSE,
    ID_CHK_IGNORESPEED,
    ID_GRP_SO_COL2,
    ID_CHK_ATRESUME,
    ID_CHK_WITHDISK,
    ID_CHK_WHENAUDIO,
    ID_GRP_TIMER_COLS,
    ID_CHK_TIMER,
    ID_LABEL_TIMER_EVERY,
    ID_INPUT_TIMER,
    ID_LABEL_TIMER_SECONDS,
    ID_GRP_SPACER_TIMER,
    ID_CHK_FORCEREAD,
    ID_GRP_HOTKEYS,
    ID_GRP_HOTTITLES,
    ID_SPACER_HT,
    ID_GRP_HOTCONTROLS,
    ID_SPACER_HC,
    ID_GRP_HOTPLUS,
    ID_SPACER_HPLUS,
    ID_GRP_HOTCHECKS,
    ID_GRP_HOTICONS,
    ID_LABEL_HOT1,
    ID_LABEL_HOT2,
    ID_LABEL_HOT3,
    ID_LABEL_HOT4,
    ID_LABEL_HOT5,
    ID_LABEL_HOT6,
    ID_LABEL_HOT7,
    ID_HOTKEY1,
    ID_HOTKEY2,
    ID_HOTKEY3,
    ID_HOTKEY4,
    ID_HOTKEY5,
    ID_HOTKEY6,
    ID_HOTKEY7,
    ID_CHK_HOT1,
    ID_CHK_HOT2,
    ID_CHK_HOT3,
    ID_CHK_HOT4,
    ID_CHK_HOT5,
    ID_CHK_HOT6,
    ID_CHK_HOT7,
    ID_LABEL_PLUS1,
    ID_LABEL_PLUS2,
    ID_LABEL_PLUS3,
    ID_LABEL_PLUS4,
    ID_LABEL_PLUS5,
    ID_LABEL_PLUS6,
    ID_LABEL_PLUS7,
    ID_ICON1,
    ID_ICON2,
    ID_ICON3,
    ID_ICON4,
    ID_ICON5,
    ID_ICON6,
    ID_ICON7,
    ID_GRP_COLUMN2,
    ID_GRP_MEDIUM,
    ID_RADIO_CD,
    ID_RADIO_DVD,
    ID_RADIO_BD,
    ID_GRP_METHOD,
    ID_RADIO_M_AUTO_CD,
    ID_RADIO_M_AUTO_DVD,
    ID_RADIO_M_AUTO_BD,
    ID_RADIO_M_SPEED_CD,
    ID_RADIO_M_SPEED_DVD,
    ID_RADIO_M_SPEED_BD,
    ID_RADIO_M_STREAM_CD,
    ID_RADIO_M_STREAM_DVD,
    ID_RADIO_M_STREAM_BD,
    ID_GRP_H_SPEED,
    ID_GRP_SPEEDLIST,
    ID_GRP_LIST,
    ID_LIST_CD,
    ID_LIST_DVD,
    ID_LIST_BD,
    ID_GRP_INPUTSPEED,
    ID_GRP_INPUT_COL1,
    ID_GRP_INPUT_COL2,
    ID_SPACER_INPUT,
    ID_GRP_INPUT_LABEL,
    ID_INPUT1,
    ID_INPUT2,
    ID_INPUT3,
    ID_INPUT4,
    ID_LABEL_X,
    ID_LABEL_KB,
    ID_SPACER_SPEED,
    ID_GRP_SPEEDBUTTONS,
    ID_LABEL_SCAN,
    ID_BUTTON_FAST,
    ID_BUTTON_FULL,
    ID_BUTTON_MEASURE,
    ID_LABEL_EDIT,
    ID_EDIT_ADD,
    ID_EDIT_DEL,
    ID_EDIT_DELALL,
    ID_GRP_SERVICE,
    ID_CHK_SHORTMENU,
    ID_CHK_AUTOLOCK,
    ID_GRP_SHORTNAME,
    ID_LABEL_SHORTNAME,
    ID_SPACER_SHORTNAME,
    ID_INPUT_SHORTNAME,
    ID_GRP_BUTTONS
    };

static struct DLG_Item Items[]=
    {
    {&CtlGroupH, ID_GROUP_DRIVENAME, NULL, 0, 0, NULL},
        {&CtlLabelCentered, ID_LABEL_DRIVENAME, NULL, 0, ID_GROUP_DRIVENAME, Label_FONT},
        {&CtlCheckBox, ID_CHK_IGNORE, NULL, STR_DRIVE_IGNORE, ID_GROUP_DRIVENAME, NULL},
    {&CtlGroupH, ID_GRP_COLUMNS, NULL, 0, 0, NULL},
        {&CtlGroupV, ID_GRP_COLUMN1, NULL, 0, ID_GRP_COLUMNS, NULL},
                {&CtlGroupBoxH, ID_GRP_SHORTNAME, NULL, 0, ID_GRP_COLUMN1, NULL},
                    {&CtlLabel, ID_LABEL_SHORTNAME, NULL, STR_DRIVE_SHORTNAME, ID_GRP_SHORTNAME, NULL},
                    {&CtlGroupBoxSpacer, ID_SPACER_SHORTNAME, NULL, 0, ID_GRP_SHORTNAME, NULL},
                    {&CtlEdit, ID_INPUT_SHORTNAME, NULL, 0, ID_GRP_SHORTNAME, NULL},
            /* Sped Options */
            {&CtlGroupBoxV, ID_GRP_SPEEDOPTIONS, NULL, STR_DRIVE_RESTSPEED, ID_GRP_COLUMN1, NULL},
                {&CtlGroupH, ID_GRP_SO_COLS, NULL, 0, ID_GRP_SPEEDOPTIONS, NULL},
                    {&CtlGroupV, ID_GRP_SO_COL1, GRP_TITLE_FILL_CX, 0, ID_GRP_SO_COLS, NULL},
                        {&CtlCheckBox, ID_CHK_ATRUN, NULL, STR_DRIVE_ATRUN, ID_GRP_SO_COL1, NULL},
                        {&CtlCheckBox, ID_CHK_ATRESUME, NULL, STR_DRIVE_ATRESUME, ID_GRP_SO_COL1, NULL},
                        {&CtlCheckBox, ID_CHK_ATCHANGE, NULL, STR_DRIVE_ATCHANGE, ID_GRP_SO_COL1, NULL},
                        {&CtlCheckBox, ID_CHK_ATCLOSE, NULL, STR_DRIVE_ATCLOSE, ID_GRP_SO_COL1, NULL},
                    {&CtlGroupV, ID_GRP_SO_COL2, NULL, 0, ID_GRP_SO_COLS, NULL},
                        {&CtlCheckBox, ID_CHK_WHENAUDIO, NULL, STR_DRIVE_WHENAUDIO, ID_GRP_SO_COL2, NULL},
                        {&CtlCheckBox, ID_CHK_WITHDISK, NULL, STR_DRIVE_WITHDISK, ID_GRP_SO_COL2, NULL},
                        {&CtlCheckBox, ID_CHK_IGNORESPEED, NULL, STR_DRIVE_IGNORESPEED, ID_GRP_SO_COL2, NULL},
                {&CtlGroupH, ID_GRP_TIMER_COLS, NULL, 0, ID_GRP_SPEEDOPTIONS, NULL},
                    {&CtlCheckBox, ID_CHK_TIMER, NULL, STR_DRIVE_ATTIMER, ID_GRP_TIMER_COLS, Check_TIMER},
                    {&CtlLabel, ID_LABEL_TIMER_EVERY, NULL, STR_DRIVE_EVERY, ID_GRP_TIMER_COLS, NULL},
                    {&CtlEditRight, ID_INPUT_TIMER, NULL, 0, ID_GRP_TIMER_COLS, NULL},
                    {&CtlLabel, ID_LABEL_TIMER_SECONDS, NULL, STR_DRIVE_SECONDS, ID_GRP_TIMER_COLS, NULL},
                    {&CtlGroupH, ID_GRP_SPACER_TIMER, GRP_TITLE_FILL_CX, 0, ID_GRP_TIMER_COLS, NULL},
                {&CtlCheckBox, ID_CHK_FORCEREAD, NULL, STR_DRIVE_FORCEREAD, ID_GRP_SPEEDOPTIONS, NULL},
            /* Hotkeys */
            {&CtlGroupBoxH, ID_GRP_HOTKEYS, NULL, STR_HKEY_HOTKEYS, ID_GRP_COLUMN1, NULL},
                {&CtlGroupV, ID_GRP_HOTTITLES, GRP_TITLE_FILL_CY, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupBoxSpacer, ID_SPACER_HT, NULL, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupV, ID_GRP_HOTCONTROLS, GRP_TITLE_FILL_CY, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupBoxSpacer, ID_SPACER_HC, NULL, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupV, ID_GRP_HOTPLUS, GRP_TITLE_FILL_CY, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupBoxSpacer, ID_SPACER_HPLUS, NULL, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupV, ID_GRP_HOTCHECKS, GRP_TITLE_FILL_CY, 0, ID_GRP_HOTKEYS, NULL},
                {&CtlGroupV, ID_GRP_HOTICONS, GRP_TITLE_FILL_CY, 0, ID_GRP_HOTKEYS, NULL},
                    {&CtlLabel, ID_LABEL_HOT1, NULL, STR_HKEY_EJECT, ID_GRP_HOTTITLES, NULL},
                    {&CtlLabel, ID_LABEL_HOT2, NULL, STR_HKEY_LOAD, ID_GRP_HOTTITLES, NULL},
                    {&CtlLabel, ID_LABEL_HOT3, NULL, STR_HKEY_MAX, ID_GRP_HOTTITLES, NULL},
                    {&CtlLabel, ID_LABEL_HOT4, NULL, STR_HKEY_UP, ID_GRP_HOTTITLES, NULL},
                    {&CtlLabel, ID_LABEL_HOT5, NULL, STR_HKEY_SELECTED, ID_GRP_HOTTITLES, NULL},
                    {&CtlLabel, ID_LABEL_HOT6, NULL, STR_HKEY_DOWN, ID_GRP_HOTTITLES, NULL},
                    {&CtlLabel, ID_LABEL_HOT7, NULL, STR_HKEY_MIN, ID_GRP_HOTTITLES, NULL},
                    {&CtlHotkey, ID_HOTKEY1, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT1, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlHotkey, ID_HOTKEY2, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT2, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlHotkey, ID_HOTKEY3, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT3, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlHotkey, ID_HOTKEY4, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT4, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlHotkey, ID_HOTKEY5, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT5, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlHotkey, ID_HOTKEY6, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT6, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlHotkey, ID_HOTKEY7, NULL, 0, ID_GRP_HOTCONTROLS, NULL},
                    {&CtlCheckBox, ID_CHK_HOT7, NULL, 0, ID_GRP_HOTCHECKS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS1, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS2, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS3, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS4, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS5, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS6, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlLabel, ID_LABEL_PLUS7, L"+", 0, ID_GRP_HOTPLUS, NULL},
                    {&CtlIcon16, ID_ICON1, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
                    {&CtlIcon16, ID_ICON2, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
                    {&CtlIcon16, ID_ICON3, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
                    {&CtlIcon16, ID_ICON4, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
                    {&CtlIcon16, ID_ICON5, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
                    {&CtlIcon16, ID_ICON6, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
                    {&CtlIcon16, ID_ICON7, L"IconWinLogo", 0, ID_GRP_HOTICONS, NULL},
        {&CtlGroupV, ID_GRP_COLUMN2, NULL, 0, ID_GRP_COLUMNS, NULL},
            {&CtlGroupBoxH, ID_GRP_MEDIUM, NULL, STR_DRIVE_MEDIUMTYPE, ID_GRP_COLUMN2, NULL},
                {&CtlRadioBoxFirst, ID_RADIO_CD, L"CD", 0, ID_GRP_MEDIUM, Radio_CDDVD},
                {&CtlRadioBox, ID_RADIO_DVD, L"DVD", 0, ID_GRP_MEDIUM, Radio_CDDVD},
                {&CtlRadioBox, ID_RADIO_BD, L"Blu-ray", 0, ID_GRP_MEDIUM, Radio_CDDVD},
            {&CtlGroupH, ID_GRP_H_SPEED, NULL, 0, ID_GRP_COLUMN2, NULL},
                {&CtlGroupV, ID_GRP_SPEEDLIST, NULL, 0, ID_GRP_H_SPEED, NULL},
                    {&CtlGroupBoxV, ID_GRP_METHOD, NULL, STR_DRIVE_DVDSPEED, ID_GRP_SPEEDLIST, NULL},
                        {&CtlRadioBoxFirst, ID_RADIO_M_AUTO_CD, NULL, STR_DRIVE_DVDAUTO, ID_GRP_METHOD, Follow_2},
                        {&CtlRadioBox, ID_RADIO_M_SPEED_CD, L"SET SPEED", 0, ID_GRP_METHOD, Follow_2},
                        {&CtlRadioBox, ID_RADIO_M_STREAM_CD, L"SET STREAMING", 0, ID_GRP_METHOD, Follow_2},
                        {&CtlRadioBoxFirst, ID_RADIO_M_AUTO_DVD, NULL, STR_DRIVE_DVDAUTO, ID_RADIO_M_AUTO_CD, NULL},
                        {&CtlRadioBox, ID_RADIO_M_SPEED_DVD, L"SET SPEED", 0, ID_RADIO_M_SPEED_CD, NULL},
                        {&CtlRadioBox, ID_RADIO_M_STREAM_DVD, L"SET STREAMING", 0, ID_RADIO_M_STREAM_CD, NULL},
                        {&CtlRadioBoxFirst, ID_RADIO_M_AUTO_BD, NULL, STR_DRIVE_DVDAUTO, ID_RADIO_M_AUTO_CD, NULL},
                        {&CtlRadioBox, ID_RADIO_M_SPEED_BD, L"SET SPEED", 0, ID_RADIO_M_SPEED_CD, NULL},
                        {&CtlRadioBox, ID_RADIO_M_STREAM_BD, L"SET STREAMING", 0, ID_RADIO_M_STREAM_CD, NULL},
                    {&CtlGroupBoxV, ID_LABEL_SCAN, NULL, STR_DRIVE_DETECTSPEED, ID_GRP_SPEEDBUTTONS, NULL},
                        {&CtlButton, ID_BUTTON_FAST, NULL, STR_DRIVE_RESCAN, ID_LABEL_SCAN, Button_FAST},
                        {&CtlButton, ID_BUTTON_FULL, NULL, STR_DRIVE_RESCANFULL, ID_LABEL_SCAN, Button_FULL},
                        {&CtlButton, ID_BUTTON_MEASURE, NULL, STR_DRIVE_RESCANREAD, ID_LABEL_SCAN, Button_MEASURE},
                    {&CtlGroupBoxV, ID_GRP_LIST, NULL, STR_DRIVE_EDITSPEED, ID_GRP_SPEEDLIST, NULL},
                        {&CtlListView, ID_LIST_CD, NULL, STR_EDIT_TITLE, ID_GRP_LIST, ListProc},
                        {&CtlListView, ID_LIST_DVD, NULL, STR_EDIT_TITLE, ID_LIST_CD, ListProc},
                        {&CtlListView, ID_LIST_BD, NULL, STR_EDIT_TITLE, ID_LIST_CD, ListProc},
                        {&CtlGroupH, ID_GRP_INPUTSPEED, NULL, 0, ID_GRP_LIST, NULL},
                            {&CtlGroupV, ID_GRP_INPUT_COL1, NULL, 0, ID_GRP_INPUTSPEED, NULL},
                            {&CtlGroupV, ID_GRP_INPUT_COL2, NULL, 0, ID_GRP_INPUTSPEED, NULL},
                            {&CtlGroupBoxSpacer, ID_SPACER_INPUT, NULL, 0, ID_GRP_INPUTSPEED, NULL},
                            {&CtlGroupV, ID_GRP_INPUT_LABEL, GRP_TITLE_FILL_CY, 0, ID_GRP_INPUTSPEED, NULL},
                                {&CtlEditRight, ID_INPUT1, NULL, 0, ID_GRP_INPUT_COL1, InputProc1},
                                {&CtlEditRight, ID_INPUT2, NULL, 0, ID_GRP_INPUT_COL2, InputProc2},
                                {&CtlEditRight, ID_INPUT3, NULL, 0, ID_GRP_INPUT_COL1, InputProc3},
                                {&CtlEditRight, ID_INPUT4, NULL, 0, ID_GRP_INPUT_COL2, InputProc4},
                                {&CtlLabel, ID_LABEL_X, NULL, STR_EDIT_X, ID_GRP_INPUT_LABEL, NULL},
                                {&CtlLabel, ID_LABEL_KB, NULL, STR_EDIT_KBS, ID_GRP_INPUT_LABEL, NULL},
                {&CtlGroupBoxSpacer, ID_SPACER_SPEED, NULL, 0, ID_GRP_H_SPEED, NULL},
                {&CtlGroupV, ID_GRP_SPEEDBUTTONS, GRP_TITLE_FILL_CY, 0, ID_GRP_H_SPEED, NULL},
                    {&CtlGroupBoxV, ID_LABEL_EDIT, NULL, STR_EDIT_EDIT, ID_GRP_SPEEDBUTTONS, NULL},
                        {&CtlButton, ID_EDIT_ADD, NULL, STR_EDIT_ADD, ID_LABEL_EDIT, Button_EDIT_ADD},
                        {&CtlButton, ID_EDIT_DEL, NULL, STR_EDIT_DEL, ID_LABEL_EDIT, Button_EDIT_DEL},
                        {&CtlButton, ID_EDIT_DELALL, NULL, STR_EDIT_DELALL, ID_LABEL_EDIT, Button_EDIT_DELALL},
            {&CtlGroupBoxV, ID_GRP_SERVICE, NULL, STR_DRIVE_SERVICE, ID_GRP_COLUMN2, NULL},
                {&CtlCheckBox, ID_CHK_SHORTMENU, NULL, STR_DRIVE_SHORTMENU, ID_GRP_SERVICE, NULL},
                {&CtlCheckBox, ID_CHK_AUTOLOCK, NULL, STR_DRIVE_AUTOLOCK, ID_GRP_SERVICE, Button_LOCK},
    {&CtlGroupBoxH, ID_GRP_BUTTONS, NULL, 0, 0, NULL},
        {&CtlDefButton, IDOK, NULL, STR_BUTTON_OK, ID_GRP_BUTTONS, Button_OK},
        {&CtlButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_BUTTONS, NULL},
    };

struct Param
    {
    struct CDROM_Desc *cd;
    char *hash;
    };

BOOL EditDriveOptions(HWND window, struct CDROM_Desc *cd, char *hash)
    {
    struct Param p;
    int ret;
    struct CDROM_Config cfg;

    cd->State.Busy=TRUE;
    p.cd=cd;
    p.hash=hash;
    cfg=cd->Config;
    ret=DlgRunU(
        window,
        AppTitle,
        STR_DRIVE_SETTINGS,
        WS_BORDER|WS_CAPTION|WS_SYSMENU,
        WS_EX_APPWINDOW,
        Items,
        sizeof(Items)/sizeof(*Items),
        &p
        );
    cd->State.Busy=FALSE;

    if(IDOK==ret)
        return TRUE;

    cd->Config=cfg;
    return FALSE;
    }

static void SetCheck(HWND window, WORD id, BOOL state)
    {
    CheckDlgButton(window, id, state ? BST_CHECKED : BST_UNCHECKED);
    }

static BOOL IsChecked(HWND window, WORD id)
    {
    return BST_CHECKED==IsDlgButtonChecked(window, id);
    }

static void FormatSpeed(unsigned speed, int div, char *buf)
    {
    _itoa(SPEED_DIV(speed, div)%1000, buf, 10);
    }

static void ListInsert(HWND window, WORD id,
    int pos, struct CDROM_SpeedPair *speed, int div)
    {
    LV_ITEM litem;
    char buf[5];

    litem.mask=LVIF_TEXT|LVIF_PARAM;
    litem.iItem=pos;
    litem.iSubItem=0;
    litem.state=0;
    litem.stateMask=0;
    litem.pszText=buf;
    litem.cchTextMax=0;
    litem.iImage=0;    
    litem.lParam=MAKELPARAM(speed->Speed, speed->Command);
    FormatSpeed(speed->Speed, div, buf);
    SendDlgItemMessage(window, id, LVM_INSERTITEM, 0, (LPARAM)&litem);
    litem.mask=LVIF_TEXT;
    litem.iSubItem=1;
    FormatSpeed(speed->Command, div, buf);
    SendDlgItemMessage(window, id, LVM_SETITEM, 0, (LPARAM)&litem);
    }

static int ListGetPos(HWND window, WORD id)
    {
    return SendDlgItemMessage(window, id, LVM_GETNEXTITEM,
        -1, LVNI_ALL|LVNI_SELECTED);
    }

static void ListDelete(HWND window, WORD id, int pos)
    {
    SendDlgItemMessage(window, id, LVM_DELETEITEM, pos, 0);
    }

static void ListDeleteAll(HWND window, WORD id)
    {
    SendDlgItemMessage(window, id, LVM_DELETEALLITEMS, 0, 0);
    }

static LPARAM ListGetParam(HWND window, WORD id, int pos)
    {
    LV_ITEM item;

    item.iItem=pos;
    item.mask=LVIF_PARAM;	
    item.iSubItem=0;
    SendDlgItemMessage(window, id, LVM_GETITEM, 0, (LPARAM)&item);
    return item.lParam;
    }

static void ListSelect(HWND window, WORD id, int pos)
    {
    LV_ITEM item;

    item.stateMask=LVIS_SELECTED|LVIS_FOCUSED;	
    item.state=LVIS_SELECTED|LVIS_FOCUSED;	
    SendDlgItemMessage(window, id, LVM_SETITEMSTATE, pos, (LPARAM)&item);
    SendDlgItemMessage(window, id, LVM_ENSUREVISIBLE, pos, FALSE);
    }

static void LoadSpeedList(HWND window, int idx, struct CDROM_SpeedList *sl)
    {
    unsigned i;

    ListDeleteAll(window, ID_LIST_CD+idx);
    for(i=0; i<sl->Count; ++i)
        ListInsert(window, ID_LIST_CD+idx, i, sl->Range+i, sl->x1);

    SetCheck(window, ID_RADIO_M_AUTO_CD+idx,
        METHOD_NONE==sl->MethodPrefered);
    SetCheck(window, ID_RADIO_M_STREAM_CD+idx,
        METHOD_STREAMING==sl->MethodPrefered);
    SetCheck(window, ID_RADIO_M_SPEED_CD+idx,
        METHOD_SPEED==sl->MethodPrefered);
    }

static void SaveSpeedList(HWND window, int idx, struct CDROM_SpeedList *sl)
    {
    unsigned i;
    LPARAM param;

    sl->Count=SendDlgItemMessage(window, ID_LIST_CD+idx,
        LVM_GETITEMCOUNT, 0, 0);
    if(sl->Count>CDROM_SPEED_SIZE)
        sl->Count=CDROM_SPEED_SIZE;
    for(i=0; i<sl->Count; ++i)
        {
        param=ListGetParam(window, ID_LIST_CD+idx, i);
        sl->Range[i].Speed=LOWORD(param);
        sl->Range[i].Command=HIWORD(param);
        }

    if(IsChecked(window, ID_RADIO_M_AUTO_CD+idx))
        sl->MethodPrefered=METHOD_NONE;
    else if(IsChecked(window, ID_RADIO_M_STREAM_CD+idx))
        sl->MethodPrefered=METHOD_STREAMING;
    else if(IsChecked(window, ID_RADIO_M_SPEED_CD+idx))
        sl->MethodPrefered=METHOD_SPEED;
    }

static int GetCurrentListIdx(HWND window)
    {
    if(IsChecked(window, ID_RADIO_BD))
        return ID_LIST_BD-ID_LIST_CD;
    else if(IsChecked(window, ID_RADIO_DVD))
        return ID_LIST_DVD-ID_LIST_CD;

    return ID_LIST_CD-ID_LIST_CD;
    }

static struct CDROM_SpeedList *GetListByIdx(HWND window, int idx)
    {
    struct DLG_Data *data;
    struct Param *p;

    data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
    p=data->param;
    switch(idx)
        {
    case ID_LIST_BD-ID_LIST_CD:
        return &p->cd->Config.BDList;
    case ID_LIST_DVD-ID_LIST_CD:
        return &p->cd->Config.DVDList;
    default:
        return &p->cd->Config.CDList;
        }
    }

static int GetDivByIdx(HWND window, int idx)
    {
    return GetListByIdx(window, idx)->x1;
    }

static void ShowCurrentList(HWND window)
    {
    int idx;
    int i;
    int show;

    idx=GetCurrentListIdx(window);
    for(i=0; i<=ID_LIST_BD-ID_LIST_CD; ++i) 
        {
        show=(idx==i ? SW_SHOW : SW_HIDE);
        ShowWindow(GetDlgItem(window, ID_LIST_CD+i), show);
        ShowWindow(GetDlgItem(window, ID_RADIO_M_AUTO_CD+i), show);
        ShowWindow(GetDlgItem(window, ID_RADIO_M_STREAM_CD+i), show);
        ShowWindow(GetDlgItem(window, ID_RADIO_M_SPEED_CD+i), show);
        }

    SetDlgItemText(window, ID_INPUT1, "");
    SetDlgItemText(window, ID_INPUT2, "");
    SetDlgItemText(window, ID_INPUT3, "");
    SetDlgItemText(window, ID_INPUT4, "");
    }

static void CheckMedium(HWND window, struct CDROM_Desc *cd)
    {
    SetCheck(window, ID_RADIO_CD, cd->State.SL==&cd->Config.CDList);
    SetCheck(window, ID_RADIO_DVD, cd->State.SL==&cd->Config.DVDList);
    SetCheck(window, ID_RADIO_BD, cd->State.SL==&cd->Config.BDList);
    ShowCurrentList(window);
    }

static void EnableTimerInput(HWND window, BOOL enable)
    {
    EnableWindow(GetDlgItem(window, ID_LABEL_TIMER_EVERY), enable);
    EnableWindow(GetDlgItem(window, ID_INPUT_TIMER), enable);
    EnableWindow(GetDlgItem(window, ID_LABEL_TIMER_SECONDS), enable);
    }

static void LoadState(HWND window, struct CDROM_Desc *cd)
    {
    int i;

    SetDlgItemText(window, ID_LABEL_DRIVENAME, cd->Config.FullName);
    EnableWindow(GetDlgItem(window, ID_RADIO_DVD), cd->Config.IsDVD);
    EnableWindow(GetDlgItem(window, ID_RADIO_BD), cd->Config.IsBD);
    SetCheck(window, ID_CHK_IGNORE, cd->Config.Ignore);
    SetCheck(window, ID_CHK_ATRUN, cd->Config.EventMask&EVENT_ATRUN);
    SetCheck(window, ID_CHK_ATCHANGE, cd->Config.EventMask&EVENT_ATCHANGE);
    SetCheck(window, ID_CHK_ATRESUME, cd->Config.EventMask&EVENT_ATRESUME);
    SetCheck(window, ID_CHK_ATCLOSE, cd->Config.EventMask&EVENT_ATCLOSE);
    SetCheck(window, ID_CHK_IGNORESPEED, cd->Config.IgnoreSpeed);
    SetCheck(window, ID_CHK_WITHDISK, cd->Config.OnlyWithDisk);
    SetCheck(window, ID_CHK_WHENAUDIO, cd->Config.WhenAudio);
    if(cd->Config.Timer)
        {
        SetCheck(window, ID_CHK_TIMER, TRUE);
        SetDlgItemInt(window, ID_INPUT_TIMER, cd->Config.Timer, FALSE);
        }
    else
        {
        SetCheck(window, ID_CHK_TIMER, FALSE);
        SetDlgItemText(window, ID_INPUT_TIMER, "");
        }
    EnableWindow(GetDlgItem(window, ID_CHK_TIMER), Config.TimerEnable);
    EnableTimerInput(window, Config.TimerEnable && cd->Config.Timer);
    SetCheck(window, ID_CHK_FORCEREAD, cd->Config.ForceRead);
    SetCheck(window, ID_CHK_SHORTMENU, cd->Config.ShortMenu);
    SetCheck(window, ID_CHK_AUTOLOCK, cd->Config.AutoLock);
    SetDlgItemTextU(window, ID_INPUT_SHORTNAME, cd->Config.ShortName);
    LoadSpeedList(window, ID_LIST_CD-ID_LIST_CD, &cd->Config.CDList);
    LoadSpeedList(window, ID_LIST_DVD-ID_LIST_CD, &cd->Config.DVDList);
    LoadSpeedList(window, ID_LIST_BD-ID_LIST_CD, &cd->Config.BDList);
    for(i=0; i<CDROM_HOTKEYS; i++)
        {
        SendDlgItemMessage(window, ID_HOTKEY1+i, HKM_SETHOTKEY,
            cd->Config.HotKeys[i]&~MAKEWORD(0, HOTKEYF_WIN), 0);
        SetCheck(window, ID_CHK_HOT1+i, HIBYTE(cd->Config.HotKeys[i])&HOTKEYF_WIN);
        }
    CheckMedium(window, cd);
    }

static void SaveState(HWND window, struct CDROM_Desc *cd)
    {
    int i;

    cd->Config.Ignore=IsChecked(window, ID_CHK_IGNORE);
    cd->Config.EventMask=0;
    if(IsChecked(window, ID_CHK_ATRUN)) cd->Config.EventMask|=EVENT_ATRUN;
    if(IsChecked(window, ID_CHK_ATCHANGE)) cd->Config.EventMask|=EVENT_ATCHANGE;
    if(IsChecked(window, ID_CHK_ATRESUME)) cd->Config.EventMask|=EVENT_ATRESUME;
    if(IsChecked(window, ID_CHK_ATCLOSE)) cd->Config.EventMask|=EVENT_ATCLOSE;
    cd->Config.IgnoreSpeed=IsChecked(window, ID_CHK_IGNORESPEED);
    cd->Config.OnlyWithDisk=IsChecked(window, ID_CHK_WITHDISK);
    cd->Config.WhenAudio=IsChecked(window, ID_CHK_WHENAUDIO);
    if(IsChecked(window, ID_CHK_TIMER))
        cd->Config.Timer=GetDlgItemInt(window, ID_INPUT_TIMER, NULL, FALSE);
    else
        cd->Config.Timer=0;
    cd->Config.ForceRead=IsChecked(window, ID_CHK_FORCEREAD);
    cd->Config.ShortMenu=IsChecked(window, ID_CHK_SHORTMENU);
    cd->Config.AutoLock=IsChecked(window, ID_CHK_AUTOLOCK);
    GetDlgItemTextU(window, ID_INPUT_SHORTNAME,
        cd->Config.ShortName, CDROM_SHORT_SIZE);
    SaveSpeedList(window, ID_LIST_CD-ID_LIST_CD, &cd->Config.CDList);
    SaveSpeedList(window, ID_LIST_DVD-ID_LIST_CD, &cd->Config.DVDList);
    SaveSpeedList(window, ID_LIST_BD-ID_LIST_CD, &cd->Config.BDList);
    for(i=0; i<CDROM_HOTKEYS; i++)
        {
        cd->Config.HotKeys[i]=SendDlgItemMessage(window, ID_HOTKEY1+i,
            HKM_GETHOTKEY, 0, 0);
        if(IsChecked(window, ID_CHK_HOT1+i))
            cd->Config.HotKeys[i]|=MAKEWORD(0, HOTKEYF_WIN);
        }
    }

static BOOL Button_OK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    struct DLG_Data *data;
    struct Param *p;

    (void)id; /* Unused */
    (void)wParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        data=(struct DLG_Data *)lParam;
        p=data->param;
        LoadState(window, p->cd);
        return TRUE;
        }
    if(WM_COMMAND==msg)
        {
        data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
        p=data->param;
        SaveState(window, p->cd);
        EndDialog(window, IDOK);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Button_EDIT_ADD(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    struct CDROM_SpeedPair speed;
    int count;
    int i;
    LPARAM param;
    int idx;
    WORD list_id;
    unsigned div;

    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND!=msg)
        return FALSE;

    speed.Speed=GetDlgItemInt(window, ID_INPUT3, NULL, FALSE);
    speed.Command=GetDlgItemInt(window, ID_INPUT4, NULL, FALSE);
    if(0==speed.Speed || 0==speed.Command)
        return TRUE;
    if(speed.Speed>65535)
        speed.Speed=65535;
    if(speed.Command>65535)
        speed.Command=65535;

    idx=GetCurrentListIdx(window);
    list_id=ID_LIST_CD+idx;
    div=GetDivByIdx(window, idx);

    count=SendDlgItemMessage(window, list_id, LVM_GETITEMCOUNT, 0, 0);
    if(count>=CDROM_SPEED_SIZE)
        return TRUE;
    for(i=0; i<count; i++)
        {
        param=ListGetParam(window, list_id, i);
        if(LOWORD(param)/div==speed.Speed/div)
            {
            ListDelete(window, list_id, i);
            break;
            }
        else if(LOWORD(param)<speed.Speed)
            break;
        }
    ListInsert(window, list_id, i, &speed, div);
    ListSelect(window, list_id, i);

    GetListByIdx(window, idx)->MaxLevel=ML_MAX;

    return TRUE;
    }

static BOOL Button_EDIT_DEL(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    int idx;
    int item;
    WORD list_id;

    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND!=msg)
        return FALSE;

    idx=GetCurrentListIdx(window);
    list_id=ID_LIST_CD+idx;

    item=ListGetPos(window, list_id);
    if(item>=0)
        ListDelete(window, list_id, item);

    GetListByIdx(window, idx)->MaxLevel=ML_MAX;

    return TRUE;
    }

static BOOL Button_EDIT_DELALL(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    int idx;

    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND!=msg)
        return FALSE;

    idx=GetCurrentListIdx(window);
    ListDeleteAll(window, ID_LIST_CD+idx);
    GetListByIdx(window, idx)->MaxLevel=ML_MAX;

    return TRUE;
    }

static void FollowItem(HWND window, WORD id, WORD slave)
    {
    RECT r;

    GetWindowRect(GetDlgItem(window, id), &r);
    MapWindowPoints(NULL, window, (void *)&r, 2);
    DlgMoveItem(window, slave, r.left, r.top, r.right-r.left, r.bottom-r.top);
    }

static BOOL ListProc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    if(WM_INITDIALOG==msg)
        {
        LV_COLUMN col;

        col.mask=LVCF_FMT;
        col.fmt=LVCFMT_RIGHT;
        if(GetComctlVersionMajor()>0)
            {
            SendDlgItemMessage(window, id, LVM_SETCOLUMN, 0, (LPARAM)&col);
            SendDlgItemMessage(window, id, LVM_SETCOLUMN, 1, (LPARAM)&col);
            }
        return TRUE;
        }
    else if(WM_NOTIFY==msg)
        {
        NMHDR *nm=(NMHDR *)lParam;

        if(LVN_ITEMCHANGED==nm->code || NM_SETFOCUS==nm->code)
            {
            int pos;
            LPARAM param;
            int div;

            div=GetDivByIdx(window, id-ID_LIST_CD);

            pos=ListGetPos(window, id);
            if(pos>=0)
                {
                param=ListGetParam(window, id, pos);
                SetDlgItemInt(window, ID_INPUT1,
                    SPEED_DIV(LOWORD(param), div), FALSE);
                SetDlgItemInt(window, ID_INPUT2,
                    SPEED_DIV(HIWORD(param), div), FALSE);
                SetDlgItemInt(window, ID_INPUT3, LOWORD(param), FALSE);
                SetDlgItemInt(window, ID_INPUT4, HIWORD(param), FALSE);
                }
            return TRUE;
            }
        }
    else if(WM_SIZE==msg && ID_LIST_CD==id)
        {
        Follow_2(window, id, msg, wParam, lParam);
        return TRUE;
        }
    return FALSE;
    }

static void CopyInt(HWND window, WORD from, WORD to, int mul, int div)
    {
    int n;

    n=GetDlgItemInt(window, from, NULL, FALSE);
    SetDlgItemInt(window, to, SPEED_DIV(n*mul, div), FALSE);
    }

static BOOL InputProc1(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    if(WM_COMMAND!=msg || EN_CHANGE!=HIWORD(wParam))
        return FALSE;

    if(GetFocus()==(HWND)lParam)
        CopyInt(window, id, ID_INPUT3,
            GetDivByIdx(window, GetCurrentListIdx(window)), 1);

    return TRUE;
    }

static BOOL InputProc2(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    if(WM_COMMAND!=msg || EN_CHANGE!=HIWORD(wParam))
        return FALSE;

    if(GetFocus()==(HWND)lParam)
        CopyInt(window, id, ID_INPUT4,
            GetDivByIdx(window, GetCurrentListIdx(window)), 1);

    return TRUE;
    }

static BOOL InputProc3(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    if(WM_COMMAND!=msg || EN_CHANGE!=HIWORD(wParam))
        return FALSE;

    if(GetFocus()==(HWND)lParam)
        CopyInt(window, id, ID_INPUT1,
            1, GetDivByIdx(window, GetCurrentListIdx(window)));

    return TRUE;
    }

static BOOL InputProc4(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    if(WM_COMMAND!=msg || EN_CHANGE!=HIWORD(wParam))
        return FALSE;

    if(GetFocus()==(HWND)lParam)
        CopyInt(window, id, ID_INPUT2,
            1, GetDivByIdx(window, GetCurrentListIdx(window)));

    return TRUE;
    }

typedef void SCAN_FUNC(struct CDROM_Desc *cd, progress_func *pf, void *param);
static void DoScan(HWND window, int title_id, SCAN_FUNC *sf)
    {
    struct DLG_Data *data;
    struct Param *p;
    HWND dlg;

    data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
    p=data->param;

    SaveSpeedList(window, ID_LIST_CD-ID_LIST_CD, &p->cd->Config.CDList);
    SaveSpeedList(window, ID_LIST_DVD-ID_LIST_CD, &p->cd->Config.DVDList);
    SaveSpeedList(window, ID_LIST_BD-ID_LIST_CD, &p->cd->Config.BDList);

    dlg=BarDialog(window, title_id, p->cd->Config.FullName);
    EnableWindow(window, FALSE);
    sf(p->cd, BarUpdate, dlg);
    EnableWindow(window, TRUE);
    DestroyWindow(dlg);

    CheckMedium(window, p->cd);
    LoadSpeedList(window, GetCurrentListIdx(window), p->cd->State.SL);

    if(p->cd->State.SL->Count<=1 && !p->cd->State.DiskPresent)
        AppMessageBox(window, STR_DRIVE_DISKHINT, MB_OK);
    }

static BOOL Button_FAST(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        DoScan(window, STR_PROGRESS_RESCANFAST, ScanCDROMSpeedFast);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Button_FULL(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        DoScan(window, STR_PROGRESS_RESCANFULL, ScanCDROMSpeedFull);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Button_MEASURE(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    struct DLG_Data *data;
    struct Param *p;

    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
        p=data->param;
        for(;;)
            {
            EjectCDROMNoSeize(p->cd, FALSE);
            CheckDriveStateOnly(p->cd);
            if(p->cd->State.DiskPresent
                && !IsCDAMedium(p->cd->State.CDI.MediumType))
                break;
            /*EjectCDROM(cd, TRUE);*/
            if(IDCANCEL==AppMessageBox(window, STR_READ_WRONG,
                    MB_ICONINFORMATION|MB_OKCANCEL))
                return TRUE;
            }
        DoScan(window, STR_PROGRESS_RESCANREAD, ScanCDROMSpeedRead);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Radio_CDDVD(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        ShowCurrentList(window);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Follow_2(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_SIZE==msg)
        {
        FollowItem(window, id, id+1);
        FollowItem(window, id, id+2);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Button_LOCK(
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
        struct DLG_Data *data;
        struct Param *p;

        data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
        p=data->param;

        if(!IsChecked(window, id))
            SetCheck(window, id, TRUE);
        else if(0==*p->hash || QueryPassword(window, p->hash, 0))
            SetCheck(window, id, FALSE);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Check_TIMER(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        EnableTimerInput(window, IsChecked(window, ID_CHK_TIMER));
        return TRUE;
        }
    return FALSE;
    }

static BOOL Label_FONT(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        HFONT fnt;
        LOGFONT lf;

        fnt=(HFONT)SendMessage(GetDlgItem(window, id), WM_GETFONT, 0, 0);
        if(NULL==fnt)
            return TRUE;
        if(0==GetObject(fnt, sizeof(lf), &lf))
            return TRUE;
        lf.lfWeight=FW_BOLD;
        fnt=CreateFontIndirect(&lf);
        if(NULL==fnt)
            return TRUE;
        SendMessage(GetDlgItem(window, id), WM_SETFONT, (WPARAM)fnt, 0);
        return TRUE;
        }
    return FALSE;
    }
