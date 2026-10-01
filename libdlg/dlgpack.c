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
#include <stdlib.h>
#include "dlglib.h"

static size_t DlgMeasureItemU(struct DLG_Item *item);
static size_t DlgPackItemU(char *buf, struct DLG_Item *item);
static size_t ResStringBytesU(int id);
static void ResLoadStringU(int id, WCHAR *wstr, int wlen);
static size_t StringBytesU(WCHAR const *wstr);
static void StringCopyU(WCHAR *to, WCHAR const *from);
    
#pragma pack(push, 1)
typedef struct
    {   
    WORD   dlgVer; 
    WORD   signature; 
    DWORD  helpID; 
    DWORD  exStyle; 
    DWORD  style; 
    WORD   cDlgItems; 
    short  x; 
    short  y; 
    short  cx; 
    short  cy; 
    } DLGTEMPLATEEX; 
#if 0
        sz_Or_Ord menu;         /* name or ordinal of a menu resource */
        sz_Or_Ord windowClass;  /* name or ordinal of a window class */
        WCHAR  title[titleLen]; /* title string of the dialog box */
#endif
typedef struct
    {
    WORD   pointsize;       /* only if DS_SETFONT flag is set */
    WORD   weight;          /* only if DS_SETFONT flag is set */
    BYTE   bItalic;         /* only if DS_SETFONT flag is set */
    BYTE   charset;
    } DLGTEMPLATEEXFONT;
#if 0
        WCHAR  font[fontLen];   /* typeface name, if DS_SETFONT is set */
#endif
typedef struct
    {  
    DWORD  helpID; 
    DWORD  exStyle; 
    DWORD  style; 
    short  x; 
    short  y; 
    short  cx; 
    short  cy; 
    DWORD   id; 
    } DLGITEMTEMPLATEEX;
#if 0
    sz_Or_Ord windowClass; /* name or ordinal of a window class */
    sz_Or_Ord title;       /* title string or ordinal of a resource */
    WORD   extraCount;     /* bytes of following creation data */
