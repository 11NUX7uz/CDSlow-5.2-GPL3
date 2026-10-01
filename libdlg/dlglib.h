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

#ifndef __DLGLIB_H__
#define __DLGLIB_H__

struct DLG_Control
    {
    WCHAR const * const Class;
    DWORD const Style;
    DWORD const ExStyle;
    BOOL const Solid;
    void (*Init)(HWND window, int id);
    void (*Estimate)(HWND window, int id, SIZE *sz, SIZE *max);
    void (*Move)(HWND window, int id, int x, int y, int w, int h);
    void (*PreInit)(void);
    };

struct DLG_Item
    {
    struct DLG_Control *Control;
    WORD Id;
    WCHAR const *Title;
    int rcid;
    WORD GroupId;
    INT_PTR (*ItemProc)(
        HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
    };

struct DLG_ItemData
    {
    WORD Id;
    WORD GroupId;
    struct DLG_Control *Control;
    INT_PTR (*ItemProc)(
        HWND window, WORD id, UINT msg, WPARAM wParam, LPARAM lParam);
    int SizeKnown;
    SIZE ESZ;
    SIZE MSZ;
    };

struct DLG_Data
    {
    void *param;
    struct DLG_ItemData *Item;
    int count;
    POINT MinTrackSize;
    };

void *PackDialogU(
    DWORD style,
    DWORD exstyle,
    int x, int y,
    int cx, int cy,
    WCHAR const *titlestr,
    int titleid,
    LOGFONTW *fnt,
    int itemcount,
    struct DLG_Item *items
    );
size_t AdjustToDWORD(size_t n);
void DlgMoveItem(HWND window, int id, int x, int y, int cx, int cy);
INT_PTR DlgRunU(
    HWND window,
    WCHAR const *strtitle,
    int rctitle,
    DWORD style,
    DWORD exstyle,
    struct DLG_Item *items,
    int itemcount,
    void *param
    );
HWND DlgShowU(
    HWND window,
    WCHAR const *strtitle,
    int rctitle,
    DWORD style,
    DWORD exstyle,
    struct DLG_Item *items,
    int itemcount,
    void *param
    );
void GetWindowStrSize(HWND window, WCHAR *str, SIZE *sz);
void GetDlgItemTextSize(HWND window, int id, SIZE *sz);

extern struct DLG_Control CtlGroupV;
extern struct DLG_Control CtlGroupH;

#define GRP_AUTO    0
#define GRP_TITLE_AUTO    L"0"
#define GRP_FIXED   1
#define GRP_TITLE_FIXED   L"1"
#define GRP_MAX     2
#define GRP_TITLE_MAX     L"2"
#define GRP_MAX_CX  3
#define GRP_TITLE_MAX_CX  L"3"
#define GRP_MAX_CY  4
#define GRP_TITLE_MAX_CY  L"4"
#define GRP_FILL    5
#define GRP_TITLE_FILL    L"5"
#define GRP_FILL_CX 6
#define GRP_TITLE_FILL_CX L"6"
#define GRP_FILL_CY 7
#define GRP_TITLE_FILL_CY L"7"

#define ITEM_MSZ_FIXED 0
#define ITEM_MSZ_MAX   1
#define ITEM_MSZ_FILL  2

extern struct DLG_Control CtlButton;
extern struct DLG_Control CtlDefButton;
extern struct DLG_Control CtlSmallButton;
extern struct DLG_Control CtlCheckBox;
extern struct DLG_Control CtlRadioBox;
extern struct DLG_Control CtlRadioBoxFirst;
extern struct DLG_Control CtlEdit;
extern struct DLG_Control CtlEditRight;
extern struct DLG_Control CtlGroupBoxV;
extern struct DLG_Control CtlGroupBoxH;
extern struct DLG_Control CtlGroupBoxSpacer;
extern struct DLG_Control CtlHotkey;
extern struct DLG_Control CtlIcon;
extern struct DLG_Control CtlIcon16;
extern struct DLG_Control CtlLabel;
extern struct DLG_Control CtlLabelCentered;
extern struct DLG_Control CtlLabelRight;
extern struct DLG_Control CtlListView;
extern struct DLG_Control CtlProgress;
extern struct DLG_Control CtlRichView;

void UnicodeInit(void);
BOOL IsRealUnicode(void);
char *TextBufWtoA(WCHAR const *textw);
WCHAR *TextBufAtoW(char const *texta);
WCHAR *MapStringU(HINSTANCE hinst, UINT id, int lng, int *len);
int LoadStringU(HINSTANCE hinst, UINT id, WCHAR *buf, int buflen, int lng);
WCHAR *strcpyU(WCHAR *to, WCHAR const *from);

typedef BOOL GetNonClientMetricsU_t(NONCLIENTMETRICSW *ncmw);
extern GetNonClientMetricsU_t *GetNonClientMetricsU;

typedef INT_PTR WINAPI DialogBoxIndirectParamU_t
    (HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern DialogBoxIndirectParamU_t *DialogBoxIndirectParamU;

typedef HWND WINAPI CreateDialogIndirectParamU_t
    (HINSTANCE,LPCDLGTEMPLATE,HWND,DLGPROC,LPARAM);
extern CreateDialogIndirectParamU_t *CreateDialogIndirectParamU;

typedef BOOL WINAPI InsertMenuItemU_t(HMENU,UINT,BOOL,LPCMENUITEMINFOW);
extern InsertMenuItemU_t *InsertMenuItemU;

typedef void DlgItemReplaceSelRcU_t(HWND dlg, int item, UINT rcid);
extern DlgItemReplaceSelRcU_t *DlgItemReplaceSelRcU;

typedef BOOL WINAPI SetDlgItemTextU_t(HWND,int,LPCWSTR);
extern SetDlgItemTextU_t *SetDlgItemTextU;

typedef UINT WINAPI GetDlgItemTextU_t(HWND,int,LPWSTR,int);
extern GetDlgItemTextU_t *GetDlgItemTextU;

typedef int WINAPI GetWindowTextLengthU_t(HWND);
extern GetWindowTextLengthU_t *GetWindowTextLengthU;

typedef BOOL WINAPI Shell_NotifyIconU_t(DWORD,PNOTIFYICONDATAW);
extern Shell_NotifyIconU_t *Shell_NotifyIconU;

WCHAR *Num(WCHAR *to, int n);
WCHAR *Rcs(WCHAR *to, int id);
WCHAR *Crlf(WCHAR *to);
WCHAR *Stra(WCHAR *to, char *from);

typedef BOOL GetVolumeLabelU_t(WCHAR *root, WCHAR *label, int size);
extern GetVolumeLabelU_t *GetVolumeLabelU;

#endif /* __DLGLIB_H__ */
