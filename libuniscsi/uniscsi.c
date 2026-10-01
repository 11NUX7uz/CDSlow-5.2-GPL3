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
#include "scsidefs.h"
#include "uniscsi.h"

const BD_TYPE BD_TYPE_UNKNOWN={"\0\0\0"};
const BD_TYPE BD_TYPE_ROM={"BDO"};
const BD_TYPE BD_TYPE_R={"BDR"};
const BD_TYPE BD_TYPE_RW={"BDW"};

static enum UNI_API SCSIAPI=UAPI_ASPI;
static BOOL (*SCSICmdSimple)(UNI_SRB *srb)=ASPISCSICmd;
static BOOL (*SCSIGetFirstCDROM)(UNI_ADDR *addr)=ASPIGetFirstCDROM;
static BOOL (*SCSIGetNextCDROM)(UNI_ADDR *addr)=ASPIGetNextCDROM;
static void *(*SCSIBatchBegin)(UNI_SRB *usrb)=ASPISCSIBatchBegin;
static BOOL (*SCSIBatchCmd)(UNI_SRB *usrb, void *batch)=ASPISCSIBatchCmd;
static void (*SCSIBatchEnd)(void *batch)=ASPISCSIBatchEnd;
static UNI_ERROR SCSILastError={-1, -1, -1, -1, -1};

#define MAX_TRY 3

BOOL SCSICmd(UNI_SRB *srb)
    {
    UNI_SRB save_srb=*srb;
    int i;
    BOOL ret;

    i=0;
    for(;;)
        {
        ret=SCSICmdSimple(srb);
        i++;
        if(ret || i>=MAX_TRY)
            break;
        else if(srb->Err.SenseKey==KEY_NOTREADY)
            Sleep(500);
        else if(srb->Err.SenseKey!=KEY_UNITATT)
            break;
        *srb=save_srb;
        }

    if(!ret)
        SCSILastError=srb->Err;

    return ret;
    }

static void SetASPI(void)
    {
    SCSIAPI=UAPI_ASPI;
    SCSICmdSimple=ASPISCSICmd;
    SCSIGetFirstCDROM=ASPIGetFirstCDROM;
    SCSIGetNextCDROM=ASPIGetNextCDROM;
    SCSIBatchBegin=ASPISCSIBatchBegin;
    SCSIBatchCmd=ASPISCSIBatchCmd;
    SCSIBatchEnd=ASPISCSIBatchEnd;
    }

static void SetSPTI(void)
    {
    SCSIAPI=UAPI_SPTI;
    SCSICmdSimple=NTSCSICmd;
    SCSIGetFirstCDROM=NTGetFirstCDROM;
    SCSIGetNextCDROM=NTGetNextCDROM;
    SCSIBatchBegin=NTSCSIBatchBegin;
    SCSIBatchCmd=NTSCSIBatchCmd;
    SCSIBatchEnd=NTSCSIBatchEnd;
    }

enum UNI_API SCSIInit(enum UNI_API prefered)
    {
    enum UNI_API api=UAPI_NONE;

    if(0==(prefered&UAPI_DISABLE_ASPI) && TRUE==ASPIInit())
        api|=UAPI_ASPI;
    if(TRUE==NTInit())
        api|=UAPI_SPTI;

    if(UAPI_ASPI==api || 0!=(api&prefered&UAPI_ASPI))
        SetASPI();
    else
        SetSPTI();

    return SCSIAPI;
    }

BOOL IsSCSIDriverError(void)
    {
    enum ASPI_STATUS as=ASPIGetStatus();
    enum NT_STATUS ns=NTGetStatus();

    return as==ASPI_BROKEN ||
        ns==SPTI_NOT_INITIALIZED ||
        (as!=ASPI_OK && ns!=SPTI_OK);
    }

enum UNI_API SCSIGetAPI(void)
    {
    return SCSIAPI;
    }

BOOL GetFirstCDROM(UNI_ADDR *addr)
    {
    return SCSIGetFirstCDROM(addr);
    }

BOOL GetNextCDROM(UNI_ADDR *addr)
    {
    return SCSIGetNextCDROM(addr);
    }

