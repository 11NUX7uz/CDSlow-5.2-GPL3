/*
Copyright 2009-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "cdlist.h"
#include "msg_ids.h"
#include "dlglib.h"
#include "dialogs.h"
#include "w6notify.h"

typedef BOOL WINAPI SBRCPROC(HWND hWnd, LPCWSTR pwszReason);

static BOOL ShutdownBlockReasonCreateDll(HWND window, WCHAR *str)
    {
    static HINSTANCE dll;
    static SBRCPROC *DllSBRC;

    if(NULL==dll)
        {
        dll=LoadLibrary("user32.dll");
        if(NULL==dll)
            return FALSE;
        }
    if(NULL==DllSBRC)
        {
        DllSBRC=(SBRCPROC *)GetProcAddress(dll, "ShutdownBlockReasonCreate");
        if(NULL==DllSBRC)
            return FALSE;
        }

    return DllSBRC(window, str);
    }

static BOOL ShutdownBlockReasonCompose(HWND window, int rcid, WCHAR *str)
    {
    int len;
    int len2=0;
    WCHAR *w, *buf;
    BOOL ret;

    w=MapStringU(NULL, rcid, 0, &len);
    if(NULL==w)
        return FALSE;

    if(NULL!=str)
        len2=lstrlenW(str);

    buf=malloc((len+len2+1)*sizeof(*buf));
    if(NULL==buf)
        return FALSE;

    memcpy(buf, w, len*sizeof(*buf));
    if(len2>0)
        memcpy(buf+len, str, len2*sizeof(*buf));
    buf[len+len2]=0;

    ret=ShutdownBlockReasonCreateDll(window, buf);

    free(buf);

    return ret;
    }

typedef BOOL WINAPI SBRDPROC(HWND hWnd);

BOOL ShutdownBlockReasonDestroyDll(HWND window)
    {
    static HINSTANCE dll;
    static SBRDPROC *DllSBRD;

    if(NULL==dll)
        {
        dll=LoadLibrary("user32.dll");
        if(NULL==dll)
            return FALSE;
        }
    if(NULL==DllSBRD)
        {
        DllSBRD=(SBRDPROC *)GetProcAddress(dll, "ShutdownBlockReasonDestroy");
        if(NULL==DllSBRD)
            return FALSE;
        }

    return DllSBRD(window);
    }

typedef BOOL WAIT_FUNC(void);

static BOOL WaitWithReason(HWND window, int rcid, WAIT_FUNC *wait_done,
    unsigned timeout, unsigned min, WCHAR *str)
    {
    BOOL ret;
    WCHAR short_buf[7];
    WCHAR *buf=short_buf;

    ret=ShutdownBlockReasonCompose(window, rcid, NULL);

    if(NULL!=str)
        {
        buf=malloc(lstrlenW(str)*sizeof(*buf)+sizeof(short_buf));
        if(NULL==buf)
            buf=short_buf;
        }

    while(timeout>0)
        {
        Sleep(1000);
        if(timeout>min && wait_done())
            {
            if(0==min)
                break;
            else
                timeout=min;
            }
        else
            --timeout;
        strcpyU(Num(strcpyU(short_buf, L" ("), timeout), L")\n");
        if(short_buf!=buf)
            strcpyU(strcpyU(buf, short_buf), str);
        ShutdownBlockReasonCompose(window, rcid, buf);
        }

    if(short_buf!=buf)
        free(buf);

    return ret;
    }

BOOL DiskInDriveW6(HWND window, unsigned timeout, unsigned pre)
    {
    WCHAR *labels=NULL;
    BOOL ret;

    if(pre>timeout)
        pre=timeout;
    if(pre>0)
        {
        DiskInDriveNotify(window, pre);
        timeout-=pre;
        }

    if(0!=timeout)
        labels=GetAllLabels();

    ret=WaitWithReason(window, STR_DNOTIFY_DISK, IsAllEmpty, timeout, 5, labels);

    if(NULL!=labels)
        free(labels);

    return ret;
    }

BOOL TrayOpenW6(HWND window, unsigned timeout, BOOL close, unsigned pre)
    {
    BOOL ret;

    if(pre>timeout)
        pre=timeout;
    if(pre>0)
        {
        TrayOpenNotify(window, pre, FALSE);
        timeout-=pre;
        }

    ret=WaitWithReason(window, STR_DNOTIFY_TRAY, IsAllClosed, timeout, 0, NULL);

    if(close)
        CloseAllDisks();

    return ret;
    }
