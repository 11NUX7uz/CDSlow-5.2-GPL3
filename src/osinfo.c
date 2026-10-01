/*
Copyright 2002-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include <shlwapi.h>
#include "log.h"
#include "regfunc.h"
#include "cmdline.h"
#include "osinfo.h"

static char *HelpExt=".chm";

static char *AutoLoadKey="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
static char *NoAutoRunKey=
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
static char *NoTypeAutoRunVal="NoDriveTypeAutoRun";
static char *AudioCDKey="AudioCD\\shell";
#define MASK_CDROM (1<<DRIVE_CDROM)
static char *TimeoutPath="Control Panel\\Desktop";
static char *HungTimeout="HungAppTimeout";

static char *OldNames[]=
    {
    "CDSlow 2.0",
    "CDSlow 2.1",
    NULL
    };

DWORD OsVersion(void)
    {
    static DWORD version=0;
    OSVERSIONINFO ovi;

    if(0==version)
        {
        ovi.dwOSVersionInfoSize=sizeof(ovi);
        if(!GetVersionEx(&ovi))
            return 0;
        version=MAKEWORD(ovi.dwMinorVersion, ovi.dwMajorVersion);
        if(VER_PLATFORM_WIN32_NT==ovi.dwPlatformId)
            version|=MAKELONG(0, 1);
        }

    return version;
    }

static DWORD GetDllVersionMajor(char *dll_name)
    {
    DWORD major=0;
    HINSTANCE dll;
    DLLGETVERSIONPROC DllGetVersion;
    DLLVERSIONINFO dvi;

    dll=LoadLibrary(dll_name);
    if(NULL==dll)
        return 0;
    DllGetVersion=(DLLGETVERSIONPROC)GetProcAddress(dll, "DllGetVersion");
    dvi.cbSize=sizeof(dvi);
    if(DllGetVersion!=NULL && NOERROR==DllGetVersion(&dvi))
        major=dvi.dwMajorVersion;
    FreeLibrary(dll);

    return major;
    }

DWORD GetShellVersionMajor(void)
    {
    static DWORD major=0;

    if(0==major)
        major=GetDllVersionMajor("shell32.dll");

    return major;
    }

DWORD GetComctlVersionMajor(void)
    {
    static DWORD major=0;

    if(0==major)
        major=GetDllVersionMajor("comctl32.dll");

    return major;
    }

char *GetExePath(void)
    {
    char *program;

    program=malloc(MAX_PATH);
    if(NULL==program)
        return NULL;
    if(0==GetModuleFileName(NULL, program, MAX_PATH))
        {
        free(program);
        return NULL;
        }
    return program;
    }

WCHAR *GetExePathW(void)
    {
    WCHAR *program = calloc(MAX_PATH, sizeof(*program));

    if(NULL == program)
        return NULL;

    if(0 == GetModuleFileNameW(NULL, program, MAX_PATH))
        {
        free(program);
        return NULL;
        }

    return program;
    }

char *GetHelpPath(void)
    {
    char *path;
    char *ext;
    char *help;

    path=GetExePath();
    if(NULL==path)
        return NULL;

    ext=strrchr(path, '.');
    if(NULL!=ext)
        {
        help=realloc(path, ext-path+strlen(HelpExt)+1);
        if(NULL!=help)
            {
            strcpy(help+(ext-path), HelpExt);
            return help;
            }
        }
    
    free(path);
    return NULL;
    }

static void AddAutoLoadCmd(WCHAR const *name, WCHAR const *cmdline)
    {
    HKEY key;
    DWORD action;
    char **oldname;

    if(!cmdline)
        return;

    if(ERROR_SUCCESS==RegCreateKeyEx(HKEY_CURRENT_USER, AutoLoadKey,
            0, NULL,  REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &key, &action)
        )
        {
        SetValStringWW(key, name, cmdline);
        oldname=OldNames;

        while(*oldname!=NULL)
            RegDeleteValue(key, *oldname++);

        RegCloseKey(key);
        }
    }

void AddAutoLoad(WCHAR const *name)
    {
    WCHAR *program = GetExePathW();
    AddAutoLoadCmd(name, program);
    free(program);
    }

void AddAutoLoadExit(WCHAR const *name)
    {
    WCHAR *program = GetExePathW();
    program = AddCmdOptExit(program);
    AddAutoLoadCmd(name, program);
    free(program);
    }

void AddAutoLoadHide(WCHAR const *name)
    {
    WCHAR *program = GetExePathW();
    program = AddCmdOptHide(program);
    AddAutoLoadCmd(name, program);
    free(program);
    }

void DeleteAutoLoad(char *name)
    {
    HKEY key;
    LONG res;

    res=RegOpenKeyEx(HKEY_CURRENT_USER, AutoLoadKey, 0, KEY_ALL_ACCESS, &key);
    if(res!=ERROR_SUCCESS)
        return;
    RegDeleteValue(key, name);
    RegCloseKey(key);
    }

static WCHAR *GetAutoLoad(WCHAR const *name)
    {
    WCHAR *regval = NULL;
    HKEY key;
    int sz;

    if(RegOpenKeyEx(HKEY_CURRENT_USER, AutoLoadKey, 0, KEY_READ, &key) == ERROR_SUCCESS)
        {
        sz = GetValStringWW(key, name, NULL, 0);

        if(sz > 0)
            {
            regval = malloc(sz);

            if(regval != NULL)
                GetValStringWW(key, name, regval, sz);
            }
        }

    return regval;
    }

static int CheckAutoLoad(WCHAR const *name, WCHAR **cmdline, WCHAR **program)
    {
    *cmdline = GetAutoLoad(name);
    *program = GetExePathW();

    return *program && *cmdline && _wcsnicmp(*program, *cmdline, lstrlenW(*program)) == 0;
    }

int IsAutoLoadAny(WCHAR const *name)
    {
    WCHAR *cmdline = NULL;
    WCHAR *program = NULL;
    int ret = 0;

    ret = CheckAutoLoad(name, &cmdline, &program);

    free(program);
    free(cmdline);

    return ret;
    }

int IsAutoLoadSimple(WCHAR const *name)
    {
    WCHAR *cmdline = NULL;
    WCHAR *program = NULL;
    int ret = 0;

    ret = CheckAutoLoad(name, &cmdline, &program) && lstrlenW(cmdline) == lstrlenW(program);

    free(program);
    free(cmdline);

    return ret;
    }

int IsAutoLoadExit(WCHAR const *name)
    {
    WCHAR *cmdline = NULL;
    WCHAR *program = NULL;
    int ret = 0;

    ret = CheckAutoLoad(name, &cmdline, &program) && IsCmdOptExit(cmdline);

    free(program);
    free(cmdline);

    return ret;
    }

int IsAutoLoadHide(WCHAR const *name)
    {
    WCHAR *cmdline = NULL;
    WCHAR *program = NULL;
    int ret = 0;

    ret = CheckAutoLoad(name, &cmdline, &program) && IsCmdOptHide(cmdline);

    free(program);
    free(cmdline);

    return ret;
    }

static BOOL IsAutoRun(HKEY base)
    {
    HKEY key;
    int res;
    BYTE notyperun[4];

    if(RegOpenKeyEx(base, NoAutoRunKey, 0, KEY_READ, &key)!=ERROR_SUCCESS)
        return TRUE;
    res=GetValArray(key, NoTypeAutoRunVal, notyperun, sizeof(notyperun));
    RegCloseKey(key);

    return res==0 || (notyperun[0]&MASK_CDROM)==0;
    }

BOOL IsAutoRunUser(void)
    {
    return IsAutoRun(HKEY_CURRENT_USER);
    }

BOOL IsAutoRunSystem(void)
    {
    return IsAutoRun(HKEY_LOCAL_MACHINE);
    }

static void ShellUpdatePolicy(void)
    {
    DWORD res;

    DebugLog(("        ShellUpdatePolicy STARTED\n"));
    DebugLog(("            SendMessageTimeout(WM_SETTINGCHANGE)...\n"));
    SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Policy",
        SMTO_NORMAL, 1000, &res);
    DebugLog(("                DONE\n"));
    DebugLog(("            ShellUpdatePolicy FINISHED\n"));
    }

static BOOL CanSetAutoRun(HKEY base)
    {
    HKEY key;
    DWORD action;

    if(RegCreateKeyEx(base, NoAutoRunKey, 0, NULL, 0, KEY_WRITE|KEY_READ,
        NULL, &key, &action)!=ERROR_SUCCESS)
        return FALSE;

    RegCloseKey(key);
    if(REG_CREATED_NEW_KEY==action)
        RegDeleteKey(base, NoAutoRunKey);

    return TRUE;
    }

BOOL CanSetAutoRunSystem(void)
    {
    return CanSetAutoRun(HKEY_LOCAL_MACHINE);
    }

BOOL CanSetAutoRunUser(void)
    {
    return CanSetAutoRun(HKEY_CURRENT_USER);
    }

static BOOL SetAutoRun(HKEY base, BOOL state)
    {
    HKEY key;
    int res;
    DWORD action;
    DWORD notyperun;

    DebugLog(("    SetAutoRun(%d) STARTED\n", state));
    DebugLog(("        RegCreateKeyEx(%s)...\n", NoAutoRunKey));
    if(RegCreateKeyEx(base, NoAutoRunKey, 0, NULL, 0, KEY_WRITE|KEY_READ, NULL,
            &key, &action)!=ERROR_SUCCESS)
        {
        DebugLog(("        SetAutoRun FAILED\n"));
        return FALSE;
        }
    DebugLog(("            DONE\n"));
    DebugLog(("        GetValArray(%s)...\n", NoTypeAutoRunVal));
    res=GetValArray(key, NoTypeAutoRunVal, &notyperun, sizeof(notyperun));
    DebugLog(("            DONE(0x%X)\n", notyperun));
    if(res==0)
        notyperun=0xFF; /* Was 0x95 ; */
    if(state)
        notyperun&=~MASK_CDROM;
    else
        notyperun|=MASK_CDROM;
    if(HIWORD(OsVersion()))
        {
        /* WinNT */
        DebugLog(("        SetValDWORD(%s, 0x%X)...\n",
                NoTypeAutoRunVal, notyperun));
        res=SetValDWORD(key, NoTypeAutoRunVal, notyperun);
        DebugLog(("            DONE(%s)\n",
                ERROR_SUCCESS==res ? "OK" : "FAILED"));
        }
    else
        {
        /* Win9X */
        DebugLog(("        SetValArray(%s, 0x%X)...\n",
                NoTypeAutoRunVal, notyperun));
        res=SetValArray(key, NoTypeAutoRunVal, &notyperun, sizeof(notyperun));
        DebugLog(("            DONE(%s)\n",
                ERROR_SUCCESS==res ? "OK" : "FAILED"));
        }
    RegCloseKey(key);

    if(res==ERROR_SUCCESS)
        {
        DebugLog(("        ShellUpdatePolicy...\n"));
        ShellUpdatePolicy();
        DebugLog(("            DONE\n"));
        }

    DebugLog(("        SetAutoRun FINISHED(%d)\n", ERROR_SUCCESS==res));
    return ERROR_SUCCESS==res;
    }

