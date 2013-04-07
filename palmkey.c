/*-
 * Copyright (c) 2000-2007 Jason K. Fritcher <jkf@wolfnet.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: palmkey.c 57 2007-02-12 05:12:27Z jkf $
 */

#include <PalmOS.h>
#include <Progress.h>

#include <sys/types.h>
#include "types.h"

#include "md4/md4.h"
#include "md5/md5.h"
#include "sha1/sha1.h"

#include "palmkey.h"
#include "palmkey_rcp.h"


struct PkeyPrefs
{
    UInt16 hash;
    UInt16 format;
    UInt16 count;
    Char seed[17];
};


static const char *file = __FILE__;

static UInt16 Hash = 0;
static UInt16 format = 0;
static UInt32 count;
static char dict = 0;
static char response[8];
static struct PkeyPrefs prefs;
static ProgressPtr pPrg;
static int prgCancelled;


static void DoAboutView(void);


static void *GetObjectPtr(FormPtr frm, UInt16 ObjID)
{
    return(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ObjID)));
}

static void sevenbit(char *s)
{
    char *p;

    /* Go through string one character at a time and strip the MSB off. */
    for(p = s; *p; p++)
        *p &= 0x7F;
}

char *put8(char *out, char *s)
{
    UInt16 i;
    Char p[3];

    /* Truncate string buffer. */
    out[0] = 0;

    /* Cycle through each byte of binary response. */
    for(i = 0; i < 8; i++)
    {
        /* Insert space, if needed. */
        if ((i > 0) && !(i % 2))
        {
            int len = StrLen(out);
            out[len] = ' ';
            out[len+1] = 0;
        }

        /* Convert high nibble of byte to hexadecimal. */
        p[0] = ((s[i] & 0xF0) >> 4) + '0';
        if (p[0] > '9')
            p[0] += 7;

        /* Convert low nibble of byte to hexadecimal. */
        p[1] =  (s[i] & 0x0F) + '0';
        if (p[1] > '9')
            p[1] += 7;

        /* Null terminate converted byte and copy it into the response string. */
        p[2] = 0;
        StrCat(out, p);
    }

    return(out);
}

static void do_md4(char *buf)
{
    EventType event;
    UInt32 results[4];
    MD4_CTX *md;
    int i;

    /* Allocate md4 context structure. */
    md = MemPtrNew(sizeof(MD4_CTX));
    FatalErrorIf(md == NULL, NullPtrMsg, file, __LINE__);

    /* Initialize context, add our string, and compute the hash. */
    MD4Init(md);
    MD4Update(md, buf, StrLen(buf));
    MD4Final((unsigned char *)results, md);

    /* Fold the 128 bit result into 64 bits. */
    results[0] ^= results[2];
    results[1] ^= results[3];

    /* Reset the cancelled variable. */
    prgCancelled = 0;

    /* Take previous hash and do everything again, count times. */
    while ((count > 0) && (prgCancelled == 0))
    {
        EvtGetEvent(&event, 0);
        if (event.eType == nilEvent)
        {
            for (i = 0; i < (count > 99 ? 100 : count); i++)
            {
                MD4Init(md);
                MD4Update(md, (unsigned char *)results, 8);
                MD4Final((unsigned char *)results, md);

                results[0] ^= results[2];
                results[1] ^= results[3];
            }
            count -= i;

            PrgUpdateDialog(pPrg, 0, count, NULL, false);
            PrgHandleEvent(pPrg, &event);
        }
        else
        {
            PrgHandleEvent(pPrg, &event);
            if(PrgUserCancel(pPrg))
                prgCancelled = 1;
        }
    }

    /* Free the md4 context. */
    MemPtrFree(md);

    /* Move the result into global storage. */
    if (!prgCancelled)
        MemMove(response, results, sizeof(response));
}

