#ifndef _FIXED_H_
#define _FIXED_H_

//#include <stddef.h>

typedef int32_t fixed_t;

#define FIXED_SHIFT_BITS	16
#define FIXED_UNIT			(1<<FIXED_SHIFT_BITS)

fixed_t FixedMul(fixed_t a, fixed_t b);
fixed_t FixedDiv(fixed_t a, fixed_t b);
#define FixedMul2(c,a,b) \
	   __asm volatile ( \
			"dmuls.l %1, %2\n\t" \
			"sts mach, r1\n\t" \
			"sts macl, r0\n\t" \
			"xtrct r1, r0\n\t" \
			"mov r0, %0\n\t" \
			: "=r" (c) \
			: "r" (a), "r" (b) \
			: "r0", "r1", "mach", "macl")
fixed_t IDiv(fixed_t a, fixed_t b);

#define FIX32_INT_BITS			22
#define FIX32_FRAC_BITS			(32 - FIX32_INT_BITS)

#define FIX32_INT_MASK			(((1 << FIX32_INT_BITS) - 1) << FIX32_FRAC_BITS)
#define FIX32_FRAC_MASK			((1 << FIX32_FRAC_BITS) - 1)

#define FIX32(value)			((fix32) ((value) * (1 << FIX32_FRAC_BITS)))

#define fix32Div(val1, val2)	(((val1) << (FIX32_FRAC_BITS / 2)) / ((val2) >> (FIX32_FRAC_BITS / 2)))

#define intToFix32(value)		((fix32) ((value) << FIX32_FRAC_BITS))
#define fix32ToInt(value)		((s32) ((value) >> FIX32_FRAC_BITS))
#define fix32Frac(value)		((value) & FIX32_FRAC_MASK)
#define fix32Mul(val1, val2)	(((val1) >> (FIX32_FRAC_BITS / 2)) * ((val2) >> (FIX32_FRAC_BITS / 2)))
#define fix32Div(val1, val2)	(((val1) << (FIX32_FRAC_BITS / 2)) / ((val2) >> (FIX32_FRAC_BITS / 2)))

#endif /* _FIXED_H_ */