/* ===================================================================
 * Copyright (c) 2002-2019 Vadim Druzhin (cdslow@mail.ru).
 * 
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ===================================================================
 */

#define STRICT
#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include "ntddscsi.h"
#include "scsidefs.h"
#include "uniscsi.h"

static BOOL NextCDROM(int StartLetter, UNI_ADDR *addr);
static UINT TestDriveType(int letter);
static enum NT_STATUS TestAccess(void);
static DWORD AccessMode=GENERIC_READ|GENERIC_WRITE;
static enum NT_STATUS NTStatus=SPTI_NOT_INITIALIZED;

BOOL NTInit(void)
    {
    OSVERSIONINFO ovi;

    ovi.dwOSVersionInfoSize=sizeof(ovi);
    if(GetVersionEx(&ovi)==FALSE)
        {
        NTStatus=SPTI_WRONG_VERSION;
        return FALSE;
        }
    if(ovi.dwMajorVersion<=4)
        AccessMode=GENERIC_READ;
    else
        AccessMode=GENERIC_READ|GENERIC_WRITE;

    if(ovi.dwPlatformId!=VER_PLATFORM_WIN32_NT)
        {
        NTStatus=SPTI_WRONG_VERSION;
        return FALSE;
        }

    NTStatus=TestAccess();
    return NTStatus!=SPTI_NO_ACCESS;
    }

static HANDLE NTOpenSCSI(unsigned letter)
    {
    HANDLE dev;
    int i;
    char buf[]="\\\\.\\X:";

    for(i=0; i<3; i++)
        {
        buf[4]=letter;
        dev=CreateFile(
            buf,
            AccessMode,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );
        if(dev!=INVALID_HANDLE_VALUE || GetLastError()!=0x20)
            break;
        Sleep(150);
        }
    return(dev);
    }

#if USRB_TIMEOUT==0
#define NT_TIMEOUT 0
#else
#define NT_TIMEOUT USRB_TIMEOUT
#endif

void *NTSCSIBatchBegin(UNI_SRB *usrb)
    {
    HANDLE dev;
    HANDLE *ret=NULL;

    dev=NTOpenSCSI(usrb->Addr.Letter);
    if(dev!=INVALID_HANDLE_VALUE)
        {
        ret=malloc(sizeof(*ret));
        if(ret!=NULL)
            {
            *ret=dev;
            return ret;
            }
        CloseHandle(dev);
        }

    usrb->Err.NTStatus=GetLastError();
    usrb->Err.ASPIStatus=-1;
    usrb->Err.HAStatus=-1;
    usrb->Err.DevStatus=-1;
    usrb->Err.SenseKey=-1;

    return ret;
    }

BOOL NTSCSIBatchCmd(UNI_SRB *usrb, void *batch)
    {
    BOOL ret;
    DWORD cb;
    SCSI_PASS_THROUGH_DIRECT *spt;
    long inbuflen=sizeof(*spt)+sizeof(usrb->SenseBuffer);
    BYTE inbuf[sizeof(*spt)+sizeof(usrb->SenseBuffer)];
    HANDLE dev;

    usrb->Err.NTStatus=-1;
    usrb->Err.ASPIStatus=-1;
    usrb->Err.HAStatus=-1;
    usrb->Err.DevStatus=-1;
    usrb->Err.SenseKey=-1;
    
    dev=*(HANDLE *)batch;

    ZeroMemory(inbuf, inbuflen);
    spt=(SCSI_PASS_THROUGH_DIRECT *)inbuf;
    spt->Length=sizeof(*spt);
    spt->PathId=usrb->Addr.Ha;
    spt->TargetId=usrb->Addr.Dev;
    spt->Lun=usrb->Addr.Lun;
    spt->CdbLength=usrb->CDBLen;
    if(usrb->SenseLen<sizeof(usrb->SenseBuffer))
        spt->SenseInfoLength=usrb->SenseLen;
    else
        spt->SenseInfoLength=sizeof(usrb->SenseBuffer);
    spt->DataIn= usrb->DataOut ? SCSI_IOCTL_DATA_OUT : SCSI_IOCTL_DATA_IN;
    spt->DataTransferLength=usrb->DataLen;
    spt->TimeOutValue=NT_TIMEOUT;
    spt->DataBuffer=usrb->DataBuffer;
    spt->SenseInfoOffset=sizeof(*spt);
    memcpy(spt->Cdb, usrb->CDB, sizeof(spt->Cdb));

    ret=DeviceIoControl(
        dev,
        IOCTL_SCSI_PASS_THROUGH_DIRECT,
        spt,
        sizeof(*spt),
        inbuf,
        inbuflen,
        &cb,
        NULL
        );

    if(ret)
        {
        usrb->Err.DevStatus=spt->ScsiStatus;
        if(spt->SenseInfoLength>0)
            usrb->Err.SenseKey=inbuf[spt->SenseInfoOffset+2];
        memcpy(usrb->SenseBuffer, inbuf+spt->SenseInfoOffset, sizeof(usrb->SenseBuffer));
        usrb->SenseLen=spt->SenseInfoLength;
        usrb->DataLen=spt->DataTransferLength;
        }
    else
        usrb->Err.NTStatus=GetLastError();
    
    return ret && usrb->Err.DevStatus==STATUS_GOOD;
    }

void NTSCSIBatchEnd(void *batch)
    {
    HANDLE dev;

    dev=*(HANDLE *)batch;
    CloseHandle(dev);
    free(batch);
    }

BOOL NTSCSICmd(UNI_SRB *usrb)
    {
    void *batch;
    BOOL ret;

    batch=NTSCSIBatchBegin(usrb);
    if(batch==NULL)
        return FALSE;

    ret=NTSCSIBatchCmd(usrb, batch);

    NTSCSIBatchEnd(batch);

    return ret;
    }

BOOL NTGetFirstCDROM(UNI_ADDR *addr)
    {
    return NextCDROM('A', addr);
    }

BOOL NTGetNextCDROM(UNI_ADDR *addr)
    {
    return NextCDROM(addr->Letter+1, addr);
    }

static BOOL NextCDROM(int StartLetter, UNI_ADDR *addr)
    {
    DWORD devmask;
    int letter;
    UINT DriveType;

    devmask=GetLogicalDrives();
    letter=StartLetter;
    devmask>>=StartLetter-'A';
    while(devmask)
        {
        if(devmask&1)
            {
            DriveType=TestDriveType(letter);
            if(DriveType==DRIVE_CDROM)
                {
                addr->Ha=0;
                addr->Dev=0;
                addr->Lun=0;
                addr->Letter=letter;
                return TRUE;
                }
            }
        devmask>>=1;
        letter++;
        }

    return FALSE;
    }

static UINT TestDriveType(int letter)
    {
    char path[]="X:\\";

    path[0]=letter;
    return GetDriveType(path);
    }

static enum NT_STATUS TestAccess(void)
    {
    UNI_ADDR addr;
    HANDLE dev;

    if(!NTGetFirstCDROM(&addr))
        return SPTI_OK;

    do
        {
        dev=NTOpenSCSI(addr.Letter);
        if(dev!=INVALID_HANDLE_VALUE)
            {
            CloseHandle(dev);
            return SPTI_OK;
            }
        }
    while(NTGetNextCDROM(&addr));

    return SPTI_NO_ACCESS;
    }

enum NT_STATUS NTGetStatus(void)
    {
    return NTStatus;
    }

