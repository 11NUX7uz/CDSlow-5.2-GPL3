/*
Copyright 2006-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __CDSHELL_H__
#define __CDSHELL_H__

#include "cdlist.h"

BOOL IsAutorunInf(struct CDROM_Desc *cd);
BOOL DoAutorunInf(struct CDROM_Desc *cd);
void DoCDROM(struct CDROM_Desc *cd, char *operation);

#endif /* __CDSHELL_H__ */