BOOL SetAutoRunUser(BOOL state)
    {
    return SetAutoRun(HKEY_CURRENT_USER, state);
    }

BOOL SetAutoRunSystem(BOOL state)
    {
    HKEY key;
    int res;
    DWORD action;

    if(!state)
        return SetAutoRun(HKEY_LOCAL_MACHINE, state);

    DebugLog(("    SetAutoRunSystem(%d) STARTED\n", state));

    DebugLog(("        RegCreateKeyEx(%s)...\n", NoAutoRunKey));
    if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, NoAutoRunKey, 0, NULL, 0,
            KEY_WRITE|KEY_READ, NULL, &key, &action)!=ERROR_SUCCESS)
        {
        DebugLog(("        SetAutoRunSystem FAILED\n"));
        return FALSE;
        }
    DebugLog(("            DONE\n"));
    DebugLog(("        RegDeleteValue(%s)...\n", NoTypeAutoRunVal));
    res=RegDeleteValue(key, NoTypeAutoRunVal);
    DebugLog(("            DONE\n"));
    RegCloseKey(key);

    if(res==ERROR_SUCCESS)
        ShellUpdatePolicy();

    DebugLog(("        SetAutoRunSystem FINISHED(%d)\n", ERROR_SUCCESS==res));
    return res==ERROR_SUCCESS;
    }

