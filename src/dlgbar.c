/*
Copyright 2003-2019 Vadim Druzhin <cdslow@mail.ru>

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
#include "log.h"
#include "dialogs.h"

static BOOL TitleProc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CancelProc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);

enum
    {
    ID_GRP_PROGRESS=101,
    ID_TITLE,
    ID_LABEL1,
    ID_LABEL2,
    ID_PROGRESS,
    ID_GRP_LABEL,
    ID_GRP_SPACER,
    ID_GRP_BUTTON
    };

static struct DLG_Item Items[]=
    {
    {&CtlGroupBoxV, ID_GRP_PROGRESS, NULL, 0, 0, NULL},
    {&CtlLabelCentered, ID_TITLE, NULL, 0, ID_GRP_PROGRESS, TitleProc},
    {&CtlProgress, ID_PROGRESS, NULL, 0, ID_GRP_PROGRESS, NULL},
    {&CtlGroupH, ID_GRP_LABEL, GRP_TITLE_FILL_CX, 0, ID_GRP_PROGRESS, NULL},
    {&CtlLabel, ID_LABEL1, L"0", 0, ID_GRP_LABEL, NULL},
    {&CtlGroupH, ID_GRP_SPACER, GRP_TITLE_FILL_CX, 0, ID_GRP_LABEL, NULL},
    {&CtlLabel, ID_LABEL2, L"100%", 0, ID_GRP_LABEL, NULL},
    {&CtlGroupBoxH, ID_GRP_BUTTON, NULL, 0, 0, NULL},
    {&CtlDefButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_BUTTON, CancelProc},
    };

static BOOL TitleProc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        struct DLG_Data *data=(struct DLG_Data *)lParam;
        char *title=data->param;

        SetDlgItemText(window, id, title);
        return TRUE;
        }
    return FALSE;
    }

static BOOL CancelProc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)window; /* Unused */
    (void)id; /* Unused */
    (void)wParam; /* Unused */

    if(WM_COMMAND==msg)
        {
        EnableWindow((HWND)lParam, FALSE);
        return TRUE;
        }
    return FALSE;
    }

HWND BarDialog(HWND window, UINT title_id, char *drive_name)
    {
    HWND dlg;

    dlg=DlgShowU(
        window,
        NULL,
        title_id,
        WS_BORDER|WS_CAPTION|WS_VISIBLE,
        0,
        Items,
        sizeof(Items)/sizeof(*Items),
        drive_name
        );

    return dlg;
    }

BOOL BarUpdate(unsigned min, unsigned max, unsigned pos, 
    unsigned speed, unsigned command, void *param)
    {
    HWND window=param;
    MSG wmsg;

    if(min!=0 || max!=0 || pos!=0)
        {
        DebugLog(("        BarUpdate: min=%u, max=%u, pos=%u (%u%%)\n",
                min, max, pos, max!=min ? (pos-min)*100/(max-min) : 0));
        SendDlgItemMessage(window, ID_PROGRESS, PBM_SETRANGE, 0,
            MAKELPARAM(min, max));
        SendDlgItemMessage(window, ID_PROGRESS, PBM_SETPOS, pos, 0);
        }

    if(0!=speed || 0!=command)
        DebugLog(("    Got speed=%u, command=%u\n", speed, command));

    while(PeekMessage(&wmsg, NULL, 0, 0, PM_REMOVE))
        DispatchMessage(&wmsg);
    UpdateWindow(window);

    return IsWindowEnabled(GetDlgItem(window, IDCANCEL));
    }
