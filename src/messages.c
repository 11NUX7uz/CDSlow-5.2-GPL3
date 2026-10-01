/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "dlglib.h"
#include "version.h"
#include "msg_ids.h"
#include "dialogs.h"

enum
    {
    ID_GRP_MESSAGE=101,
    ID_ICON,
    ID_ICON_SPACER,
    ID_LABEL_MSG,
    ID_GRP_OK
    };

static struct DLG_Item Items[]=
    {
    {&CtlGroupBoxH, ID_GRP_MESSAGE, NULL, 0, 0, NULL},
    {&CtlIcon, ID_ICON, L"IconApp", 0, ID_GRP_MESSAGE, NULL},
    {&CtlGroupBoxSpacer, ID_ICON_SPACER, NULL, 0, ID_GRP_MESSAGE, NULL},
    {&CtlLabel, ID_LABEL_MSG, NULL, 0, ID_GRP_MESSAGE, NULL},
    {&CtlGroupBoxH, ID_GRP_OK, NULL, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_BUTTON_OK, ID_GRP_OK, NULL},
    {&CtlButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_OK, NULL},
    };

int AppMessageBox(HWND window, DWORD rcid, UINT flags)
    {
    struct DLG_Item *items;
    unsigned i;
    int ret;

    items=malloc(sizeof(Items));
    if(NULL==items)
        return IDCANCEL;

    memcpy(items, Items, sizeof(Items));
    for(i=0; i<sizeof(Items)/sizeof(*Items); ++i)
        if(ID_LABEL_MSG==items[i].Id)
            {
            items[i].rcid=rcid;
            break;
            }

    if(MB_OKCANCEL==(flags&MB_TYPEMASK))
        i=sizeof(Items)/sizeof(*Items);
    else
        i=sizeof(Items)/sizeof(*Items)-1;

    MessageBeep(flags&MB_TYPEMASK);
    ret=DlgRunU(
        window,
        AppTitle,
        0,
        WS_BORDER|WS_CAPTION|WS_SYSMENU,
        MB_TASKMODAL==(flags&MB_TASKMODAL) ? WS_EX_APPWINDOW : 0,
        items,
        i,
        NULL
        );

    free(items);

    return ret;
    }

