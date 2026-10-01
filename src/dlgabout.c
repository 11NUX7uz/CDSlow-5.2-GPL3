/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include "dlglib.h"
#include "msg_ids.h"
#include "version.h"
#ifndef NDEBUG
#include "build.h"
#endif /* NDEBUG */
#include "dialogs.h"

static BOOL URL_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Build_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static void ExecDlgURL(HWND window, int item);

enum
    {
    ID_GRP_ABOUT=101,
    ID_ICON,
    ID_ICON_SPACER,
    ID_GRP_VERSION,
    ID_GRP_VERSTR,
    ID_LABEL_CDSLOW,
    ID_LABEL_VERSION,
    ID_LABEL_BUILD,
    ID_LABEL_LICENSE,
    ID_V_SPACER1,
    ID_V_SPACER2,
    ID_GRP_COPYRIGHT,
    ID_LABEL_C_SIGN,
    ID_LABEL_C_YEAR,
    ID_LABEL_C_NAME,
    ID_LABEL_MAIL,
    ID_LABEL_URL,
    ID_GRP_OK
    };

static struct DLG_Item Items[]=
    {
    {&CtlGroupBoxH, ID_GRP_ABOUT, NULL, 0, 0, NULL},
    {&CtlIcon, ID_ICON, L"IconApp", 0, ID_GRP_ABOUT, NULL},
    {&CtlGroupBoxSpacer, ID_ICON_SPACER, NULL, 0, ID_GRP_ABOUT, NULL},
    {&CtlGroupV, ID_GRP_VERSION, NULL, 0, ID_GRP_ABOUT, NULL},
    {&CtlGroupH, ID_GRP_VERSTR, NULL, 0, ID_GRP_VERSION, NULL},
    {&CtlLabel, ID_LABEL_CDSLOW, NULL, STR_ABOUT_VERSION, ID_GRP_VERSTR, NULL},
    {&CtlLabel, ID_LABEL_VERSION, APP_VERSION_STR, 0, ID_GRP_VERSTR, NULL},
    {&CtlLabel, ID_LABEL_BUILD, NULL, 0, ID_GRP_VERSTR, Build_Proc},
    {&CtlLabel, ID_LABEL_LICENSE, NULL, STR_ABOUT_LICENSE, ID_GRP_VERSION, NULL},
    {&CtlLabel, ID_V_SPACER1, L" ", 0, ID_GRP_VERSION, NULL},
    {&CtlGroupH, ID_GRP_COPYRIGHT, NULL, 0, ID_GRP_VERSION, NULL},
    {&CtlLabel, ID_LABEL_C_SIGN, NULL, STR_ABOUT_C_SIGN, ID_GRP_COPYRIGHT, NULL},
    {&CtlLabel, ID_LABEL_C_YEAR, APP_YEARS_STR, 0, ID_GRP_COPYRIGHT, NULL},
    {&CtlLabel, ID_LABEL_C_NAME, NULL, STR_ABOUT_C_NAME, ID_GRP_COPYRIGHT, NULL},
    {&CtlLabel, ID_V_SPACER2, L" ", 0, ID_GRP_VERSION, NULL},
    {&CtlLabel, ID_LABEL_URL, NULL, STR_ABOUT_URL, ID_GRP_VERSION, URL_Proc},
    {&CtlLabel, ID_LABEL_MAIL, NULL, STR_ABOUT_EMAIL, ID_GRP_VERSION, URL_Proc},
    {&CtlGroupBoxH, ID_GRP_OK, NULL, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_BUTTON_OK, ID_GRP_OK, NULL},
    };

static BOOL URL_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)lParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        if(HIWORD(wParam)==STN_CLICKED)
            {
            ExecDlgURL(window, id);
            return TRUE;
            }
        }
    else if(WM_CTLCOLORSTATIC==msg)
        {
        SetTextColor((HDC)wParam, RGB(0, 0, 255));
        SetBkColor((HDC)wParam, GetSysColor(COLOR_3DFACE));
        return (LRESULT)GetSysColorBrush(COLOR_3DFACE);
        }
    
    return FALSE;
    }

#if 0
static BOOL Version_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        SetDlgItemTextU(window, id, AppVersion);
        return TRUE;
        }
    
    return FALSE;
    }
#endif

static BOOL Build_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
#ifdef BUILD_ID
        SetDlgItemTextA(window, id, BUILD_ID);
#else
        (void)window; /* Unused */
        (void)id; /* Unused */
#endif /* BUILD_ID */

        return TRUE;
        }
    
    return FALSE;
    }

void AboutDialog(HWND window)
    {
    DlgRunU(
        window,
        AppTitle,
        STR_MNU_ABOUT,
        WS_BORDER|WS_CAPTION|WS_SYSMENU,
        WS_EX_APPWINDOW,
        Items,
        sizeof(Items)/sizeof(*Items),
        NULL
        );
    }

static void ExecDlgURL(HWND window, int item)
    {
    char buf[256];
    
    GetDlgItemText(window, item, buf, sizeof(buf));
    ShellExecute(NULL, "open", buf, NULL, NULL, 0);
    }
