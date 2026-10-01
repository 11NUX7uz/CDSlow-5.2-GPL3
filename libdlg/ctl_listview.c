/* ===================================================================
 * Copyright (c) 2005-2019 Vadim Druzhin (cdslow@mail.ru).
 * 
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ===================================================================
 */

#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "dlglib.h"

#define EXTRA_W 4
#define EXTRA_H 4
#define EXTRA_COLUMN 16/*12*/
static void InitColumns(HWND window, int id);
static void ListView_InsertColumnU(HWND item, int iCol, LV_COLUMNW *pcol);

static WCHAR *CopyDlgItemTextU(HWND window, int id)
    {
    int len;
    WCHAR *buf;

    len=GetWindowTextLengthU(GetDlgItem(window, id));
    if(0==len)
        return NULL;
    ++len;
    buf=malloc(len*sizeof(*buf));
    if(NULL==buf)
        return NULL;
    if(0==GetDlgItemTextU(window, id, buf, len))
        {
        free(buf);
        return NULL;
        }

    return buf;
    }

static void Estimate(HWND window, int id, SIZE *sz, SIZE *max)
    {
    DWORD vr;
    NONCLIENTMETRICSW metrics;
    WCHAR *buf;
    WCHAR *s;

    vr=ListView_ApproximateViewRect(GetDlgItem(window, id), -1, -1, 5);
    if(0!=vr)
        {
        sz->cx=LOWORD(vr);
        sz->cy=HIWORD(vr);
        }
    else
        {
        GetDlgItemTextSize(window, id, sz);
        sz->cx+=EXTRA_COLUMN;
        sz->cy*=8;
        buf=CopyDlgItemTextU(window, id);
        if(NULL!=buf)
            {
            for(s=buf; *s!=0; ++s)
                if(L'\t'==*s)
                    sz->cx+=EXTRA_COLUMN;
            free(buf);
            }
        }
    sz->cx+=EXTRA_W;
    sz->cy+=EXTRA_H;
    GetNonClientMetricsU(&metrics);
    sz->cx+=metrics.iScrollWidth;
    max->cx=ITEM_MSZ_MAX;
    max->cy=ITEM_MSZ_MAX;
    }

static void Init(HWND window, int id)
    {
    InitColumns(window, id);
    SendDlgItemMessage(window, id, LVM_SETEXTENDEDLISTVIEWSTYLE,
        LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    }

static void InitColumns(HWND window, int id)
    {
    HWND item=GetDlgItem(window, id);
    LV_COLUMNW col;
    WCHAR *buf;
    int i;
    WCHAR *s, *e;
    int end;
    HFONT font;
    HDC dc;
    SIZE sz;

    buf=CopyDlgItemTextU(window, id);
    if(NULL==buf)
        return;

    font=(HFONT)SendMessage(item, WM_GETFONT, 0, 0);
    dc=GetDC(item);
    SelectObject(dc, font);
    
    i=0;
    s=e=buf;
    end=0;
    do
        {
        while(*e!=0 && *e!=L'\t') {++e;}
        if(0==*e)
            end=1;
        else
            *e=0;
        GetTextExtentPoint32W(dc, s, (int)(e-s), &sz);
        col.mask=LVCF_TEXT|LVCF_WIDTH;
        col.pszText=s;
        col.cx=sz.cx+EXTRA_COLUMN;
        ListView_InsertColumnU(item, i, &col);
        ++e;
        s=e;
        ++i;
        }
    while(!end);

    ReleaseDC(item, dc);
    free(buf);
    }

static void ListView_InsertColumnU(HWND item, int iCol, LV_COLUMNW *pcol)
    {
    LV_COLUMNA col;

    if(IsRealUnicode())
        {
        SendMessageW(item, LVM_INSERTCOLUMNW, iCol, (LPARAM)pcol);
        return;
        }

    col.pszText=TextBufWtoA(pcol->pszText);
    if(NULL==col.pszText)
        return;
    col.mask=pcol->mask;
    col.fmt=pcol->fmt;
    col.cx=pcol->cx;
    col.cchTextMax=pcol->cchTextMax;
    col.iSubItem=pcol->iSubItem;
#if 0 /*(_WIN32_IE >= 0x0300)*/
	int iImage;
	int iOrder;
#endif

    SendMessageA(item, LVM_INSERTCOLUMNA, iCol, (LPARAM)&col);

    free(col.pszText);
    }

struct DLG_Control CtlListView=
    {
    WC_LISTVIEWW,
    WS_TABSTOP|LVS_REPORT|LVS_SINGLESEL|LVS_NOSORTHEADER|WS_BORDER,
    0,
    TRUE,
    Init,
    Estimate,
    DlgMoveItem,
    NULL
    };

