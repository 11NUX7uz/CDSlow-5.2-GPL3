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

#ifndef __UNI_SCSI__
#define __UNI_SCSI__

#define USRB_TIMEOUT 30

enum UNI_API
    {
    UAPI_NONE=0,
    UAPI_SPTI=1,
    UAPI_ASPI=2,
    UAPI_ALL=3,
    UAPI_DISABLE_ASPI=4
    };
enum ASPI_STATUS
    {
    ASPI_NOT_INITIALIZED=0,
    ASPI_NOT_PRESENT,
    ASPI_BROKEN,
    ASPI_OK
    };
enum NT_STATUS
    {
    SPTI_NOT_INITIALIZED=0,
    SPTI_WRONG_VERSION,
    SPTI_NO_DRIVES,
    SPTI_NO_ACCESS,
    SPTI_OK
    };
enum ATAPI_MEDIUM_TYPE
    {
    ATAPI_UNKNOWN=0x00,
    ATAPI_CD_MIN=0x01,
    ATAPI_CDROM_MIN=0x01,
    ATAPI_5INCH_DATA=0x01,
    ATAPI_5INCH_AUDIO=0x02,
    ATAPI_5INCH_COMBO=0x03,
    ATAPI_5INCH_HYBRID=0x04,
    ATAPI_3INCH_DATA=0x05,
    ATAPI_3INCH_AUDIO=0x06,
    ATAPI_3INCH_COMBO=0x07,
    ATAPI_3INCH_HYBRID=0x08,
    ATAPI_CDROM_MAX=0x08,
    ATAPI_CDR_MIN=0x10,
    ATAPI_CLOSED_CDR=0x10,
    ATAPI_5INCH_CDRDATA=0x11,
    ATAPI_5INCH_CDRAUDIO=0x12,
    ATAPI_5INCH_CDRCOMBO=0x13,
    ATAPI_5INCH_CDRHYBRID=0x14,
    ATAPI_3INCH_CDRDATA=0x15,
    ATAPI_3INCH_CDRAUDIO=0x16,
    ATAPI_3INCH_CDRCOMBO=0x17,
    ATAPI_3INCH_CDRHYBRID=0x18,
    ATAPI_CDR_MAX=0x18,
    ATAPI_CDRW_MIN=0x20,
    ATAPI_CLOSED_CDE=0x20,
    ATAPI_5INCH_CDEDATA=0x21,
    ATAPI_5INCH_CDEAUDIO=0x22,
    ATAPI_5INCH_CDECOMBO=0x23,
    ATAPI_5INCH_CDEHYBRID=0x24,
    ATAPI_3INCH_CDEDATA=0x25,
    ATAPI_3INCH_CDEAUDIO=0x26,
    ATAPI_3INCH_CDECOMBO=0x27,
    ATAPI_3INCH_CDEHYBRID=0x28,
    ATAPI_CDRW_MAX=0x28,
    ATAPI_CD_MAX=0x28,
    ATAPI_CLOSED_UNKNOWN=0x30,
    ATAPI_5INCH_HD=0x31,
    ATAPI_3INCH_HD=0x35,
    ATAPI_0x40=0x40,
    ATAPI_0x41=0x41,
    ATAPI_0x43=0x43,
    ATAPI_0x61=0x61,
    ATAPI_CLOSED_EMPTY=0x70,
    ATAPI_OPEN=0x71,
    ATAPI_CLOSED_ERROR=0x72
    };
enum CDROM_AUDIO_STATUS
    {
    AUDIO_STATUS_FAILED=-1,
    AUDIO_INVALID_STATUS=0x00,
    AUDIO_PLAY=0x11,
    AUDIO_PAUSE=0x12,
    AUDIO_COMPLETED=0x13,
    AUDIO_ERROR=0x14,
    AUDIO_NO_STATUS=0x15
    };
enum CDROM_PROFILE
    {
    PROFILE_ERROR=-1,
    PROFILE_UNDEFINED=0,
    PROFILE_FIXED=0x0001,
    PROFILE_REMOVABLE=0x0002,
    PROFILE_MO_E=0x0003,
    PROFILE_MO_WO=0x0004,
    PROFILE_AS_MO=0x0005,
    PROFILE_CD_MIN=0x0008,
    PROFILE_CDROM=0x0008,
    PROFILE_CDR=0x0009,
    PROFILE_CDRW=0x000A,
    PROFILE_CD_MAX=0x000A,
    PROFILE_DVD_MIN=0x0010,
    PROFILE_DVDROM=0x0010,
    PROFILE_DVDR=0x0011,
    PROFILE_DVDRAM=0x0012,
    PROFILE_DVDRWR=0x0013,
    PROFILE_DVDRWS=0x0014,
    PROFILE_DVDRDLS=0x0015,
    PROFILE_DVDRDLJ=0x0016,
    PROFILE_DVDRWDL=0x0017,
    PROFILE_DVDDOWN=0x0018,
    PROFILE_DVDPLUSRW=0x001A,
    PROFILE_DVDPLUSR=0x001B,
    PROFILE_DVDPLUSRDL=0x002B,
    PROFILE_DVD_MAX=0x002B,
    PROFILE_BD_MIN=0x0040,
    PROFILE_BDROM=0x0040,
    PROFILE_BDRS=0x0041,
    PROFILE_BDRR=0x0042,
    PROFILE_BDRE=0x0043,
    PROFILE_BD_MAX=0x0043,
    PROFILE_HDDVD_MIN=0x0050,
    PROFILE_HDDVDROM=0x0050,
    PROFILE_HDDVDR=0x0051,
    PROFILE_HDDVDRAM=0x0052,
    PROFILE_HDDVDRW=0x0053,
    PROFILE_HDDVDRDL=0x0058,
    PROFILE_HDDVDRWDL=0x005A,
    PROFILE_HDDVD_MAX=0x005A
    };
