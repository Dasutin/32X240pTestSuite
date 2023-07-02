/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2023 Artemio Urbina
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

#include <stddef.h>
#include <stdarg.h>
#include "types.h"
#include "string.h"
#include "shared_objects.h"

#define P01 10
#define P02 100
#define P03 1000
#define P04 10000
#define P05 100000
#define P06 1000000
#define P07 10000000
#define P08 100000000
#define P09 1000000000
#define P10 10000000000

//static const char hexchars[] = "0123456789ABCDEF";

static const char digits[] =
	"0001020304050607080910111213141516171819"
	"2021222324252627282930313233343536373839"
	"4041424344454647484950515253545556575859"
	"6061626364656667686970717273747576777879"
	"8081828384858687888990919293949596979899";

static u16 digits10(const u16 v);
static u16 uint16ToStr(u16 value, char *str, u16 minsize);

size_t strlen(const char *str)
{
	const char *src = str;
	while (*src++);
	return (src - str) - 1;
}

char* strcpy(char *to, const char *from)
{
	const char *src;
	char *dst;

	src = from;
	dst = to;
	while ((*dst++ = *src++));

	return to;
}

char* strcat(char *to, const char *from)
{
	const char *src;
	char *dst;

	src = from;
	dst = to;
	while (*dst++);

	--dst;
	while ((*dst++ = *src++));

	return to;
}

void* memcpy(volatile void *dest, const void *src, size_t len)
{
	char *d = dest;
	const char *s = src;
	while (len--)
	*d++ = *s++;
	return dest;
}

void memset(void* str, char ch, size_t n)
{
	int i;
	//type cast the str from void* to char*
	char *s = (char*) str;
	//fill "n" elements/blocks with ch
	for (i=0; i<n; i++)
		s[i]=ch;
}

/*

size_t strnlen(const char *str, size_t maxlen) {
	const char *src;
	for (src = str; maxlen-- && *src != '\0'; ++src);
	return src - str;
}

static size_t skip_atoi(const char **s) {
	size_t i = 0;
	while (isdigit(**s)) {
		i = (i * 10) + *((*s)++) - '0';
	}
	return i;
} */

/* size_t vsprintf(char *buf, const char *fmt, va_list args) {
	char tmp_buffer[12];
	char *str;
	for (str = buf; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
		int field_width = -1;
		int precision = -1;
		size_t longint = 0;
		size_t zero_pad = 0;
		size_t left_align = 0;
		char sign = 0;
		char *s;
		
		// Process the flags
		for (;;) {
			++fmt;          // this also skips first '%'
			switch (*fmt) {
				case '-':
					left_align = 1;
					continue;
				case '+':
					sign = '+';
					continue;
				case ' ':
					if (sign != '+') sign = ' ';
					continue;
				case '0':
					zero_pad = 1;
					continue;
			}
			break;
		}
		
		// Process field width and precision
		field_width = precision = -1;
		if (isdigit(*fmt)) {
			field_width = skip_atoi(&fmt);
		} else if (*fmt == '*') {
			++fmt;
			// it's the next argument
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				left_align = 1;
			}
		}
		
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt)) {
				precision = skip_atoi(&fmt);
			} else if (*fmt == '*') {
				++fmt;
				// it's the next argument
				precision = va_arg(args, int);
			}
			if (precision < 0) {
				precision = 0;
			}
		}
		
		if (*fmt == 'h') ++fmt;
		if ((*fmt == 'l') || (*fmt == 'L')) {
			longint = 1;
			++fmt;
		}
		if (left_align) {
			zero_pad = 0;
		}
		switch (*fmt) {
			case 'c': {
				if (!left_align) {
					while (--field_width > 0) {
						*str++ = ' ';
					}
				}
				*str++ = (char) va_arg(args, int);
				while (--field_width > 0) {
					*str++ = ' ';
				}
				continue;
			}
			case 's': {
				s = va_arg(args, char*);
				if (!s) {
					s = "<NULL>";
				}
				size_t len = strnlen(s, precision);
				if (!left_align) {
					while (len < field_width--) {
						*str++ = ' ';
					}
				}
				for (size_t i = 0; i < len; ++i) {
					*str++ = *s++;
				}
				while (len < field_width--) {
					*str++ = ' ';
				}
				continue;
			}
			
			case 'p':
				longint = 1;
				if (field_width == -1) {
					field_width = 2 * sizeof(void *);
					zero_pad = 1;
				} // fallthrough
			case 'x':
			case 'X': {
				s = &tmp_buffer[12];
				*--s = 0;
				u32 num = longint ? va_arg(args, u32) : va_arg(args, u16);
				if (!num) {
					*--s = '0';
				}
				while (num) {
					*--s = hexchars[num & 0xF];
					num >>= 4;
				}
				sign = 0;
				break;
			}
			case 'n': {
				int *ip = va_arg(args, int * );
				*ip = (str - buf);
				continue;
			}
			case 'u': {
				s = &tmp_buffer[12];
				*--s = 0;
				u32 num = longint ? va_arg(args, u32) : va_arg(args, u16);
				if (!num) {
					*--s = '0';
				}
				while (num) {
					*--s = (num % 10) + 0x30;
					num /= 10;
				}
				sign = 0;
				break;
			}
			case 'd':
			case 'i': {
				s = &tmp_buffer[12];
				*--s = 0;
				int num = longint ? va_arg(args, int) : va_arg(args, s16);
				if (!num) {
					*--s = '0';
				}
				if (num < 0) {
					sign = '-';
					while (num) {
						*--s = 0x30 - (num % 10);
						num /= 10;
					}
				} else {
					while (num) {
						*--s = (num % 10) + 0x30;
						num /= 10;
					}
				}
				break;
			}
			default: continue;
		}
		
		size_t len = strnlen(s, precision);
		if (sign) {
			*str++ = sign;
			field_width--;
		}
		if (!left_align) {
			if (zero_pad) {
				while (len < field_width--)
					*str++ = '0';
			} else {
				while (len < field_width--)
					*str++ = ' ';
			}
		}
		for (size_t i = 0; i < len; ++i) {
			*str++ = *s++;
		}
		while (len < field_width--) {
			*str++ = ' ';
		}
	}
	
	*str = '\0';
	return str - buf;
} */

