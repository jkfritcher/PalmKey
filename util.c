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
 *	$Id: util.c 57 2007-02-12 05:12:27Z jkf $
 */

#include <PalmOS.h>

#include "palmkey.h"


static const char *file = __FILE__;


void FatalErrorIf(int cond, int MsgResource, const char *fname, int line)
{
    if (cond)
    {
        DmOpenRef dbp;
        MemHandle msg;
        UInt16 idx;

        /* Open ourselves, find the string resource, and get a handle to it. */
        dbp = DmOpenDatabaseByTypeCreator('appl', APPID, dmModeReadOnly);
        idx = DmFindResource(dbp, 'tSTR', MsgResource, NULL);
        msg = DmGetResourceIndex(dbp, idx);

        /* Display error message. */
        ErrDisplayFileLineMsg(fname, line, MemHandleLock(msg));
    }
}

void NonFatalError(int AlertID, int MsgResource)
{
    DmOpenRef dbp;
    MemHandle msg;
    UInt16 idx;

    /* Open ourselves, find the string resource, and get a handle to it. */
    dbp = DmOpenDatabaseByTypeCreator('appl', APPID, dmModeReadOnly);
    idx = DmFindResource(dbp, 'tSTR', MsgResource, NULL);
    msg = DmGetResourceIndex(dbp, idx);

    /* Show error message. */
    FrmCustomAlert(AlertID, MemHandleLock(msg), " ", " ");

    /* Clean up after ourself. */
    MemHandleUnlock(msg);
    DmCloseDatabase(dbp);
}
