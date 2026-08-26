#include "32x.h"
#include "diagnostics.h"
#include "diagnostic_hw.h"
#include "hw_32x.h"
#include "shared_objects.h"
#include "sound.h"

#define SH2_SCI_SMR (*(volatile u8 *)0xFFFFFE00)
#define SH2_SCI_BRR (*(volatile u8 *)0xFFFFFE01)
#define SH2_SCI_SCR (*(volatile u8 *)0xFFFFFE02)
#define SH2_SCI_TDR (*(volatile u8 *)0xFFFFFE03)
#define SH2_SCI_SSR (*(volatile u8 *)0xFFFFFE04)
#define SH2_SCI_RDR (*(volatile u8 *)0xFFFFFE05)
#define SH2_VDP_CONTROL (*(volatile u8 *)0x2000410B)

extern const u16 diagnosticDmaPattern[];
extern const u16 diagnosticSoundPattern[];
extern const u16 diagnosticDisplayPattern[];
extern const u16 diagnosticWramPattern[];
extern const u8 ebu_Palette[];
extern const u32 diagnosticDmaPatternWords;
extern const u32 diagnosticSoundPatternWords;
extern volatile u32 diagnosticPrimaryVblankCount;
extern volatile u32 diagnosticPrimaryHblankCount;
extern volatile u32 diagnosticPrimaryCommandCount;
extern volatile u32 diagnosticPrimaryPwmCount;
extern volatile u32 diagnosticPrimarySciCount;
extern volatile u8 diagnosticPrimarySciValue;
extern volatile u32 diagnosticSecondaryVblankCount;
extern volatile u32 diagnosticSecondaryHblankCount;
extern volatile u32 diagnosticSecondaryCommandCount;
extern volatile u32 diagnosticSecondaryPwmCount;
extern volatile u32 diagnosticSecondarySciCount;
extern volatile u8 diagnosticSecondarySciValue;
extern volatile u8 diagnosticRuntimeActive;

static volatile u32 diagnosticPrimaryPwmSampleIndex;
static volatile u32 diagnosticSecondaryPwmSampleIndex;
static volatile u8 diagnosticPrimaryPwmStreamActive;
static volatile u8 diagnosticSecondaryPwmStreamActive;

static volatile u32 *diagnosticFrameCounter(u8 cpu)
{
	if (cpu == DIAG_TARGET_SLAVE)
		return (volatile u32 *)((u32)&diagnosticPrimaryVblankCount |
			0x20000000);
	return &diagnosticPrimaryVblankCount;
}

typedef struct
{
	u8 smr;
	u8 brr;
	u8 scr;
	u8 tdr;
	u8 ssr;
	u16 iprb;
	u16 vcra;
	u16 vcrb;
	int level;
} DiagnosticSciState;

static DiagnosticStatus resultPass(DiagnosticResult *result)
{
	result->status = DIAG_PASS;
	return DIAG_PASS;
}

static DiagnosticStatus resultFail(DiagnosticResult *result, u32 address,
	u32 expected, u32 actual)
{
	result->status = DIAG_FAIL;
	result->address = address;
	result->expected = expected;
	result->actual = actual;
	return DIAG_FAIL;
}

static DiagnosticStatus resultTimeout(DiagnosticResult *result, u32 address,
	u8 phase, u32 remaining)
{
	result->status = DIAG_TIMEOUT;
	result->address = address;
	result->phase = phase;
	result->remaining = remaining;
	return DIAG_TIMEOUT;
}