BOOL GetDeviceInfo(UNI_ADDR *addr, struct SCSI_DevInfo *DI)
    {
    UNI_SRB usrb;
    char buf[36];
    BOOL ret;

    ZeroMemory(DI, sizeof(*DI));

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=6;
    usrb.CDB[0]=SCSI_INQUIRY;
    usrb.CDB[1]=0;
    usrb.CDB[2]=0;
    usrb.CDB[4]=sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ret=SCSICmd(&usrb);

    if(ret)
        {
        DI->DType=buf[0];
        DI->RFormat=buf[3]&0xF;
        memcpy(DI->Vendor, buf+8, sizeof(DI->Vendor)-1);
        memcpy(DI->Product, buf+16, sizeof(DI->Product)-1);
        memcpy(DI->Revision, buf+32, sizeof(DI->Revision)-1);
        }

    return ret;
    }

BOOL GetCDROMInfo(UNI_ADDR *addr, struct SCSI_CDInfo *CDI)
    {
    UNI_SRB usrb;
    BOOL ret;
    unsigned char buf[256];
    unsigned char *buf2A;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=10;
    usrb.CDB[0]=SCSI_MODE_SEN10;
    usrb.CDB[1]=0;
    usrb.CDB[2]=0x2A;
    usrb.CDB[7]=sizeof(buf)>>8;
    usrb.CDB[8]=(BYTE)sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ret=SCSICmd(&usrb);

    CDI->MaxSpeed=0;
    CDI->Speed=0;
    CDI->WriteSpeed=0;
    CDI->SCSI=0;
    CDI->IByte2=0;
    CDI->IByte3=0;
    CDI->IByte4=0;
    CDI->IByte5=0;
    CDI->IByte6=0;
    CDI->IByte7=0;
    CDI->MediumType=0;

    if(ret)
        {
        if((buf[16]&0x3F)==0x2A)
            {
            buf2A=buf+16;
            CDI->SCSI=TRUE;
            }
        else if((buf[8]&0x3F)==0x2A)
            {
            buf2A=buf+8;
            CDI->SCSI=FALSE;
            CDI->MediumType=buf[2];
            }
        else if((buf[0]&0x3F)==0x2A)
            {
            buf2A=buf;
            CDI->SCSI=TRUE;
            }
        else
            return FALSE;
        
        CDI->MaxSpeed=((unsigned)buf2A[8]<<8)+buf2A[9];
        CDI->Speed=((unsigned)buf2A[14]<<8)+buf2A[15];
        if((buf2A[3]&1) || (buf2A[3]&2) || (buf2A[3]&0x10) || (buf2A[3]&0x20))
            CDI->WriteSpeed=((unsigned)buf2A[20]<<8)+buf2A[21];
        CDI->IByte2=buf2A[2];
        CDI->IByte3=buf2A[3];
        CDI->IByte4=buf2A[4];
        CDI->IByte5=buf2A[5];
        CDI->IByte6=buf2A[6];
        CDI->IByte7=buf2A[7];
        CDI->NLevels=((unsigned)buf2A[10]<<8)+buf2A[11];
        CDI->BufSize=((unsigned)buf2A[12]<<8)+buf2A[13];
#ifndef NDEBUG
        memcpy(CDI->RawBuf, buf, sizeof(CDI->RawBuf));
#endif
        }

    return ret;
    }

unsigned GetCDROMSpeed(UNI_ADDR *addr)
    {
    struct SCSI_CDInfo DI;

    if(GetCDROMInfo(addr, &DI))
        return DI.Speed;

    return 0;
    }

BOOL SetCDROMSpeed(UNI_ADDR *addr, unsigned speed, unsigned wspeed)
    {
    UNI_SRB usrb;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=12;
    usrb.CDB[0]=0xBB;
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5);
    usrb.CDB[2]=(BYTE)(speed>>8);
    usrb.CDB[3]=(BYTE)speed;
    usrb.CDB[4]=(BYTE)(wspeed>>8);
    usrb.CDB[5]=(BYTE)wspeed;
    usrb.DataLen=0;
    usrb.DataBuffer=NULL;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    return SCSICmd(&usrb);
    }

BOOL SetCDROMEject(UNI_ADDR *addr, BOOL eject)
    {
    UNI_SRB usrb;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=6;
    usrb.CDB[0]=SCSI_START_STP;
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5);
    usrb.CDB[4]=(BYTE)(eject ? 2 : 3);
    usrb.DataLen=0;
    usrb.DataBuffer=NULL;
    usrb.DataOut=0;

    return SCSICmd(&usrb);
    }