enum DVD_TYPE
    {
    DVD_TYPE_UNKNOWN=-1,
    DVD_TYPE_ROM=0x0,
    DVD_TYPE_RAM=0x1,
    DVD_TYPE_R=0x2,
    DVD_TYPE_RW=0x3,
    DVD_TYPE_PLUSRW=0x9,
    DVD_TYPE_PLUSR=0xA
    };
enum CDROM_FEATURE
    {
    FEATURE_PROFILE_LIST=0x0000,
    FEATURE_CORE=0x0001,
    FEATURE_MORPHING=0x0002,
    FEATURE_REMOVABLE=0x0003,
    FEATURE_WP=0x0004,
    FEATURE_RANDOM_READ=0x0010,
    FEATURE_MULTIREAD=0x001D,
    FEATURE_CD=0x001E,
    FEATURE_DVD=0x001F,
    FEATURE_RANDOM_WRITE=0x0020,
    FEATURE_STREAM_WRITE=0x0021,
    FEATURE_ERASABLE=0x0022,
    FEATURE_FORMATTABLE=0x0023,
    FEATURE_DEFECT=0x0024,
    FEATURE_WRITE_ONCE=0x0025,
    FEATURE_R_OVERWRITE=0x0026,
    FEATURE_CDRW_CAV=0x0027,
    FEATURE_MRW=0x0028,
    FEATURE_DEFECT_ENHANCED=0x0029,
    FEATURE_DVD_PLUSRW=0x002A,
    FEATURE_DVD_PLUSR=0x002B,
    FEATURE_RR_OVERWRITE=0x002C,
    FEATURE_CD_TAO=0x002D,
    FEATURE_CD_MASTER=0x002E,
    FEATURE_DVD_WRITE=0x002F,
    FEATURE_LAYER_JUMP=0x0033,
    FEATURE_STOP=0x0035,
    FEATURE_CD_RW=0x0037,
    FEATURE_BD_R_PSEUDO=0x0038,
    FEATURE_DVD_PLUSRW_DL=0x003A,
    FEATURE_DVD_PLUSR_DL=0x003B,
    FEATURE_BD=0x0040,
    FEATURE_BD_WRITE=0x0041,
    FEATURE_TSR=0x0042,
    FEATURE_HDDVD=0x0050,
    FEATURE_HDDVD_WRITE=0x0051,
    FEATURE_HDDVD_RW=0x0052,
    FEATURE_HYBRID=0x0080,
    FEATURE_POWER=0x0100,
    FEATURE_SMART=0x0101,
    FEATURE_CHANGER=0x0102,
    FEATURE_AUDIO=0x0103,
    FEATURE_UPGRADE=0x0104,
    FEATURE_TIMEOUT=0x0105,
    FEATURE_DVD_CSS=0x0106,
    FEATURE_RT_STREAMING=0x0107,
    FEATURE_USN=0x0108,
    FEATURE_MSN=0x0109,
    FEATURE_DCB=0x010A,
    FEATURE_DVD_CPRM=0x010B,
    FEATURE_FIRMWARE=0x010C,
    FEATURE_AACS=0x010D,
    FEATURE_DVD_CSS_R=0x010E,
    FEATURE_VCPS=0x0110,
    FEATURE_SECURDISC=0x0113
    };

typedef struct
    {
    char type_str[4];
    } BD_TYPE;
extern const BD_TYPE BD_TYPE_UNKNOWN;
extern const BD_TYPE BD_TYPE_ROM;
extern const BD_TYPE BD_TYPE_R;
extern const BD_TYPE BD_TYPE_RW;

typedef struct
    {
    unsigned Ha;
    unsigned Dev;
    unsigned Lun;
    unsigned Letter;
    } UNI_ADDR, *PUNI_ADDR;

typedef struct
    {
    int NTStatus;
    int ASPIStatus;
    int HAStatus;
    int DevStatus;  
    int SenseKey;
    } UNI_ERROR, *PUNI_ERROR;

