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

static void GetGroupTextSize(HWND window, int id, SIZE *sz);

#define GRPBOX_EXTRA_LEFT   9
#define GRPBOX_EXTRA_RIGHT  9
#define GRPBOX_EXTRA_BOTTOM 4
#define GRPBOX_EXTRA_TOP    4
static void GroupEstimateSize(
    HWND window, int id,
    SIZE *sz, SIZE *max,
    struct DLG_Control *base
    )
    {
    SIZE gtsz;

    base->Estimate(window, id, sz, max);
    GetGroupTextSize(window, id, &gtsz);
    if(sz->cx<gtsz.cx) sz->cx=gtsz.cx;
    sz->cx+=GRPBOX_EXTRA_LEFT+GRPBOX_EXTRA_RIGHT;
    sz->cy+=gtsz.cy+GRPBOX_EXTRA_BOTTOM;
    if(ITEM_MSZ_FIXED==max->cx)
        max->cx=ITEM_MSZ_FILL;
    if(ITEM_MSZ_FIXED==max->cy)
        max->cy=ITEM_MSZ_FILL;
    }

static void VGroupEstimateSize(HWND window, int id, SIZE *sz, SIZE *max)
    {
    GroupEstimateSize(window, id, sz, max, &CtlGroupV);
    }

static void HGroupEstimateSize(HWND window, int id, SIZE *sz, SIZE *max)
    {
    GroupEstimateSize(window, id, sz, max, &CtlGroupH);
    }

static void GroupMove(
    HWND window, int id,
    int x, int y,
    int w, int h,
    struct DLG_Control *base
    )
    {
    SIZE gtsz;

    GetGroupTextSize(window, id, &gtsz);
    DlgMoveItem(window, id, x, y, w, h);
    base->Move(window, id, x+GRPBOX_EXTRA_LEFT, y+gtsz.cy,
        w-GRPBOX_EXTRA_LEFT-GRPBOX_EXTRA_RIGHT, h-gtsz.cy-GRPBOX_EXTRA_BOTTOM);
    }

static void VGroupMove(HWND window, int id, int x, int y, int w, int h)
    {
    GroupMove(window, id, x, y, w, h, &CtlGroupV);
    }

static void HGroupMove(HWND window, int id, int x, int y, int w, int h)
    {
    GroupMove(window, id, x, y, w, h, &CtlGroupH);
    }

static void GetGroupTextSize(HWND window, int id, SIZE *sz)
    {
    HWND item=GetDlgItem(window, id);
    HFONT font;
    HDC dc;

    sz->cx=0;
    sz->cy=0;

    GetDlgItemTextSize(window, id, sz);
    if(sz->cy==0)
        {
        font=(HFONT)SendMessage(item, WM_GETFONT, 0, 0);
        dc=GetDC(item);
        SelectObject(dc, font);
        GetTextExtentPoint32W(dc, L" ", 1, sz);
        ReleaseDC(item, dc);
        sz->cy=sz->cy/2+GRPBOX_EXTRA_TOP;
        }
    }

static void SpacerEstimateSize(HWND window, int id, SIZE *sz, SIZE *max)
    {
    (void)window; /* Unused */
    (void)id;     /* Unused */
    sz->cy=sz->cx=(GRPBOX_EXTRA_LEFT+GRPBOX_EXTRA_RIGHT)/2;
    /*sz->cy=(GRPBOX_EXTRA_TOP+GRPBOX_EXTRA_BOTTOM)/2;*/
    max->cx=ITEM_MSZ_FIXED;
    max->cy=ITEM_MSZ_FIXED;
    }

static void SpacerMove(HWND window, int id, int x, int y, int w, int h)
    {
    (void)window; /* Unused */
    (void)id;     /* Unused */
    (void)x;      /* Unused */
    (void)y;      /* Unused */
    (void)w;      /* Unused */
    (void)h;      /* Unused */
    }

struct DLG_Control CtlGroupBoxV=
    {
    L"BUTTON",
    BS_GROUPBOX,
    0,
    FALSE,
    NULL,
    VGroupEstimateSize,
    VGroupMove,
    NULL
    };

struct DLG_Control CtlGroupBoxH=
    {
    L"BUTTON",
    BS_GROUPBOX,
    0,
    FALSE,
    NULL,
    HGroupEstimateSize,
    HGroupMove,
    NULL
    };

struct DLG_Control CtlGroupBoxSpacer=
    {
    L"STATIC",
    SS_OWNERDRAW,
    0,
    FALSE,
    NULL,
    SpacerEstimateSize,
    SpacerMove,
    NULL
    };

