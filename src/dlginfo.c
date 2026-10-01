/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "dlglib.h"
#include "uniscsi.h"
#include "cdlist.h"
#include "version.h"
#include "msg_ids.h"
#include "dialogs.h"

struct InfoParam
    {
    struct CDROM_Desc *cd;
    int count;
    };

static WCHAR *InfoText(struct CDROM_Desc *cdl, unsigned count);
static WCHAR *InfoSpeedList(WCHAR *s, struct CDROM_SpeedList *sl);
static BOOL View_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);

enum
    {
    ID_VIEW=101,
    ID_GRP_OK
    };

static struct DLG_Item Items[]=
    {
    {&CtlRichView, ID_VIEW, NULL, 0, 0, View_Proc},
    {&CtlGroupH, ID_GRP_OK, GRP_TITLE_FILL_CX, 0, 0, NULL},
    {&CtlDefButton, IDOK, NULL, STR_BUTTON_OK, ID_GRP_OK, NULL},
    };

static BOOL View_Proc(
    HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    (void)wParam; /* Unused */

    if(WM_INITDIALOG==msg)
        {
        struct DLG_Data *data=(struct DLG_Data *)lParam;
        struct InfoParam *ip=data->param;
        WCHAR *vtext;

        vtext=InfoText(ip->cd, ip->count);
        SetDlgItemTextU(window, id, vtext);
        free(vtext);
        return TRUE;
        }
    return FALSE;
    }

void InfoDialog(HWND window, struct CDROM_Desc *cd, int count)
    {
    struct InfoParam ip;

    ip.cd=cd;
    ip.count=count;
    DlgRunU(
        window,
        AppTitle,
        STR_MNU_INFO,
        WS_OVERLAPPEDWINDOW,
        WS_EX_APPWINDOW,
        Items,
        sizeof(Items)/sizeof(*Items),
        &ip
        );
    }

#define str(to, from) strcpyU((to), (from))

static WCHAR *InfoText(struct CDROM_Desc *cdl, unsigned count)
    {
    unsigned i;
    WCHAR *text;
    WCHAR *s;
    int id, n;
    char *aspi_param[]=
        {
        "LegalCopyright",
        "ProductName",
        "FileVersion",
        };
    char *t;

    text=malloc(10000*sizeof(*text));
    if(NULL==text)
        return NULL;

    s=text;
    switch(NTGetStatus())
        {
    case SPTI_NOT_INITIALIZED: n=STR_SPTI_NOTINIT; break;
    case SPTI_WRONG_VERSION:   n=STR_SPTI_WRONGVERSION; break;
    case SPTI_NO_DRIVES:       n=STR_SPTI_NODRIVES; break;
    case SPTI_NO_ACCESS:       n=STR_SPTI_NOACCESS; break;
    case SPTI_OK:              n=STR_SPTI_OK; break;
    default:                   n=STR_SCSI_UNKNOWN;
        }
    s=Crlf(Rcs(str(s, L"SPTI: "), n));
    switch(ASPIGetStatus())
        {
    case ASPI_NOT_INITIALIZED: n=STR_ASPI_NOTINIT; break;
    case ASPI_NOT_PRESENT:     n=STR_ASPI_NOTPRESENT; break;
    case ASPI_BROKEN:          n=STR_ASPI_BROKEN; break;
    case ASPI_OK:              n=STR_ASPI_OK; break;
    default:                   n=STR_SCSI_UNKNOWN;
        }
    s=Crlf(Crlf(Rcs(str(s, L"ASPI: "), n)));

    if(STR_ASPI_NOTPRESENT!=n)
        {
        for(i=0; i<sizeof(aspi_param)/sizeof(*aspi_param); ++i)
            {
            t=ASPIGetVersionInfo(aspi_param[i]);
            s=Crlf(Stra(str(Stra(str(s, L"ASPI "), aspi_param[i]), L": "), t));
            }
        s=Crlf(s);
        }

    if(UAPI_ASPI==SCSIGetAPI())
        n=STR_CONF_ASPI;
    else if(UAPI_SPTI==SCSIGetAPI())
        n=STR_CONF_SPTI;
    else
        n=STR_SCSI_UNKNOWN;
    s=Crlf(Rcs(str(Rcs(s, STR_CONF_ACCESS), L": "), n));
    if(0==count)
        s=Crlf(Rcs(Crlf(s), STR_SPTI_NODRIVES));
    for(i=0; i<count; i++)
        {
        s=str(Stra(str(Crlf(s), L"["), cdl[i].Config.FullName), L"] (");
        if(cdl[i].State.Addr.Letter!=0)
            {
            *s++=cdl[i].State.Addr.Letter;
            *s=0;
            }
        else
            s=Num(str(Num(str(Num(s, cdl[i].State.Addr.Ha%100), L", "),
                cdl[i].State.Addr.Dev%100), L", "),
                cdl[i].State.Addr.Lun%100);
        s=Crlf(str(s, L")"));
        id=cdl[i].Config.CanSenseMedium ? STR_INFO_SUPPORTED : STR_INFO_NOTSUP;
        s=Crlf(Rcs(Rcs(s, STR_INFO_SENSE), id));
        s=Crlf(Rcs(s, STR_INFO_CAPSCD));
        s=InfoSpeedList(s, &cdl[i].Config.CDList);
        if(cdl[i].Config.IsDVD)
            {
            s=Crlf(Rcs(s, STR_INFO_CAPSDVD));
            s=InfoSpeedList(s, &cdl[i].Config.DVDList);
            }
        if(cdl[i].Config.IsBD)
            {
            s=Crlf(Rcs(s, STR_INFO_CAPSBD));
            s=InfoSpeedList(s, &cdl[i].Config.BDList);
            }
        }

    return text;
    }

