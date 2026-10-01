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

#define BUTTON_EXTRA_LEFT   5
#define BUTTON_EXTRA_RIGHT  5
#define BUTTON_EXTRA_TOP    5
#define BUTTON_EXTRA_BOTTOM 5
/* 8: 79x13 (21x13) */
/* 24: 242x58 (62x39) */
static void Estimate(HWND window, int id, SIZE *sz, SIZE *max)
    {
    int w, h;

    GetDlgItemTextSize(window, id, sz);

    h=sz->cy*8/5;
    sz->cy+=BUTTON_EXTRA_TOP+BUTTON_EXTRA_BOTTOM;
    if(h>sz->cy) sz->cy=h;

    w=sz->cy*8/2;
    sz->cx+=BUTTON_EXTRA_LEFT+BUTTON_EXTRA_RIGHT;
    if(w>sz->cx) sz->cx=w;
    max->cx=ITEM_MSZ_FIXED;
    max->cy=ITEM_MSZ_FIXED;
    }

static void EstimateSmall(HWND window, int id, SIZE *sz, SIZE *max)
    {
    int w, h;

    GetDlgItemTextSize(window, id, sz);

    h=sz->cy*8/5;
    sz->cy+=BUTTON_EXTRA_TOP+BUTTON_EXTRA_BOTTOM;
    if(h>sz->cy) sz->cy=h;

    w=h;
    sz->cx+=BUTTON_EXTRA_LEFT+BUTTON_EXTRA_RIGHT;
    if(w>sz->cx) sz->cx=w;
    max->cx=ITEM_MSZ_FIXED;
    max->cy=ITEM_MSZ_FIXED;
    }

static void Init(HWND window, int id)
    {
    SetFocus(GetDlgItem(window, id));
    }

struct DLG_Control CtlButton=
    {
    L"BUTTON",
    WS_TABSTOP,
    0,
    TRUE,
    NULL,
    Estimate,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlDefButton=
    {
    L"BUTTON",
    WS_TABSTOP|BS_DEFPUSHBUTTON,
    0,
    TRUE,
    Init,
    Estimate,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlSmallButton=
    {
    L"BUTTON",
    WS_TABSTOP,
    0,
    TRUE,
    NULL,
    EstimateSmall,
    DlgMoveItem,
    NULL
    };

