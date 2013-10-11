/* Copyright (c) 2013, Vsevolod Stakhov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RCL_CHARTABLE_H_
#define RCL_CHARTABLE_H_

#include "rcl_internal.h"

static const unsigned char rcl_chartable[255] = {
RCL_CHARACTER_VALUE_END, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_WHITESPACE, RCL_CHARACTER_WHITESPACE|RCL_CHARACTER_VALUE_END,
RCL_CHARACTER_WHITESPACE, RCL_CHARACTER_WHITESPACE,
RCL_CHARACTER_WHITESPACE|RCL_CHARACTER_VALUE_END, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_DENIED, RCL_CHARACTER_DENIED,
RCL_CHARACTER_WHITESPACE|RCL_CHARACTER_VALUE_STR /*   */,
RCL_CHARACTER_VALUE_STR /* ! */, RCL_CHARACTER_VALUE_STR /* " */,
RCL_CHARACTER_VALUE_END /* # */, RCL_CHARACTER_VALUE_STR /* $ */,
RCL_CHARACTER_VALUE_STR /* % */, RCL_CHARACTER_VALUE_STR /* & */,
RCL_CHARACTER_VALUE_STR /* ' */, RCL_CHARACTER_VALUE_STR /* ( */,
RCL_CHARACTER_VALUE_STR /* ) */, RCL_CHARACTER_VALUE_STR /* * */,
RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* + */,
RCL_CHARACTER_VALUE_END /* , */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* - */,
RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* . */,
RCL_CHARACTER_VALUE_STR /* / */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 0 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 1 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 2 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 3 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 4 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 5 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 6 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 7 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 8 */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT_START|RCL_CHARACTER_VALUE_DIGIT /* 9 */,
RCL_CHARACTER_VALUE_STR /* : */, RCL_CHARACTER_VALUE_END /* ; */,
RCL_CHARACTER_VALUE_STR /* < */, RCL_CHARACTER_VALUE_STR /* = */,
RCL_CHARACTER_VALUE_STR /* > */, RCL_CHARACTER_VALUE_STR /* ? */,
RCL_CHARACTER_VALUE_STR /* @ */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* A */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* B */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* C */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* D */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* E */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* F */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* G */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* H */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* I */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* J */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* K */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* L */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* M */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* N */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* O */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* P */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* Q */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* R */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* S */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* T */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* U */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* V */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* W */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* X */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* Y */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* Z */,
RCL_CHARACTER_VALUE_STR /* [ */, RCL_CHARACTER_VALUE_STR /* \ */,
RCL_CHARACTER_VALUE_END /* ] */, RCL_CHARACTER_VALUE_STR /* ^ */,
RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR /* _ */,
RCL_CHARACTER_VALUE_STR /* ` */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* a */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* b */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* c */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* d */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* e */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* f */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* g */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* h */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* i */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* j */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* k */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* l */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* m */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* n */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* o */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* p */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* q */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* r */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* s */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* t */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* u */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* v */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* w */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* x */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* y */,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR|RCL_CHARACTER_VALUE_DIGIT /* z */,
RCL_CHARACTER_VALUE_STR /* { */, RCL_CHARACTER_VALUE_STR /* | */,
RCL_CHARACTER_VALUE_END /* } */, RCL_CHARACTER_VALUE_STR /* ~ */,
RCL_CHARACTER_DENIED,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR,
RCL_CHARACTER_KEY_START|RCL_CHARACTER_KEY|RCL_CHARACTER_VALUE_STR
};



#endif /* RCL_CHARTABLE_H_ */
