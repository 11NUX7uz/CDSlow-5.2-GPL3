/*
Copyright 2000-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __CDSLOW_H__
#define __CDSLOW_H__

#include "version.h"

void ShowIcon(HWND window);
void HideIcon(HWND window);
void CDSlowPrepare(void);
void CDSlowFinish(void);
void CDSlowOnce(void);
void CDSlowSet(size_t count, struct CMD_DriveOption const *dopts);
void CDSlowEject(size_t count, struct CMD_DriveOption const *dopts);
void CDSlowClose(size_t count, struct CMD_DriveOption const *dopts);
void CDSlow(HINSTANCE hInstance, BOOL hide);

#endif /* __CDSLOW_H__ */
