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

#define CTL_EXTRA_LEFT   3
#define CTL_EXTRA_RIGHT  3
#define CTL_EXTRA_TOP    3
#define CTL_EXTRA_BOTTOM 3
static void Estimate(HWND window, int id, SIZE *sz, SIZE *max)
    {
    int len;

    len=GetWindowTextLengthU(GetDlgItem(window, id));
    if(len>0)
        {
        GetDlgItemTextSize(window, id, sz);
        sz->cx+=sz->cx/len;
        }
    else
        GetWindowStrSize(GetDlgItem(window, id), L"  ", sz);

    sz->cy+=CTL_EXTRA_TOP+CTL_EXTRA_BOTTOM;
    sz->cx+=CTL_EXTRA_LEFT+CTL_EXTRA_RIGHT;
    if(sz->cy>sz->cx) sz->cx=sz->cy;
    max->cx=ITEM_MSZ_FILL;
    max->cy=ITEM_MSZ_FIXED;
    }

struct DLG_Control CtlEdit=
    {
    L"EDIT",
    WS_TABSTOP|ES_AUTOHSCROLL|WS_BORDER,
    0,
    TRUE,
    NULL,
    Estimate,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlEditRight=
    {
    L"EDIT",
    WS_TABSTOP|ES_AUTOHSCROLL|ES_RIGHT|WS_BORDER,
    0,
    TRUE,
    NULL,
    Estimate,
    DlgMoveItem,
    NULL
    };

