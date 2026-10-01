/*
Copyright 2005-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "version.h"

#define NAME "CDSlow"
#define LNAME L"CDSlow"

char *AppName=NAME;
WCHAR *AppNameW=LNAME;
WCHAR *AppTitle=LNAME L" " APP_VERSION_STR;
