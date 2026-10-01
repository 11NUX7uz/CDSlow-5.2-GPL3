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
#include "dlglib.h"

#define CHKBOX_EXTRA_H 2
#define CHKBOX_EXTRA_W 8
static void ChkBoxMinSize(HWND window, int id, SIZE *sz, SIZE *max)
    {
    HWND item=GetDlgItem(window, id);
    HDC dc;
    HFONT font;
    TEXTMETRIC metric;
    int dpi;

    GetDlgItemTextSize(window, id, sz);

    font=(HFONT)SendMessage(item, WM_GETFONT, 0, 0);
    dc=GetDC(item);
    SelectObject(dc, font);
    GetTextMetrics(dc, &metric);
    dpi=GetDeviceCaps(dc, LOGPIXELSX);
    if(dpi<96)
        dpi=96;
    ReleaseDC(item, dc);
    sz->cx+=CHKBOX_EXTRA_W+dpi/8+metric.tmMaxCharWidth/8;

    if(sz->cy<dpi/8+CHKBOX_EXTRA_H)
        sz->cy=dpi/8+CHKBOX_EXTRA_H;
    max->cx=ITEM_MSZ_FIXED;
    max->cy=ITEM_MSZ_FIXED;
    }

struct DLG_Control CtlCheckBox=
    {
    L"BUTTON",
    WS_TABSTOP|BS_AUTOCHECKBOX|BS_MULTILINE,
    0,
    TRUE,
    NULL,
    ChkBoxMinSize,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlRadioBox=
    {
    L"BUTTON",
    WS_TABSTOP|BS_AUTORADIOBUTTON|BS_MULTILINE,
    0,
    TRUE,
    NULL,
    ChkBoxMinSize,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlRadioBoxFirst=
    {
    L"BUTTON",
    WS_GROUP|WS_TABSTOP|BS_AUTORADIOBUTTON|BS_MULTILINE,
    0,
    TRUE,
    NULL,
    ChkBoxMinSize,
    DlgMoveItem,
    NULL
    };

