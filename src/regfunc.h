/*
Copyright 2001-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#ifndef __REGFUNC_H__
#define __REGFUNC_H__

struct REG_ValueUnsigned
    {
    char *Name;
    size_t Off;
    };

HKEY OpenSubkey(char *keyname);
HKEY CreateSubkey(char *keyname);
HKEY OpenMainKey(void);
HKEY CreateMainKey(void);
BOOL GetValDWORD(HKEY key, char *valname, DWORD *value);
int GetValArray(HKEY key, char *valname, void *value, DWORD valsize);
unsigned GetValString(HKEY key, char const *valname, char *value, DWORD valsize);
int GetValStringW(HKEY key, char *valname, WCHAR *value, DWORD valsize);
int GetValStringWW(HKEY key, WCHAR const *valname, WCHAR *value, DWORD valsize);
int SetValDWORD(HKEY key, char *valname, DWORD value);
int SetValArray(HKEY key, char *valname, void *value, DWORD valsize);
int SetValString(HKEY key, char const *valname, char const *value);
int SetValStringW(HKEY key, char *valname, WCHAR *value);
int SetValStringWW(HKEY key, WCHAR const *valname, WCHAR const *value);
void ClearConfigToRegistry(void);
BOOL GetAllUnsigned(
    HKEY key,
    void *array,
    struct REG_ValueUnsigned all[],
    int n
    );
void SetAllUnsigned(
    HKEY key,
    void *array,
    struct REG_ValueUnsigned all[],
    int n
    );
HKEY OpenIndexKey(char *keyname, int index);
HKEY CreateIndexKey(char *keyname, int index);
void SaveConfigToFile(char *path);

#endif /* __REGFUNC_H__ */
