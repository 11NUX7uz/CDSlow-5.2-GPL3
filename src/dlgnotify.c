/*
Copyright 2005-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <time.h>
#include "dlglib.h"
#include "msg_ids.h"
#include "version.h"
#include "cdlist.h"
#include "dialogs.h"

enum
    {
    ID_GRP_NOTIFY=101,
    ID_LABEL_TEXT,
    ID_GRP_LABELS,
    ID_LABEL1,
    ID_SPACER1,
    ID_LABEL2,
    ID_GRP_BUTTON,
    ID_SPACER2
    };

static BOOL Button_PROC(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Label_TEXT(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);

static struct DLG_Item ItemsEject[]=
    {
    {&CtlGroupBoxV, ID_GRP_NOTIFY, NULL, 0, 0, NULL},
    {&CtlLabel, ID_LABEL_TEXT, NULL, 0, ID_GRP_NOTIFY, Label_TEXT},
    {&CtlGroupH, ID_GRP_LABELS, GRP_TITLE_FIXED, 0, ID_GRP_NOTIFY, NULL},
    {&CtlLabel, ID_LABEL1, NULL, STR_DNOTIFY_TIME, ID_GRP_LABELS, NULL},
    {&CtlGroupBoxSpacer, ID_SPACER1, NULL, 0, ID_GRP_LABELS, NULL},
    {&CtlLabelRight, ID_LABEL2, L"00", 0, ID_GRP_LABELS, Button_PROC},
    {&CtlGroupBoxH, ID_GRP_BUTTON, NULL, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_DNOTIFY_EJECT, ID_GRP_BUTTON, Button_PROC},
    {&CtlGroupBoxSpacer, ID_SPACER2, NULL, 0, ID_GRP_BUTTON, NULL},
    {&CtlButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_BUTTON, Button_PROC},
    };

static struct DLG_Item ItemsClose[]=
    {
    {&CtlGroupBoxV, ID_GRP_NOTIFY, NULL, 0, 0, NULL},
    {&CtlGroupH, ID_GRP_LABELS, GRP_TITLE_FIXED, 0, ID_GRP_NOTIFY, NULL},
    {&CtlLabel, ID_LABEL1, NULL, STR_DNOTIFY_TIME, ID_GRP_LABELS, NULL},
    {&CtlGroupBoxSpacer, ID_SPACER1, NULL, 0, ID_GRP_LABELS, NULL},
    {&CtlLabelRight, ID_LABEL2, L"00", 0, ID_GRP_LABELS, Button_PROC},
    {&CtlGroupBoxH, ID_GRP_BUTTON, NULL, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_DNOTIFY_CLOSE, ID_GRP_BUTTON, Button_PROC},
    {&CtlGroupBoxSpacer, ID_SPACER2, NULL, 0, ID_GRP_BUTTON, NULL},
    {&CtlButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_BUTTON, Button_PROC},
    };

struct Param
    {
    unsigned timeout;
    BOOL (*test_function)(void);
    };

void DiskInDriveNotify(HWND window, unsigned timeout)
    {
    struct Param p;

    p.timeout=timeout;
    p.test_function=IsAllEmpty;
    SetForegroundWindow(window);
    if(IDOK==DlgRunU(
            window,
            AppTitle,
            STR_DNOTIFY_DISK,
            WS_BORDER|WS_CAPTION|WS_SYSMENU,
            WS_EX_APPWINDOW,
            ItemsEject,
            sizeof(ItemsEject)/sizeof(*ItemsEject),
            &p
            )
        )
        EjectAllDisks();
    }

void TrayOpenNotify(HWND window, unsigned timeout, unsigned close)
    {
    struct Param p;
    unsigned ret;

    p.timeout=timeout;
    p.test_function=IsAllClosed;
    SetForegroundWindow(window);
    ret=DlgRunU(
        window,
        AppTitle,
        STR_DNOTIFY_TRAY,
        WS_BORDER|WS_CAPTION|WS_SYSMENU,
        WS_EX_APPWINDOW,
        ItemsClose,
        sizeof(ItemsClose)/sizeof(*ItemsClose),
        &p
        );
    if(IDOK==ret || (close && ID_LABEL2==ret))
        CloseAllDisks();
    }

static VOID CALLBACK TimerProc(HWND window, UINT uMsg,
    UINT idEvent, DWORD dwTime)
    {
    unsigned t;
    struct DLG_Data *data;
    struct Param *p;

    (void)uMsg; /* Unused */
    (void)idEvent; /* Unused */
    (void)dwTime; /* Unused */

    data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
    p=data->param;

    t=GetDlgItemInt(window, ID_LABEL2, NULL, FALSE);

    if(t<=1)
        PostMessage(window, WM_COMMAND,
            MAKEWPARAM(ID_LABEL2, 0), (LPARAM)GetDlgItem(window, ID_LABEL2));
    else if(NULL!=p->test_function && p->test_function())
        PostMessage(window, WM_COMMAND,
            MAKEWPARAM(IDCANCEL, 0), (LPARAM)GetDlgItem(window, IDCANCEL));
    else
        SetDlgItemInt(window, ID_LABEL2, --t, FALSE);
    }

#define TIMER_ID 1

static BOOL Button_PROC(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */

    if(WM_INITDIALOG==msg && IDCANCEL==id)
        {
        struct DLG_Data *data=(struct DLG_Data *)lParam;
        struct Param *p=data->param;

        SetDlgItemInt(window, ID_LABEL2, p->timeout, FALSE);
        SetTimer(window, TIMER_ID, 1000, TimerProc);
        return TRUE;
        }
    if(WM_COMMAND==msg)
        {
        KillTimer(window, TIMER_ID);
        EndDialog(window, id);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Label_TEXT(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)id; /* Unused */
    (void)wParam; /* Unused */
    (void)lParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        WCHAR *text;

        text=GetAllLabels();
        if(NULL!=text)
            {
            SetDlgItemTextU(window, ID_LABEL_TEXT, text);
            free(text);
            }

        return TRUE;
        }
    return FALSE;
    }