static void do_md5(char *buf)
{
    EventType event;
    UInt32 results[4];
    MD5_CTX *md;
    int i;

    /* Allocate md5 context structure. */
    md = MemPtrNew(sizeof(MD5_CTX));
    FatalErrorIf(md == NULL, NullPtrMsg, file, __LINE__);

    /* Initialize context, add our string, and compute the hash. */
    MD5Init(md);
    MD5Update(md, buf, StrLen(buf));
    MD5Final((unsigned char *)results, md);

    /* Fold the 128 bit result into 64 bits. */
    results[0] ^= results[2];
    results[1] ^= results[3];

    /* Reset the cancelled variable. */
    prgCancelled = 0;

    /* Take previous hash and do everything again, count times. */
    while ((count > 0) && (prgCancelled == 0))
    {
        EvtGetEvent(&event, 0);
        if (event.eType == nilEvent)
        {
            for (i = 0; i < (count > 99 ? 100 : count); i++)
            {
                MD5Init(md);
                MD5Update(md, (unsigned char *)results, 8);
                MD5Final((unsigned char *)results, md);

                results[0] ^= results[2];
                results[1] ^= results[3];
            }
            count -= i;

            PrgUpdateDialog(pPrg, 0, count, NULL, false);
            PrgHandleEvent(pPrg, &event);
        }
        else
        {
            PrgHandleEvent(pPrg, &event);
            if(PrgUserCancel(pPrg))
                prgCancelled = 1;
        }
    }

    /* Free the md5 context. */
    MemPtrFree(md);

    /* Move the result into global storage. */
    if (!prgCancelled)
        MemMove(response, results, sizeof(response));
}

static void do_sha1(char *buf)
{
    EventType event;
    UInt32 results[5];
    SHA1_CTX *sha;
    int i;

    /* Allocate sha1 context structure. */
    sha = MemPtrNew(sizeof(SHA1_CTX));
    FatalErrorIf(sha == NULL, NullPtrMsg, file, __LINE__);

    /* Initialize context, add our string, and compute the hash. */
    SHA1Init(sha);
    SHA1Update(sha, buf, StrLen(buf));
    SHA1Final((unsigned char *)results, sha);

    /* Fold the 160 bit result into 64 bits. */
    results[0] ^= results[2];
    results[1] ^= results[3];
    results[0] ^= results[4]; 

    /* Swap endianess */
    results[0] = EndianSwap32(results[0]);
    results[1] = EndianSwap32(results[1]);

    /* Reset the cancelled variable. */
    prgCancelled = 0;

    /* Take previous hash and do everything again, count times. */
    while((count > 0) && (prgCancelled == 0))
    {
        EvtGetEvent(&event, 0);
        if (event.eType == nilEvent)
        {
            for (i = 0; i < (count >= 100 ? 100 : count); i++)
            {
                SHA1Init(sha);
                SHA1Update(sha, (unsigned char *)results, 8);
                SHA1Final((unsigned char *)results, sha);

                /* Fold the 160 bit result into 64 bits. */
                results[0] ^= results[2];
                results[1] ^= results[3];
                results[0] ^= results[4];

                /* Swap endianness */
                results[0] = EndianSwap32(results[0]);
                results[1] = EndianSwap32(results[1]);
            }
            count -= i;

            PrgUpdateDialog(pPrg, 0, count, NULL, false);
            PrgHandleEvent(pPrg, &event);
        }
        else
        {
            PrgHandleEvent(pPrg, &event);
            if(PrgUserCancel(pPrg))
                prgCancelled = 1;
        }
    }

    /* Free the sha1 context. */
    MemPtrFree(sha);

    /* Move the result into global storage. */
    if (!prgCancelled)
        MemMove(response, results, sizeof(response));
}

static void MainViewShowResponse(void)
{
    FieldPtr fld;
    FormPtr frm;
    char buf[30];

    /* Get pointer to form. */
    frm = FrmGetFormPtr(MainView);
    FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);

    /* Get pointer to response field. */
    fld = GetObjectPtr(frm, ResponseField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);

    /* Build response. */
    switch(format)
    {
        case ResponseEngFormat:
            btoe(buf, response);
            break;
        case ResponseHexFormat:
            put8(buf, response);
            break;
    }

    /* Stuff the response into the field and have the field redraw itself. */
    FldSetTextPtr(fld, buf);
    FldDrawField(fld);
}

static Boolean CalcCallback(PrgCallbackDataPtr CBData)
{
    char buf[24];

    StrPrintF(buf, "Iterations left: %i", CBData->stage);
    StrCopy(CBData->textP, buf);

    return(true);
}