int TestUnitReady(UNI_ADDR *addr)
    {
    UNI_SRB usrb;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=6;
    usrb.CDB[0]=SCSI_TST_U_RDY;
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5);
    usrb.DataLen=0;
    usrb.DataBuffer=NULL;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    if(SCSICmdSimple(&usrb))
        return 0;

    SCSILastError=usrb.Err;

    if(KEY_NOTREADY==usrb.Err.SenseKey)
        return 1;

    return -1;
    }

void SCSIGetLastError(UNI_ERROR *error)
    {
    *error=SCSILastError;
    }

void SCSIClearError(void)
    {
    SCSILastError.NTStatus=-1;
    SCSILastError.ASPIStatus=-1;
    SCSILastError.HAStatus=-1;
    SCSILastError.DevStatus=-1;
    SCSILastError.SenseKey=-1;
    }

unsigned ReadCDROMCapacity(UNI_ADDR *addr)
    {
    UNI_SRB usrb;
    unsigned char buf[16];
    BOOL ret;
    unsigned LBA;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=10;
    usrb.CDB[0]=SCSI_READCDCAP;
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ret=SCSICmd(&usrb);

    if(ret)
        LBA=(buf[0]<<24)|(buf[1]<<16)|
            (buf[2]<<8)|buf[3];
    else
        LBA=0;

    return LBA;
    }

#define SEC_SIZE 2048
#define MAX_COUNT 32
BOOL ReadCDROM(UNI_ADDR *addr, unsigned LBA, unsigned count, void *buf)
    {
    UNI_SRB usrb;
    BOOL ret=FALSE;
    unsigned read_now;
    void *batch;

    usrb.Addr=*addr;
    batch=SCSIBatchBegin(&usrb);
    if(batch==NULL)
        {
        SCSILastError=usrb.Err;
        return FALSE;
        }

    while(count>0)
        {
        if(count>MAX_COUNT)
            read_now=MAX_COUNT;
        else
            read_now=count;
        ZeroMemory(&usrb, sizeof(usrb));
        usrb.Addr=*addr;
        usrb.CDBLen=10;
        usrb.CDB[0]=SCSI_READ10;
        usrb.CDB[2]=(unsigned char)(LBA>>24);
        usrb.CDB[3]=(unsigned char)(LBA>>16);
        usrb.CDB[4]=(unsigned char)(LBA>>8);
        usrb.CDB[5]=(unsigned char)LBA;
        usrb.CDB[7]=(unsigned char)(read_now>>8);
        usrb.CDB[8]=(unsigned char)read_now;
        usrb.DataLen=read_now*SEC_SIZE;
        usrb.DataBuffer=buf;
        usrb.DataOut=0;
        usrb.SenseLen=sizeof(usrb.SenseBuffer);

        ret=SCSIBatchCmd(&usrb, batch);

        if(!ret)
            {
            SCSILastError=usrb.Err;
            break;
            }

        count-=read_now;
        LBA+=read_now;
        buf=read_now*SEC_SIZE+(unsigned char *)buf;
        }

    SCSIBatchEnd(batch);

    return ret;
    }

BOOL SetCDROMLock(UNI_ADDR *addr, BOOL lock)
    {
    UNI_SRB usrb;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=6;
    usrb.CDB[0]=SCSI_MED_REMOVL;
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5);
    usrb.CDB[4]=(BYTE)(lock ? 1 : 0);
    usrb.DataLen=0;
    usrb.DataBuffer=NULL;
    usrb.DataOut=0;

    return SCSICmd(&usrb);
    }

enum CDROM_AUDIO_STATUS ReadCDROMAudioStatus(UNI_ADDR *addr)
    {
    UNI_SRB usrb;
    unsigned char buf[16];
    enum CDROM_AUDIO_STATUS status;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=10;
    usrb.CDB[0]=SCSI_SUBCHANNEL;
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5);
    usrb.CDB[3]=0x01; /* CD current position */
    usrb.CDB[7]=(BYTE)(sizeof(buf)>>8);
    usrb.CDB[8]=(BYTE)sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    if(SCSICmd(&usrb))
        status=buf[1];
    else
        status=AUDIO_STATUS_FAILED;

    return status;
    }

enum CDROM_PROFILE GetCDROMProfile(UNI_ADDR *addr)
    {
    UNI_SRB usrb;
    unsigned char buf[8];
    enum CDROM_PROFILE profile;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=10;
    usrb.CDB[0]=0x46; /* GET CONFIGURATION */
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5); /* All features */
    usrb.CDB[2]=0; /* Start from feature 0 */
    usrb.CDB[3]=0;
    usrb.CDB[7]=(BYTE)(sizeof(buf)>>8);
    usrb.CDB[8]=(BYTE)sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    if(SCSICmd(&usrb))
        profile=(buf[6]<<8)|buf[7];
    else
        profile=PROFILE_ERROR;

    return profile;
    }

