/*
Copyright 2001-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "regfunc.h"
#include "osinfo.h"
#include "log.h"

static char *RegKey="Software\\HalfBit Software\\CDSlow\\4.0";

static BOOL ZapKey(HKEY basekey, char *name);
static char *MakeIndexName(char *keyname, int index);
static char *UpperKeyName(char *keyname);
static BOOL IsKeyEmpty(HKEY basekey, char *keyname);

HKEY OpenSubkey(char *keyname)
    {
    HKEY key;
    HKEY subkey=NULL;

    key=OpenMainKey();
    if(key!=NULL)
        {
        RegOpenKeyEx(key, keyname, 0, KEY_READ, &subkey);
        RegCloseKey(key);
        }

    return subkey;
    }

HKEY CreateSubkey(char *keyname)
    {
    HKEY key;
    HKEY subkey=NULL;
    DWORD action;

    key=CreateMainKey();
    if(key!=NULL)
        {
        RegCreateKeyEx(
            key, keyname, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &subkey, &action);
        RegCloseKey(key);
        }

    return subkey;
    }

HKEY OpenMainKey(void)
    {
    HKEY key;
    LONG res;

    res=RegOpenKeyEx(HKEY_CURRENT_USER, RegKey, 0, KEY_READ, &key);
    if(res==ERROR_SUCCESS)
        return key;

    return NULL;
    }

HKEY CreateMainKey(void)
    {
    HKEY key;
    DWORD action;
    LONG res;

    res=RegCreateKeyEx(
        HKEY_CURRENT_USER, RegKey, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS, NULL, &key, &action);
    if(res!=ERROR_SUCCESS)
        return NULL;
    
    return key;
    }

BOOL GetValDWORD(HKEY key, char *valname, DWORD *value)
    {
    DWORD valsize;
    LONG res;

    valsize=sizeof(*value);
    res=RegQueryValueEx(key, valname, 0, NULL, (LPBYTE)value, &valsize);

    return res==ERROR_SUCCESS;
    }

int GetValArray(HKEY key, char *valname, void *value, DWORD valsize)
    {
    LONG res;

    res=RegQueryValueEx(key, valname, 0, NULL, (LPBYTE)value, &valsize);

    return (res==ERROR_SUCCESS) ? valsize : 0;
    }

unsigned GetValString(HKEY key, char const *valname, char *value, DWORD valsize)
    {
    LONG res;

    res=RegQueryValueEx(key, valname, 0, NULL, (LPBYTE)value, &valsize);

    return (res==ERROR_SUCCESS) ? valsize : 0;
    }

int SetValDWORD(HKEY key, char *valname, DWORD value)
    {
    return RegSetValueEx(key, valname, 0,
        REG_DWORD, (LPBYTE)&value, sizeof(value));
    }

int SetValArray(HKEY key, char *valname, void *value, DWORD valsize)
    {
    return RegSetValueEx(key, valname, 0, REG_BINARY, (LPBYTE)value, valsize);
    }

int SetValString(HKEY key, char const *valname, char const *value)
    {
    return RegSetValueEx(key, valname, 0,
        REG_SZ, (LPCBYTE)value, lstrlen(value)+1);
    }

int SetValStringW(HKEY key, char *valname, WCHAR *value)
    {
    return RegSetValueEx(key, valname, 0, REG_BINARY, (LPBYTE)value,
        (lstrlenW(value)+1)*sizeof(*value));
    }

int SetValStringWW(HKEY key, WCHAR const *valname, WCHAR const *value)
    {
    return RegSetValueExW(key, valname, 0,
        REG_SZ, (LPCBYTE)value, (lstrlenW(value)+1) * sizeof(*value));
    }

int GetValStringW(HKEY key, char *valname, WCHAR *value, DWORD valsize)
    {
    LONG res;

    res=RegQueryValueEx(key, valname, 0, NULL, (LPBYTE)value, &valsize);

    return (res==ERROR_SUCCESS) ? valsize : 0;
    }

int GetValStringWW(HKEY key, WCHAR const *valname, WCHAR *value, DWORD valsize)
    {
    LONG res;

    res = RegQueryValueExW(key, valname, 0, NULL, (LPBYTE)value, &valsize);

    return (res==ERROR_SUCCESS) ? valsize : 0;
    }

void ClearConfigToRegistry(void)
    {
    char *progkey;
    char *companykey;

    progkey=UpperKeyName(RegKey);
    if(progkey!=NULL)
        {
        ZapKey(HKEY_CURRENT_USER, progkey);
        companykey=UpperKeyName(progkey);
        if(companykey!=NULL)
            {
            if(IsKeyEmpty(HKEY_CURRENT_USER, companykey))
                RegDeleteKey(HKEY_CURRENT_USER, companykey);
            free(companykey);
            }
        free(progkey);
        }
    }

static BOOL ZapKey(HKEY basekey, char *name)
    {
    HKEY key;
    char *buf;
    DWORD klen;
    FILETIME t;

    /* Try to delete key */
    if(RegDeleteKey(basekey, name)==ERROR_SUCCESS)
        return TRUE;

    /* Delete subkeys */
    if((buf=malloc(MAX_PATH))==NULL)
        return FALSE;
    if(RegOpenKeyEx(basekey, name, 0,
        KEY_ALL_ACCESS, &key)!=ERROR_SUCCESS)
        {
        free(buf);
        return FALSE;
        }
    while(
        RegEnumKeyEx(
            key, 0, buf, (klen=MAX_PATH, &klen),
            NULL, NULL, NULL, &t
            )==ERROR_SUCCESS
        )
        {
        if(!ZapKey(key, buf))
            break;
        }
    RegCloseKey(key);
    free(buf);

    /* Second try */
    if(RegDeleteKey(basekey, name)==ERROR_SUCCESS)
        return TRUE;

    /* Sorry */
    return FALSE;
    }

