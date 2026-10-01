/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __HOTKEYS_H__
#define __HOTKEYS_H__

#include "cdlist.h"

#define HOTKEYF_WIN 16

extern unsigned HotKeyCmd[CDROM_HOTKEYS];

void SetHotKeys(HWND window);
void ClearHotKeys(HWND window);

#endif /* __HOTKEYS_H__ */
