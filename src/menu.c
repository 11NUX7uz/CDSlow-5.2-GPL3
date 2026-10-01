/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "cdlist.h"
#include "configdata.h"
#include "cdscmd.h"
#include "dlglib.h"
#include "osinfo.h"
#include "cdshell.h"
#include "msg_ids.h"
#include "log.h"
#include "menu.h"

/* Fix constants from winuser.h */
#undef MFS_GRAYED
#define MFS_GRAYED 1
#undef MFS_DISABLED
#define MFS_DISABLED 2

static HBITMAP BitmapCSpeed;
static HBITMAP BitmapClose;
static HBITMAP BitmapEject;

static HMENU RMenu;
static HMENU LMenu;

struct MenuInfo
    {
    UINT Flags;
    UINT Id;
    int rcid;
    };

static void UpdateRMenu(void);
static BOOL InsertSpeedItem(
    HMENU menu,
    unsigned flags,
    unsigned item,
    unsigned speed,
    BOOL selected,
    BOOL current,
    int tdiv
    );
static BOOL IsSpeedCurrent(struct CDROM_Desc *cd, unsigned speed);
static BOOL IsSpeedSelected(struct CDROM_Desc *cd, unsigned speed);
static void RunMenu(HWND window, HMENU menu);
static void RMenuInsertList(void);
static void RMenuClearList(void);
static void LMenuInsertList(void);
static void InsertEjectMenu(HMENU menu, int n, BOOL single);
static void InsertSpeedMenu(HMENU menu, int n);
static void InsertSeparator(HMENU menu);
static BOOL MenuCreateItems(HMENU menu, struct MenuInfo *mnu, int n);
static BOOL MenuInsertItem(HMENU menu, int i, struct MenuInfo *item);
static void InsertDriveName(HMENU menu, WCHAR *name,
    HBITMAP eject, int n, HMENU sub, BOOL disabled);
static void ZapMenu(HMENU menu, BOOL destroy);
static void InsertShellMenu(HMENU menu, int i);

static struct MenuInfo RMenuItemsFirst[]=
    {
    {MF_STRING, CMD_ABOUT, STR_MNU_ABOUT},
    {MF_STRING, CMD_HELP, STR_MNU_HELP},
    {MF_SEPARATOR, 0, 0},
    {MF_STRING, CMD_INFO, STR_MNU_INFO},
    {MF_SEPARATOR, 0, 0},
    {MF_STRING, CMD_CONFIG, STR_MNU_CONFIG},
    {MF_STRING, CMD_CLEARCONFIG, STR_MNU_CLEAR},
    {MF_SEPARATOR, 0, 0},
    {MF_STRING, CMD_FIND_CDROM, STR_MNU_FIND},
    };
#define RMENU_LIST_POSITION (sizeof(RMenuItemsFirst)/sizeof(*RMenuItemsFirst))
static struct MenuInfo RMenuItemsSecond[]=
    {
    {MF_SEPARATOR, 0, 0},
    {MF_STRING, CMD_EXIT, STR_MNU_EXIT},
    };

BOOL CreateRMenu(void)
    {
    RMenu=CreatePopupMenu();
    if(NULL==RMenu)
        return FALSE;

    if(FALSE==MenuCreateItems(RMenu, RMenuItemsSecond,
            sizeof(RMenuItemsSecond)/sizeof(*RMenuItemsSecond)))
        return FALSE;
    if(FALSE==MenuCreateItems(RMenu, RMenuItemsFirst,
            sizeof(RMenuItemsFirst)/sizeof(*RMenuItemsFirst)))
        return FALSE;

    return TRUE;
    }

static BOOL MenuCreateItems(HMENU menu, struct MenuInfo *mnu, int n)
    {
    int i;

    for(i=0; i<n; i++)
        if(FALSE==MenuInsertItem(menu, i, mnu+i))
            return FALSE;
    
    return TRUE;
    }