static WCHAR *InfoSpeedList(WCHAR *s, struct CDROM_SpeedList *sl)
    {
    int id;
    int si;
    WCHAR *l=L">", *r=L"<";

    id=sl->CanGetSpeed ? STR_INFO_SUPPORTED : STR_INFO_NOTSUP;
    s=Crlf(Rcs(Rcs(s, STR_INFO_DETECT), id));
    s=Rcs(s, STR_INFO_SET);
    if(sl->CanSetSpeed)
        {
        if(METHOD_SPEED==ListSetMethod(sl, METHOD_NONE))
            s=str(Rcs(str(s, l), STR_INFO_SETSPEED), r);
        else
            s=Rcs(s, STR_INFO_SETSPEED);
        }
    if(sl->CanSetSpeed && sl->CanSetStreaming)
        s=str(s, L", ");
    if(sl->CanSetStreaming)
        {
        if(METHOD_STREAMING==ListSetMethod(sl, METHOD_NONE))
            s=str(Rcs(str(s, l), STR_INFO_SETSTREAMING), r);
        else
            s=Rcs(s, STR_INFO_SETSTREAMING);
        }
    if(!sl->CanSetSpeed && !sl->CanSetStreaming)
        s=Rcs(s, STR_INFO_NOTSUP);
    s=Crlf(s);
    if(sl->Count>0)
        {
        s=Rcs(s, STR_INFO_SPEEDLIST);
        for(si=sl->Count-1; si>=0; si--)
            {
            s=Num(s,
                SPEED_DIV(sl->Range[si].Speed, sl->x1)%1000);
            s=str(s, L"x");
            if(si!=0)
                s=str(s, L",");
            }
        if(sl->MaxLevel<=ML_UNKNOWN)
            s=str(s, L" (?)");
        else if(sl->MaxLevel==ML_RW)
            s=str(s, L" (RW)");
        else if(sl->MaxLevel==ML_R)
            s=str(s, L" (R)");
        s=Crlf(s);
        }

    return s;
    }
