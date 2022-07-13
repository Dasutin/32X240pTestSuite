/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin
 * Copyright (C)2011-2022 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __STRING_H__
#define __STRING_H__

#define isdigit(c)      ((c) >= '0' && (c) <= '9')

#ifndef SEEK_SET
#define	SEEK_SET	0	      // Set file offset to offset
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	      // Set file offset to current plus offset
#endif
#ifndef SEEK_END
#define	SEEK_END	2	      // Set file offset to EOF plus offset
#endif

typedef void *__gnuc_va_list;
typedef __gnuc_va_list va_list;

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG)                                           \
 (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))

#define va_end(AP)      ((void)0)

#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP)                                    \
                       - ((sizeof (TYPE) < __va_rounded_size (char)     \
                           ? sizeof (TYPE) : __va_rounded_size (TYPE))))))

size_t strlen(const char *str);
size_t strnlen(const char *str, size_t maxlen);
char* strcpy(char *dest, const char *src);
char* strcat(char *dest, const char *src);
//size_t vsprintf(char *buf, const char *fmt, va_list args);
//size_t sprintf(char *buffer,const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
u16 intToStr(s32 value, char *str, u16 minsize);
u32 intToHex(u32 value, char *str, u16 minsize);
u16 uintToStr(u32 value, char *str, u16 minsize);
void fix32ToStr(fix32 value, char *str, u16 numdec);

#endif