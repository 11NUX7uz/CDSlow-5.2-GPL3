/* ===================================================================
 * Copyright (c) 2005-2019 Vadim Druzhin (cdslow@mail.ru).
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
#include "uniscsi.h"
#include "scsidefs.h"
#include "wnaspi32.h"
#include "log.h"

#ifndef SS_SECURITY_VIOLATION
#define SS_SECURITY_VIOLATION SS_ILLEGAL_MODE
#endif /* SS_SECURITY_VIOLATION */

void LogLastSCSIError(char *tag)
    {
    UNI_ERROR err;
    char *s;
    char buf[512];

    if(NULL!=tag)
        DebugLog(("%s\n", tag));

    DebugLog(("*** SCSI Error Info ***\n"));

    SCSIGetLastError(&err);

    if(err.NTStatus!=-1)
        {
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err.NTStatus,
            0,
            buf,
            sizeof(buf),
            NULL
            );
        buf[strlen(buf)-2]=0;
        s=buf;
        DebugLog(("SPTI: %s (0x%02X)\n", s, err.NTStatus));
        }

    if(err.ASPIStatus!=-1)
        {
        switch(err.ASPIStatus)
            {
        case SS_PENDING:       s="SRB being processed"; break;
        case SS_COMP:          s="SRB completed without error"; break;
        case SS_ABORTED:       s="SRB aborted"; break;
        case SS_ABORT_FAIL:    s="Unable to abort SRB"; break;
        case SS_ERR:           s="SRB completed with error"; break;
        case SS_INVALID_CMD:   s="Invalid ASPI command"; break;
        case SS_INVALID_HA:    s="Invalid host adapter number"; break;
        case SS_NO_DEVICE:     s="SCSI device not installed"; break;
        case SS_INVALID_SRB:   s="Invalid parameter set in SRB"; break;
        case SS_BUFFER_ALIGN:  s="Buffer alignment problem"; break;
        case SS_SECURITY_VIOLATION: s="Device access security violation"; break;
        case SS_FAILED_INIT:   s="ASPI for windows failed init"; break;
        case SS_BUFFER_TO_BIG: s="Buffer size too big to handle!"; break;
        default: s="UNKNOWN ERROR CODE";
            }
        DebugLog(("ASPI: %s (0x%02X)\n", s, err.ASPIStatus));
        }

    if(err.HAStatus!=-1)
        {
        switch(err.HAStatus)
            {
        case HASTAT_OK:              s="Host adapter did not detect an error"; break;
        case HASTAT_SEL_TO:          s="Selection Timeout"; break;
        case HASTAT_DO_DU:           s="Data overrun data underrun"; break;
        case HASTAT_BUS_FREE:        s="Unexpected bus free"; break;
        case HASTAT_PHASE_ERR:       s="Target bus phase sequence failure"; break;
        case HASTAT_TIMEOUT:         s="Timed out while SRB was waiting to beprocessed."; break;
        case HASTAT_COMMAND_TIMEOUT: s="While processing the SRB, the adapter timed out."; break;
        case HASTAT_MESSAGE_REJECT:  s="While processing SRB, the adapter received a MESSAGE REJECT."; break;
        case HASTAT_BUS_RESET:       s="A bus reset was detected."; break;
        case HASTAT_PARITY_ERROR:    s="A parity error was detected."; break;
        case HASTAT_REQUEST_SENSE_FAILED: s="The adapter failed in issuing REQUEST SENSE."; break;
        default: s="UNKNOWN ERROR CODE";
            }
        DebugLog(("HA: %s (0x%02X)\n", s, err.HAStatus));
        }

    if(err.DevStatus!=-1)
        {
        switch(err.DevStatus)
            {
        case STATUS_GOOD:     s="Status Good"; break;
        case STATUS_CHKCOND:  s="Check Condition"; break;
        case STATUS_CONDMET:  s="Condition Met"; break;
        case STATUS_BUSY:     s="Busy"; break;
        case STATUS_INTERM:   s="Intermediate"; break;
        case STATUS_INTCDMET: s="Intermediate-condition met"; break;
        case STATUS_RESCONF:  s="Reservation conflict"; break;
        case STATUS_COMTERM:  s="Command Terminated"; break;
        case STATUS_QFULL:    s="Queue full"; break;
        default: s="UNKNOWN ERROR CODE";
            }
        DebugLog(("Dev: %s (0x%02X)\n", s, err.DevStatus));
        }

    if(err.SenseKey!=-1)
        {
        switch(err.SenseKey)
            {
        case KEY_NOSENSE:   s="No Sense"; break;
        case KEY_RECERROR:  s="Recovered Error"; break;
        case KEY_NOTREADY:  s="Not Ready"; break;
        case KEY_MEDIUMERR: s="Medium Error"; break;
        case KEY_HARDERROR: s="Hardware Error"; break;
        case KEY_ILLGLREQ:  s="Illegal Request"; break;
        case KEY_UNITATT:   s="Unit Attention"; break;
        case KEY_DATAPROT:  s="Data Protect"; break;
        case KEY_BLANKCHK:  s="Blank Check"; break;
        case KEY_VENDSPEC:  s="Vendor Specific"; break;
        case KEY_COPYABORT: s="Copy Abort"; break;
        case KEY_EQUAL:     s="Equal (Search)"; break;
        case KEY_VOLOVRFLW: s="Volume Overflow"; break;
        case KEY_MISCOMP:   s="Miscompare (Search)"; break;
        case KEY_RESERVED:  s="Reserved"; break;
        default: s="UNKNOWN SENSE KEY";
            }
        DebugLog(("Sense: %s (0x%02X)\n", s, err.SenseKey));
        }

    DebugLog(("***********************\n"));
    }