typedef struct
    {
    UNI_ADDR Addr;
    unsigned CDBLen;
    BYTE CDB[16];
    unsigned SenseLen;
    BYTE SenseBuffer[16];
    unsigned DataLen;
    void *DataBuffer;
    unsigned DataOut;
    UNI_ERROR Err;
    } UNI_SRB, *PUNI_SRB;

struct SCSI_DevInfo
    {
    unsigned DType;
    unsigned RFormat;
    char Vendor[9];
    char Product[17];
    char Revision[5];
    };

struct SCSI_CDInfo
    {
    unsigned MaxSpeed;
    unsigned Speed;
    unsigned WriteSpeed;
    unsigned SCSI;
    unsigned NLevels;
    unsigned BufSize;
    unsigned char IByte2;
    unsigned char IByte3;
    unsigned char IByte4;
    unsigned char IByte5;
    unsigned char IByte6;
    unsigned char IByte7;
    unsigned char MediumType;
#ifndef NDEBUG
    unsigned char RawBuf[256];
#endif
    };

BOOL SCSICmd(UNI_SRB *srb);
enum UNI_API SCSIInit(enum UNI_API prefered);
BOOL IsSCSIDriverError(void);
enum UNI_API SCSIGetAPI(void);
BOOL GetFirstCDROM(UNI_ADDR *addr);
BOOL GetNextCDROM(UNI_ADDR *addr);
BOOL GetDeviceInfo(UNI_ADDR *addr, struct SCSI_DevInfo *DI);
BOOL GetCDROMInfo(UNI_ADDR *addr, struct SCSI_CDInfo *CDI);
unsigned GetCDROMSpeed(UNI_ADDR *addr);
BOOL SetCDROMSpeed(UNI_ADDR *addr, unsigned speed, unsigned wspeed);
BOOL SetCDROMEject(UNI_ADDR *addr, BOOL eject);
int TestUnitReady(UNI_ADDR *addr);
enum ASPI_STATUS ASPIGetStatus(void);
enum NT_STATUS NTGetStatus(void);
void SCSIGetLastError(UNI_ERROR *error);
void SCSIClearError(void);
unsigned ReadCDROMCapacity(UNI_ADDR *addr);
BOOL ReadCDROM(UNI_ADDR *addr, unsigned LBA, unsigned count, void *buf);
BOOL SetCDROMLock(UNI_ADDR *addr, BOOL lock);
enum CDROM_AUDIO_STATUS ReadCDROMAudioStatus(UNI_ADDR *addr);
enum CDROM_PROFILE GetCDROMProfile(UNI_ADDR *addr);
BOOL SetDVDSpeed(UNI_ADDR *addr, unsigned speed, unsigned wspeed);
unsigned GetDVDSpeed(UNI_ADDR *addr);
char *ASPIGetVersionInfo(char *param);
enum DVD_TYPE ReadDVDBookType(UNI_ADDR *addr);
BD_TYPE ReadBlurayDiscType(UNI_ADDR *addr);
BOOL CheckCDROMFeature(UNI_ADDR *addr, unsigned feature);

BOOL IsCDMedium(unsigned mt);
BOOL IsCDROMMedium(unsigned mt);
BOOL IsCDRMedium(unsigned mt);
BOOL IsCDRWMedium(unsigned mt);
BOOL IsCDAMedium(unsigned mt);
BOOL IsEmptyMedium(unsigned mt);
BOOL IsMediumReady(unsigned mt);
BOOL IsCDProfile(enum CDROM_PROFILE profile);
BOOL IsDVDProfile(enum CDROM_PROFILE profile);
BOOL IsBDProfile(enum CDROM_PROFILE profile);
BOOL IsProfileReady(enum CDROM_PROFILE profile);
BOOL DriveHasTray(struct SCSI_CDInfo *CDI);
BOOL DriveIsDVD(struct SCSI_CDInfo *CDI);
BOOL IsAudioPlaying(enum CDROM_AUDIO_STATUS as);

void LogLastSCSIError(char *tag);

BOOL ASPIInit(void);
BOOL ASPISCSICmd(UNI_SRB *usrb);
BOOL ASPIGetFirstCDROM(UNI_ADDR *addr);
BOOL ASPIGetNextCDROM(UNI_ADDR *addr);
void *ASPISCSIBatchBegin(UNI_SRB *usrb);
BOOL ASPISCSIBatchCmd(UNI_SRB *usrb, void *batch);
void ASPISCSIBatchEnd(void *batch);
int ASPIGetAdapterCount(void);
void ASPIDestroy(void);
char *ASPIGetVersionInfo(char *param);

BOOL NTInit(void);
BOOL NTSCSICmd(UNI_SRB *usrb);
BOOL NTGetFirstCDROM(UNI_ADDR *addr);
BOOL NTGetNextCDROM(UNI_ADDR *addr);
void *NTSCSIBatchBegin(UNI_SRB *usrb);
BOOL NTSCSIBatchCmd(UNI_SRB *usrb, void *batch);
void NTSCSIBatchEnd(void *batch);

#endif /* __UNI_SCSI__ */