#endif
#pragma pack(pop)

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
    )
    {
    size_t dmenu_size;
    size_t dclass_size;
    size_t dtitle_size;
    size_t dfont_size;
    int title_id_len;
    size_t title_str_len;
    size_t title_space_len;
    WCHAR *titlespace=L" - ";
    WCHAR *dmenu;
    WCHAR *dclass;
    WCHAR *dtitle;
    WCHAR *dfont;
    size_t dlg_size;
    size_t items_size;
    DLGTEMPLATEEX *dlg;
    DLGTEMPLATEEXFONT *dfparam;
    size_t item_offset;
    char *buf;
    int i;

    /* Get size of strings */
    dmenu_size=sizeof(WCHAR);
    dclass_size=sizeof(WCHAR);
    /* Calculate size of title */
    dtitle_size=1;
    title_id_len=LoadStringU(NULL, titleid, NULL, 0, 0);
    dtitle_size+=title_id_len;
    title_str_len= titlestr ? lstrlenW(titlestr) : 0;
    dtitle_size+=title_str_len;
    title_space_len=lstrlenW(titlespace);
    if(title_id_len>0 && title_str_len>0)
        dtitle_size+=title_space_len;
    dtitle_size*=sizeof(WCHAR);
    /* Get size of face name string, if font present */
    if(fnt!=NULL)
        dfont_size=StringBytesU(fnt->lfFaceName);
    else
        dfont_size=0;

    /* Calculate size of template */
    dlg_size=sizeof(*dlg)+dmenu_size+dclass_size+dtitle_size;
    /* Add font parameters size, if present */
    if(dfont_size!=0)
        dlg_size+=sizeof(DLGTEMPLATEEXFONT)+dfont_size;

    /* Adjust to DWORD boundary */
    dlg_size=AdjustToDWORD(dlg_size);

    /* Calculate dialog items size */
    items_size=0;
    for(i=0; i<itemcount; i++)
        items_size+=DlgMeasureItemU(items+i);

    /* Allocate buffer for template */
    buf=malloc(dlg_size+items_size+sizeof(DWORD));
    if(buf==NULL)
        return NULL;

    /* Set pointers in buffer */
    dlg=(DLGTEMPLATEEX *)AdjustToDWORD((size_t)buf);
    dmenu=(WCHAR *)((char *)(dlg)+sizeof(DLGTEMPLATEEX));
    dclass=(WCHAR *)((char *)(dmenu)+dmenu_size);
    dtitle=(WCHAR *)((char *)(dclass)+dclass_size);

    /* Fill template with data */
    dlg->dlgVer=1; 
    dlg->signature=0xFFFF; 
    dlg->helpID=0; 
    dlg->exStyle=exstyle; 
    dlg->style=style; 
    dlg->cDlgItems=itemcount; 
    dlg->x=x;
    dlg->y=y; 
    dlg->cx=cx; 
    dlg->cy=cy;
    
    *dmenu=0;
    *dclass=0;
    if(title_str_len>0)
        {
        strcpyU(dtitle, titlestr);
        if(title_str_len>0 && title_id_len>0)
            {
            strcpyU(dtitle+title_str_len, titlespace);
            title_str_len+=title_space_len;
            }
        }
    ResLoadStringU(titleid, dtitle+title_str_len, title_id_len+1);

    /* Set font parameters */
    if(fnt!=NULL)
        {
        HDC dc;
        int pointsize;

        dc=GetDC(NULL);
        pointsize=-MulDiv(fnt->lfHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
        ReleaseDC(NULL, dc);

        dlg->style|=DS_SETFONT;

        dfparam=(DLGTEMPLATEEXFONT *)((char *)(dtitle)+dtitle_size);
        dfont=(WCHAR *)((char *)(dfparam)+sizeof(*dfparam));

        dfparam->pointsize=pointsize;
        dfparam->weight=(WORD)fnt->lfWeight;
        dfparam->bItalic=fnt->lfItalic;
        dfparam->charset=fnt->lfCharSet;
        StringCopyU(dfont, fnt->lfFaceName);
        }
    else
        dlg->style&=~DS_SETFONT;

    /* Fill items data */
    item_offset=dlg_size;
    for(i=0; i<itemcount; i++)
        {
        item_offset+=DlgPackItemU((char *)(dlg)+item_offset, items+i);
        if(items[i].Control->PreInit!=NULL)
            items[i].Control->PreInit();
        }

    return buf;
    }

static size_t DlgMeasureItemU(struct DLG_Item *item)
    {
    size_t item_size;

    item_size=sizeof(DLGITEMTEMPLATEEX);
    item_size+=StringBytesU(item->Control->Class);
    if(item->Title!=NULL)
        item_size+=StringBytesU(item->Title);
    else
        item_size+=ResStringBytesU(item->rcid);
    item_size+=sizeof(WORD);

    return AdjustToDWORD(item_size);
    }

static size_t DlgPackItemU(char *buf, struct DLG_Item *item)
    {
    size_t dclass_size;
    size_t dtitle_size;
    DLGITEMTEMPLATEEX *di;
    WCHAR *dclass;
    WCHAR *dtitle;
    WORD *extra;
    size_t item_size;

    dclass_size=StringBytesU(item->Control->Class);
    if(item->Title!=NULL)
        dtitle_size=StringBytesU(item->Title);
    else
        dtitle_size=ResStringBytesU(item->rcid);

    di=(DLGITEMTEMPLATEEX *)buf;
    dclass=(WCHAR *)((char *)(di)+sizeof(*di));
    dtitle=(WCHAR *)((char *)(dclass)+dclass_size);
    extra=(WORD *)((char *)(dtitle)+dtitle_size);

    di->helpID=0; 
    di->exStyle=item->Control->ExStyle; 
    di->style=item->Control->Style|WS_CHILD|WS_VISIBLE; 
    di->x=0; 
    di->y=0; 
    di->cx=0; 
    di->cy=0; 
    di->id=item->Id;
    StringCopyU(dclass, item->Control->Class);
    if(item->Title!=NULL)
        StringCopyU(dtitle, item->Title);
    else
        ResLoadStringU(item->rcid, dtitle, (int)dtitle_size/sizeof(WCHAR));
    *extra=0;

    item_size=sizeof(*di)+dclass_size+dtitle_size+sizeof(*extra);

    return AdjustToDWORD(item_size);
    }

static size_t ResStringBytesU(int id)
    {
    return (LoadStringU(NULL, id, NULL, 0, 0)+1)*sizeof(WCHAR);
    }

static void ResLoadStringU(int id, WCHAR *wstr, int wlen)
    {
    if(LoadStringU(NULL, id, wstr, wlen, 0)==0)
        *wstr=0;
    }

static size_t StringBytesU(WCHAR const *wstr)
    {
    if(wstr==NULL)
        return sizeof(WCHAR);
    return (lstrlenW(wstr)+1)*sizeof(WCHAR);
    }

static void StringCopyU(WCHAR *to, WCHAR const *from)
    {
    if(from!=NULL)
        strcpyU(to, from);
    else
        *to=0;
    }

size_t AdjustToDWORD(size_t n)
    {
    n+=sizeof(DWORD)-1;
    n/=sizeof(DWORD);
    n*=sizeof(DWORD);
    return n;
    }

