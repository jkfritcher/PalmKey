/*
 * S/Key dictionary extraction and lookup routines.
 *
 * Authors:
 *          Neil M. Haller <nmh@thumper.bellcore.com>
 *          Philip R. Karn <karn@chicago.qualcomm.com>
 *          John S. Walden <jsw@thumper.bellcore.com>
 *          Scott Chasin <chasin@crimelab.com>
 *
 *	$Id: dict.c 57 2007-02-12 05:12:27Z jkf $
 */

#include <PalmOS.h>

#include "palmkey.h"
#include "palmkey_rcp.h"


static const char *file = __FILE__;


/* Extract 'length' bits from the char array 's'
 * starting with bit 'start'
 */
static unsigned long extract(char *s, int start, int length)
{
    unsigned char cl, cc, cr;
    unsigned long x;

    /* Some black magic I haven't bothered taking the time to understand. */
    cl = s[start / 8];
    cc = s[start / 8 + 1];
    cr = s[start / 8 + 2];
    x = ((long)(cl << 8 | cc) << 8  | cr);
    x = x >> (24 - (length + (start % 8)));
    x = (x & (0xffff >> (16-length)));

    return(x);
}

/* Encode 8 bytes in 'c' as a string of English words.
 * Returns a pointer to a static buffer
 */
char *btoe(char *engout, char *c)
{
    char *dict, *tmp;
    DmOpenRef dbp;
    MemHandle dictH;
    int p, i;
    char cp[9];

    /* Open dictionary. */
    dbp = DmOpenDatabaseByTypeCreator('Dict', APPID, dmModeReadOnly);
    FatalErrorIf(dbp == 0, NullPtrMsg, file, __LINE__);

    /* Get handle to the dictionary. */
    dictH = DmGetRecord(dbp, 0);
    FatalErrorIf(dictH == NULL, NullHandleMsg, file, __LINE__);

    /* Truncate string buffer. */
    engout[0] = '\0';

    /* Make copy of buffer for conversion. */
    MemMove(cp, c, 8);

    /* compute parity */
    for(p = 0, i = 0; i < 64; i += 2)
        p += extract(cp, i, 2);
    cp[8] = (char)p << 6;

    /* Lock dictionary handle and get a pointer to it. */
    dict = MemHandleLock(dictH);

    /* Convert binary response to english. */
    tmp = engout;
    StrNCat(tmp, &dict[(extract(cp,  0, 11) << 2)], 5);
    StrCat(tmp, " ");
    tmp = engout + StrLen(engout);
    StrNCat(tmp, &dict[(extract(cp, 11, 11) << 2)], 5);
    StrCat(tmp, " ");
    tmp = engout + StrLen(engout);
    StrNCat(tmp, &dict[(extract(cp, 22, 11) << 2)], 5);
    StrCat(tmp, " ");
    tmp = engout + StrLen(engout);
    StrNCat(tmp, &dict[(extract(cp, 33, 11) << 2)], 5);
    StrCat(tmp, " ");
    tmp = engout + StrLen(engout);
    StrNCat(tmp, &dict[(extract(cp, 44, 11) << 2)], 5);
    StrCat(tmp, " ");
    tmp = engout + StrLen(engout);
    StrNCat(tmp, &dict[(extract(cp, 55, 11) << 2)], 5);

    /* Unlock dictionary handle. */
    MemHandleUnlock(dictH);

    /* Release dictionary handle. */
    DmReleaseRecord(dbp, 0, false);

    /* Close dictionary. */
    DmCloseDatabase(dbp);

    /* Return results. */
    return(engout);
}
