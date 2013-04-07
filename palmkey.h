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
 *	$Id: palmkey.h 56 2007-01-01 21:16:48Z jkf $
 */

#ifndef PALMKEY_H
#define PALMKEY_H

#ifndef EndianSwap16
#define EndianSwap16(n)	\
          ( (((n) & 0xFF00) >> 8) | \
            (((n) & 0x00FF) << 8) )
#endif
#ifndef EndianSwap32
#define EndianSwap32(n)	\
          ( (((n) & 0xFF000000) >> 24) | \
            (((n) & 0x00FF0000) >> 8)  | \
            (((n) & 0x0000FF00) << 8)  | \
            (((n) & 0x000000FF) << 24) )
#endif

#ifndef isalnum
#define isalnum(c)	\
          ( ((c) >= 'a' && (c) <= 'z') || \
            ((c) >= 'A' && (c) <= 'Z') || \
            ((c) >= '0' && (c) <= '9') )
#endif

/* AppID global definition */
#define APPID	'Pkey'

/* dict.c */
extern char *btoe(char *, char *);

/* util.c */
extern void FatalErrorIf(int cond, int MsgResource, const char *fname, int line);
extern void NonFatalError(int AlertID, int MsgResource);

#endif /* PALMKEY_H */
