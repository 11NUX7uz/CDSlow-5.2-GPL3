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

static void PreInit(void)
    {
    InitCommonControls();
    }

static void Estimate(HWND window, int id, SIZE *sz, SIZE *max)
    {
    NONCLIENTMETRICSW metrics;

    sz->cx=GetDlgItemInt(window, id, NULL, FALSE);
    if(0==sz->cx)
        sz->cx=100;
    GetNonClientMetricsU(&metrics);
    sz->cy=metrics.iScrollHeight;
    max->cx=ITEM_MSZ_FILL;
    max->cy=ITEM_MSZ_FIXED;
    }

struct DLG_Control CtlProgress=
    {
    PROGRESS_CLASSW,
    PBS_SMOOTH,
    0,
    TRUE,
    NULL,
    Estimate,
    DlgMoveItem,
    PreInit
    };

