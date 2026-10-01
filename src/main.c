/*
Copyright 2000-2021 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "log.h"
#include "build.h"
#include "cmdline.h"
#include "cdslow.h"

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpszCmdLine, int nCmdShow)
    {
    WCHAR *cmdline;
    struct CMD_Options opts;
    HWND window;

    (void)hPrevInstance; /* Unused */
    (void)lpszCmdLine; /* Unused */
    (void)nCmdShow; /* Unused */

    cmdline = GetCommandLineW();
    ParseCmdLine(&opts, cmdline);

    if(opts.Log)
        InitLog(NULL, NULL);

    if(opts.CloseCount != 0 || opts.EjectCount != 0 || opts.SetCount != 0)
        {
        CDSlowPrepare();

        if(opts.EjectCount != 0)
            {
            DebugLog(("Eject and exit...\n"));
            CDSlowEject(opts.EjectCount, opts.EjectList);
            }

        if(opts.CloseCount != 0)
            {
            DebugLog(("Close tray and exit...\n"));
            CDSlowClose(opts.CloseCount, opts.CloseList);
            }

        if(opts.SetCount != 0)
            {
            DebugLog(("Set arbitrary speed and exit...\n"));
            CDSlowSet(opts.SetCount, opts.SetList);
            }

        CDSlowFinish();
        return 0;
        }

    if(opts.Exit)
        {
        DebugLog(("Set default speed and exit...\n"));
        CDSlowOnce();
        return 0;
        }

    window=FindWindow(AppName, AppName);
    if(window!=NULL)
        {
        DebugLog(("Another instance of %s found\n", AppName));
        if(opts.Hide)
            {
            DebugLog(("Sending HIDE to 0x%x\n", window));
            HideIcon(window);
            return 0;
            }
        else if(opts.Restart)
            {
            DebugLog(("Sending CLOSE to 0x%x\n", window));
            SendMessage(window, WM_CLOSE, 0, 0);
            }
        else
            {
            DebugLog(("Sending SHOW to 0x%x\n", window));
            ShowIcon(window);
            return 0;
            }
        }
    else
        {
#ifdef BUILD_ID
        DebugLog(("First instance of %s%s started\n", AppName, BUILD_ID));
#else
        DebugLog(("First instance of %s started\n", AppName));
#endif
        }

    CDSlow(hInstance, opts.Hide);

    return 0;
    }
