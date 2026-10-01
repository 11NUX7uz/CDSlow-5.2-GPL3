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
#include "wnaspi32.h"
#include "scsidefs.h"
#include "uniscsi.h"
#include "log.h"

static HINSTANCE WNASPI32=NULL;
static DWORD (*DllGetASPI32SupportInfo)(VOID)=NULL;
static DWORD (*DllSendASPI32Command)(LPSRB)=NULL;
static BOOL (*DllTranslateASPI32Address)(PDWORD, PDWORD)=NULL;
static unsigned SCSICount;
static enum ASPI_STATUS ASPIStatus=ASPI_NOT_INITIALIZED;
static void *ASPIVersionInfo=NULL;
static char ASPIVersionLang[9];

static unsigned GetHADevLimit(unsigned n);
static unsigned GetSCSIDeviceType(unsigned ha, unsigned dev, unsigned lun);
static BOOL NextCDROM(unsigned StartHa, unsigned StartDev, UNI_ADDR *addr);
static void StoreVersionInfo(void);
static void FreeVersionInfo(void);

BOOL ASPIInit(void)
    {
    DWORD SCSInfo, SCSIstatus;

    if(WNASPI32==NULL)
        WNASPI32=LoadLibrary("wnaspi32.dll");
    if(WNASPI32==NULL)
        {
        ASPIStatus=ASPI_NOT_PRESENT;
        return FALSE;
        }
    StoreVersionInfo();

    DllGetASPI32SupportInfo=
        (DWORD (*)(VOID))GetProcAddress(WNASPI32, "GetASPI32SupportInfo");
    DllSendASPI32Command=
        (DWORD (*)(LPSRB))GetProcAddress(WNASPI32, "SendASPI32Command");
    DllTranslateASPI32Address=
        (BOOL (*)(PDWORD, PDWORD))GetProcAddress(WNASPI32,
                                                 "TranslateASPI32Address");
    if(DllGetASPI32SupportInfo==NULL || DllSendASPI32Command==NULL)
        {
        ASPIStatus=ASPI_BROKEN;
        return FALSE;
        }

    SCSInfo=DllGetASPI32SupportInfo();
    SCSICount=(BYTE)SCSInfo;
    SCSIstatus=(BYTE)(SCSInfo>>8);
    if(SCSIstatus!=SS_COMP && SCSIstatus!=SS_NO_ADAPTERS)
        {
        ASPIStatus=ASPI_BROKEN;
        return FALSE;
        }

    ASPIStatus=ASPI_OK;
    return TRUE;
    }

void ASPIDestroy(void)
    {
    FreeVersionInfo();
    DllGetASPI32SupportInfo=NULL;
    DllSendASPI32Command=NULL;
    DllTranslateASPI32Address=NULL;
    if(NULL!=WNASPI32)
        {
        FreeLibrary(WNASPI32);
        WNASPI32=NULL;
        }
    }

static void StoreVersionInfo(void)
    {
    char path[MAX_PATH];
    DWORD blank;
    DWORD info_size;
    void *vi;
    unsigned len;
    unsigned lang, charset;

    FreeVersionInfo();
    if(NULL==WNASPI32)
        return;
    if(0==GetModuleFileName(WNASPI32, path, sizeof(path)))
        return;
    info_size=GetFileVersionInfoSize(path, &blank);
    if(0==info_size)
        return;
    ASPIVersionInfo=malloc(info_size);
    if(NULL==ASPIVersionInfo)
        return;
    if(0==GetFileVersionInfo(path, 0, info_size, ASPIVersionInfo))
        FreeVersionInfo();
    if(!VerQueryValue(ASPIVersionInfo, "\\VarFileInfo\\Translation", &vi, &len) || 0==len)
        {
        DebugLog(("StoreVersionInfo: VerQueryValue(%s) failed!\n",
                "\\VarFileInfo\\Translation"));
        lang=0x409;
        charset=0x4b0;
        }
    else
        {
        lang=((WORD *)vi)[0];
        charset=((WORD *)vi)[1];
        }
    wsprintf(ASPIVersionLang, "%04x%04x", lang, charset);
    }

static void FreeVersionInfo(void)
    {
    if(NULL!=ASPIVersionInfo)
        {
        free(ASPIVersionInfo);
        ASPIVersionInfo=NULL;
        *ASPIVersionLang=0;
        }
    }

char *ASPIGetVersionInfo(char *param)
    {
    void *vr=ASPIVersionInfo;
    void *vi;
    unsigned len;
    char *prefix="\\StringFileInfo\\";
    char *buf;

    if(NULL==vr)
        {
        DebugLog(("ASPIGetVersionInfo: version not loaded!\n"));
        return NULL;
        }

    buf=malloc(strlen(prefix)+strlen(ASPIVersionLang)+1+strlen(param)+1);
    if(NULL==buf)
        return NULL;
    strcat(strcat(strcat(strcpy(buf, prefix), ASPIVersionLang), "\\"), param);

    if(!VerQueryValue(vr, buf, &vi, &len) || 0==len)
        {
        DebugLog(("ASPIGetVersionInfo: VerQueryValue(%s) failed!\n", buf));
        vi=NULL;
        }

    free(buf);

    return vi;
    }