static void MainViewComputeResponse(void)
{
    FormPtr frm;
    FieldPtr fld;
    Char buf[80], *chr, *p;

    /* Truncate string buffer. */
    buf[0] = 0;

    /* Get pointer to the form. */
    frm = FrmGetFormPtr(MainView);
    FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);

    /* Get pointer to count field. */
    fld = GetObjectPtr(frm, CountField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);

    /* Check field for content, and process it if anything is there. */
    if (FldGetTextLength(fld) > 0)
    {
        count = StrAToI(FldGetTextPtr(fld));
        if (count < 0)
        {
            NonFatalError(InputAlert, NegativeCount);
            return;
        }
    }
    else
    {
        NonFatalError(InputAlert, BlankCount);
        return;
    }

    /* Get pointer to seed field. */
    fld = GetObjectPtr(frm, SeedField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);

    /* Is the field empty? */
    if (FldGetTextLength(fld) == 0)
    {
        NonFatalError(InputAlert, BlankSeed);
        return;
    }

    /* Is the field too large. */
    if (FldGetTextLength(fld) > 16)
    {
        NonFatalError(InputAlert, LargeSeed);
        return;
    }

    p = chr = FldGetTextPtr(fld);
    FatalErrorIf(chr == NULL, NullPtrMsg, file, __LINE__);

    /* Cycle through the field and check for invalid characters,
       displaying an error message if there are any. */
    while(*p)
    {
        if (!isalnum(*p))
        {
            NonFatalError(InputAlert, InvalidSeed);
            return;
        }
        p++;
    }

    /* Copy to our buffer and convert to lowercase. */
    StrToLower(buf, chr);

    /* Get pointer to passphrase field. */
    fld = GetObjectPtr(frm, PassphraseField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);

    /* Retrieve password field and check it for content. Display
       error message if things are not satisfactory. */
    if (FldGetTextLength(fld) < 10)
    {
        NonFatalError(InputAlert, ShortPassphrase1);
        return;
    }

    StrCat(buf, FldGetTextPtr(fld));

    /* Strip the MSB from all characters in the buffer. */
    sevenbit(buf);

    /* Throw up the progress dialog. */
    pPrg = PrgStartDialogV31("Calculating response...", CalcCallback);
    FatalErrorIf(pPrg == NULL, NullPtrMsg, file, __LINE__);

    /* Initialize the dialog. */
    PrgUpdateDialog(pPrg, 0, count, NULL, true);

    /* Determine the currently selected hash and do the work. */
    switch(Hash)
    {
        case 0:
            do_md4(buf);
            break;
        case 1:
            do_md5(buf);
            break;
        case 2:
            do_sha1(buf);
            break;
    }

    /* Remove the progress dialog. */
    PrgStopDialog(pPrg, true);

    /* Clear buffer contents. */
    MemSet(buf, sizeof(buf), 0);

    /* Display the results. */
    if (!prgCancelled)
        MainViewShowResponse();
}

static void MainViewSaveForm(void)
{
    FormPtr frm;
    FieldPtr fld;
    Char *txt;

    /* Get pointer to the main form. */
    frm = FrmGetFormPtr(MainView);
    FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);

    /* Save the current hash and format selections. */
    prefs.hash = Hash;
    prefs.format = format;

    /* Get pointer to count field. */
    fld = GetObjectPtr(frm, CountField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);

    /* Get pointer to field's text and save it. */
    txt = FldGetTextPtr(fld);
    if (txt == NULL)
        prefs.count = 0;
    else
        prefs.count = StrAToI(txt);

    /* Get pointer to seed field. */
    fld = GetObjectPtr(frm, SeedField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);

    /* Get pointer to field's text and save it. */
    txt = FldGetTextPtr(fld);
    if (txt == NULL)
        prefs.seed[0] = 0;
    else
        StrCopy(prefs.seed, txt);
}

static void MainViewClearFields(void)
{
    FormPtr frm;
    FieldPtr fld;

    /* Get pointer to currently active form. */
    frm = FrmGetFormPtr(MainView);
    FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);

    /* Get pointer to passphrase field and clear it's text. */
    fld = GetObjectPtr(frm, PassphraseField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);
    if (FldGetTextLength(fld) > 0)
    {
        FldDelete(fld, 0, FldGetTextLength(fld));
        return;
    }

    /* Get pointer to count field and clear it's text. */
    fld = GetObjectPtr(frm, CountField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);
    FldDelete(fld, 0, FldGetTextLength(fld));

    /* Get pointer to seed field and clear it's text. */
    fld = GetObjectPtr(frm, SeedField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);
    FldDelete(fld, 0, FldGetTextLength(fld));

    /* Get pointer to response field and clear it's text. */
    fld = GetObjectPtr(frm, ResponseField);
    FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);
    FldSetTextPtr(fld, "");
    FldDrawField(fld);

    /* Remove any current focus. */
    FrmSetFocus(frm, noFocus);
}

