/*
Copyright 2005-2019 Vadim Druzhin <cdslow@mail.ru>

You can redistribute and/or modify this file under the terms
of the GNU General Public License version 3 or any later version.

See file COPYING.txt or visit <http://www.gnu.org/licenses/>.
*/

#define STRICT
#include <windows.h>
#include "dialogs.h"

BOOL SaveDialog(HWND window, char *namebuf, int bufsize)
    {
    static OPENFILENAME ofn;

    if(ofn.lStructSize==0)
        {
        ofn.lStructSize=sizeof(ofn);
        /*ofn.hwndOwner=window;*/
        ofn.hInstance=NULL;
        ofn.lpstrFilter="*.reg\0*.reg\0*.*\0*.*\0";
        ofn.lpstrCustomFilter=NULL;
        ofn.nMaxCustFilter=0;
        ofn.nFilterIndex=0;
        /*ofn.lpstrFile=namebuf;*/
        /*ofn.nMaxFile=bufsize;*/
        ofn.lpstrFileTitle=NULL;
        ofn.nMaxFileTitle=0;
        ofn.lpstrInitialDir=NULL;
        ofn.lpstrTitle=NULL;
        ofn.Flags=OFN_HIDEREADONLY|
                  OFN_LONGNAMES|
                  OFN_NOREADONLYRETURN|
                  OFN_OVERWRITEPROMPT;
        ofn.nFileOffset=0;
        ofn.nFileExtension=0;
        ofn.lpstrDefExt="reg";
        ofn.lCustData=0;
        ofn.lpfnHook=NULL;
        ofn.lpTemplateName=NULL;
        }
    ofn.hwndOwner=window;
    ofn.lpstrFile=namebuf;
    ofn.nMaxFile=bufsize;

    return GetSaveFileName(&ofn);
    }