BOOL SetDVDSpeed(UNI_ADDR *addr, unsigned speed, unsigned wspeed)
    {
    UNI_SRB usrb;
    unsigned char buf[28];
    BOOL ret;
    unsigned lba;

    lba=0xFFFFFFFFu;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=12;
    usrb.CDB[0]=0xB6; /* SET STREAMING */
    usrb.CDB[8]=0; /* Performance descriptor */
    usrb.CDB[9]=(BYTE)(sizeof(buf)>>8);
    usrb.CDB[10]=(BYTE)sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=1;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ZeroMemory(buf, sizeof(buf));
    buf[0]=0; /* CLV, no RDD, no EXACT, no RA */
#define INT32_TO_BUF(buf, i) \
    ( \
    ((unsigned char *)(buf))[0]=(unsigned char)((i)>>24), \
    ((unsigned char *)(buf))[1]=(unsigned char)((i)>>16), \
    ((unsigned char *)(buf))[2]=(unsigned char)((i)>>8), \
    ((unsigned char *)(buf))[3]=(unsigned char)(i) \
    )
    INT32_TO_BUF(buf+4, 0); /* Start LBA */
    INT32_TO_BUF(buf+8, lba); /* End LBA */
    INT32_TO_BUF(buf+12, speed); /* Read Size (kilobytes) */
    INT32_TO_BUF(buf+16, 1000); /* Read Time (milliseconds) */
    INT32_TO_BUF(buf+20, wspeed); /* Write Size (kilobytes) */
    INT32_TO_BUF(buf+24, 1000); /* Write Time (milliseconds) */

    ret=SCSICmd(&usrb);

    return ret;
    }

unsigned GetDVDSpeed(UNI_ADDR *addr)
    {
    UNI_SRB usrb;
    unsigned char buf[8+16*8];
    BOOL ret;
    unsigned len;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=12;
    usrb.CDB[0]=0xAC; /* GET PERFORMANCE */
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5)|0x10; /* Nominal Read Performance */
    usrb.CDB[8]=(BYTE)(((sizeof(buf)-8)/16)>>8); /* Max. Number of Descriptors */
    usrb.CDB[9]=(BYTE)((sizeof(buf)-8)/16);      /* -------------------------- */
    usrb.CDB[10]=0x00; /* 00h  Performance data */
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ret=SCSICmd(&usrb);

    if(ret)
        {
        len=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        if(20<len || 0!=(buf[4]&0x03)) /* Bad responce format */
            return 0;
        len=(len-4)/16*16+4;
        return (buf[len]<<24)|(buf[len+1]<<16)|(buf[len+2]<<8)|buf[len+3];
        }

    return 0;
    }

enum DVD_TYPE ReadDVDBookType(UNI_ADDR *addr)
    {
    UNI_SRB usrb;
    unsigned char buf[21];
    int book;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=12;
    usrb.CDB[0]=0xAD; /* READ DVD STRUCTURE */
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5);
    usrb.CDB[7]=0x00; /* Returns information in the DVD Lead-in area */
    usrb.CDB[8]=(BYTE)(sizeof(buf)>>8);
    usrb.CDB[9]=(BYTE)sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    if(SCSICmd(&usrb))
        book=buf[4]>>4;
    else
        book=-1;

    return book;
    }

BD_TYPE ReadBlurayDiscType(UNI_ADDR *addr)
    {
    UNI_SRB usrb;
    unsigned char buf[4100];
    int di_length;
    BD_TYPE bt;

    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=12;
    usrb.CDB[0]=0xAD; /* READ DVD STRUCTURE */
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5)|1; /* Blu-ray media type */
    usrb.CDB[7]=0x00; /* Return Disc Information */
    usrb.CDB[8]=(BYTE)(sizeof(buf)>>8);
    usrb.CDB[9]=(BYTE)sizeof(buf);
    usrb.DataLen=sizeof(buf);
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ZeroMemory(&bt, sizeof(bt));
    if(SCSICmd(&usrb))
        {
        di_length=((unsigned)(buf[0])<<8)+(unsigned)(buf[1]);
        if(di_length>12)
            memcpy(bt.type_str, buf+12, 3);
        }

    return bt;
    }

