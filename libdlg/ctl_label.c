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

static void MinSize(HWND window, int id, SIZE *sz, SIZE *max)
    {
    GetDlgItemTextSize(window, id, sz);
    max->cx=ITEM_MSZ_FIXED;
    max->cy=ITEM_MSZ_FIXED;
    }

static void MinSizeCentered(HWND window, int id, SIZE *sz, SIZE *max)
    {
    GetDlgItemTextSize(window, id, sz);
    max->cx=ITEM_MSZ_FILL;
    max->cy=ITEM_MSZ_FIXED;
    }

struct DLG_Control CtlLabel=
    {
    L"STATIC",
    SS_LEFT|SS_NOTIFY,
    0,
    TRUE,
    NULL,
    MinSize,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlLabelCentered=
    {
    L"STATIC",
    SS_CENTER|SS_NOTIFY,
    0,
    TRUE,
    NULL,
    MinSizeCentered,
    DlgMoveItem,
    NULL
    };

struct DLG_Control CtlLabelRight=
    {
    L"STATIC",
    SS_RIGHT|SS_NOTIFY,
    0,
    TRUE,
    NULL,
    MinSizeCentered,
    DlgMoveItem,
    NULL
    };