#if USRB_TIMEOUT==0
#define ASPI_TIMEOUT INFINITE
#else
#define ASPI_TIMEOUT (USRB_TIMEOUT*1000l)
#endif

BOOL ASPISCSICmd(UNI_SRB *usrb)
    {
    int SCSIstatus;
    HANDLE CmdComplete;
    DWORD WaitStatus;
    SRB_ExecSCSICmd SRB;

    usrb->Err.NTStatus=-1;
    usrb->Err.ASPIStatus=-1;
    usrb->Err.HAStatus=-1;
    usrb->Err.DevStatus=-1;
    usrb->Err.SenseKey=-1;

    if(DllSendASPI32Command==NULL)
        return FALSE;

    ZeroMemory(&SRB, sizeof(SRB));
    SRB.SRB_Cmd=SC_EXEC_SCSI_CMD;
    SRB.SRB_HaId=(BYTE)usrb->Addr.Ha;
    SRB.SRB_Target=(BYTE)usrb->Addr.Dev;
    SRB.SRB_Lun=(BYTE)usrb->Addr.Lun;
    if(usrb->DataLen>0)
        SRB.SRB_Flags= usrb->DataOut ? SRB_DIR_OUT : SRB_DIR_IN;
    else
        SRB.SRB_Flags=0;
    SRB.SRB_BufLen=usrb->DataLen;
    SRB.SRB_BufPointer=usrb->DataBuffer;
    SRB.SRB_SenseLen=SENSE_LEN;
    SRB.SRB_CDBLen=usrb->CDBLen;
    memcpy(SRB.CDBByte, usrb->CDB, sizeof(SRB.CDBByte));

    CmdComplete=CreateEvent(NULL,FALSE,FALSE,NULL);
    if(CmdComplete!=NULL)
        {
        SRB.SRB_Flags|=SRB_EVENT_NOTIFY;
        *(void **)(&SRB.SRB_PostProc)=CmdComplete;
        }

    SCSIstatus=DllSendASPI32Command((LPSRB)&SRB);

    if(SCSIstatus==SS_PENDING && CmdComplete!=NULL)
        {
        WaitStatus=WaitForSingleObject(CmdComplete, ASPI_TIMEOUT);
        if(WaitStatus!=WAIT_OBJECT_0)
            {
            usrb->Err.ASPIStatus=SCSIstatus;
            return FALSE;
            }
        SCSIstatus=SRB.SRB_Status;
        }
    if(CmdComplete!=NULL)
        CloseHandle(CmdComplete);

    usrb->Err.ASPIStatus=SRB.SRB_Status;
    usrb->Err.HAStatus=SRB.SRB_HaStat;
    usrb->Err.DevStatus=SRB.SRB_TargStat;
    if(SRB.SRB_SenseLen>0)
        usrb->Err.SenseKey=SRB.SenseArea[2];
    memcpy(usrb->SenseBuffer, SRB.SenseArea, sizeof(usrb->SenseBuffer));
    usrb->SenseLen=SRB.SRB_SenseLen;
    usrb->DataLen=SRB.SRB_BufLen;

    return SCSIstatus==SS_COMP;
    }

void *ASPISCSIBatchBegin(UNI_SRB *usrb)
    {
    static char notnull;

    (void)usrb; /* Unused */

    return (void *)&notnull;
    }

BOOL ASPISCSIBatchCmd(UNI_SRB *usrb, void *batch)
    {
    (void)batch; /* Unused */

    return ASPISCSICmd(usrb);
    }

void ASPISCSIBatchEnd(void *batch)
    {
    (void)batch; /* Unused */
    }

BOOL ASPIGetFirstCDROM(UNI_ADDR *addr)
    {
    return NextCDROM(0, 0, addr);
    }

BOOL ASPIGetNextCDROM(UNI_ADDR *addr)
    {
    return NextCDROM(addr->Ha, addr->Dev+1, addr);
    }

enum ASPI_STATUS ASPIGetStatus(void)
    {
    return ASPIStatus;
    }

int ASPIGetAdapterCount(void)
    {
    return SCSICount;
    }

