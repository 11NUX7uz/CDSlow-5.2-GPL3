/*
Copyright 2002-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#include "cdlist.h"
#include "configdata.h"

void AboutDialog(HWND window);
HWND BarDialog(HWND window, UINT title_id, char *drive_name);
BOOL BarUpdate(unsigned min, unsigned max, unsigned pos,
    unsigned speed, unsigned command, void *param);
BOOL EditConfig(HWND window, struct CDS_Config *cfg);
BOOL EditDriveOptions(HWND window, struct CDROM_Desc *cd, char *hash);
void InfoDialog(HWND window, struct CDROM_Desc *cd, int count);
void DiskInDriveNotify(HWND window, unsigned timeout);
void TrayOpenNotify(HWND window, unsigned timeout, unsigned close);
BOOL QueryPassword(HWND window, char *hash, int hash_size);
BOOL AppQueryPassword(HWND window, char *hash);
BOOL SaveDialog(HWND window, char *namebuf, int bufsize);
int AppMessageBox(HWND window, DWORD rcid, UINT flags);
