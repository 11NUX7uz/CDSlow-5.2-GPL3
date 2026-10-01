/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __CDS_CMD__
#define __CDS_CMD__
enum
    {
    CMD_SHOW_RMENU=100,
    CMD_SHOW_LMENU,
    CMD_ABOUT,
    CMD_HELP,
    CMD_INFO,
    CMD_FIND_CDROM,
    CMD_CONFIG,
    CMD_CDLIST_ITEM,
    CMD_CLEARCONFIG,
    CMD_EXIT,
    CMD_MEDIA_CHANGED,
    CMD_MEDIA_REMOVED,
    CMD_TIMER,
    CMD_POWER_RESUME,
    CMD_TIMER_AUTOCLOSE,
    CMD_ICON_HIDE=200,
    CMD_ICON_SHOW=201
    };
enum
    {
    CMD_CD_EDIT,
    CMD_CD_EJECT,
    CMD_CD_CLOSE,
    CMD_CD_SETMAX,
    CMD_CD_SETSEL,
    CMD_CD_SETMIN,
    CMD_CD_SPEEDUP,
    CMD_CD_SPEEDDOWN,
    CMD_CD_TIMER,
    CMD_CD_BROWSE,
    CMD_CD_EXPLORE,
    CMD_CD_AUTORUN,
    CMD_SELECT_SPEED
    };

#define ISCDCMD(cmd)     ((cmd)&0x8000)
#define GETCDCMD(cmd)    ((cmd)&0x000F)
#define GETCDNUM(cmd)   (((cmd)&0x00F0)>>4)
#define GETCDSPEED(cmd) (((cmd)&0x7F00)>>8)
#define MAXCOUNT_CMD    16
#define MKCDCMD(cmd, drive) (((cmd)&0x0F)|(((drive)&0x0F)<<4)|0x8000)
#define MKSPEEDCMD(drive, speed) \
    (MKCDCMD(CMD_SELECT_SPEED, drive)|(((speed)&0x7F)<<8))
    
#endif /* __CDS_CMD__ */