BOOL IsAutoPlayOn(void)
    {
    HKEY key;
    int res;
    char buf[64];

    if(RegOpenKeyEx(HKEY_CLASSES_ROOT, AudioCDKey, 0, KEY_READ, &key)!=
        ERROR_SUCCESS)
        return TRUE;
    res=GetValString(key, NULL, buf, sizeof(buf));
    RegCloseKey(key);

    return res!=0 && *buf!=0;
    }

BOOL SetAutoPlay(BOOL state)
    {
    HKEY key;
    char *buf;
    int res;

    if(RegOpenKeyEx(HKEY_CLASSES_ROOT, AudioCDKey, 0, KEY_WRITE, &key)!=
        ERROR_SUCCESS)
        return FALSE;
    if(state)
        buf="play";
    else
        buf="";
    res=SetValString(key, NULL, buf);
    RegCloseKey(key);

    return res==ERROR_SUCCESS;
    }

static unsigned GetTimeout(char *val_name)
    {
    HKEY key;
    int res;
    char buf[16];

    if(RegOpenKeyEx(HKEY_CURRENT_USER, TimeoutPath, 0, KEY_READ, &key)!=
        ERROR_SUCCESS)
        return 20000;
    res=GetValString(key, val_name, buf, sizeof(buf));
    RegCloseKey(key);

    if(res!=0)
        res=atoi(buf);
    else
        res=20000;

    return res;
    }

unsigned GetHungTimeout(void)
    {
    if(OsVersion()<0x010600)
        return GetTimeout(HungTimeout);
    else
        return 15000;
    }