static int waitCounterFrames(volatile u32 *counter, u32 frames,
	u8 timeoutClass, u32 *actual, u32 *remaining)
{
	u32 start = *counter;
	DiagnosticDeadline deadline;

	diagnosticDeadlineStart(&deadline, timeoutClass);
	while (*counter - start < frames && diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (actual)
		*actual = *counter;
	if (remaining)
		*remaining = diagnosticDeadlineRemaining(&deadline);
	return *counter - start >= frames;
}

static DiagnosticStatus testRw8(u32 address, u8 mask,
	DiagnosticResult *result)
{
	static const u8 patterns[] = { 0x00, 0xFF, 0xAA, 0x55 };
	volatile u8 *reg = (volatile u8 *)address;
	u8 saved = *reg;
	int i;

	for (i = 0; i < 4; i++)
	{
		u8 expected = (saved & ~mask) | (patterns[i] & mask);
		u8 actual;

		*reg = expected;
		__asm__ volatile ("" ::: "memory");
		actual = *reg;
		if ((actual & mask) != (expected & mask))
		{
			*reg = saved;
			return resultFail(result, address, expected & mask,
				actual & mask);
		}
	}
	*reg = saved;
	return resultPass(result);
}

static DiagnosticStatus testRw16(u32 address, u16 mask,
	DiagnosticResult *result)
{
	static const u16 patterns[] = { 0x0000, 0xFFFF, 0xAAAA, 0x5555 };
	volatile u16 *reg = (volatile u16 *)address;
	u16 saved = *reg;
	int i;

	for (i = 0; i < 4; i++)
	{
		u16 expected = (saved & ~mask) | (patterns[i] & mask);
		u16 actual;

		*reg = expected;
		__asm__ volatile ("" ::: "memory");
		actual = *reg;
		if ((actual & mask) != (expected & mask))
		{
			*reg = saved;
			return resultFail(result, address, expected & mask,
				actual & mask);
		}
	}
	*reg = saved;
	return resultPass(result);
}

static DiagnosticStatus testCommunication(u16 catalog,
	DiagnosticResult *result)
{
	volatile u16 *registers = (volatile u16 *)0x20004028;
	u16 saved[4];
	int i;

	for (i = 0; i < 4; i++)
		saved[i] = registers[i];
	for (i = 0; i < 4; i++)
	{
		u16 expected = (u16)(0x1357 ^ (catalog << i));

		registers[i] = expected;
		if (registers[i] != expected)
		{
			u16 actual = registers[i];
			int restore;

			for (restore = 0; restore < 4; restore++)
				registers[restore] = saved[restore];
			return resultFail(result,
				0x20004028 + (u32)i * 2, expected, actual);
		}
	}
	for (i = 0; i < 4; i++)
		registers[i] = saved[i];
	return resultPass(result);
}

static DiagnosticStatus testCommunicationPeer(u16 catalog,
	DiagnosticResult *result)
{
	volatile u16 *words = (volatile u16 *)0x20004020;
	volatile u16 *phase = DIAG_SCRATCH_SECONDARY + 16;
	DiagnosticDeadline deadline;
	int index;

	*phase = DIAG_COMM_READY;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
	while (*phase != DIAG_COMM_GO && diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (*phase != DIAG_COMM_GO)
	{
		result->expected = DIAG_COMM_GO;
		result->actual = *phase;
		return resultTimeout(result, (u32)phase, 1,
			diagnosticDeadlineRemaining(&deadline));
	}
	if (catalog == 17)
	{
		for (index = 1; index < 8; index++)
			words[index] = (u16)(0x5000 | ((catalog & 0x00FF) << 3) |
				index);
		words[0] = (u16)(0x5000 | ((catalog & 0x00FF) << 3));
		*phase = DIAG_COMM_DONE;
	}
	else if (catalog == 40)
	{
		diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
		while (words[0] !=
			(u16)(0x5000 | ((catalog & 0x00FF) << 3)) &&
			diagnosticDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		if (words[0] != (u16)(0x5000 | ((catalog & 0x00FF) << 3)))
		{
			result->expected = (u16)(0x5000 |
				((catalog & 0x00FF) << 3));
			result->actual = words[0];
			return resultTimeout(result, 0x20004020, 2,
				diagnosticDeadlineRemaining(&deadline));
		}
		for (index = 0; index < 8; index++)
		{
			u16 expected = (u16)(0x5000 |
				((catalog & 0x00FF) << 3) | index);
			u16 actual = words[index];

			if (actual != expected)
			{
				result->status = DIAG_FAIL;
				result->expected = expected;
				result->actual = actual;
				result->address = 0x20004020 + index * 2;
			}
		}
		*phase = DIAG_COMM_DONE;
	}
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
	while (*phase != DIAG_COMM_RESTORE &&
		diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (*phase != DIAG_COMM_RESTORE)
	{
		result->expected = DIAG_COMM_RESTORE;
		result->actual = *phase;
		return resultTimeout(result, (u32)phase, 3,
			diagnosticDeadlineRemaining(&deadline));
	}
	return result->status == DIAG_FAIL ? DIAG_FAIL : resultPass(result);
}

static DiagnosticStatus testRomMarkers(DiagnosticResult *result)
{
	static const u32 addresses[] =
	{
		0x220FFFFC, 0x221FFFFC, 0x222FFFFC, 0x223FFFFC
	};
	static const u32 expected[] =
	{
		0x424B3030, 0x424B3031, 0x424B3130, 0x424B3131
	};
	int i;

	for (i = 0; i < 4; i++)
	{
		u32 actual = *(volatile u32 *)addresses[i];

		if (actual != expected[i])
			return resultFail(result, addresses[i], expected[i], actual);
	}
	return resultPass(result);
}

static void saveSci(DiagnosticSciState *state)
{
	state->level = SetSH2SR(15);
	state->smr = SH2_SCI_SMR;
	state->brr = SH2_SCI_BRR;
	state->scr = SH2_SCI_SCR;
	state->tdr = SH2_SCI_TDR;
	state->ssr = SH2_SCI_SSR;
	state->iprb = SH2_INT_IPRB;
	state->vcra = SH2_INT_VCRA;
	state->vcrb = SH2_INT_VCRB;
}

static void restoreSci(const DiagnosticSciState *state)
{
	SH2_SCI_SCR = 0;
	SH2_SCI_TDR = state->tdr;
	SH2_SCI_BRR = state->brr;
	SH2_SCI_SMR = state->smr;
	SH2_SCI_SSR = state->ssr;
	SH2_SCI_SCR = state->scr;
	SH2_INT_IPRB = state->iprb;
	SH2_INT_VCRA = state->vcra;
	SH2_INT_VCRB = state->vcrb;
	SetSH2SR(state->level);
}

static DiagnosticStatus testSciEcho(DiagnosticResult *result)
{
	volatile u16 *phase = DIAG_SCRATCH_SECONDARY + 16;
	DiagnosticSciState state;
	volatile u32 *receiveCount;
	volatile u8 *receiveValue;
	u32 before;
	DiagnosticDeadline deadline;
	u8 value;
	int ready;

	saveSci(&state);
	if (result->cpu == DIAG_TARGET_SLAVE)
	{
		receiveCount = &diagnosticSecondarySciCount;
		receiveValue = &diagnosticSecondarySciValue;
	}
	else
	{
		receiveCount = &diagnosticPrimarySciCount;
		receiveValue = &diagnosticPrimarySciValue;
	}
	before = *receiveCount;
	SH2_SCI_SCR = 0;
	SH2_SCI_SMR = 0x30;
	SH2_SCI_BRR = 0;
	SH2_SCI_SCR = result->cpu == DIAG_TARGET_SLAVE ? 3 : 1;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_REGISTER);
	while (diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	value = SH2_SCI_RDR;
	SH2_SCI_SSR &= 0x87;
	SH2_INT_VCRA = 0x4242;
	SH2_INT_VCRB = 0x4242;
	SH2_INT_IPRB = (SH2_INT_IPRB & 0x0FFF) | 0x4000;
	SH2_SCI_SCR = 0x70;
	SetSH2SR(1);
	if (result->cpu == DIAG_TARGET_SLAVE)
	{
		*phase = 0x534C;
		diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
		while (*receiveCount == before &&
			diagnosticDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		if (*receiveCount == before)
		{
			result->expected = before + 1;
			result->actual = *receiveCount;
			restoreSci(&state);
			return resultTimeout(result, 0xFFFFFE04, 1,
				diagnosticDeadlineRemaining(&deadline));
		}
		value = *receiveValue;
		diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
		while (!(SH2_SCI_SSR & 0x04) &&
			diagnosticDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		ready = (SH2_SCI_SSR & 0x04) != 0;
		restoreSci(&state);
		if (!ready)
		{
			result->expected = 0x04;
			result->actual = 0;
			return resultTimeout(result, 0xFFFFFE04, 3,
				diagnosticDeadlineRemaining(&deadline));
		}
		result->expected = 0xAA;
		result->actual = value;
		return value == 0xAA ? resultPass(result) :
			resultFail(result, 0xFFFFFE05, 0xAA, value);
	}
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
	while (*phase != 0x534C && diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (*phase != 0x534C)
	{
		result->expected = 0x534C;
		result->actual = *phase;
		restoreSci(&state);
		return resultTimeout(result, (u32)phase, 1,
			diagnosticDeadlineRemaining(&deadline));
	}
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
	while (!(SH2_SCI_SSR & 0x80) && diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (!(SH2_SCI_SSR & 0x80))
	{
		result->expected = 0x80;
		result->actual = SH2_SCI_SSR;
		restoreSci(&state);
		return resultTimeout(result, 0xFFFFFE04, 2,
			diagnosticDeadlineRemaining(&deadline));
	}
	SH2_SCI_TDR = 0xAA;
	SH2_SCI_SSR &= (u8)~0x80;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
	while (*receiveCount == before && diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (*receiveCount == before)
	{
		result->expected = before + 1;
		result->actual = *receiveCount;
		restoreSci(&state);
		return resultTimeout(result, 0xFFFFFE04, 3,
			diagnosticDeadlineRemaining(&deadline));
	}
	value = *receiveValue;
	restoreSci(&state);
	*phase = 0;
	result->expected = 0xAA;
	result->actual = value;
	return value == 0xAA ? resultPass(result) :
		resultFail(result, 0xFFFFFE05, 0xAA, value);
}

static DiagnosticStatus testPwmFifo(u16 catalog,
	DiagnosticResult *result)
{
	volatile u16 *fifo;
	volatile u32 *counter = diagnosticFrameCounter(result->cpu);
	u16 savedControl = MARS_PWM_CTRL;
	u16 savedCycle = MARS_PWM_CYCLE;
	DiagnosticDeadline deadline;
	DiagnosticStatus status = DIAG_PASS;
	u16 route = 5;
	u16 actualFifo;
	u32 start;
	u32 actualCounter;
	u32 remaining;

	if (catalog == 30 || catalog == 31)
	{
		fifo = &MARS_PWM_LEFT;
		route = 1;
	}
	else if (catalog == 32 || catalog == 33)
	{
		fifo = &MARS_PWM_RIGHT;
		route = 4;
	}
	else
		fifo = &MARS_PWM_MONO;
	MARS_PWM_CTRL = 0;
	MARS_PWM_CYCLE = 0x0800;
	MARS_PWM_CTRL = route;
	start = *counter;
	if (!waitCounterFrames(counter, 30, DIAG_TIMEOUT_STIMULUS,
		&actualCounter, &remaining))
	{
		result->expected = start + 30;
		result->actual = actualCounter;
		status = resultTimeout(result, (u32)counter, 1, remaining);
		goto finish;
	}
	start = *counter;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_INTERRUPT);
	while (!(*fifo & 0x4000) && *counter - start < 5 &&
		diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	actualFifo = *fifo;
	if (!(actualFifo & 0x4000))
	{
		result->expected = 0x4000;
		result->actual = actualFifo & 0xC000;
		status = resultTimeout(result, (u32)fifo, 2,
			diagnosticDeadlineRemaining(&deadline));
		goto finish;
	}
	if (actualFifo & 0x8000)
	{
		status = resultFail(result, (u32)fifo, 0,
			actualFifo & 0x8000);
		goto finish;
	}
	*fifo = 0;
	*fifo = 0;
	actualFifo = *fifo;
	if (actualFifo & 0x4000)
	{
		status = resultFail(result, (u32)fifo, 0,
			actualFifo & 0x4000);
		goto finish;
	}
	start = *counter;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_STIMULUS);
	while (!(*fifo & 0x8000) && *counter - start < 120 &&
		diagnosticDeadlineActive(&deadline))
	{
		*fifo = 0;
		*fifo = 0;
	}
	actualFifo = *fifo;
	if (!(actualFifo & 0x8000))
	{
		result->expected = 0x8000;
		result->actual = actualFifo & 0xC000;
		status = resultTimeout(result, (u32)fifo, 3,
			diagnosticDeadlineRemaining(&deadline));
	}
finish:
	MARS_PWM_CTRL = 0;
	MARS_PWM_CYCLE = savedCycle;
	MARS_PWM_CTRL = savedControl;
	if (status == DIAG_PASS)
		return resultPass(result);
	return status;
}

static DiagnosticStatus testSh2System(u16 catalog,
	DiagnosticResult *result)
{
	switch (catalog)
	{
		case 16:
		case 17:
			return testCommunication(catalog, result);
		case 18:
		case 19:
			return testRw8(0x20004000, 0x80, result);
		case 20:
		case 21:
			return testRw8(0x20004001, 0x8F, result);
		case 22:
		case 23:
			return testRw8(0x20004005, 0xFF, result);
		case 24:
		case 25:
			return testRw16(0x20004030, 0x0F00, result);
		case 26:
		case 27:
			return testRw16(0x20004030, 0x000F, result);
		case 28:
		case 29:
			return testRw16(0x20004032, 0x0FFF, result);
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
			return testPwmFifo(catalog, result);
		case 36:
		case 37:
			return testRomMarkers(result);
		case 38:
			return testSciEcho(result);
		default:
			return resultFail(result, catalog, 1, 0);
	}
}

static DiagnosticStatus waitVdpStatus(u16 mask, DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_VDP_FBCTL, mask, mask,
		DIAG_TIMEOUT_INTERRUPT, &actual, &remaining))
	{
		result->expected = mask;
		result->actual = actual;
		return resultTimeout(result, 0x2000410A, 1, remaining);
	}
	if (!diagnosticWait16(&MARS_VDP_FBCTL, mask, 0,
		DIAG_TIMEOUT_INTERRUPT, &actual, &remaining))
	{
		result->expected = 0;
		result->actual = actual;
		return resultTimeout(result, 0x2000410A, 2, remaining);
	}
	return resultPass(result);
}

static DiagnosticStatus testSh2Vdp(u16 catalog,
	DiagnosticResult *result)
{
	switch (catalog)
	{
		case 67:
		case 68:
			return testRw16(0x20004100, 0x00C3, result);
		case 69:
		case 70:
			return testRw16(0x20004102, 0x0001, result);
		case 71:
		case 72:
			return testRw8(0x20004105, 0xFF, result);
		case 73:
		case 74:
			return testRw16(0x20004106, 0xFFFF, result);
		case 75:
		case 76:
			return waitVdpStatus(MARS_VDP_VBLK, result);
		case 77:
		case 78:
			return waitVdpStatus(MARS_VDP_HBLK, result);
		case 79:
		case 80:
			return waitVdpStatus(MARS_VDP_PEN, result);
		case 81:
		case 82:
		{
			u16 before = SH2_VDP_CONTROL & MARS_VDP_FS;
			u16 actual;
			u32 remaining;

			SH2_VDP_CONTROL = before ^ 1;
			if (!diagnosticWait16(&MARS_VDP_FBCTL, MARS_VDP_FS,
				before ^ 1, DIAG_TIMEOUT_INTERRUPT, &actual, &remaining))
			{
				u16 failureActual = actual;
				u32 failureRemaining = remaining;

				SH2_VDP_CONTROL = before;
				if (!diagnosticWait16(&MARS_VDP_FBCTL, MARS_VDP_FS,
					before, DIAG_TIMEOUT_INTERRUPT, &actual, &remaining))
				{
					result->status = DIAG_INFRASTRUCTURE_ERROR;
					result->phase = 2;
					result->address = 0x2000410A;
					result->expected = before;
					result->actual = actual;
					result->remaining = remaining;
					return DIAG_INFRASTRUCTURE_ERROR;
				}
				result->actual = failureActual;
				result->expected = before ^ 1;
				return resultTimeout(result, 0x2000410A, 1,
					failureRemaining);
			}
			SH2_VDP_CONTROL = before;
			if (!diagnosticWait16(&MARS_VDP_FBCTL, MARS_VDP_FS,
				before, DIAG_TIMEOUT_INTERRUPT, &actual, &remaining))
			{
				result->status = DIAG_INFRASTRUCTURE_ERROR;
				result->phase = 2;
				result->address = 0x2000410A;
				result->expected = before;
				result->actual = actual;
				result->remaining = remaining;
				return DIAG_INFRASTRUCTURE_ERROR;
			}
			return resultPass(result);
		}
		default:
			return resultFail(result, catalog, 1, 0);
	}
}

static int selectFramebuffer(int framebuffer, u32 *remaining)
{
	u16 current = SH2_VDP_CONTROL & MARS_VDP_FS;

	if (current != (framebuffer & 1))
	{
		SH2_VDP_CONTROL = framebuffer & 1;
		return diagnosticWait16(&MARS_VDP_FBCTL, MARS_VDP_FS,
			framebuffer & 1, DIAG_TIMEOUT_INTERRUPT, 0, remaining);
	}
	if (remaining)
		*remaining = 0;
	return 1;
}

static int waitAutofill(u16 *actual, u32 *remaining)
{
	return diagnosticWait16(&MARS_VDP_FBCTL, MARS_VDP_FEN, 0,
		DIAG_TIMEOUT_DMA, actual, remaining);
}

static DiagnosticStatus finishFramebufferSelection(u16 saved,
	DiagnosticStatus status, DiagnosticResult *result)
{
	u32 remaining;

	if (!selectFramebuffer(saved, &remaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->phase = 3;
		result->address = 0x2000410A;
		result->expected = saved & 1;
		result->actual = SH2_VDP_CONTROL & MARS_VDP_FS;
		result->remaining = remaining;
		return DIAG_INFRASTRUCTURE_ERROR;
	}
	return status;
}

static DiagnosticStatus testFramebufferMemory(u16 catalog,
	DiagnosticResult *result)
{
	volatile u16 *words = (volatile u16 *)0x24000200;
	volatile u8 *bytes = (volatile u8 *)0x24000200;
	volatile u16 *overwrite = (volatile u16 *)0x24020200;
	u16 saved[16];
	u16 savedFb = SH2_VDP_CONTROL & MARS_VDP_FS;
	u16 actual;
	u32 remaining;
	int framebuffer = ((catalog == 84) || (catalog == 87) ||
		(catalog == 88) || (catalog == 90) || (catalog == 92) ||
		(catalog == 95) || (catalog == 96) || (catalog == 99) ||
		(catalog == 100) || (catalog == 102) || (catalog == 104) ||
		(catalog == 107) || (catalog == 108) || (catalog == 111) ||
		(catalog == 112));
	int selection = framebuffer ^ 1;
	int i;

	if (!waitAutofill(&actual, &remaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->phase = 1;
		result->address = 0x2000410A;
		result->expected = 0;
		result->actual = actual;
		result->remaining = remaining;
		return DIAG_INFRASTRUCTURE_ERROR;
	}
	if (!selectFramebuffer(selection, &remaining))
	{
		result->expected = selection;
		result->actual = SH2_VDP_CONTROL & MARS_VDP_FS;
		return finishFramebufferSelection(savedFb,
			resultTimeout(result, 0x2000410A, 1, remaining), result);
	}
	for (i = 0; i < 16; i++)
		saved[i] = words[i];
	if ((catalog >= 91 && catalog <= 92) ||
		(catalog >= 97 && catalog <= 100))
	{
		for (i = 0; i < 16; i++)
			bytes[i] = (u8)(0xA0 + i);
		for (i = 0; i < 16; i++)
		{
			if (bytes[i] != (u8)(0xA0 + i))
			{
				u8 actual = bytes[i];
				int restore;

				for (restore = 0; restore < 16; restore++)
					words[restore] = saved[restore];
				return finishFramebufferSelection(savedFb,
					resultFail(result, 0x24000200 + i,
						0xA0 + i, actual), result);
			}
		}
	}
	else if ((catalog >= 89 && catalog <= 90) ||
		(catalog >= 93 && catalog <= 96))
	{
		for (i = 0; i < 16; i++)
		{
			words[i] = 0x5500 | (i + 1);
			overwrite[i] = i & 1 ? 0x00AA : 0xAA00;
			if ((words[i] & (i & 1 ? 0x00FF : 0xFF00)) !=
				(i & 1 ? 0x00AA : 0xAA00))
			{
				u16 actual = words[i];
				int restore;

				for (restore = 0; restore < 16; restore++)
					words[restore] = saved[restore];
				return finishFramebufferSelection(savedFb,
					resultFail(result, 0x24020200 + i * 2,
						i & 1 ? 0x00AA : 0xAA00, actual), result);
			}
		}
	}
	else if (catalog >= 101)
	{
		volatile u8 *fillLength = (volatile u8 *)0x20004105;
		u8 savedLength = *fillLength;
		u16 savedAddress = MARS_VDP_FILADR;

		*fillLength = 15;
		MARS_VDP_FILADR = 0x0100;
		MARS_VDP_FILDAT = 0x6D3A;
		if (!waitAutofill(&actual, &remaining))
		{
			result->expected = 0;
			result->actual = actual;
			resultTimeout(result, 0x2000410A, 1, remaining);
			result->status = DIAG_INFRASTRUCTURE_ERROR;
			return DIAG_INFRASTRUCTURE_ERROR;
		}
		for (i = 0; i < 16; i++)
		{
			if ((catalog == 105 || catalog == 106 || catalog == 107 ||
				catalog == 108) && words[i] != 0x6D3A)
			{
				u16 mismatch = words[i];
				*fillLength = savedLength;
				MARS_VDP_FILADR = savedAddress;
				for (i = 0; i < 16; i++)
					words[i] = saved[i];
				return finishFramebufferSelection(savedFb,
					resultFail(result, 0x24000200 + i * 2,
						0x6D3A, mismatch), result);
			}
		}
		if (catalog >= 109 && (MARS_VDP_FILADR & 0x00FF) != 0x0010)
		{
			u16 mismatch = MARS_VDP_FILADR & 0x00FF;

			*fillLength = savedLength;
			MARS_VDP_FILADR = savedAddress;
			for (i = 0; i < 16; i++)
				words[i] = saved[i];
			return finishFramebufferSelection(savedFb,
				resultFail(result, 0x20004106, 0x0010,
					mismatch), result);
		}
		*fillLength = savedLength;
		MARS_VDP_FILADR = savedAddress;
	}
	else
	{
		for (i = 0; i < 16; i++)
			words[i] = (u16)(0x5A00 | i);
		for (i = 0; i < 16; i++)
		{
			if (words[i] != (u16)(0x5A00 | i))
			{
				u16 actual = words[i];
				int restore;

				for (restore = 0; restore < 16; restore++)
					words[restore] = saved[restore];
				return finishFramebufferSelection(savedFb,
					resultFail(result, 0x24000200 + i * 2,
						0x5A00 | i, actual), result);
			}
		}
	}
	for (i = 0; i < 16; i++)
		words[i] = saved[i];
	return finishFramebufferSelection(savedFb, resultPass(result),
		result);
}

static DiagnosticStatus testPalette(u16 catalog, DiagnosticResult *result)
{
	volatile u16 *palette = (volatile u16 *)0x20004200;
	volatile u16 *fillWords = (volatile u16 *)0x2400E000;
	volatile u8 *fillLength = (volatile u8 *)0x20004105;
	u16 saved[16];
	u16 fillSaved[256];
	u16 savedMode = MARS_VDP_DISPMODE;
	u8 savedLength = *fillLength;
	u16 savedAddress = MARS_VDP_FILADR;
	DiagnosticStatus resultStatus = DIAG_PASS;
	u16 fillStatus = 0;
	u32 remaining;
	int autofill = catalog == 115 || catalog == 120 || catalog == 121;
	int i;

	for (i = 0; i < 16; i++)
		saved[i] = palette[i];
	if (autofill)
		for (i = 0; i < 256; i++)
			fillSaved[i] = fillWords[i];
	if (catalog == 113 || catalog == 116 || catalog == 117 || autofill)
		MARS_VDP_DISPMODE = savedMode & ~3;
	else
		MARS_VDP_DISPMODE = (savedMode & ~3) | MARS_VDP_MODE_256;
	if (autofill)
	{
		*fillLength = 0xFF;
		MARS_VDP_FILADR = 0x7000;
		MARS_VDP_FILDAT = 0x4D44;
	}
	for (i = 0; i < 16; i++)
	{
		u16 expected = diagnosticDisplayPattern[i & 7];

		palette[i] = expected;
		if (palette[i] != expected)
		{
			u16 actual = palette[i];

			resultStatus = resultFail(result, 0x20004200 + i * 2,
				expected, actual);
			break;
		}
	}
	for (i = 0; i < 16; i++)
		palette[i] = saved[i];
	if (autofill)
	{
		if (!waitAutofill(&fillStatus, &remaining))
		{
			if (resultStatus == DIAG_PASS)
			{
				resultTimeout(result, 0x2000410A, 1, remaining);
				result->status = DIAG_INFRASTRUCTURE_ERROR;
				resultStatus = DIAG_INFRASTRUCTURE_ERROR;
			}
		}
		for (i = 0; i < 256; i++)
			fillWords[i] = fillSaved[i];
		*fillLength = savedLength;
		MARS_VDP_FILADR = savedAddress;
	}
	MARS_VDP_DISPMODE = savedMode;
	if (resultStatus == DIAG_PASS)
		return resultPass(result);
	return resultStatus;
}

static int stopDma(volatile u32 *channel);

static DiagnosticStatus runDma(u32 source, u32 destination, u32 words,
	DiagnosticResult *result)
{
	u32 savedSar = SH2_DMA_SAR0;
	u32 savedDar = SH2_DMA_DAR0;
	u32 savedTcr = SH2_DMA_TCR0;
	u32 savedChcr = SH2_DMA_CHCR0;
	u32 savedDmaor = SH2_DMA_DMAOR;
	u8 savedDrcr = SH2_DMA_DRCR0;
	u32 actual;
	u32 deadlineRemaining;
	u32 transferRemaining;
	int quiescent;

	SH2_DMA_CHCR0 = 0;
	SH2_DMA_DMAOR = 0;
	SH2_DMA_DRCR0 = 0;
	SH2_DMA_SAR0 = source;
	SH2_DMA_DAR0 = destination;
	SH2_DMA_TCR0 = words;
	SH2_DMA_CHCR0 = 0x00005EE1;
	SH2_DMA_DMAOR = 1;
	if (!diagnosticWait32(&SH2_DMA_CHCR0, 2, 2, DIAG_TIMEOUT_DMA,
		&actual, &deadlineRemaining))
	{
		transferRemaining = SH2_DMA_TCR0;
		quiescent = stopDma(&SH2_DMA_CHCR0);
		SH2_DMA_SAR0 = savedSar;
		SH2_DMA_DAR0 = savedDar;
		SH2_DMA_TCR0 = savedTcr;
		SH2_DMA_DRCR0 = savedDrcr;
		SH2_DMA_CHCR0 = savedChcr;
		SH2_DMA_DMAOR = savedDmaor;
		result->actual = actual;
		if (!quiescent)
		{
			result->status = DIAG_INFRASTRUCTURE_ERROR;
			result->address = 0xFFFFFF8C;
			return DIAG_INFRASTRUCTURE_ERROR;
		}
		return resultTimeout(result, 0xFFFFFF8C, 1,
			transferRemaining ? transferRemaining : deadlineRemaining);
	}
	SH2_DMA_CHCR0 = 0;
	SH2_DMA_DMAOR = 0;
	SH2_DMA_SAR0 = savedSar;
	SH2_DMA_DAR0 = savedDar;
	SH2_DMA_TCR0 = savedTcr;
	SH2_DMA_DRCR0 = savedDrcr;
	SH2_DMA_CHCR0 = savedChcr;
	SH2_DMA_DMAOR = savedDmaor;
	return resultPass(result);
}

static u16 dreqPattern(u16 index)
{
	return (u16)(0x5AA5 ^ (index * 0x1111));
}

static int stopDma(volatile u32 *channel)
{
	*channel = 0;
	SH2_DMA_DMAOR = 0;
	return ((*channel & 1) == 0) && ((SH2_DMA_DMAOR & 1) == 0);
}

static DiagnosticStatus runDreqDma(DiagnosticResult *result)
{
	volatile u16 *scratch = DIAG_SCRATCH_DMA;
	u32 savedSar = SH2_DMA_SAR0;
	u32 savedDar = SH2_DMA_DAR0;
	u32 savedTcr = SH2_DMA_TCR0;
	u32 savedChcr = SH2_DMA_CHCR0;
	u32 savedDmaor = SH2_DMA_DMAOR;
	u8 savedDrcr = SH2_DMA_DRCR0;
	DiagnosticResult mdResult = *result;
	u32 actual;
	u32 deadlineRemaining;
	u32 transferRemaining;
	int index;
	int quiescent;

	for (index = 0; index < 64; index++)
		scratch[index] = 0;
	SH2_DMA_CHCR0 = 0;
	SH2_DMA_DMAOR = 0;
	SH2_DMA_DRCR0 = 0;
	SH2_DMA_SAR0 = 0x20004012;
	SH2_DMA_DAR0 = 0x06020000;
	SH2_DMA_TCR0 = 64;
	SH2_DMA_CHCR0 = 0x000044E1;
	SH2_DMA_DMAOR = 1;
	if (!diagnosticMdCommand(DIAG_MD_EXECUTE,
		result->cpu == DIAG_TARGET_SLAVE ? 123 : 122, &mdResult) ||
		mdResult.status != DIAG_PASS)
	{
		quiescent = stopDma(&SH2_DMA_CHCR0);
		SH2_DMA_SAR0 = savedSar;
		SH2_DMA_DAR0 = savedDar;
		SH2_DMA_TCR0 = savedTcr;
		SH2_DMA_DRCR0 = savedDrcr;
		SH2_DMA_CHCR0 = savedChcr;
		SH2_DMA_DMAOR = savedDmaor;
		*result = mdResult;
		if (!quiescent)
		{
			result->status = DIAG_INFRASTRUCTURE_ERROR;
			result->address = 0xFFFFFF8C;
		}
		return result->status;
	}
	if (!diagnosticWait32(&SH2_DMA_CHCR0, 2, 2, DIAG_TIMEOUT_DMA,
		&actual, &deadlineRemaining))
	{
		transferRemaining = SH2_DMA_TCR0;
		quiescent = stopDma(&SH2_DMA_CHCR0);
		SH2_DMA_SAR0 = savedSar;
		SH2_DMA_DAR0 = savedDar;
		SH2_DMA_TCR0 = savedTcr;
		SH2_DMA_DRCR0 = savedDrcr;
		SH2_DMA_CHCR0 = savedChcr;
		SH2_DMA_DMAOR = savedDmaor;
		result->actual = actual;
		if (!quiescent)
		{
			result->status = DIAG_INFRASTRUCTURE_ERROR;
			result->address = 0xFFFFFF8C;
			return DIAG_INFRASTRUCTURE_ERROR;
		}
		return resultTimeout(result, 0xFFFFFF8C, 2,
			transferRemaining ? transferRemaining : deadlineRemaining);
	}
	SH2_DMA_CHCR0 = 0;
	SH2_DMA_DMAOR = 0;
	SH2_DMA_SAR0 = savedSar;
	SH2_DMA_DAR0 = savedDar;
	SH2_DMA_TCR0 = savedTcr;
	SH2_DMA_DRCR0 = savedDrcr;
	SH2_DMA_CHCR0 = savedChcr;
	SH2_DMA_DMAOR = savedDmaor;
	for (index = 0; index < 64; index++)
	{
		u16 expected = dreqPattern(index);

		if (scratch[index] != expected)
			return resultFail(result, (u32)&scratch[index], expected,
				scratch[index]);
	}
	return resultPass(result);
}

static DiagnosticStatus compareWords(volatile u16 *actual,
	const u16 *expected, u32 words, DiagnosticResult *result)
{
	u32 i;

	for (i = 0; i < words; i++)
	{
		if (actual[i] != expected[i])
			return resultFail(result, (u32)&actual[i], expected[i],
				actual[i]);
	}
	return resultPass(result);
}

static DiagnosticStatus __attribute__((section(".ramcode")))
	testDma(u16 catalog, DiagnosticResult *result)
{
	volatile u16 *scratch = DIAG_SCRATCH_DMA;
	u32 words = diagnosticDmaPatternWords;
	u32 source = (u32)diagnosticDmaPattern | 0x20000000;
	u32 destination = 0x06020000;
	int i;

	if (catalog == 122 || catalog == 123)
		return runDreqDma(result);
	if (catalog == 124 || catalog == 125)
	{
		volatile u16 *capture = (volatile u16 *)0x26020000;
		const u16 *expected = (const u16 *)
			((u32)diagnosticWramPattern | 0x20000000);
		u32 savedSar = SH2_DMA_SAR0;
		u32 savedDar = SH2_DMA_DAR0;
		u32 savedTcr = SH2_DMA_TCR0;
		u32 savedChcr = SH2_DMA_CHCR0;
		u32 savedDmaor = SH2_DMA_DMAOR;
		u8 savedDrcr = SH2_DMA_DRCR0;
		u8 savedAdapter = *(volatile u8 *)0x20004000;
		u8 savedIntmsk = *(volatile u8 *)0x20004001;
		DiagnosticResult mdResult = *result;
		u32 transferWords;
		u32 actual;
		u32 remaining;
		u32 deadlineRemaining;
		int dmaComplete;
		int mdComplete;
		int quiescent;

		*(volatile u8 *)0x20004000 = 0x80;
		SH2_DMA_CHCR0 = 0;
		SH2_DMA_DMAOR = 0;
		SH2_DMA_DRCR0 = 0;
		SH2_DMA_SAR0 = 0x20004012;
		SH2_DMA_DAR0 = 0x06020000;
		transferWords = MARS_SYS_DMALEN;
		if (!transferWords)
			transferWords = 0x10000;
		SH2_DMA_TCR0 = transferWords;
		*(volatile u8 *)0x20004001 = 2;
		SH2_DMA_CHCR0 = 0x000044E1;
		SH2_DMA_DMAOR = 1;
		if (!diagnosticMdDmaBegin(catalog, &mdResult))
		{
			quiescent = stopDma(&SH2_DMA_CHCR0);
			SH2_DMA_SAR0 = savedSar;
			SH2_DMA_DAR0 = savedDar;
			SH2_DMA_TCR0 = savedTcr;
			SH2_DMA_DRCR0 = savedDrcr;
			SH2_DMA_CHCR0 = savedChcr;
			SH2_DMA_DMAOR = savedDmaor;
			*(volatile u8 *)0x20004001 = savedIntmsk;
			*(volatile u8 *)0x20004000 = savedAdapter;
			*result = mdResult;
			if (!quiescent)
			{
				result->status = DIAG_INFRASTRUCTURE_ERROR;
				result->address = 0xFFFFFF8C;
			}
			return result->status;
		}
		dmaComplete = diagnosticWait32(&SH2_DMA_CHCR0, 2, 2,
			DIAG_TIMEOUT_SEGA_CD, &actual, &deadlineRemaining);
		remaining = SH2_DMA_TCR0;
		mdComplete = diagnosticMdDmaFinish(catalog, &mdResult);
		if (!mdComplete || mdResult.status != DIAG_PASS)
		{
			quiescent = stopDma(&SH2_DMA_CHCR0);
			SH2_DMA_SAR0 = savedSar;
			SH2_DMA_DAR0 = savedDar;
			SH2_DMA_TCR0 = savedTcr;
			SH2_DMA_DRCR0 = savedDrcr;
			SH2_DMA_CHCR0 = savedChcr;
			SH2_DMA_DMAOR = savedDmaor;
			*(volatile u8 *)0x20004001 = savedIntmsk;
			*(volatile u8 *)0x20004000 = savedAdapter;
			*result = mdResult;
			if (!quiescent)
			{
				result->status = DIAG_INFRASTRUCTURE_ERROR;
				result->address = 0xFFFFFF8C;
			}
			return result->status;
		}
		if (!dmaComplete)
		{
			quiescent = stopDma(&SH2_DMA_CHCR0);
			SH2_DMA_SAR0 = savedSar;
			SH2_DMA_DAR0 = savedDar;
			SH2_DMA_TCR0 = savedTcr;
			SH2_DMA_DRCR0 = savedDrcr;
			SH2_DMA_CHCR0 = savedChcr;
			SH2_DMA_DMAOR = savedDmaor;
			*(volatile u8 *)0x20004001 = savedIntmsk;
			*(volatile u8 *)0x20004000 = savedAdapter;
			result->expected = 2;
			result->actual = actual;
			if (!quiescent)
			{
				result->status = DIAG_INFRASTRUCTURE_ERROR;
				result->address = 0xFFFFFF8C;
				return DIAG_INFRASTRUCTURE_ERROR;
			}
			return resultTimeout(result, 0xFFFFFF8C, 2,
				remaining ? remaining : deadlineRemaining);
		}
		SH2_DMA_DMAOR = 0;
		SH2_DMA_CHCR0 = 0;
		SH2_DMA_SAR0 = savedSar;
		SH2_DMA_DAR0 = savedDar;
		SH2_DMA_TCR0 = savedTcr;
		SH2_DMA_DRCR0 = savedDrcr;
		SH2_DMA_CHCR0 = savedChcr;
		SH2_DMA_DMAOR = savedDmaor;
		*(volatile u8 *)0x20004001 = savedIntmsk;
		*(volatile u8 *)0x20004000 = savedAdapter;
		for (i = 0; i < 0x7FFF; i++)
		{
			if (capture[i + 1] != expected[i])
				return resultFail(result, (u32)&capture[i + 1],
					expected[i], capture[i + 1]);
		}
		result->remaining = 0;
		return resultPass(result);
	}
	if (catalog == 127 || catalog == 128)
	{
		for (i = 0; i < words; i++)
			scratch[i] = 0;
		if (runDma(source, destination, words, result) != DIAG_PASS)
			return result->status;
		return compareWords(scratch, diagnosticDmaPattern, words,
			result);
	}
	if (catalog == 129 || catalog == 130)
	{
		volatile u16 *framebuffer = (volatile u16 *)0x24000400;
		u16 saved[64];
		DiagnosticStatus status;

		for (i = 0; i < words; i++)
			saved[i] = framebuffer[i];
		status = runDma(source, (u32)framebuffer, words, result);
		if (status == DIAG_PASS)
			status = compareWords(framebuffer, diagnosticDmaPattern,
				words, result);
		for (i = 0; i < words; i++)
			framebuffer[i] = saved[i];
		return status;
	}
	if (catalog == 131 || catalog == 132)
	{
		volatile u16 *framebuffer = (volatile u16 *)0x24000400;
		u16 saved[64];
		DiagnosticStatus status;

		for (i = 0; i < words; i++)
			saved[i] = framebuffer[i];
		for (i = 0; i < words; i++)
			scratch[i] = diagnosticDmaPattern[i];
		status = runDma(destination, (u32)framebuffer, words, result);
		if (status == DIAG_PASS)
			status = compareWords(framebuffer, diagnosticDmaPattern,
				words, result);
		for (i = 0; i < words; i++)
			framebuffer[i] = saved[i];
		return status;
	}
	if (catalog == 133 || catalog == 134)
	{
		volatile u16 *framebuffer = (volatile u16 *)0x24000400;
		u16 saved[64];
		DiagnosticStatus status;

		for (i = 0; i < words; i++)
			saved[i] = framebuffer[i];
		for (i = 0; i < words; i++)
			framebuffer[i] = diagnosticDmaPattern[i];
		status = runDma((u32)framebuffer, destination, words, result);
		if (status == DIAG_PASS)
			status = compareWords(scratch, diagnosticDmaPattern, words,
				result);
		for (i = 0; i < words; i++)
			framebuffer[i] = saved[i];
		return status;
	}
	return resultFail(result, catalog, 1, 0);
}

void diagnosticPrimaryPwmIrqHandler(void)
{
	u32 index;

	if (!diagnosticPrimaryPwmStreamActive)
		return;
	index = diagnosticPrimaryPwmSampleIndex;
	MARS_PWM_MONO = diagnosticSoundPattern[index];
	index++;
	if (index >= diagnosticSoundPatternWords)
		index = 0;
	diagnosticPrimaryPwmSampleIndex = index;
}

void diagnosticSecondaryPwmIrqHandler(void)
{
	u32 index;

	if (!diagnosticSecondaryPwmStreamActive)
	{
		if (!diagnosticRuntimeActive)
			sec_pwm_tone_handler();
		return;
	}
	index = diagnosticSecondaryPwmSampleIndex;
	MARS_PWM_MONO = diagnosticSoundPattern[index];
	index++;
	if (index >= diagnosticSoundPatternWords)
		index = 0;
	diagnosticSecondaryPwmSampleIndex = index;
}

static int drainPwmFifos(DiagnosticResult *result, u8 phase)
{
	DiagnosticDeadline deadline;
	u16 left;
	u16 right;

	MARS_PWM_CTRL = 0;
	MARS_PWM_CYCLE = 0x0209;
	MARS_PWM_CTRL = 5;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_FIFO);
	left = MARS_PWM_LEFT;
	right = MARS_PWM_RIGHT;
	while ((!(left & 0x4000) || !(right & 0x4000)) &&
		diagnosticDeadlineActive(&deadline))
	{
		left = MARS_PWM_LEFT;
		right = MARS_PWM_RIGHT;
	}
	MARS_PWM_CTRL = 0;
	if (!(left & 0x4000) || !(right & 0x4000))
	{
		result->actual = ((u32)left << 16) | right;
		resultTimeout(result, !(left & 0x4000) ?
			0x20004034 : 0x20004036, phase,
			diagnosticDeadlineRemaining(&deadline));
		return 0;
	}
	return 1;
}

static DiagnosticStatus playPwmInterrupt(DiagnosticResult *result)
{
	volatile u8 *interruptMask = (volatile u8 *)0x20004001;
	volatile u32 *counter = result->cpu == DIAG_TARGET_SLAVE ?
		&diagnosticSecondaryPwmCount : &diagnosticPrimaryPwmCount;
	volatile u32 *frameCounter = diagnosticFrameCounter(result->cpu);
	volatile u8 *streamActive = result->cpu == DIAG_TARGET_SLAVE ?
		&diagnosticSecondaryPwmStreamActive :
		&diagnosticPrimaryPwmStreamActive;
	volatile u32 *sampleIndex = result->cpu == DIAG_TARGET_SLAVE ?
		&diagnosticSecondaryPwmSampleIndex :
		&diagnosticPrimaryPwmSampleIndex;
	u16 savedControl = MARS_PWM_CTRL;
	u16 savedCycle = MARS_PWM_CYCLE;
	u8 savedMask = *interruptMask;
	u32 before = *counter;
	u32 startFrame = *frameCounter;
	u32 actualFrame;
	u32 remaining;
	int durationComplete;
	int oldLevel = SetSH2SR(15);

	if (!drainPwmFifos(result, 1))
	{
		MARS_PWM_CYCLE = savedCycle;
		MARS_PWM_CTRL = savedControl;
		SetSH2SR(oldLevel);
		return result->status;
	}
	*streamActive = 0;
	*sampleIndex = 0;
	*interruptMask &= (u8)~1;
	MARS_SYS_PWMI_CLR = 0;
	MARS_PWM_CTRL = 0;
	MARS_PWM_CYCLE = 0x0209;
	MARS_PWM_MONO = 0x0200;
	MARS_PWM_MONO = 0x0200;
	MARS_PWM_MONO = 0x0200;
	*streamActive = 1;
	MARS_PWM_CTRL = 0x0105;
	*interruptMask |= 1;
	SetSH2SR(2);
	durationComplete = waitCounterFrames(frameCounter, 60,
		DIAG_TIMEOUT_SEGA_CD, &actualFrame, &remaining);
	SetSH2SR(15);
	*streamActive = 0;
	*interruptMask = savedMask;
	MARS_SYS_PWMI_CLR = 0;
	MARS_PWM_CTRL = 0;
	if (!drainPwmFifos(result, 3))
		durationComplete = 0;
	MARS_PWM_CYCLE = savedCycle;
	MARS_PWM_CTRL = savedControl;
	SetSH2SR(oldLevel);
	if (result->status == DIAG_TIMEOUT)
		return DIAG_TIMEOUT;
	if (!durationComplete)
	{
		result->expected = startFrame + 60;
		result->actual = actualFrame;
		return resultTimeout(result, (u32)frameCounter, 2, remaining);
	}
	if (*counter == before)
	{
		result->expected = before + 1;
		result->actual = *counter;
		return resultTimeout(result, (u32)counter, 1,
			remaining);
	}
	result->status = DIAG_MANUAL;
	return DIAG_MANUAL;
}

static DiagnosticStatus playPwmDma(DiagnosticResult *result)
{
	volatile u16 *samples = DIAG_SCRATCH_GENERAL;
	u32 sampleCount = 0x2C00;
	u32 savedSar = SH2_DMA_SAR1;
	u32 savedDar = SH2_DMA_DAR1;
	u32 savedTcr = SH2_DMA_TCR1;
	u32 savedChcr = SH2_DMA_CHCR1;
	u32 savedDmaor = SH2_DMA_DMAOR;
	u8 savedDrcr = SH2_DMA_DRCR1;
	u16 savedControl = MARS_PWM_CTRL;
	u16 savedCycle = MARS_PWM_CYCLE;
	volatile u32 *frameCounter = diagnosticFrameCounter(result->cpu);
	u32 startFrame;
	u32 actual = 0;
	u32 stopActual;
	u32 deadlineRemaining = 0;
	u32 transferRemaining = 0;
	DiagnosticDeadline duration;
	int i;
	int quiescent;
	int timedOut = 0;

	if (!drainPwmFifos(result, 1))
	{
		MARS_PWM_CYCLE = savedCycle;
		MARS_PWM_CTRL = savedControl;
		return result->status;
	}
	for (i = 0; i < (int)sampleCount; i++)
		samples[i] = diagnosticSoundPattern[
			(i >> 1) % diagnosticSoundPatternWords];
	SH2_DMA_CHCR1 = 0;
	SH2_DMA_DMAOR = 0;
	MARS_PWM_CTRL = 0;
	MARS_PWM_CYCLE = 0x0209;
	MARS_PWM_MONO = samples[sampleCount - 1];
	MARS_PWM_MONO = samples[sampleCount - 1];
	MARS_PWM_MONO = samples[sampleCount - 1];
	MARS_PWM_CTRL = 0x0185;
	SH2_DMA_DRCR1 = 0;
	startFrame = *frameCounter;
	diagnosticDeadlineStart(&duration, DIAG_TIMEOUT_SEGA_CD);
	while (*frameCounter - startFrame < 60 &&
		diagnosticDeadlineActive(&duration))
	{
		SH2_DMA_CHCR1 = 0;
		SH2_DMA_DMAOR = 0;
		SH2_DMA_SAR1 = 0x06010000;
		SH2_DMA_DAR1 = 0x20004034;
		SH2_DMA_TCR1 = sampleCount;
		SH2_DMA_CHCR1 = 0x000018E1;
		SH2_DMA_DMAOR = 1;
		actual = SH2_DMA_CHCR1;
		while (!(actual & 2) && *frameCounter - startFrame < 60 &&
			diagnosticDeadlineActive(&duration))
			actual = SH2_DMA_CHCR1;
	}
	deadlineRemaining = diagnosticDeadlineRemaining(&duration);
	transferRemaining = SH2_DMA_TCR1;
	if (*frameCounter - startFrame < 60)
		timedOut = 1;
	quiescent = stopDma(&SH2_DMA_CHCR1);
	stopActual = SH2_DMA_CHCR1;
	MARS_PWM_CTRL = 0;
	SH2_DMA_SAR1 = savedSar;
	SH2_DMA_DAR1 = savedDar;
	SH2_DMA_TCR1 = savedTcr;
	SH2_DMA_CHCR1 = savedChcr;
	SH2_DMA_DRCR1 = savedDrcr;
	SH2_DMA_DMAOR = savedDmaor;
	if (!drainPwmFifos(result, 3))
		timedOut = 1;
	MARS_PWM_CYCLE = savedCycle;
	MARS_PWM_CTRL = savedControl;
	if (!quiescent)
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->address = 0xFFFFFF9C;
		result->actual = stopActual;
		return DIAG_INFRASTRUCTURE_ERROR;
	}
	if (timedOut)
	{
		if (result->status == DIAG_TIMEOUT)
			return DIAG_TIMEOUT;
		result->actual = actual;
		return resultTimeout(result, 0xFFFFFF9C, 1,
			transferRemaining ? transferRemaining : deadlineRemaining);
	}
	result->status = DIAG_MANUAL;
	return DIAG_MANUAL;
}

static DiagnosticStatus playPwm(u16 catalog, DiagnosticResult *result)
{
	volatile u16 *fifo = &MARS_PWM_MONO;
	u16 savedControl = MARS_PWM_CTRL;
	u16 savedCycle = MARS_PWM_CYCLE;
	u16 route = 5;
	volatile u32 *frameCounter = diagnosticFrameCounter(result->cpu);
	u32 startFrame;
	u32 durationFrames = 60;
	u32 sampleIndex = 0;
	DiagnosticDeadline duration;
	int switched = 0;

	if (catalog == 150 || catalog == 151)
		return playPwmInterrupt(result);
	if (catalog == 152 || catalog == 153)
		return playPwmDma(result);

	if (catalog == 141 || catalog == 146)
	{
		fifo = &MARS_PWM_LEFT;
		route = 1;
	}
	else if (catalog == 142 || catalog == 147)
	{
		fifo = &MARS_PWM_RIGHT;
		route = 4;
	}
	else if (catalog == 143 || catalog == 148)
	{
		fifo = &MARS_PWM_LEFT;
		route = 1;
		durationFrames = 120;
	}
	else if (catalog == 144 || catalog == 149)
	{
		fifo = &MARS_PWM_RIGHT;
		route = 4;
		durationFrames = 120;
	}
	if (!drainPwmFifos(result, 1))
	{
		MARS_PWM_CYCLE = savedCycle;
		MARS_PWM_CTRL = savedControl;
		return result->status;
	}
	MARS_PWM_CTRL = 0;
	MARS_PWM_MONO = 0;
	MARS_PWM_MONO = 0;
	MARS_PWM_MONO = 0;
	MARS_PWM_CYCLE = 0x0209;
	MARS_PWM_CTRL = route;
	startFrame = *frameCounter;
	diagnosticDeadlineStart(&duration, DIAG_TIMEOUT_SEGA_CD);
	while (*frameCounter - startFrame < durationFrames &&
		diagnosticDeadlineActive(&duration))
	{
		DiagnosticDeadline deadline;
		u32 elapsed = *frameCounter - startFrame;

		diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_FIFO);
		while ((*fifo & 0x8000) &&
			*frameCounter - startFrame < durationFrames &&
			diagnosticDeadlineActive(&deadline) &&
			diagnosticDeadlineActive(&duration))
			__asm__ volatile ("" ::: "memory");
		if (*frameCounter - startFrame >= durationFrames)
			break;
		if (*fifo & 0x8000)
		{
			result->expected = 0;
			result->actual = *fifo;
			MARS_PWM_CTRL = 0;
			MARS_PWM_CYCLE = savedCycle;
			MARS_PWM_CTRL = savedControl;
			return resultTimeout(result, (u32)fifo, 1,
				diagnosticDeadlineRemaining(&deadline));
		}
		*fifo = diagnosticSoundPattern[sampleIndex];
		sampleIndex++;
		if (sampleIndex >= diagnosticSoundPatternWords)
			sampleIndex = 0;
		if (!switched && elapsed >= 60 &&
			(catalog == 143 || catalog == 148))
		{
			MARS_PWM_CTRL = 8;
			switched = 1;
		}
		else if (!switched && elapsed >= 60 &&
			(catalog == 144 || catalog == 149))
		{
			MARS_PWM_CTRL = 2;
			switched = 1;
		}
	}
	MARS_PWM_CTRL = 0;
	if (!drainPwmFifos(result, 3))
	{
		MARS_PWM_CYCLE = savedCycle;
		MARS_PWM_CTRL = savedControl;
		return result->status;
	}
	MARS_PWM_CYCLE = savedCycle;
	MARS_PWM_CTRL = savedControl;
	if (*frameCounter - startFrame < durationFrames)
	{
		result->expected = startFrame + durationFrames;
		result->actual = *frameCounter;
		return resultTimeout(result, (u32)frameCounter, 2,
			diagnosticDeadlineRemaining(&duration));
	}
	result->status = DIAG_MANUAL;
	return DIAG_MANUAL;
}

static void displayLineTable(volatile u16 *framebuffer, u16 pitch)
{
	int line;

	for (line = 0; line < 224; line++)
		framebuffer[line] = 0x0100 + line * pitch;
}

static u16 diagnosticResourceColor(int index)
{
	int offset = index * 3;

	return ((ebu_Palette[offset] >> 3) & 31) |
		(((ebu_Palette[offset + 1] >> 3) & 31) << 5) |
		(((ebu_Palette[offset + 2] >> 3) & 31) << 10) |
		0x8000;
}

static DiagnosticStatus graphicsWaitFrame(DiagnosticResult *result,
	u8 phase)
{
	u32 before = diagnosticPrimaryVblankCount;
	DiagnosticDeadline deadline;

	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_INTERRUPT);
	while (diagnosticPrimaryVblankCount == before &&
		diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (diagnosticPrimaryVblankCount == before)
	{
		result->expected = before + 1;
		result->actual = diagnosticPrimaryVblankCount;
		return resultTimeout(result,
			(u32)&diagnosticPrimaryVblankCount, phase,
			diagnosticDeadlineRemaining(&deadline));
	}
	return DIAG_PASS;
}

static DiagnosticStatus graphicsTest(u16 catalog, DiagnosticResult *result)
{
	volatile u16 *framebuffer = (volatile u16 *)0x24000000;
	volatile u16 *palette = (volatile u16 *)0x20004200;
	u16 savedMode = MARS_VDP_DISPMODE;
	u16 savedShift = MARS_VDP_SHIFTREG;
	u16 savedFbctl = SH2_VDP_CONTROL;
	u16 savedPalette[8];
	volatile u16 modes[224];
	DiagnosticStatus status = DIAG_MANUAL;
	u32 remaining;
	int line;

	for (line = 0; line < 8; line++)
	{
		savedPalette[line] = palette[line];
		palette[line] = diagnosticResourceColor(line);
	}

	if (catalog == 157)
	{
		for (line = 0; line < 224; line++)
			framebuffer[line] = 0x0100 +
				(line < 200 ? line : 199) * 320;
		for (line = 0; line < 200; line++)
		{
			int x;
			volatile u16 *pixels = framebuffer + 0x100 + line * 320;

			for (x = 0; x < 320; x++)
				pixels[x] = diagnosticResourceColor(x / 40);
		}
		MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_VDP_MODE_32K;
	}
	else if (catalog == 158 || catalog == 161)
	{
		displayLineTable(framebuffer, 160);
		for (line = 0; line < 224; line++)
		{
			int x;
			volatile u8 *pixels = (volatile u8 *)(framebuffer + 0x100 +
				line * 160);

			for (x = 0; x < 320; x++)
				pixels[x] = x / 40;
		}
		MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_VDP_MODE_256;
	}
	else if (catalog == 159)
	{
		volatile u16 *encoded = framebuffer + 0x0100;
		u16 offset = 0x0100;

		for (line = 0; line < 224; line++)
		{
			int bar;

			framebuffer[line] = offset;
			for (bar = 0; bar < 8; bar++)
				*encoded++ = (39 << 8) | bar;
			offset += 8;
		}
		MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_VDP_MODE_RLE;
	}
	else
	{
		volatile u16 *encoded = framebuffer + 0x0100;
		u16 offset = 0x0100;

		for (line = 0; line < 224; line++)
		{
			int x;

			framebuffer[line] = offset;
			if (line < 74)
			{
				volatile u8 *pixels = (volatile u8 *)encoded;

				modes[line] = MARS_VDP_PRIO_32X | MARS_VDP_MODE_256;
				for (x = 0; x < 320; x++)
					pixels[x] = x / 40;
				encoded += 160;
				offset += 160;
			}
			else if (line < 149)
			{
				modes[line] = MARS_VDP_PRIO_32X | MARS_VDP_MODE_32K;
				for (x = 0; x < 320; x++)
					*encoded++ = diagnosticResourceColor(x / 40);
				offset += 320;
			}
			else
			{
				modes[line] = MARS_VDP_PRIO_32X | MARS_VDP_MODE_RLE;
				for (x = 0; x < 8; x++)
					*encoded++ = (39 << 8) | x;
				offset += 8;
			}
		}
		MARS_VDP_DISPMODE = modes[0];
	}
	if (status == DIAG_MANUAL)
	{
		int settle = catalog == 161 ? 0 : 2;
		int frame;

		for (frame = 0; frame < settle; frame++)
		{
			status = graphicsWaitFrame(result, 2);
			if (status != DIAG_PASS)
				break;
		}
		if (status == DIAG_PASS || status == DIAG_MANUAL)
		{
			u32 startFrame = diagnosticPrimaryVblankCount;

			status = DIAG_MANUAL;
			while (diagnosticPrimaryVblankCount - startFrame < 300)
			{
				if (catalog == 160)
				{
					u32 before = diagnosticPrimaryVblankCount;
					u32 hblankRemaining;

					if (diagnosticHblankModes(modes, 224,
						&hblankRemaining))
					{
						result->expected = 0;
						result->actual = MARS_VDP_FBCTL;
						status = resultTimeout(result, 0x2000410A,
							1, hblankRemaining);
						break;
					}
					if (diagnosticPrimaryVblankCount == before)
					{
						status = graphicsWaitFrame(result, 2);
						if (status != DIAG_PASS)
							break;
						status = DIAG_MANUAL;
					}
				}
				else
				{
					status = graphicsWaitFrame(result, 2);
					if (status != DIAG_PASS)
						break;
					status = DIAG_MANUAL;
				}
			}
		}
	}
	marsVDP256Start();
	if (!selectFramebuffer(savedFbctl & 1, &remaining))
	{
		status = DIAG_INFRASTRUCTURE_ERROR;
		result->status = status;
		result->phase = 3;
		result->address = 0x2000410A;
		result->remaining = remaining;
	}
	for (line = 0; line < 8; line++)
		palette[line] = savedPalette[line];
	MARS_VDP_SHIFTREG = savedShift;
	MARS_VDP_DISPMODE = savedMode;
	if (status != DIAG_MANUAL)
		return status;
	result->status = DIAG_MANUAL;
	return DIAG_MANUAL;
}

static DiagnosticStatus __attribute__((section(".ramcode"), noinline))
	testCrossDreqControl(u16 saved, DiagnosticResult *result)
{
	volatile u16 *reg = (volatile u16 *)0x20004006;
	DiagnosticStatus status = DIAG_PASS;
	int pattern;

	for (pattern = 7; pattern >= 0; pattern--)
	{
		DiagnosticDeadline deadline;
		u16 phase;
		u16 actual;

		diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
		phase = MARS_SYS_COMM2;
		while (phase != (DIAG_DREQ_PATTERN | pattern) &&
			diagnosticDeadlineActive(&deadline))
			phase = MARS_SYS_COMM2;
		if (phase != (DIAG_DREQ_PATTERN | pattern))
		{
			*reg = saved;
			result->status = DIAG_TIMEOUT;
			result->phase = pattern + 1;
			result->address = 0x20004022;
			result->remaining = diagnosticDeadlineRemaining(&deadline);
			return DIAG_TIMEOUT;
		}
		actual = *reg & 0x0087;
		if (status == DIAG_PASS && actual != (u16)pattern)
		{
			status = DIAG_FAIL;
			result->status = DIAG_FAIL;
			result->expected = pattern;
			result->actual = actual;
			result->address = (u32)reg;
		}
		MARS_SYS_COMM2 = DIAG_DREQ_ACK | actual;
	}
	*reg = saved;
	if (status == DIAG_PASS)
		result->status = DIAG_PASS;
	return status;
}

static DiagnosticStatus testCrossCpu(u16 catalog,
	DiagnosticResult *result)
{
	volatile u32 *crossState = (volatile u32 *)DIAG_SCRATCH_GENERAL;

	if (catalog == 39 || catalog == 40)
		return testCommunication(catalog, result);
	if (catalog == 41 || catalog == 42)
	{
		volatile u8 *reg = (volatile u8 *)0x20004000;
		u8 actual = *reg;

		*reg = (u8)crossState[0];
		return (actual & 0x80) == (crossState[1] & 0x80) ?
			resultPass(result) : resultFail(result, (u32)reg,
				crossState[1] & 0x80, actual & 0x80);
	}
	if (catalog == 43 || catalog == 44)
		return testCrossDreqControl((u16)crossState[0], result);
	if (catalog == 45 || catalog == 46)
	{
		volatile u32 *reg = (volatile u32 *)0x20004008;
		u32 actual = *reg;

		*reg = crossState[0];
		return (actual & 0x00FFFFFF) ==
			(crossState[1] & 0x00FFFFFF) ? resultPass(result) :
			resultFail(result, (u32)reg,
				crossState[1] & 0x00FFFFFF, actual & 0x00FFFFFF);
	}
	if (catalog == 47 || catalog == 48)
	{
		volatile u32 *reg = (volatile u32 *)0x2000400C;
		u32 actual = *reg;

		*reg = crossState[0];
		return (actual & 0x00FFFFFF) ==
			(crossState[1] & 0x00FFFFFF) ? resultPass(result) :
			resultFail(result, (u32)reg,
				crossState[1] & 0x00FFFFFF, actual & 0x00FFFFFF);
	}
	if (catalog == 49 || catalog == 50)
	{
		volatile u16 *reg = (volatile u16 *)0x20004010;
		u16 actual = *reg;

		*reg = (u16)crossState[0];
		return (actual & 0xFFFC) == (crossState[1] & 0xFFFC) ?
			resultPass(result) : resultFail(result, (u32)reg,
				crossState[1] & 0xFFFC, actual & 0xFFFC);
	}
	if (catalog >= 51 && catalog <= 58)
	{
		volatile u8 *interruptMask = (volatile u8 *)0x20004001;
		volatile u8 *hblankCount = (volatile u8 *)0x20004005;
		volatile u16 *phase = DIAG_SCRATCH_SECONDARY + 16;
		volatile u32 *counter;
		DiagnosticResult mdResult = *result;
		u8 savedMask = *interruptMask;
		u8 savedHblank = *hblankCount;
		u32 before;
		DiagnosticDeadline deadline;

		if (catalog <= 52)
		{
			counter = (catalog & 1) ? &diagnosticPrimaryVblankCount :
				&diagnosticSecondaryVblankCount;
			*interruptMask = savedMask | 0x08;
		}
		else
		{
			counter = (catalog & 1) ? &diagnosticPrimaryHblankCount :
				&diagnosticSecondaryHblankCount;
			*hblankCount = (catalog - 53) >> 1;
			*interruptMask = savedMask | 0x04;
		}
		before = *counter;
		*phase = DIAG_CROSS_READY;
		if (result->cpu == DIAG_TARGET_MASTER)
		{
			if (!diagnosticMdCommand(DIAG_MD_EXECUTE, catalog,
				&mdResult) || mdResult.status != DIAG_PASS)
			{
				*interruptMask = savedMask;
				*hblankCount = savedHblank;
				*phase = 0;
				*result = mdResult;
				if (!result->address)
					result->address = 0x20004003;
				return result->status;
			}
		}
		diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_INTERRUPT);
		while (*counter == before && diagnosticDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		*interruptMask = savedMask;
		*hblankCount = savedHblank;
		*phase = 0;
		if (*counter == before)
		{
			result->expected = before + 1;
			result->actual = *counter;
			return resultTimeout(result, (u32)counter, 1,
				diagnosticDeadlineRemaining(&deadline));
		}
		return resultPass(result);
	}
	return resultFail(result, catalog, 1, 0);
}

static DiagnosticStatus executeLocal(const DiagnosticDescriptor *descriptor,
	DiagnosticResult *result)
{
	switch (descriptor->operation)
	{
		case DIAG_OP_SH2_SYSTEM:
			return testSh2System(descriptor->catalog, result);
		case DIAG_OP_CROSS_CPU:
			return testCrossCpu(descriptor->catalog, result);
		case DIAG_OP_SH2_VDP:
			return testSh2Vdp(descriptor->catalog, result);
		case DIAG_OP_FRAMEBUFFER:
			return testFramebufferMemory(descriptor->catalog, result);
		case DIAG_OP_PALETTE:
			return testPalette(descriptor->catalog, result);
		case DIAG_OP_DMA:
			return testDma(descriptor->catalog, result);
		case DIAG_OP_SOUND:
			return playPwm(descriptor->catalog, result);
		case DIAG_OP_GRAPHICS:
			return graphicsTest(descriptor->catalog, result);
		default:
			return resultFail(result, descriptor->catalog, 1, 0);
	}
}

DiagnosticStatus diagnosticExecuteSecondary(u16 catalog,
	DiagnosticResult *result)
{
	if (catalog < 1 || catalog > DIAGNOSTIC_TEST_COUNT)
		return resultFail(result, catalog, 1, 0);
	result->cpu = DIAG_TARGET_SLAVE;
	if (catalog == 1 || catalog == 16 || catalog == 17 || catalog == 39 ||
		catalog == 40)
		return testCommunicationPeer(catalog, result);
	return executeLocal(&diagnosticManifest[catalog - 1], result);
}

static DiagnosticStatus executeMd(const DiagnosticDescriptor *descriptor,
	DiagnosticResult *result)
{
	if (!diagnosticMdCommand(DIAG_MD_EXECUTE, descriptor->catalog, result))
		return result->status;
	if (result->status != DIAG_PASS && result->status != DIAG_MANUAL &&
		result->status != DIAG_UNAVAILABLE && !result->address)
	{
		switch (descriptor->catalog)
		{
			case 2: result->address = 0xA15100; break;
			case 3: result->address = 0xA15103; break;
			case 4: result->address = 0x9FFFFC; break;
			case 5: result->address = 0xA15106; break;
			case 6: result->address = 0xA15108; break;
			case 7: result->address = 0xA1510C; break;
			case 8: result->address = 0xA15110; break;
			case 9: result->address = 0xA1511B; break;
			case 10: result->address = 0xC00004; break;
			case 11: result->address = 0xA15130; break;
			case 12: result->address = 0xA15132; break;
			case 13: result->address = 0xA15134; break;
			case 14: result->address = 0xA15136; break;
			case 15: result->address = 0xA15138; break;
			case 41:
			case 42: result->address = 0xA15100; break;
			case 43:
			case 44: result->address = 0xA15106; break;
			case 45:
			case 46: result->address = 0xA15108; break;
			case 47:
			case 48: result->address = 0xA1510C; break;
			case 49:
			case 50: result->address = 0xA15110; break;
			case 59: result->address = 0xA15180; break;
			case 60: result->address = 0xA15182; break;
			case 61: result->address = 0xA15185; break;
			case 62: result->address = 0xA15186; break;
			case 63:
			case 64:
			case 65:
			case 66: result->address = 0xA1518A; break;
			case 83:
			case 84:
			case 89:
			case 90:
			case 91:
			case 92:
			case 101:
			case 102: result->address = 0x840200; break;
			case 103:
			case 104: result->address = 0xA15186; break;
			case 113:
			case 114:
			case 115: result->address = 0xA15200; break;
			case 122:
			case 123: result->address = 0xA15112; break;
			case 124:
			case 125: result->address = 0x600000; break;
			case 126: result->address = 0xC00000; break;
			case 135:
			case 138:
			case 139: result->address = 0xA15138; break;
			case 136:
			case 155: result->address = 0xA15134; break;
			case 137:
			case 156: result->address = 0xA15136; break;
			case 154: result->address = 0xA15138; break;
			default: result->address = descriptor->catalog; break;
		}
	}
	return result->status;
}

static u32 crossRegisterAddress(u16 catalog)
{
	if (catalog <= 42)
		return 0x20004000;
	if (catalog <= 44)
		return 0x20004006;
	if (catalog <= 46)
		return 0x20004008;
	if (catalog <= 48)
		return 0x2000400C;
	return 0x20004010;
}

static void restoreCrossRegister(u16 catalog, u32 address, u32 saved)
{
	if (catalog <= 42)
		*(volatile u8 *)address = (u8)saved;
	else if (catalog <= 44 || catalog >= 49)
		*(volatile u16 *)address = (u16)saved;
	else
		*(volatile u32 *)address = saved;
}

static DiagnosticStatus executeCrossRegister(
	const DiagnosticDescriptor *descriptor, DiagnosticResult *result)
{
	volatile u32 *crossState = (volatile u32 *)DIAG_SCRATCH_GENERAL;
	u32 address = crossRegisterAddress(descriptor->catalog);
	u32 saved;
	u32 expected;
	DiagnosticStatus status;

	if (descriptor->catalog <= 42)
	{
		saved = *(volatile u8 *)address;
		expected = saved ^ 0x80;
	}
	else if (descriptor->catalog <= 44)
	{
		saved = *(volatile u16 *)address;
		expected = saved ^ 0x0007;
	}
	else if (descriptor->catalog <= 48)
	{
		saved = *(volatile u32 *)address;
		expected = 0x06020000;
	}
	else
	{
		saved = *(volatile u16 *)address;
		expected = (saved & 0x0003) | 0xA55C;
	}
	crossState[0] = saved;
	crossState[1] = expected;
	if (descriptor->catalog == 43)
	{
		status = executeMd(descriptor, result);
		restoreCrossRegister(descriptor->catalog, address, saved);
		if (status == DIAG_PASS)
			result->cpu = DIAG_TARGET_MASTER;
		return status;
	}
	if (descriptor->catalog == 44)
	{
		DiagnosticResult secondary = *result;
		DiagnosticResult md = *result;
		int secondaryOk;

		secondary.cpu = DIAG_TARGET_SLAVE;
		if (!diagnosticSecondaryBegin(descriptor->catalog, &secondary))
		{
			restoreCrossRegister(descriptor->catalog, address, saved);
			*result = secondary;
			return result->status;
		}
		status = executeMd(descriptor, &md);
		secondaryOk = diagnosticSecondaryFinish(&secondary);
		restoreCrossRegister(descriptor->catalog, address, saved);
		if (status != DIAG_PASS)
		{
			*result = md;
			return result->status;
		}
		if (!secondaryOk || secondary.status != DIAG_PASS)
		{
			*result = secondary;
			return result->status;
		}
		result->status = DIAG_PASS;
		result->cpu = DIAG_TARGET_SLAVE;
		return DIAG_PASS;
	}
	status = executeMd(descriptor, result);
	if (status != DIAG_PASS)
	{
		restoreCrossRegister(descriptor->catalog, address, saved);
		return status;
	}
	if (descriptor->target == DIAG_TARGET_MD_MASTER)
	{
		result->cpu = DIAG_TARGET_MASTER;
		status = executeLocal(descriptor, result);
	}
	else if (!diagnosticSecondaryCommand(descriptor->catalog, result))
		status = result->status;
	else
		status = result->status;
	restoreCrossRegister(descriptor->catalog, address, saved);
	return status;
}

static DiagnosticStatus executeCrossInterruptSlave(
	const DiagnosticDescriptor *descriptor, DiagnosticResult *result)
{
	volatile u16 *phase = DIAG_SCRATCH_SECONDARY + 16;
	DiagnosticResult mdResult = *result;
	DiagnosticDeadline deadline;
	int mdOk;

	*phase = 0;
	if (!diagnosticSecondaryBegin(descriptor->catalog, result))
		return result->status;
	diagnosticDeadlineStart(&deadline, DIAG_TIMEOUT_CROSS_CPU);
	while (*phase != DIAG_CROSS_READY &&
		diagnosticDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (*phase != DIAG_CROSS_READY)
	{
		DiagnosticResult cleanup = *result;

		diagnosticSecondaryFinish(&cleanup);
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->cpu = DIAG_TARGET_SLAVE;
		result->phase = 1;
		result->address = (u32)phase;
		result->remaining = diagnosticDeadlineRemaining(&deadline);
		return DIAG_INFRASTRUCTURE_ERROR;
	}
	mdOk = diagnosticMdCommand(DIAG_MD_EXECUTE, descriptor->catalog,
		&mdResult) && mdResult.status == DIAG_PASS;
	if (!diagnosticSecondaryFinish(result))
		return result->status;
	*phase = 0;
	if (!mdOk)
	{
		*result = mdResult;
		if (!result->address)
			result->address = 0x20004003;
		return result->status;
	}
	return result->status;
}

DiagnosticStatus diagnosticsExecute(const DiagnosticDescriptor *descriptor,
	DiagnosticResult *result)
{
	DiagnosticStatus status;

	result->cpu = descriptor->target;
	if (descriptor->catalog == 124 || descriptor->catalog == 125)
		return resultPass(result);
	if (descriptor->catalog == 1 || descriptor->catalog == 16 ||
		descriptor->catalog == 17 || descriptor->catalog == 39 ||
		descriptor->catalog == 40)
		return diagnosticCommunicationTransaction(descriptor->catalog,
			result);
	if (descriptor->catalog >= 41 && descriptor->catalog <= 50)
		return executeCrossRegister(descriptor, result);
	if (descriptor->catalog >= 51 && descriptor->catalog <= 58)
	{
		if (descriptor->target == DIAG_TARGET_MD_MASTER)
		{
			result->cpu = DIAG_TARGET_MASTER;
			return executeLocal(descriptor, result);
		}
		return executeCrossInterruptSlave(descriptor, result);
	}
	if ((descriptor->requirements & DIAG_NEEDS_SEGA_CD) &&
		!diagnosticContext.segaCd)
	{
		result->status = DIAG_UNAVAILABLE;
		return DIAG_UNAVAILABLE;
	}
	if (descriptor->catalog == 124 || descriptor->catalog == 125)
	{
		DiagnosticResult prepared = *result;

		if (!diagnosticMdCommand(DIAG_MD_PREPARE,
			descriptor->catalog, &prepared) || prepared.status != DIAG_PASS)
		{
			*result = prepared;
			return result->status;
		}
	}
	if (descriptor->target == DIAG_TARGET_MD ||
		descriptor->target == DIAG_TARGET_Z80)
	{
		if (descriptor->catalog == 115)
		{
			volatile u16 *fillWords = (volatile u16 *)0x2400E000;
			u16 fillSaved[256];
			int i;

			for (i = 0; i < 256; i++)
				fillSaved[i] = fillWords[i];
			status = executeMd(descriptor, result);
			for (i = 0; i < 256; i++)
				fillWords[i] = fillSaved[i];
			return status;
		}
		return executeMd(descriptor, result);
	}
	if (descriptor->target == DIAG_TARGET_SLAVE)
	{
		if (!diagnosticSecondaryCommand(descriptor->catalog, result))
			return result->status;
		return result->status;
	}
	if (descriptor->target == DIAG_TARGET_MD_MASTER)
	{
		if (descriptor->catalog == 122)
			return executeLocal(descriptor, result);
		status = executeMd(descriptor, result);
		if (status != DIAG_PASS && status != DIAG_MANUAL)
			return status;
		return executeLocal(descriptor, result);
	}
	if (descriptor->target == DIAG_TARGET_MD_SLAVE)
	{
		if (descriptor->catalog == 123)
		{
			if (!diagnosticSecondaryCommand(descriptor->catalog, result))
				return result->status;
			return result->status;
		}
		status = executeMd(descriptor, result);
		if (status != DIAG_PASS && status != DIAG_MANUAL)
			return status;
		if (!diagnosticSecondaryCommand(descriptor->catalog, result))
			return result->status;
		return result->status;
	}
	if (descriptor->target == DIAG_TARGET_SEGA_CD_MASTER)
	{
		DiagnosticResult secondary = *result;

		if (!diagnosticSecondaryDmaIsolation(1, &secondary))
		{
			*result = secondary;
			return result->status;
		}
		status = executeLocal(descriptor, result);
		secondary = *result;
		if (!diagnosticSecondaryDmaIsolation(0, &secondary))
		{
			*result = secondary;
			return result->status;
		}
		return status;
	}
	if (descriptor->target == DIAG_TARGET_SEGA_CD_SLAVE)
	{
		u8 savedIntmsk = *(volatile u8 *)0x20004001;
		u32 savedChcr0 = SH2_DMA_CHCR0;
		int completed;

		SH2_DMA_CHCR0 = 0;
		*(volatile u8 *)0x20004001 = 2;
		completed = diagnosticSecondaryCommand(descriptor->catalog, result);
		*(volatile u8 *)0x20004001 = savedIntmsk;
		SH2_DMA_CHCR0 = savedChcr0;
		if (!completed)
			return result->status;
		return result->status;
	}
	if (descriptor->target == DIAG_TARGET_MASTER_SLAVE)
	{
		DiagnosticResult secondary = *result;

		secondary.cpu = DIAG_TARGET_SLAVE;
		if (!diagnosticSecondaryBegin(descriptor->catalog, &secondary))
		{
			*result = secondary;
			return result->status;
		}
		result->cpu = DIAG_TARGET_MASTER;
		status = executeLocal(descriptor, result);
		if (!diagnosticSecondaryFinish(&secondary))
		{
			*result = secondary;
			return result->status;
		}
		if (status != DIAG_PASS)
			return status;
		if (secondary.status != DIAG_PASS)
		{
			*result = secondary;
			return result->status;
		}
		return status;
	}
	return executeLocal(descriptor, result);
}
