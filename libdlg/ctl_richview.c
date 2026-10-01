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
#include <richedit.h>
#include "dlglib.h"

#define EXTRA_W 34
#define EXTRA_H 22
#define LIMIT_W 512
#define LIMIT_H (LIMIT_W/4*3)

static void Estimate(HWND window, int id, SIZE *sz, SIZE *max)
    {
    NONCLIENTMETRICSW metrics;

    GetNonClientMetricsU(&metrics);
    GetDlgItemTextSize(window, id, sz);
    sz->cx+=EXTRA_W;
    sz->cy+=EXTRA_H;
    if(sz->cx>LIMIT_W)
        {
        sz->cx=LIMIT_W;
        sz->cy+=metrics.iScrollHeight;
        }
    if(sz->cy>LIMIT_H)
        {
        sz->cy=LIMIT_H;
        sz->cx+=metrics.iScrollWidth;
        }
    max->cx=ITEM_MSZ_MAX;
    max->cy=ITEM_MSZ_MAX;
    }

static void PreInit(void)
    {
    static HINSTANCE dll=NULL;

    if(!dll)
        dll=LoadLibrary("RICHED32.DLL");
    }

static void Init(HWND window, int id)
    {
    HWND item;
    HFONT font;
    HDC dc;
    TEXTMETRIC tm;
    CHARFORMAT fmt;

    item=GetDlgItem(window, id);
    font=(HFONT)SendMessage(window, WM_GETFONT, 0, 0);
    dc=GetDC(item);
    SelectObject(dc, font);
    GetTextMetrics(dc, &tm);
    ReleaseDC(item, dc);

    fmt.cbSize=sizeof(fmt);
    SendMessage(item, EM_GETCHARFORMAT, 0, (LPARAM)&fmt);
    fmt.bCharSet=tm.tmCharSet;
    SendMessage(item, EM_SETCHARFORMAT, 0, (LPARAM)&fmt);
    }

struct DLG_Control CtlRichView=
    {
    L"RichEdit",
    WS_TABSTOP|WS_HSCROLL|WS_VSCROLL|
        ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_READONLY|
        WS_BORDER,
    0,
    TRUE,
    Init,
    Estimate,
    DlgMoveItem,
    PreInit
    };

