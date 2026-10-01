/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __MENU_H__
#define __MENU_H__

BOOL CreateRMenu(void);
void DestroyRMenu(void);
void RunRMenu(HWND window);
void CreateLMenu(void);
void DestroyLMenu(void);
void RunLMenu(HWND window);

#endif /* __MENU_H__ */