/* size_t sprintf(char *buffer, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	size_t i = vsprintf(buffer, fmt, args);
	va_end(args);
	return i;
} */

u16 intToStr(s32 value, char *str, u16 minsize)
{
	if (value < -500000000)
	{
		*str = '-';
		return intToHex(-value, str + 1, minsize) + 1;
	}

	if (value < 0)
	{
		*str = '-';
		return uintToStr(-value, str + 1, minsize) + 1;
	}
	else return uintToStr(value, str, minsize);
}

u16 uintToStr(u32 value, char *str, u16 minsize)
{
	// the implentation cannot encode > 500000000 value --> use hexa
	if (value > 500000000)
		return intToHex(value, str, minsize);

	u16 len;

	// need to split in 2 conversions ?
	if (value > 10000)
	{
		const u16 v1 = value / (u16) 10000;
		const u16 v2 = value % (u16) 10000;

		len = uint16ToStr(v1, str, (minsize > 4)?(minsize - 4):1);
		len += uint16ToStr(v2, str + len, 4);
	}
	else len = uint16ToStr(value, str, minsize);

	return len;
}

static u16 uint16ToStr(u16 value, char *str, u16 minsize)
{
	u16 length;
	char *dst;
	u16 v;

	length = digits10(value);
	if (length < minsize) length = minsize;
	dst = &str[length];
	*dst = 0;
	v = value;

	while (v >= 100)
	{
		const u16 quot = v / 100;
		const u16 remain = v % 100;

		const u16 i = remain * 2;
		v = quot;

		*--dst = digits[i + 1];
		*--dst = digits[i + 0];
	}

	// handle last 1-2 digits
	if (v < 10) *--dst = '0' + v;
	else
	{
		const u16 i = v * 2;

		*--dst = digits[i + 1];
		*--dst = digits[i + 0];
	}

	// pad with '0'
	while (dst != str) *--dst = '0';

	return length;
}

u32 intToHex(u32 value, char *str, u16 minsize)
{
	u32 res;
	u16 cnt;
	u16 left;
	char data[16];
	char *src;
	char *dst;
	const u16 maxsize = 16;

	src = &data[16];
	res = value;
	left = minsize;

	cnt = 0;
	while (res)
	{
		u8 c;

		c = res & 0xF;

		if (c >= 10) c += ('A' - 10);
		else c += '0';

		*--src = c;
		cnt++;
		left--;
		res >>= 4;
	}
	
	while (left > 0)
	{
		*--src = '0';
		cnt++;
		left--;
	}

	if (cnt > maxsize) cnt = maxsize;

	dst = str;
	while (cnt--) *dst++ = *src++;
	*dst = 0;

	return 0;
}

void fix32ToStr(fix32 value, char *str, u16 numdec)
{
	char *dst = str;
	fix32 v = value;

	if (v < 0)
	{
		v = -v;
		*dst++ = '-';
	}

	dst += uintToStr(fix32ToInt(v), dst, 1);
	*dst++ = '.';

	// get fractional part
	const u16 frac = (((u16) fix32Frac(v)) * (u16) 1000) / ((u16) 1 << FIX32_FRAC_BITS);
	u16 len = uint16ToStr(frac, dst, 3);

	if (len < numdec)
	{
		// need to add ending '0'
		dst += len;
		while (len++ < numdec) *dst++ = '0';
		// mark end here
		*dst = 0;
	}
	else dst[numdec] = 0;
}

static u16 digits10(const u16 v)
{
	if (v < P02)
	{
		if (v < P01) return 1;
		return 2;
	} else {
		if (v < P03) return 3;
		if (v < P04) return 4;
		return 5;
	}
}