static int GetLetter(int ha, int dev, int lun)
    {
    int letter=0;
    DWORD SCSIPath;
    DWORD DevNode;
    char node[]="Config Manager\\Enum\\00000000";
    LONG res;
    DWORD len;
    char lmkey[]="Enum\\";
    char letter_buf[2];
    HKEY k=NULL;
    char *lmpath=NULL;
    
    if(NULL==DllTranslateASPI32Address)
        return 0;

    SCSIPath=((BYTE)(ha)<<16)+((BYTE)(dev)<<8)+(BYTE)lun;
    DevNode=0;
    if(!DllTranslateASPI32Address(&SCSIPath, &DevNode) || 0==DevNode)
        {
        DebugLog(("GetLetter: DllTranslateASPI32Address *FAILED*\n"));
        return 0;
        }

    wsprintf(node+sizeof(node)-9, "%08X", DevNode);
    DebugLog(("GetLetter: DevNode=%s\n", node));
    res=RegOpenKeyEx(HKEY_DYN_DATA, node, 0, KEY_READ, &k);
    if(ERROR_SUCCESS!=res)
        return 0;

    res=RegQueryValueEx(k, "HardWareKey", NULL, NULL, NULL, &len);
    if(ERROR_SUCCESS!=res)
        goto finish;
    lmpath=malloc(len+sizeof(lmkey)-1);
    if(NULL==lmpath)
        goto finish;
    strcpy(lmpath, lmkey);
    res=RegQueryValueEx(k, "HardWareKey", NULL, NULL,
        (BYTE *)(lmpath+sizeof(lmkey)-1), &len);
    if(ERROR_SUCCESS!=res)
        goto finish;
    RegCloseKey(k);
    k=NULL;
    res=RegOpenKeyEx(HKEY_LOCAL_MACHINE, lmpath, 0, KEY_READ, &k);
    if(ERROR_SUCCESS!=res)
        goto finish;
    len=sizeof(letter_buf);
    res=RegQueryValueEx(k, "CurrentDriveLetterAssignment", NULL, NULL,
        (BYTE *)letter_buf, &len);
    if(ERROR_SUCCESS!=res)
        goto finish;

    letter=*letter_buf;

    finish:
    if(NULL!=lmpath)
        free(lmpath);
    if(NULL!=k)
        RegCloseKey(k);

    return letter;
    }

static BOOL NextCDROM(unsigned StartHa, unsigned StartDev, UNI_ADDR *addr)
    {
    unsigned ha, dev;
    unsigned status;
    unsigned DevLimit;
    unsigned DevType;

    ha=StartHa;
    dev=StartDev;
    for(; ha<SCSICount; ha++, dev=0)
        {
        status=GetHADevLimit(ha);
        DevLimit=(BYTE)status;
        status>>=8;
        if(status!=SS_COMP)
            continue;
        for(; dev<DevLimit; dev++)
            {
            status=GetSCSIDeviceType(ha, dev, 0);
            DevType=(BYTE)status;
            status>>=8;
            if(status!=SS_COMP)
                continue;
            if(DevType==DTYPE_CROM)
                {
                addr->Ha=ha;
                addr->Dev=dev;
                addr->Lun=0;
                addr->Letter=GetLetter(ha, dev, 0);
                return TRUE;
                }
            }
        }

    return FALSE;
    }

static unsigned GetHADevLimit(unsigned n)
    {
    SRB_HAInquiry SRB_HAI;
    unsigned SCSIstatus;
    unsigned limit;

    if(DllSendASPI32Command==NULL)
        return SS_ERR<<8;

    ZeroMemory(&SRB_HAI, sizeof(SRB_HAI));
    SRB_HAI.SRB_Cmd=SC_HA_INQUIRY;
    SRB_HAI.SRB_HaId=(BYTE)n;
    SCSIstatus=DllSendASPI32Command((LPSRB)&SRB_HAI);
    if(SCSIstatus!=SS_COMP)
        return SCSIstatus<<8;

    limit=SRB_HAI.HA_Unique[3];
    if(limit==0)
        limit=8;
    else if(limit>16)
        limit=16;

    return (SCSIstatus<<8)|limit;
    }

static unsigned GetSCSIDeviceType(unsigned ha, unsigned dev, unsigned lun)
    {
    SRB_GDEVBlock SRB_GDEV;
    int SCSIstatus;

    if(DllSendASPI32Command==NULL)
        return (SS_ERR<<8)|DTYPE_UNKNOWN;

    ZeroMemory(&SRB_GDEV, sizeof(SRB_GDEV));
    SRB_GDEV.SRB_Cmd=SC_GET_DEV_TYPE;
    SRB_GDEV.SRB_HaId=(BYTE)ha;
    SRB_GDEV.SRB_Target=(BYTE)dev;
    SRB_GDEV.SRB_Lun=(BYTE)lun;
    SCSIstatus=DllSendASPI32Command((LPSRB)&SRB_GDEV);
    if(SCSIstatus!=SS_COMP)
        return (SCSIstatus<<8)|DTYPE_UNKNOWN;

    return (SCSIstatus<<8)|SRB_GDEV.SRB_DeviceType;
    }


