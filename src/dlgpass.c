/*
Copyright 2005-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <time.h>
#include "dlglib.h"
#include "msg_ids.h"
#include "md5_crypt.h"
#include "version.h"
#include "dialogs.h"

#define PASS_SIZE 23

struct Pass_Param
    {
    char *hash;
    int hash_size;
    int pass_size;
    };

enum
    {
    ID_GRP_PASSWORD=101,
    ID_GRP_LABELS,
    ID_LABEL1,
    ID_LABEL2,
    ID_SPACER,
    ID_GRP_EDIT,
    ID_EDIT1,
    ID_EDIT2,
    ID_GRP_BUTTON,
    ID_SPACER1
    };

static BOOL Button_OK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL Edit1(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);

static struct DLG_Item Items[]=
    {
    {&CtlGroupBoxH, ID_GRP_PASSWORD, NULL, 0, 0, NULL},
    {&CtlGroupV, ID_GRP_LABELS, GRP_TITLE_FILL_CY, 0, ID_GRP_PASSWORD, NULL},
    {&CtlLabel, ID_LABEL1, NULL, STR_PASS_PASSWORD, ID_GRP_LABELS, NULL},
    {&CtlLabel, ID_LABEL2, NULL, STR_PASS_AGAIN, ID_GRP_LABELS, NULL},
    {&CtlGroupBoxSpacer, ID_SPACER, NULL, 0, ID_GRP_PASSWORD, NULL},
    {&CtlGroupV, ID_GRP_EDIT, NULL, 0, ID_GRP_PASSWORD, NULL},
    {&CtlEdit, ID_EDIT1, NULL, 0, ID_GRP_EDIT, Edit1},
    {&CtlEdit, ID_EDIT2, NULL, 0, ID_GRP_EDIT, Edit1},
    {&CtlGroupBoxH, ID_GRP_BUTTON, NULL, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_BUTTON_OK, ID_GRP_BUTTON, Button_OK},
    {&CtlGroupBoxSpacer, ID_SPACER1, NULL, 0, ID_GRP_BUTTON, NULL},
    {&CtlButton, IDCANCEL, NULL, STR_BUTTON_CANCEL, ID_GRP_BUTTON, NULL},
    };

BOOL QueryPassword(HWND window, char *hash, int hash_size)
    {
    struct Pass_Param p;
    WORD title_id;

    p.hash=hash;
    p.hash_size=hash_size;
    p.pass_size=PASS_SIZE;
    if(0==hash_size)
        title_id=STR_PASS_QUERY;
    else
        title_id=STR_PASS_NEW;
    return IDOK==DlgRunU(
        window,
        NULL,
        title_id,
        WS_BORDER|WS_CAPTION,
        0,
        Items,
        sizeof(Items)/sizeof(*Items),
        &p
        );
    }

BOOL AppQueryPassword(HWND window, char *hash)
    {
    struct Pass_Param p;

    p.hash=hash;
    p.hash_size=0;
    p.pass_size=PASS_SIZE;
    SetForegroundWindow(window);
    return IDOK==DlgRunU(
        window,
        AppTitle,
        STR_PASS_QUERY,
        WS_BORDER|WS_CAPTION|WS_SYSMENU,
        WS_EX_APPWINDOW,
        Items,
        sizeof(Items)/sizeof(*Items),
        &p
        );
    }

static char *GetPassword(HWND window, int id)
    {
    int sz;
    char *pass;

    sz=GetWindowTextLength(GetDlgItem(window, id))+1;
    pass=malloc(sz);
    if(NULL==pass)
        return NULL;
    GetDlgItemText(window, id, pass, sz);

    return pass;
    }

static BOOL Get2Pass(HWND window, char **pass1, char **pass2)
    {
    *pass1=GetPassword(window, ID_EDIT1);
    if(NULL==*pass1)
        return FALSE;
    *pass2=GetPassword(window, ID_EDIT2);
    if(NULL==*pass2)
        {
        free(*pass1);
        return FALSE;
        }
    return TRUE;
    }

static BOOL Button_OK(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    struct DLG_Data *data;
    struct Pass_Param *p;

    (void)wParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        data=(struct DLG_Data *)lParam;
        p=data->param;
        if(0==p->hash_size)
            {
            ShowWindow(GetDlgItem(window, ID_LABEL2), SW_HIDE);
            ShowWindow(GetDlgItem(window, ID_EDIT2), SW_HIDE);
            }
        SetFocus(GetDlgItem(window, ID_EDIT1));
        return TRUE;
        }
    if(WM_COMMAND==msg)
        {
        char *pass1;
        char *pass2;
        char *hash;
        char salt[5];

        data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
        p=data->param;
        if(!Get2Pass(window, &pass1, &pass2))
            return TRUE;
        if(0==p->hash_size)
            {
            hash=crypt_md5(pass1, p->hash);
            if(0==strcmp(p->hash, hash))
                EndDialog(window, id);
            else
                {
                SetDlgItemText(window, ID_EDIT1, "");
                MessageBeep(MB_OK);
                }
            free(hash);
            }
        else
            {
            if(0==strcmp(pass1, pass2))
                {
                if(0==*pass1)
                    *p->hash=0;
                else
                    {
                    to64(salt, (unsigned long)time(NULL), sizeof(salt)-1);
                    salt[sizeof(salt)-1]=0;
                    hash=crypt_md5(pass1, salt);
                    strncpy(p->hash, hash, p->hash_size);
                    free(hash);
                    }
                EndDialog(window, id);
                }
            else
                MessageBeep(MB_OK);
            }
        free(pass2);
        free(pass1);
        return TRUE;
        }
    return FALSE;
    }

static BOOL Edit1(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    struct DLG_Data *data;
    struct Pass_Param *p;

    if(WM_INITDIALOG==msg)
        {
        data=(struct DLG_Data *)lParam;
        p=data->param;
        SendDlgItemMessage(window, id, EM_LIMITTEXT, p->pass_size-1, 0);
        SendDlgItemMessage(window, id, EM_SETPASSWORDCHAR, '*', 0);
        return TRUE;
        }
    if(WM_COMMAND==msg && EN_CHANGE==HIWORD(wParam))
        {
        char *pass1;
        char *pass2;

        data=(struct DLG_Data *)GetWindowLong(window, GWL_USERDATA);
        p=data->param;
        if(0==p->hash_size || !Get2Pass(window, &pass1, &pass2))
            return TRUE;
        EnableWindow(GetDlgItem(window, IDOK), 0==strcmp(pass1, pass2));
        free(pass2);
        free(pass1);
        }
    return FALSE;
    }