#define BUF_SIZE 65535
BOOL CheckCDROMFeature(UNI_ADDR *addr, unsigned feature)
    {
    UNI_SRB usrb;
    unsigned char *buf=NULL;
    BOOL ret;
    unsigned len;

    buf=malloc(BUF_SIZE);
    if(NULL==buf)
        return FALSE;
    ZeroMemory(&usrb, sizeof(usrb));
    usrb.Addr=*addr;
    usrb.CDBLen=10;
    usrb.CDB[0]=0x46; /* GET CONFIGURATION */
    usrb.CDB[1]=(BYTE)(usrb.Addr.Lun<<5)|2; /* Only one feature */
    usrb.CDB[2]=(BYTE)(feature>>8); /* Feature number */
    usrb.CDB[3]=(BYTE)(feature);
    usrb.CDB[7]=(BYTE)(BUF_SIZE>>8);
    usrb.CDB[8]=(BYTE)BUF_SIZE;
    usrb.DataLen=BUF_SIZE;
    usrb.DataBuffer=buf;
    usrb.DataOut=0;
    usrb.SenseLen=sizeof(usrb.SenseBuffer);

    ret=SCSICmd(&usrb);

    if(ret)
        {
        len=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        if(len+4<10)
            ret=FALSE;
        else 
            ret=((((unsigned)(buf[8])<<8)|(unsigned)(buf[8+1]))==feature);
        }

    free(buf);

    return ret;
    }

BOOL IsCDMedium(unsigned mt)
    {
    return ATAPI_CD_MIN<=mt && ATAPI_CD_MAX>=mt;
    }

BOOL IsCDROMMedium(unsigned mt)
    {
    return ATAPI_CDROM_MIN<=mt && ATAPI_CDROM_MAX>=mt;
    }

BOOL IsCDRMedium(unsigned mt)
    {
    return ATAPI_CDR_MIN<=mt && ATAPI_CDR_MAX>=mt;
    }

BOOL IsCDRWMedium(unsigned mt)
    {
    return ATAPI_CDRW_MIN<=mt && ATAPI_CDRW_MAX>=mt;
    }

BOOL IsEmptyMedium(unsigned mt)
    {
    return ATAPI_CLOSED_EMPTY==mt || ATAPI_OPEN==mt;
    }

BOOL IsMediumReady(unsigned mt)
    {
    return ATAPI_UNKNOWN!=mt
        && ATAPI_CLOSED_UNKNOWN!=mt
        && ATAPI_CLOSED_ERROR!=mt;
    }

BOOL IsCDAMedium(unsigned mt)
    {
    switch(mt)
        {
    case ATAPI_5INCH_AUDIO:
    case ATAPI_5INCH_COMBO:
    case ATAPI_5INCH_CDRAUDIO:
    case ATAPI_5INCH_CDRCOMBO:
    case ATAPI_5INCH_CDEAUDIO:
    case ATAPI_5INCH_CDECOMBO:
    case ATAPI_3INCH_AUDIO:
    case ATAPI_3INCH_COMBO:
    case ATAPI_3INCH_CDRAUDIO:
    case ATAPI_3INCH_CDRCOMBO:
    case ATAPI_3INCH_CDEAUDIO:
    case ATAPI_3INCH_CDECOMBO:
        return TRUE;
        }
    return FALSE;
    }

BOOL IsCDProfile(enum CDROM_PROFILE profile)
    {
    return PROFILE_CD_MIN<=profile && PROFILE_CD_MAX>=profile;
    }

BOOL IsDVDProfile(enum CDROM_PROFILE profile)
    {
    return PROFILE_DVD_MIN<=profile && PROFILE_DVD_MAX>=profile;
    }

BOOL IsBDProfile(enum CDROM_PROFILE profile)
    {
    return PROFILE_BD_MIN<=profile && PROFILE_BD_MAX>=profile;
    }

BOOL IsProfileReady(enum CDROM_PROFILE profile)
    {
    return PROFILE_ERROR!=profile && PROFILE_UNDEFINED!=profile;
    }

BOOL DriveHasTray(struct SCSI_CDInfo *CDI)
    {
    return 0x20==(CDI->IByte6&0xE0);
    }

BOOL DriveIsDVD(struct SCSI_CDInfo *CDI)
    {
    return 0!=(CDI->IByte2&0x08);
    }

BOOL IsAudioPlaying(enum CDROM_AUDIO_STATUS as)
    {
    return AUDIO_PLAY==as || AUDIO_PAUSE==as;
    }