static void MainViewInit(FormPtr frm)
{
    ListPtr lst;
    ControlPtr ctl;
    Char *label;
    FieldPtr fld;
    MemHandle tmp;

    /* Initialize list to select an item. */
    lst = GetObjectPtr(frm, HashList);
    FatalErrorIf(lst == NULL, NullPtrMsg, file, __LINE__);
    LstSetSelection(lst, Hash);

    /* Make popup trigger's label match selection made above. */
    ctl = GetObjectPtr(frm, HashListTrigger);
    FatalErrorIf(ctl == NULL, NullPtrMsg, file, __LINE__);
    label = LstGetSelectionText(lst, Hash);
    FatalErrorIf(label == NULL, NullPtrMsg, file, __LINE__);
    CtlSetLabel(ctl, label);

    /* Check format type and set default if the current one is invalid. */
    if ((format != ResponseEngFormat) && (format != ResponseHexFormat))
        format = (dict ? ResponseEngFormat : ResponseHexFormat);
    ctl = GetObjectPtr(frm, format);
    FatalErrorIf(ctl == NULL, NullPtrMsg, file, __LINE__);
    CtlSetValue(ctl, 1);

    /* Is count greater than zero? If so, stuff it into the count field. */
    if (prefs.count > 0)
    {
        Char *txt;
        tmp = MemHandleNew(5);
        FatalErrorIf(tmp == NULL, NullHandleMsg, file, __LINE__);
        txt = MemHandleLock(tmp);
        FatalErrorIf(txt == NULL, NullPtrMsg, file, __LINE__);
        StrIToA(txt, prefs.count);
        MemHandleUnlock(tmp);
        fld = GetObjectPtr(frm, CountField);
        FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);
        FldSetTextHandle(fld, (MemHandle)tmp);
    }

    /* Does seed contain something? If so, stuff it into the seed field. */
    if (prefs.seed[0] != 0)
    {
        Char *txt;
        tmp = MemHandleNew(StrLen(prefs.seed) + 1);
        FatalErrorIf(tmp == NULL, NullHandleMsg, file, __LINE__);
        txt = MemHandleLock(tmp);
        FatalErrorIf(txt == NULL, NullPtrMsg, file, __LINE__);
        StrCopy(txt, prefs.seed);
        MemHandleUnlock(tmp);
        fld = GetObjectPtr(frm, SeedField);
        FatalErrorIf(fld == NULL, NullPtrMsg, file, __LINE__);
        FldSetTextHandle(fld, (MemHandle)tmp);
    }

    /* Remove the english button if the dictionary is not present. */
    if (dict == 0)
    {
        ctl = GetObjectPtr(frm, ResponseEngFormat);
        CtlHideControl(ctl);
    }
}

static Boolean MainViewHandleEvent(EventPtr event)
{
    Boolean handled = false;
    FormPtr frm;

    /* Check out eType to see what kind of event this is. */
    switch(event->eType)
    {
        case ctlSelectEvent:
            /* See which control was tapped. */
            switch (event->data.ctlSelect.controlID)
            {
                case ComputeButton:
                    MainViewComputeResponse();
                    handled = true;
                    break;
                case ClearButton:
                    MainViewClearFields();
                    handled = true;
                    break;
                case ResponseEngFormat:
                    format = ResponseEngFormat;
                    MainViewShowResponse();
                    handled = false;
                    break;
                case ResponseHexFormat:
                    format = ResponseHexFormat;
                    MainViewShowResponse();
                    handled = false;
                    break;
                default:
            }
            break;
        case popSelectEvent:
            /* See which popup item was selected. */
            switch(event->data.popSelect.controlID)
            {
                case HashListTrigger:
                    Hash = event->data.popSelect.selection;
                    break;
                default:
            }
            break;
        case menuEvent:
            /* See which menu the user wants. */
            switch(event->data.menu.itemID)
            {
                case AboutMenuID:
                    DoAboutView();
                    handled = true;
                    break;
                default:
            }
            break;
        case frmUpdateEvent:
            frm = FrmGetFormPtr(event->data.frmUpdate.formID);
            FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);
            FrmDrawForm(frm);
            handled = true;
            break;
        case frmSaveEvent:
            MainViewSaveForm();
            handled = true;
            break;
        case frmOpenEvent:
            frm = FrmGetFormPtr(event->data.frmOpen.formID);
            FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);
            MainViewInit(frm);
            FrmSetActiveForm(frm);
            FrmDrawForm(frm);
            handled = true;
            break;
        default:
    }

    return(handled);
}