static BOOL MenuInsertItem(HMENU menu, int i, struct MenuInfo *item)
    {
    int res;
    MENUITEMINFOW mii;
    int len;
    WCHAR *buf=NULL;
    

    ZeroMemory(&mii, sizeof(mii));
    /* Fill mii structure with item data */
    mii.cbSize=sizeof(mii);
    mii.fMask=MIIM_ID|MIIM_STATE|MIIM_TYPE;
    mii.fType=item->Flags;
    mii.fState=0;
    mii.wID=item->Id;
    /* Set item string */
    if(item->rcid!=0)
        {
        len=LoadStringU(NULL, item->rcid, NULL, 0, 0);
        if(len>0 && (buf=calloc(len+1, sizeof(*buf))))
            {
            LoadStringU(NULL, item->rcid, buf, len+1, 0);
            mii.dwTypeData=buf;
            }
        }
    /* Insert item */
    res=InsertMenuItemU(menu, i, TRUE, &mii);
    
    if(buf)
        free(buf);

    return res!=0;
    }

static void UpdateRMenu(void)
    {
    char *help;

    if(NULL==RMenu)
        return;

    RMenuClearList();
    RMenuInsertList();

    help=GetHelpPath();
    if(NULL==help || 0xFFFFFFFF==GetFileAttributes(help))
        EnableMenuItem(RMenu, CMD_HELP, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
    else    
        EnableMenuItem(RMenu, CMD_HELP, MF_BYCOMMAND|MF_ENABLED);
    free(help);
    }

static void RMenuInsertList(void)
    {
    MENUITEMINFO mii;
    MENUITEMINFOW miiw;
    HMENU ign_menu=NULL;
    int len;
    unsigned i;
    int pos_r, pos_ign;

    DebugLog(("RMenuInsertList STARTED\n"));

    ign_menu=CreatePopupMenu();
    DebugLog(("    ign_menu=0x%X\n", ign_menu));

    pos_r=RMENU_LIST_POSITION;
    pos_ign=0;
    ZeroMemory(&mii, sizeof(mii));
    for(i=0; i<CDROM_Count; i++)
        {
        mii.cbSize=sizeof(mii);
        mii.fMask=MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_TYPE;
        mii.dwItemData=CMD_CDLIST_ITEM;
        mii.wID=MKCDCMD(CMD_CD_EDIT, i);
        mii.fState=0;
        mii.fType=MFT_STRING;
        mii.dwTypeData=CDROM_List[i].Config.FullName;
        if(CDROM_List[i].Config.Ignore && NULL!=ign_menu)
            {
            DebugLog(("    Inserting [%s] into ign_menu\n", 
                CDROM_List[i].Config.FullName));
            InsertMenuItem(ign_menu, pos_ign++, TRUE, &mii);
            }
        else
            {
            DebugLog(("    Inserting [%s] into RMenu\n", 
                CDROM_List[i].Config.FullName));
            InsertMenuItem(RMenu, pos_r++, TRUE, &mii);
            }
        }

    ZeroMemory(&miiw, sizeof(miiw));
    miiw.cbSize=sizeof(miiw);
    miiw.fMask=MIIM_DATA|MIIM_STATE|MIIM_TYPE|MIIM_SUBMENU;
    miiw.dwItemData=CMD_CDLIST_ITEM;
    miiw.fState= pos_ign>0 ? 0 : MFS_DISABLED|MFS_GRAYED;
    miiw.hSubMenu=ign_menu;
    miiw.fType=MFT_STRING;
    len=LoadStringU(NULL, STR_MNU_IGNORED, NULL, 0, 0);
    if(len>0 && NULL!=(miiw.dwTypeData=calloc(len+1, sizeof(WCHAR))))
        LoadStringU(NULL, STR_MNU_IGNORED, miiw.dwTypeData, len+1, 0);
    DebugLog(("    STR_MNU_IGNORED=0x%X\n", miiw.dwTypeData));
    InsertMenuItemU(RMenu, pos_r, TRUE, &miiw);
    free(miiw.dwTypeData);

    DebugLog(("    RMenuInsertList FINISHED\n"));
    }

static void RMenuClearList(void)
    {
    MENUITEMINFO mii;

    /* Delete all items from list */
    for(;;)
        {
        mii.cbSize=sizeof(mii);
        mii.fMask=MIIM_DATA|MIIM_SUBMENU;
        if(GetMenuItemInfo(RMenu, RMENU_LIST_POSITION, TRUE, &mii)==0)
            break;
        if(mii.dwItemData!=CMD_CDLIST_ITEM)
            break;
        DeleteMenu(RMenu, RMENU_LIST_POSITION, MF_BYPOSITION);
        if(NULL!=mii.hSubMenu)
            {
            DebugLog(("RMenuClearList: DestroyMenu(0x%X)\n", mii.hSubMenu));
            DestroyMenu(mii.hSubMenu);
            }
        }
    }

void DestroyRMenu(void)
    {
    if(RMenu)
        {
        RMenuClearList();
        DestroyMenu(RMenu);
        RMenu=NULL;
        }
    }

void CreateLMenu(void)
    {
    LMenu=CreatePopupMenu();
    BitmapCSpeed=LoadBitmap(GetModuleHandle(NULL), "BitmapCSpeed");
    BitmapEject=LoadBitmap(GetModuleHandle(NULL), "BitmapEject");
    BitmapClose=LoadBitmap(GetModuleHandle(NULL), "BitmapClose");
    }

static void UpdateLMenu(void)
    {
    ZapMenu(LMenu, FALSE);
    LMenuInsertList();
    }

static void LMenuInsertList(void)
    {
    int i;
    BOOL drive_separator;
    BOOL item_separator;
    HMENU cmenu;
    WCHAR buf[CDROM_NAME_SIZE];

    drive_separator=FALSE;
    for(i=CDROM_Count-1; i>=0; i--)
        {
        if(CDROM_List[i].Config.Ignore)
            continue;
        /* Drive title */
        if(Config.NumNames)
            {
            if(*CDROM_List[i].Config.ShortName)
                strcpyU(buf, CDROM_List[i].Config.ShortName);
            else
                strcpyU(buf, CDROM_List[i].Config.AutoShortName);
            }
        else
            Stra(buf, CDROM_List[i].Config.FullName);
        /* Busy drive title */
        if(CDROM_List[i].State.Busy)
            {
            InsertDriveName(LMenu, buf, NULL, 0, NULL, TRUE);
            drive_separator=TRUE;
            continue;
            }
        /* Drive submenu */
        if(Config.DeepMenu)
            {
            cmenu=CreatePopupMenu();
            if(NULL==cmenu)
                continue;
            InsertDriveName(LMenu, buf, NULL, 0, cmenu, FALSE);
            }
        else
            {
            cmenu=LMenu;
            if(drive_separator)
                {
                InsertSeparator(LMenu);
                InsertSeparator(LMenu);
                }
            drive_separator=TRUE;
            }
        item_separator=FALSE;
        /* Bottom drive title */
        if(!Config.DeepMenu && !Config.TopNames)
            {
            InsertDriveName(LMenu, buf,
                Config.EjectSingle ? BitmapEject : NULL, i, NULL, FALSE);
            item_separator=TRUE;
            }
        /* Eject menu */
        if(Config.DeepMenu || Config.TopNames || !Config.EjectSingle)
            {
            if(item_separator) InsertSeparator(cmenu);
            InsertEjectMenu(cmenu, i, Config.EjectSingle);
            }
        /* Shell commands */
        if(0!=CDROM_List[i].State.Addr.Letter &&
            (Config.CmdOpen || Config.CmdExplore || Config.CmdAutorun)
            )
            {
            InsertSeparator(cmenu);
            InsertShellMenu(cmenu, i);
            }
        /* Speed menu */
        if(CDROM_List[i].State.SL->Count>0)
            {
            InsertSeparator(cmenu);
            InsertSpeedMenu(cmenu, i);
            }
        /* Top drive title */
        if(!Config.DeepMenu && Config.TopNames)
            {
            InsertSeparator(cmenu);
            InsertDriveName(LMenu, buf, NULL, 0, NULL, FALSE);
            }
        }
    }

static void InsertDriveName(HMENU menu, WCHAR *name,
    HBITMAP eject, int n, HMENU sub, BOOL disabled)
    {
    MENUITEMINFOW mii;

    ZeroMemory(&mii, sizeof(mii));
    mii.cbSize=sizeof(mii);
    mii.fMask=MIIM_TYPE|MIIM_STATE;
    if(disabled)
        mii.fState=MFS_DISABLED|MFS_GRAYED;
    else
        {
        if(eject)
            {
            mii.fMask|=MIIM_ID|MIIM_CHECKMARKS;
            mii.fState=MFS_CHECKED;
            }
        else if(sub)
            {
            mii.fMask|=MIIM_SUBMENU;
            mii.fState=MFS_ENABLED;        
            }
        else
            mii.fState=MFS_DISABLED;
        }
    mii.wID=MKCDCMD(CMD_CD_EJECT, n);
    mii.hSubMenu=sub;
    mii.fType=MFT_STRING;
    mii.dwTypeData=name;
    mii.hbmpChecked=eject;
    InsertMenuItemU(menu, 0, TRUE, &mii);
    }

static void InsertEjectMenu(HMENU menu, int n, BOOL single)
    {
    if(!single)
        InsertMenu(menu, 0, MF_BYPOSITION|MF_BITMAP, MKCDCMD(CMD_CD_CLOSE, n),
            (LPCSTR)BitmapClose);
    InsertMenu(menu, 0, MF_BYPOSITION|MF_BITMAP, MKCDCMD(CMD_CD_EJECT, n),
        (LPCSTR)BitmapEject);
    }

static void InsertSpeedMenu(HMENU menu, int n)
    {
    int i;
    int cten, ten, tensel, tencur;
    HMENU submenu;
    unsigned speed, selected, current;
    int tdiv;
    BOOL ExtraMenu=FALSE;
    struct CDROM_Desc *cd=CDROM_List+n;

    /* No menu if no speeds */
    if(0==cd->State.SL->Count)
        return;

    tdiv=cd->State.SL->x1;
    if(cd->Config.ShortMenu) /* Creating short menu */
        {
        /* Insert main speed values */
        cten=0;
        for(i=cd->State.SL->Count-1; i>=0; i--)
            {
            speed=cd->State.SL->Range[i].Speed;
            selected=IsSpeedSelected(cd, speed);
            current=IsSpeedCurrent(cd, speed);
            ten=SPEED_DIV(speed, tdiv)/10;
            /* First ten, fisrst speed in each ten and last speed */
            if(ten==0 || ten!=cten || i==0)
                {
                InsertSpeedItem(menu, 0, MKSPEEDCMD(n, i),
                    speed, selected, current, tdiv);
                cten=ten;
                }
            else
                ExtraMenu=TRUE;
            }
        /* Insert submenus */
        if(ExtraMenu)
            {
            InsertMenu(menu, 0, MF_BYPOSITION|MF_SEPARATOR, 0, NULL);
            cten=0;
            submenu=NULL;
            tensel=FALSE;
            tencur=FALSE;
            for(i=cd->State.SL->Count-1; i>=0; i--)
                {
                speed=cd->State.SL->Range[i].Speed;
                selected=IsSpeedSelected(cd, speed);
                current=IsSpeedCurrent(cd, speed);
                ten=SPEED_DIV(speed, tdiv)/10;
                /* Skip first ten and last speed */
                if(ten==0 || i==0)
                    continue;
                if(ten!=cten) /* First speed in ten */
                    {
                    /* Insert previous submenu if new ten */
                    if(submenu!=NULL)
                        InsertSpeedItem(menu, MF_POPUP, (UINT)submenu,
                            cten*10*tdiv, tensel, tencur, tdiv);
                    submenu=NULL;
                    tensel=FALSE;
                    tencur=FALSE;
                    cten=ten;
                    }
                else /* Not first speed */
                    {
                    /* Create submenu if not created already */
                    if(submenu==NULL)
                        submenu=CreatePopupMenu();
                    /* Insert speed item in submenu */
                    InsertSpeedItem(submenu, 0,
                        MKSPEEDCMD(n, i),
                        speed, selected, current, tdiv);
                    if(selected) tensel=TRUE;
                    if(current) tencur=TRUE;
                    }
                }
            /* Insert previous submenu at end */
            if(submenu!=NULL)
                InsertSpeedItem(menu, MF_POPUP, (UINT)submenu,
                    cten*10*tdiv, tensel, tencur, tdiv);
            }
        }
    else /* Creating full menu */
        {
        for(i=cd->State.SL->Count-1; i>=0; i--)
            {
            speed=cd->State.SL->Range[i].Speed;
            selected=IsSpeedSelected(cd, speed);
            current=IsSpeedCurrent(cd, speed);
            if(InsertSpeedItem(menu, 0, MKSPEEDCMD(n, i),
                    speed, selected, current, tdiv)==FALSE)
                break;
            }
        }
    }
    
static BOOL InsertSpeedItem(
    HMENU menu,
    unsigned flags,
    unsigned item,
    unsigned speed,
    BOOL selected,
    BOOL current,
    int tdiv
    )
    {
    char buf[7];
    char *prefix;
    char suffix='x';
    
    if(flags==MF_POPUP)
        prefix=">";
    else
        prefix="";
    wsprintf(buf, "%s%u%c", prefix, SPEED_DIV(speed, tdiv)%1000, suffix);
    flags|=MF_STRING;
    if(selected || current)
        flags|=MF_CHECKED;
    if(InsertMenu(menu, 0, MF_BYPOSITION|flags, item, buf)==FALSE)
        return FALSE;
    if(!selected && current)
        SetMenuItemBitmaps(menu, item, MF_BYCOMMAND, NULL, BitmapCSpeed);
    return TRUE;
    }

static BOOL IsSpeedCurrent(struct CDROM_Desc *cd, unsigned speed)
    {
    return speed/CD1X==cd->State.Speed/CD1X;
    }

static BOOL IsSpeedSelected(struct CDROM_Desc *cd, unsigned speed)
    {
    return speed==cd->State.SL->Selected;
    }

static void ZapMenu(HMENU menu, BOOL destroy)
    {
    MENUITEMINFO mii;

    for(;;)
        {
        mii.cbSize=sizeof(mii);
        mii.fMask=MIIM_SUBMENU;
        mii.hSubMenu=NULL;
        if(GetMenuItemInfo(menu, 0, TRUE, &mii)==0)
            {
            if(destroy)
                DestroyMenu(menu);
            return;
            }
        DeleteMenu(menu, 0, MF_BYPOSITION);
        if(mii.hSubMenu!=NULL)
            ZapMenu(mii.hSubMenu, TRUE);
        }
    }
    
void DestroyLMenu(void)
    {
    if(LMenu)
        {
        ZapMenu(LMenu, TRUE);
        LMenu=NULL;
        }
    }

void RunLMenu(HWND window)
    {
    UpdateLMenu();
    RunMenu(window, LMenu);
    }

void RunRMenu(HWND window)
    {
    UpdateRMenu();
    RunMenu(window, RMenu);
    }

static void RunMenu(HWND window, HMENU menu)
    {
    POINT p;

    GetCursorPos(&p);
    SetForegroundWindow(window);
    TrackPopupMenuEx(menu, TPM_RIGHTALIGN|TPM_LEFTBUTTON, p.x, p.y,
        window, NULL);
    }

static void InsertSeparator(HMENU menu)
    {
    InsertMenu(menu, 0, MF_BYPOSITION|MF_SEPARATOR, 0, NULL);
    }

static void InsertShellMenu(HMENU menu, int i)
    {
    struct MenuInfo mi;

    mi.Flags=MF_STRING;

    if(Config.CmdOpen)
        {
        mi.Id=MKCDCMD(CMD_CD_BROWSE, i);
        mi.rcid=STR_MNU_BROWSE;
        MenuInsertItem(menu, 0, &mi);
        if(!CDROM_List[i].State.DiskPresent)
            EnableMenuItem(menu, mi.Id, MF_BYCOMMAND|MF_GRAYED);
        }

    if(Config.CmdExplore)
        {
        mi.Id=MKCDCMD(CMD_CD_EXPLORE, i);
        mi.rcid=STR_MNU_EXPLORE;
        MenuInsertItem(menu, 0, &mi);
        if(!CDROM_List[i].State.DiskPresent)
            EnableMenuItem(menu, mi.Id, MF_BYCOMMAND|MF_GRAYED);
        }

    if(Config.CmdAutorun)
        {
        mi.Id=MKCDCMD(CMD_CD_AUTORUN, i);
        mi.rcid=STR_MNU_AUTORUN;
        MenuInsertItem(menu, 0, &mi);
        if(!IsAutorunInf(CDROM_List+i))
            EnableMenuItem(menu, mi.Id, MF_BYCOMMAND|MF_GRAYED);
        }
    }