static BOOL IsKeyEmpty(HKEY basekey, char *keyname)
    {
    HKEY key;
    DWORD subnum;

    if(RegOpenKeyEx(basekey, keyname, 0, KEY_READ, &key)!=ERROR_SUCCESS)
        return FALSE;

    subnum=0;
    RegQueryInfoKey(key, NULL, NULL, NULL, &subnum,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    RegCloseKey(key);

    return subnum==0;
    }

static char *UpperKeyName(char *keyname)
    {
    char *p;
    char *buf;
    int buflen;

    p=strrchr(keyname, '\\');
    if(p==NULL)
        return NULL;
    buflen=p-keyname+1;

    if((buf=malloc(buflen))==NULL)
        return NULL;

    lstrcpyn(buf, keyname, buflen);
    buf[buflen-1]=0;

    return buf;
    }

BOOL GetAllUnsigned(
    HKEY key,
    void *array,
    struct REG_ValueUnsigned all[],
    int n
    )
    {
    int i;
    DWORD dw;
    BOOL ret=TRUE;

    for(i=0; i<n; i++)
        {
        if(GetValDWORD(key, all[i].Name, &dw))
            *(unsigned *)((BYTE *)array+all[i].Off)=dw;
        else
            ret=FALSE;
        }

    return ret;
    }

void SetAllUnsigned(
    HKEY key,
    void *array,
    struct REG_ValueUnsigned all[],
    int n
    )
    {
    int i;

    for(i=0; i<n; i++)
        SetValDWORD(key, all[i].Name, *(unsigned *)((BYTE *)array+all[i].Off));
    }

HKEY OpenIndexKey(char *keyname, int index)
    {
    HKEY subkey;
    char *buf;

    buf=MakeIndexName(keyname, index);
    if(buf==NULL)
        return NULL;

    subkey=OpenSubkey(buf);

    free(buf);

    return subkey;
    }

HKEY CreateIndexKey(char *keyname, int index)
    {
    HKEY subkey;
    char *buf;

    buf=MakeIndexName(keyname, index);
    if(buf==NULL)
        return NULL;

    subkey=CreateSubkey(buf);

    free(buf);

    return subkey;
    }

static char *MakeIndexName(char *keyname, int index)
    {
    char *buf=NULL;

    /* Allocate temporary string */
    buf=malloc(strlen(keyname)+4);
    if(buf!=NULL)
        {
        /* Make subkey name in form keyname\index */
        wsprintf(buf, "%s\\%02d", keyname, index%100);
        }

    return buf;
    }

void SaveConfigToFile(char *path)
    {
    char *keyprefix="HKEY_CURRENT_USER\\";
    char *prog1="regedit.exe";
    char *format1="/ea \"%s\" \"%s%s\"";
    char *prog2="reg.exe";
    char *format2="export \"%s%s\" \"%s\"";
    char *prog, *format, *s1, *s2, *s3;
    char *p;

    if(OsVersion()<0x010600)
        {
        prog=prog1;
        format=format1;
        s1=path;
        s2=keyprefix;
        s3=RegKey;
        }
    else
        {
        prog=prog2;
        format=format2;
        s1=keyprefix;
        s2=RegKey;
        s3=path;
        }

    p=malloc(strlen(format)+strlen(s1)+strlen(s2)+strlen(s3)+1);
    if(NULL==p)
        return;

    wsprintf(p, format, s1, s2, s3);
    DebugLog(("SaveConfigToFile: '%s' '%s'\n", prog, p));

    ShellExecute(NULL, NULL, prog, p, NULL, SW_HIDE);	

    free(p);
    }