static void DoAboutView(void)
{
    FormPtr frm, oldfrm;

    /* Get pointer to the currently active form. */
    oldfrm = FrmGetActiveForm();
    FatalErrorIf(oldfrm == NULL, NullPtrMsg, file, __LINE__);

    /* Get pointer to About form. */
    frm = FrmInitForm(AboutView);
    FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);

    /* Make About form active and wait for it to finish. */
    FrmSetActiveForm(frm);
    FrmDoDialog(frm);

    /* Restore the previously active form. */
    FrmSetActiveForm(oldfrm);

    /* Delete About form. */
    FrmDeleteForm(frm);
}

static Boolean ApplicationHandleEvent(EventPtr event)
{
    FormPtr frm;
    Boolean handled = false;

    /* If this is a load form event, load the requested form,
       initialize it, make it active, and set its handler. */
    if (event->eType == frmLoadEvent)
    {
        frm = FrmInitForm(event->data.frmLoad.formID);
        FatalErrorIf(frm == NULL, NullPtrMsg, file, __LINE__);

        switch(event->data.frmLoad.formID)
        {
            case MainView:
                FrmSetEventHandler(frm, MainViewHandleEvent);
                break;
        }

        handled = true;
    }

    return(handled);
}

static void EventLoop(void)
{
    EventType event;
    UInt16      error;

    /* Boiler plate event loop. */
    do
    {
        EvtGetEvent(&event, evtWaitForever);
        if (!SysHandleEvent(&event))
            if (!MenuHandleEvent(NULL, &event, &error))
                if (!ApplicationHandleEvent(&event))
                    FrmDispatchEvent(&event);
    }
    while (event.eType != appStopEvent);
}

static void CheckForDict(void)
{
    DmOpenRef dbp;

    dbp = DmOpenDatabaseByTypeCreator('Dict', APPID, dmModeReadOnly);
    if (dbp != 0)
    {
        dict = 1;
        DmCloseDatabase(dbp);
    }
    else
    {
        dict = 0;
    }
}

static void LoadPreferences(void)
{
    UInt16 prefsize = sizeof(prefs);
    Err err;

    /* Load our preferences from the preferences database. */
    err = PrefGetAppPreferences(APPID, 0, &prefs, &prefsize, false);
    if ((err == noPreferenceFound))
    {
        Hash = 1;
        format = (dict ? ResponseEngFormat : ResponseHexFormat);
    }
    else
    {
        Hash = prefs.hash;
        format = (dict ? prefs.format : ResponseHexFormat);
    }
}

static void SavePreferences(void)
{
    /* Save our preferences into the preferences database. */
    PrefSetAppPreferences(APPID, 0, 0, &prefs, sizeof(prefs), false);
}

static Err StartApplication(void)
{
    /* Check for the dictionary. */
    CheckForDict();

    /* Load our preferences. */
    LoadPreferences();

    /* Initiate main form loading. */
    FrmGotoForm(MainView);

    return(0);
}

static void StopApplication(void)
{
    /* Have all active forms save their data and then close. */
    FrmSaveAllForms();
    FrmCloseAllForms();

    /* Save our preferences. */
    SavePreferences();
}

static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
    UInt32	romVersion;

    /* Boiler plate rom version checking. */
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
    if (romVersion < requiredVersion)
    {
        if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) == 
            (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
        {
            FrmAlert(RomAlert);

            if (romVersion < 0x02000000)
            {
                AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
            }
        }

        return(sysErrRomIncompatible);
    }

    return(0);
}

UInt32 PilotMain(UInt16 launchCode, MemPtr cmdPBP, UInt16 launchFlags)
{
    Err err;

    err = RomVersionCompatible(0x03003000, launchFlags);
    if (err)
        return(err);

    /* Only respond to the normal startup launch code. */
    if (launchCode == sysAppLaunchCmdNormalLaunch)
    {
        if ((err = StartApplication()) == 0)
        {
            EventLoop();
            StopApplication();
        }
    }

    return(err);
}
