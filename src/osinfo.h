/*
Copyright 2002-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __OSINFO_H__
#define __OSINFO_H__

DWORD OsVersion(void);
DWORD GetShellVersionMajor(void);
DWORD GetComctlVersionMajor(void);
BOOL IsAutoPlayOn(void);
BOOL SetAutoPlay(BOOL state);
void AddAutoLoad(WCHAR const *name);
void AddAutoLoadExit(WCHAR const *name);
void AddAutoLoadHide(WCHAR const *name);
void DeleteAutoLoad(char *name);
int IsAutoLoadAny(WCHAR const *name);
int IsAutoLoadSimple(WCHAR const *name);
int IsAutoLoadExit(WCHAR const *name);
int IsAutoLoadHide(WCHAR const *name);
BOOL IsAutoRunUser(void);
BOOL IsAutoRunSystem(void);
BOOL SetAutoRunUser(BOOL state);
BOOL SetAutoRunSystem(BOOL state);
BOOL CanSetAutoRunSystem(void);
BOOL CanSetAutoRunUser(void);
char *GetExePath(void);
char *GetHelpPath(void);
unsigned GetHungTimeout(void);

#endif /* __OSINFO_H__ */
