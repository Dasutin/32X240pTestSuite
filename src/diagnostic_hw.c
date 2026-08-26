#include "32x.h"
#include "diagnostics.h"
#include "diagnostic_hw.h"
#include "hw_32x.h"
#include "shared_objects.h"
#include "sound.h"

#define SH2_VDP_CONTROL (*(volatile u8 *)0x2000410B)

typedef struct
{
	u16 status;
	u16 cpu;
	u16 phase;
	u16 reserved;
	u32 expected;
	u32 actual;
	u32 address;
	u32 remaining;
} DiagnosticPeerWire;

typedef struct
{
	u16 intmsk;
	u16 dmactr;
	u32 dmasar;
	u32 dmadar;
	u16 dmalen;
	u16 pwmCtrl;
	u16 pwmCycle;
	u16 vdpMode;
	u16 vdpShift;
	u16 vdpFillLength;
	u16 vdpFillAddress;
	u16 vdpFbctl;
	u8 sciSmr;
	u8 sciBrr;
	u8 sciScr;
	u8 sciTdr;
	u8 sciSsr;
	u8 drcr0;
	u8 drcr1;
	u32 dmaor;
	u32 sar0;
	u32 dar0;
	u32 tcr0;
	u32 chcr0;
	u32 sar1;
	u32 dar1;
	u32 tcr1;
	u32 chcr1;
	u32 vbr;
	u32 sr;
} DiagnosticSavedState;

static DiagnosticSavedState savedState;
static DiagnosticSavedState secondarySavedState;
static u8 runtimeEntered;
static u8 secondaryRequested;
static u8 secondaryEntered;
static u8 mdRequested;
static u8 mdEntered;
static u8 stateSaved;
volatile u8 diagnosticRuntimeActive;

volatile u32 diagnosticPrimaryVblankCount;
volatile u32 diagnosticPrimaryHblankCount;
volatile u32 diagnosticPrimaryCommandCount;
volatile u32 diagnosticPrimaryPwmCount;
volatile u32 diagnosticPrimarySciCount;
volatile u8 diagnosticPrimarySciValue;
volatile u32 diagnosticSecondaryVblankCount;
volatile u32 diagnosticSecondaryHblankCount;
volatile u32 diagnosticSecondaryCommandCount;
volatile u32 diagnosticSecondaryPwmCount;
volatile u32 diagnosticSecondarySciCount;
volatile u8 diagnosticSecondarySciValue;
volatile u16 diagnosticSecondaryDmaReleaseFlag;

extern DiagnosticStatus diagnosticExecuteSecondary(u16 catalog,
	DiagnosticResult *result);

enum
{
	DIAG_STEP_WAIT_SLAVE_IDLE = 1,
	DIAG_STEP_STOP_PWM,
	DIAG_STEP_STOP_MIXER,
	DIAG_STEP_STOP_DMA,
	DIAG_STEP_ENTER_SLAVE,
	DIAG_STEP_SLAVE_VERSION,
	DIAG_STEP_ENTER_MD_TRANSPORT,
	DIAG_STEP_ENTER_MD_STATUS,
	DIAG_STEP_PING_MD_TRANSPORT,
	DIAG_STEP_PING_MD_VERSION,
	DIAG_STEP_PING_SLAVE,
	DIAG_STEP_RESTORE_HANDSHAKE,
	DIAG_STEP_EXIT_SLAVE,
	DIAG_STEP_RESTORE_MD_TRANSPORT,
	DIAG_STEP_RESTORE_MD_STATUS,
	DIAG_STEP_EXIT_MD_TRANSPORT,
	DIAG_STEP_EXIT_MD_STATUS,
	DIAG_STEP_START_MIXER_IDLE,
	DIAG_STEP_START_MIXER_DONE
};

static int runtimeFailure(DiagnosticResult *result, u16 cpu, u16 phase,
	u16 step, u32 address, u32 expected, u32 actual, u32 remaining)
{
	result->status = DIAG_INFRASTRUCTURE_ERROR;
	result->cpu = cpu;
	result->phase = phase;
	result->reserved = step;
	result->address = address;
	result->expected = expected;
	result->actual = actual;
	result->remaining = remaining;
	return 0;
}

static void rememberFailure(DiagnosticResult *result, u8 *failed,
	const DiagnosticResult *candidate, u16 phase, u16 step)
{
	if (*failed)
		return;
	*result = *candidate;
	result->status = DIAG_INFRASTRUCTURE_ERROR;
	result->phase = phase;
	result->reserved = step;
	*failed = 1;
}

static int secondaryExit(DiagnosticResult *result);
static int secondaryPing(DiagnosticResult *result);

static u8 __attribute__((section(".ramcode"))) catalogTimeout(u16 catalog)
{
	if (catalog >= 1 && catalog <= DIAGNOSTIC_TEST_COUNT)
		return diagnosticManifest[catalog - 1].timeout;
	return DIAG_TIMEOUT_CROSS_CPU;
}

u8 diagnosticRead8(u32 address)
{
	return *(volatile u8 *)address;
}

u16 diagnosticRead16(u32 address)
{
	return *(volatile u16 *)address;
}

u32 diagnosticRead32(u32 address)
{
	return *(volatile u32 *)address;
}

void diagnosticWrite8(u32 address, u8 value)
{
	*(volatile u8 *)address = value;
}

void diagnosticWrite16(u32 address, u16 value)
{
	*(volatile u16 *)address = value;
}

void diagnosticWrite32(u32 address, u32 value)
{
	*(volatile u32 *)address = value;
}

static u16 __attribute__((section(".ramcode")))
	diagnosticFrtRead(void)
{
	u8 high = SH2_FRT_FRCH;
	u8 low = SH2_FRT_FRCL;
	u8 verify = SH2_FRT_FRCH;

	if (high != verify)
	{
		high = verify;
		low = SH2_FRT_FRCL;
	}
	return ((u16)high << 8) | low;
}

void __attribute__((section(".ramcode"))) diagnosticDeadlineStart(
	DiagnosticDeadline *deadline,
	u8 timeoutClass)
{
	static const u32 limits[] =
	{
		0x00010000, 0x00020000, 0x00020000, 0x00080000,
		0x00200000, 0x01000000, 0x02000000
	};

	if (timeoutClass > DIAG_TIMEOUT_SEGA_CD)
		timeoutClass = DIAG_TIMEOUT_CROSS_CPU;
	deadline->remaining = limits[timeoutClass];
	deadline->sample = diagnosticFrtRead();
	deadline->reserved = 0;
}

int __attribute__((section(".ramcode"))) diagnosticDeadlineActive(
	DiagnosticDeadline *deadline)
{
	u16 sample;
	u16 elapsed;

	if (!deadline->remaining)
		return 0;
	sample = diagnosticFrtRead();
	elapsed = sample - deadline->sample;
	deadline->sample = sample;
	if (!elapsed)
	{
		if (++deadline->reserved < 16)
			return 1;
		deadline->reserved = 0;
		elapsed = 1;
	}
	else
		deadline->reserved = 0;
	if ((u32)elapsed >= deadline->remaining)
	{
		deadline->remaining = 0;
		return 0;
	}
	deadline->remaining -= elapsed;
	return 1;
}

u32 __attribute__((section(".ramcode")))
	diagnosticDeadlineRemaining(
	const DiagnosticDeadline *deadline)
{
	return deadline->remaining;
}

static void __attribute__((section(".ramcode")))
	diagnosticDmaQuietDelay(void)
{
	u32 count = 0x00010000;

	__asm__ volatile (
		"1:\n\t"
		"dt %0\n\t"
		"bf 1b"
		: "+r" (count)
		:
		: "memory");
}

static void __attribute__((section(".ramcode")))
	diagnosticDmaQuietWindow(void)
{
	u32 count = 0x00040000;

	__asm__ volatile (
		"1:\n\t"
		"dt %0\n\t"
		"bf 1b"
		: "+r" (count)
		:
		: "memory");
}

int __attribute__((section(".ramcode"))) diagnosticWait16(
	volatile u16 *address, u16 mask, u16 expected,
	u8 timeoutClass, u16 *actual, u32 *remaining)
{
	DiagnosticDeadline deadline;
	u16 value = *address;

	diagnosticDeadlineStart(&deadline, timeoutClass);
	while ((value & mask) != expected &&
		diagnosticDeadlineActive(&deadline))
	{
		__asm__ volatile ("" ::: "memory");
		value = *address;
	}
	if (actual)
		*actual = value;
	if (remaining)
		*remaining = diagnosticDeadlineRemaining(&deadline);
	return (value & mask) == expected;
}

int __attribute__((section(".ramcode"))) diagnosticWait32(
	volatile u32 *address, u32 mask, u32 expected,
	u8 timeoutClass, u32 *actual, u32 *remaining)
{
	DiagnosticDeadline deadline;
	u32 value = *address;

	diagnosticDeadlineStart(&deadline, timeoutClass);
	while ((value & mask) != expected &&
		diagnosticDeadlineActive(&deadline))
	{
		__asm__ volatile ("" ::: "memory");
		value = *address;
	}
	if (actual)
		*actual = value;
	if (remaining)
		*remaining = diagnosticDeadlineRemaining(&deadline);
	return (value & mask) == expected;
}

static void readPeerResult(DiagnosticResult *result)
{
	volatile DiagnosticPeerWire *wire =
		(volatile DiagnosticPeerWire *)DIAG_SCRATCH_SECONDARY;

	result->status = wire->status;
	result->cpu = wire->cpu;
	result->phase = wire->phase;
	result->expected = wire->expected;
	result->actual = wire->actual;
	result->address = wire->address;
	result->remaining = wire->remaining;
}

static void writePeerResult(const DiagnosticResult *result)
{
	volatile DiagnosticPeerWire *wire =
		(volatile DiagnosticPeerWire *)DIAG_SCRATCH_SECONDARY;

	wire->status = result->status;
	wire->cpu = result->cpu;
	wire->phase = result->phase;
	wire->reserved = 0;
	wire->expected = result->expected;
	wire->actual = result->actual;
	wire->address = result->address;
	wire->remaining = result->remaining;
}

int __attribute__((section(".ramcode"))) diagnosticMdCommand(
	u16 command, u16 argument,
	DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;
	DiagnosticDeadline dreqDeadline;
	u8 timeoutClass = (command == DIAG_MD_EXECUTE ||
		command == DIAG_MD_PREPARE) ?
		catalogTimeout(argument) : DIAG_TIMEOUT_CROSS_CPU;
	if (command == DIAG_MD_RESTORE || command == DIAG_MD_EXIT ||
		command == DIAG_MD_STATUS)
		timeoutClass = DIAG_TIMEOUT_SEGA_CD;

	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
		timeoutClass, &actual, &remaining))
	{
		if (result)
		{
			result->status = DIAG_TIMEOUT;
			result->cpu = DIAG_TARGET_MD;
			result->phase = 1;
			result->address = 0x20004020;
			result->actual = actual;
			result->remaining = remaining;
		}
		return 0;
	}
	if (command == DIAG_MD_EXECUTE && argument == 43)
		diagnosticDeadlineStart(&dreqDeadline, timeoutClass);
	MARS_SYS_COMM2 = argument;
	MARS_SYS_COMM0 = command;
	if (command == DIAG_MD_EXECUTE && argument == 43)
	{
		actual = MARS_SYS_COMM0;
		while (actual && diagnosticDeadlineActive(&dreqDeadline))
		{
			u16 phase = MARS_SYS_COMM2;

			if ((phase & 0xFF00) == DIAG_DREQ_PATTERN)
				MARS_SYS_COMM2 = DIAG_DREQ_ACK |
					(MARS_SYS_DMACTR & 0x0087);
			actual = MARS_SYS_COMM0;
		}
		remaining = dreqDeadline.remaining;
	}
	else if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
		timeoutClass, &actual, &remaining))
		actual = MARS_SYS_COMM0;
	if (actual)
	{
		if (result)
		{
			result->status = DIAG_TIMEOUT;
			result->cpu = DIAG_TARGET_MD;
			result->phase = 2;
			result->address = 0x20004020;
			result->actual = actual;
			result->remaining = remaining;
		}
		return 0;
	}
	if (result)
	{
		result->status = MARS_SYS_COMM2;
		result->cpu = DIAG_TARGET_MD;
		result->expected = ((u32)MARS_SYS_COMM8 << 16) | MARS_SYS_COMM10;
		result->actual = ((u32)MARS_SYS_COMM12 << 16) | MARS_SYS_COMM14;
		if ((command == DIAG_MD_EXECUTE || command == DIAG_MD_PREPARE) &&
			result->status != DIAG_PASS)
		{
			DiagnosticResult details = *result;
			u32 expected = result->expected;
			u32 actualResult = result->actual;

			if (diagnosticMdCommand(DIAG_MD_DETAILS, 0, &details))
			{
				result->phase = details.status;
				result->address = details.expected;
				result->remaining = details.actual;
			}
			result->expected = expected;
			result->actual = actualResult;
		}
	}
	return 1;
}

int __attribute__((section(".ramcode"))) diagnosticMdDmaBegin(
	u16 catalog, DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
		DIAG_TIMEOUT_SEGA_CD, &actual, &remaining))
	{
		result->status = DIAG_TIMEOUT;
		result->cpu = DIAG_TARGET_MD;
		result->phase = 1;
		result->address = 0x20004020;
		result->actual = actual;
		result->remaining = remaining;
		return 0;
	}
	MARS_SYS_COMM2 = catalog;
	MARS_SYS_COMM0 = DIAG_MD_EXECUTE;
	return 1;
}

int __attribute__((section(".ramcode"))) diagnosticMdDmaFinish(
	u16 catalog, DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
		DIAG_TIMEOUT_SEGA_CD, &actual, &remaining))
	{
		result->status = DIAG_TIMEOUT;
		result->cpu = DIAG_TARGET_MD;
		result->phase = 2;
		result->address = 0x20004020;
		result->actual = actual;
		result->remaining = remaining;
		return 0;
	}
	result->status = MARS_SYS_COMM2;
	result->cpu = DIAG_TARGET_MD;
	result->expected = ((u32)MARS_SYS_COMM8 << 16) | MARS_SYS_COMM10;
	result->actual = ((u32)MARS_SYS_COMM12 << 16) | MARS_SYS_COMM14;
	if (result->status != DIAG_PASS)
	{
		DiagnosticResult details = *result;
		u32 expected = result->expected;
		u32 actualResult = result->actual;

		if (diagnosticMdCommand(DIAG_MD_DETAILS, catalog, &details))
		{
			result->phase = details.status;
			result->address = details.expected;
			result->remaining = details.actual;
		}
		result->expected = expected;
		result->actual = actualResult;
	}
	return 1;
}

int __attribute__((section(".ramcode"))) diagnosticSecondaryBegin(
	u16 catalog, DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		result->status = DIAG_TIMEOUT;
		result->cpu = DIAG_TARGET_SLAVE;
		result->phase = 1;
		result->address = 0x20004024;
		result->actual = actual;
		result->remaining = remaining;
		return 0;
	}
	MARS_SYS_COMM6 = catalog;
	MARS_SYS_COMM4 = DIAG_SEC_EXECUTE;
	return 1;
}

int __attribute__((section(".ramcode"))) diagnosticSecondaryFinish(
	DiagnosticResult *result)
{
	DiagnosticDeadline deadline;
	u16 command;
	u32 readyRemaining;
	u16 extensions = result->catalog == 125 ? 3 : 0;
	u8 timeoutClass = catalogTimeout(result->catalog);
	u8 dmaQuiet = result->catalog == 123 || result->catalog == 125 ||
		result->catalog == 128 || result->catalog == 130 ||
		result->catalog == 132 || result->catalog == 134;

	if (result->catalog == 125)
		diagnosticDmaQuietWindow();
	diagnosticDeadlineStart(&deadline, timeoutClass);
	command = dmaQuiet ? DIAG_SEC_EXECUTE : MARS_SYS_COMM4;
	for (;;)
	{
		while (command != DIAG_SEC_DONE && command != DIAG_SEC_ERROR &&
			diagnosticDeadlineActive(&deadline))
		{
			if (dmaQuiet)
				diagnosticDmaQuietDelay();
			command = MARS_SYS_COMM4;
		}
		if (command == DIAG_SEC_DONE || command == DIAG_SEC_ERROR ||
			!extensions)
			break;
		extensions--;
		diagnosticDeadlineStart(&deadline, timeoutClass);
		command = dmaQuiet ? DIAG_SEC_EXECUTE : MARS_SYS_COMM4;
	}
	if (command != DIAG_SEC_DONE)
	{
		result->status = command == DIAG_SEC_ERROR ?
			DIAG_INFRASTRUCTURE_ERROR : DIAG_TIMEOUT;
		result->cpu = DIAG_TARGET_SLAVE;
		result->phase = 2;
		result->address = 0x20004024;
		result->actual = command;
		result->remaining = diagnosticDeadlineRemaining(&deadline);
		MARS_SYS_COMM4 = 0;
		return 0;
	}
	readPeerResult(result);
	MARS_SYS_COMM4 = 0;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &command, &readyRemaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->cpu = DIAG_TARGET_SLAVE;
		result->phase = 3;
		result->address = 0x20004024;
		result->actual = command;
		return 0;
	}
	return 1;
}

int __attribute__((section(".ramcode"))) diagnosticSecondaryCommand(
	u16 catalog, DiagnosticResult *result)
{
	if (!diagnosticSecondaryBegin(catalog, result))
		return 0;
	return diagnosticSecondaryFinish(result);
}

int __attribute__((section(".ramcode"))) diagnosticSecondaryDmaIsolation(int enable,
	DiagnosticResult *result)
{
	volatile u16 *release = (volatile u16 *)
		((u32)&diagnosticSecondaryDmaReleaseFlag | 0x20000000);
	u16 actual;
	u32 remaining;

	if (enable)
	{
		*release = 0;
		if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
			DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
			return runtimeFailure(result, DIAG_TARGET_SLAVE, 7, enable,
				0x20004024, DIAG_SEC_READY, actual, remaining);
		MARS_SYS_COMM4 = DIAG_SEC_DMA_MASK;
		if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_DONE,
			DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		{
			MARS_SYS_COMM4 = 0;
			return runtimeFailure(result, DIAG_TARGET_SLAVE, 8, enable,
				0x20004024, DIAG_SEC_DONE, actual, remaining);
		}
		return 1;
	}
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_DONE,
		DIAG_TIMEOUT_SEGA_CD, &actual, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE, 7, enable,
			0x20004024, DIAG_SEC_DONE, actual, remaining);
	*release = 1;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE, 9, enable,
			0x20004024, DIAG_SEC_READY, actual, remaining);
	return 1;
}

static void saveSecondaryRuntime(void)
{
	secondarySavedState.intmsk = MARS_SYS_INTMSK;
	secondarySavedState.sciSmr = *(volatile u8 *)0xFFFFFE00;
	secondarySavedState.sciBrr = *(volatile u8 *)0xFFFFFE01;
	secondarySavedState.sciScr = *(volatile u8 *)0xFFFFFE02;
	secondarySavedState.sciTdr = *(volatile u8 *)0xFFFFFE03;
	secondarySavedState.sciSsr = *(volatile u8 *)0xFFFFFE04;
	secondarySavedState.drcr0 = SH2_DMA_DRCR0;
	secondarySavedState.drcr1 = SH2_DMA_DRCR1;
	secondarySavedState.dmaor = SH2_DMA_DMAOR;
	secondarySavedState.sar0 = SH2_DMA_SAR0;
	secondarySavedState.dar0 = SH2_DMA_DAR0;
	secondarySavedState.tcr0 = SH2_DMA_TCR0;
	secondarySavedState.chcr0 = SH2_DMA_CHCR0;
	secondarySavedState.sar1 = SH2_DMA_SAR1;
	secondarySavedState.dar1 = SH2_DMA_DAR1;
	secondarySavedState.tcr1 = SH2_DMA_TCR1;
	secondarySavedState.chcr1 = SH2_DMA_CHCR1;
	secondarySavedState.vbr = diagnosticReadVbr();
	secondarySavedState.sr = diagnosticReadSr();
}

static void restoreSecondaryRuntime(void)
{
	SH2_DMA_CHCR0 = 0;
	SH2_DMA_CHCR1 = 0;
	SH2_DMA_DMAOR = 0;
	*(volatile u8 *)0xFFFFFE02 = 0;
	SH2_DMA_SAR0 = secondarySavedState.sar0;
	SH2_DMA_DAR0 = secondarySavedState.dar0;
	SH2_DMA_TCR0 = secondarySavedState.tcr0;
	SH2_DMA_DRCR0 = secondarySavedState.drcr0;
	SH2_DMA_CHCR0 = secondarySavedState.chcr0;
	SH2_DMA_SAR1 = secondarySavedState.sar1;
	SH2_DMA_DAR1 = secondarySavedState.dar1;
	SH2_DMA_TCR1 = secondarySavedState.tcr1;
	SH2_DMA_DRCR1 = secondarySavedState.drcr1;
	SH2_DMA_CHCR1 = secondarySavedState.chcr1;
	SH2_DMA_DMAOR = secondarySavedState.dmaor;
	*(volatile u8 *)0xFFFFFE03 = secondarySavedState.sciTdr;
	*(volatile u8 *)0xFFFFFE01 = secondarySavedState.sciBrr;
	*(volatile u8 *)0xFFFFFE00 = secondarySavedState.sciSmr;
	*(volatile u8 *)0xFFFFFE04 = secondarySavedState.sciSsr;
	*(volatile u8 *)0xFFFFFE02 = secondarySavedState.sciScr;
	diagnosticWriteVbr(secondarySavedState.vbr);
	MARS_SYS_INTMSK = secondarySavedState.intmsk;
	diagnosticWriteSr(secondarySavedState.sr);
}

void __attribute__((section(".ramcode"))) diagnosticSecondaryAgent(void)
{
	volatile u16 *dmaRelease = (volatile u16 *)
		((u32)&diagnosticSecondaryDmaReleaseFlag | 0x20000000);
	DiagnosticDeadline idle;
	u16 idleExtensions = 0;
	u8 dmaSavedIntmsk = 0;
	u32 dmaSavedChcr0 = 0;
	u8 dmaIsolated = 0;

	saveSecondaryRuntime();
	diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_SEGA_CD);
	MARS_SYS_COMM6 = DIAG_AGENT_VERSION;
	MARS_SYS_COMM4 = DIAG_SEC_READY;
	for (;;)
	{
		u16 command = MARS_SYS_COMM4;

		if (!diagnosticDeadlineActive(&idle))
		{
			if (idleExtensions < 4)
			{
				idleExtensions++;
				diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_SEGA_CD);
				continue;
			}
			break;
		}

		if (command == DIAG_SEC_READY)
			continue;
		if (command == DIAG_SEC_PING)
		{
			MARS_SYS_COMM6 = DIAG_AGENT_VERSION;
			MARS_SYS_COMM4 = DIAG_SEC_READY;
			idleExtensions = 0;
			diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_SEGA_CD);
			continue;
		}
		if (command == DIAG_SEC_DMA_MASK)
		{
			if (!dmaIsolated)
			{
				dmaSavedIntmsk = *(volatile u8 *)0x20004001;
				dmaSavedChcr0 = SH2_DMA_CHCR0;
				SH2_DMA_CHCR0 = 0;
				*(volatile u8 *)0x20004001 = 2;
				dmaIsolated = 1;
			}
			MARS_SYS_COMM6 = DIAG_AGENT_VERSION;
			MARS_SYS_COMM4 = DIAG_SEC_DONE;
			while (!*dmaRelease)
				diagnosticDmaQuietDelay();
			SH2_DMA_CHCR0 = dmaSavedChcr0;
			*(volatile u8 *)0x20004001 = dmaSavedIntmsk;
			dmaIsolated = 0;
			MARS_SYS_COMM6 = DIAG_AGENT_VERSION;
			MARS_SYS_COMM4 = DIAG_SEC_READY;
			idleExtensions = 0;
			diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_SEGA_CD);
			continue;
		}
		if (command == DIAG_SEC_EXECUTE)
		{
			DiagnosticResult result;
			u16 catalog = MARS_SYS_COMM6;

			result.status = DIAG_NOT_RUN;
			result.cpu = DIAG_TARGET_SLAVE;
			result.phase = 0;
			result.reserved = 0;
			result.catalog = catalog;
			result.expected = 0;
			result.actual = 0;
			result.address = 0;
			result.remaining = 0;
			result.status = diagnosticExecuteSecondary(catalog, &result);
			writePeerResult(&result);
			MARS_SYS_COMM6 = result.status;
			MARS_SYS_COMM4 = DIAG_SEC_DONE;
			diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_CROSS_CPU);
			while (MARS_SYS_COMM4 == DIAG_SEC_DONE &&
				diagnosticDeadlineActive(&idle))
				__asm__ volatile ("" ::: "memory");
			if (MARS_SYS_COMM4 == DIAG_SEC_DONE)
			{
				restoreSecondaryRuntime();
				return;
			}
			MARS_SYS_COMM6 = DIAG_AGENT_VERSION;
			MARS_SYS_COMM4 = DIAG_SEC_READY;
			idleExtensions = 0;
			diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_SEGA_CD);
			continue;
		}
		if (command == DIAG_SEC_EXIT)
		{
			if (dmaIsolated)
			{
				SH2_DMA_CHCR0 = dmaSavedChcr0;
				*(volatile u8 *)0x20004001 = dmaSavedIntmsk;
			}
			MARS_SYS_COMM6 = DIAG_AGENT_VERSION;
			MARS_SYS_COMM4 = DIAG_SEC_DONE;
			diagnosticDeadlineStart(&idle, DIAG_TIMEOUT_CROSS_CPU);
			while (MARS_SYS_COMM4 == DIAG_SEC_DONE &&
				diagnosticDeadlineActive(&idle))
				__asm__ volatile ("" ::: "memory");
			restoreSecondaryRuntime();
			return;
		}
	}
	if (dmaIsolated)
	{
		SH2_DMA_CHCR0 = dmaSavedChcr0;
		*(volatile u8 *)0x20004001 = dmaSavedIntmsk;
	}
	restoreSecondaryRuntime();
	MARS_SYS_COMM4 = DIAG_SEC_ERROR;
}

static u16 communicationPattern(u16 catalog, u16 index)
{
	return (u16)(0x5000 | ((catalog & 0x00FF) << 3) | index);
}

DiagnosticStatus diagnosticCommunicationTransaction(u16 catalog,
	DiagnosticResult *result)
{
	volatile u16 *words = (volatile u16 *)0x20004020;
	volatile u16 *secondaryPhase = DIAG_SCRATCH_SECONDARY + 16;
	DiagnosticResult secondary = *result;
	DiagnosticResult md = *result;
	DiagnosticResult ping = *result;
	DiagnosticResult details = *result;
	u16 actual = 0;
	u32 remaining = 0;
	u8 failurePhase = 0;
	u8 secondaryStarted = 0;
	u8 mdStarted = 0;
	int index;
	int mismatch = 0;

	*secondaryPhase = 0;
	secondary.cpu = DIAG_TARGET_SLAVE;
	if (!diagnosticSecondaryBegin(catalog, &secondary))
	{
		*result = secondary;
		return result->status;
	}
	secondaryStarted = 1;
	if (!diagnosticWait16(secondaryPhase, 0xFFFF, DIAG_COMM_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		failurePhase = 1;
		goto infrastructure;
	}
	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		failurePhase = 2;
		goto infrastructure;
	}
	MARS_SYS_COMM2 = catalog;
	MARS_SYS_COMM0 = DIAG_MD_COMMUNICATION;
	mdStarted = 1;
	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, DIAG_COMM_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		failurePhase = 3;
		goto infrastructure;
	}
	MARS_SYS_COMM0 = DIAG_COMM_GO;
	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, DIAG_COMM_ACK,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		failurePhase = 4;
		goto infrastructure;
	}
	*secondaryPhase = DIAG_COMM_GO;
	if (catalog == 1 || catalog == 39 || catalog == 40)
		MARS_SYS_COMM0 = DIAG_COMM_GO;
	if (catalog == 16)
	{
		for (index = 1; index < 8; index++)
			words[index] = communicationPattern(catalog, index);
		words[0] = communicationPattern(catalog, 0);
	}
	if (catalog == 16 && !diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF,
		DIAG_COMM_DONE, DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		failurePhase = 5;
		goto infrastructure;
	}
	if (catalog == 17 || catalog == 40)
	{
		if (!diagnosticWait16(secondaryPhase, 0xFFFF, DIAG_COMM_DONE,
			DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		{
			failurePhase = 6;
			goto infrastructure;
		}
	}
	if (catalog == 1 || catalog == 17 || catalog == 39)
	{
		if (!diagnosticWait16(words, 0xFFFF,
			communicationPattern(catalog, 0), DIAG_TIMEOUT_CROSS_CPU,
			&actual, &remaining))
		{
			failurePhase = 7;
			goto infrastructure;
		}
		for (index = 0; index < 8; index++)
		{
			u16 expected = communicationPattern(catalog, index);
			u16 actual = words[index];

			if (!mismatch && actual != expected)
			{
				result->status = DIAG_FAIL;
				result->expected = expected;
				result->actual = actual;
				result->address = 0x20004020 + index * 2;
				mismatch = 1;
			}
		}
	}
	*secondaryPhase = DIAG_COMM_RESTORE;
	if (!diagnosticSecondaryFinish(&secondary))
	{
		failurePhase = 8;
		actual = MARS_SYS_COMM4;
		remaining = secondary.remaining;
		goto infrastructure;
	}
	secondaryStarted = 0;
	MARS_SYS_COMM0 = DIAG_COMM_RESTORE;
	if (!diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
	{
		failurePhase = 9;
		goto infrastructure;
	}
	md.status = MARS_SYS_COMM2;
	md.cpu = DIAG_TARGET_MD;
	md.expected = ((u32)MARS_SYS_COMM8 << 16) | MARS_SYS_COMM10;
	md.actual = ((u32)MARS_SYS_COMM12 << 16) | MARS_SYS_COMM14;
	md.address = 0x20004020;
	mdStarted = 0;
	if (md.status != DIAG_PASS &&
		diagnosticMdCommand(DIAG_MD_DETAILS, 0, &details))
	{
		md.phase = details.status;
		md.address = details.expected;
		md.remaining = details.actual;
	}
	if (!diagnosticMdCommand(DIAG_MD_PING, DIAG_AGENT_VERSION, &ping) ||
		MARS_SYS_COMM2 != DIAG_AGENT_VERSION)
		goto infrastructure;
	if (catalog == 16 && md.status != DIAG_PASS)
	{
		*result = md;
		return result->status;
	}
	if (catalog == 40 && secondary.status != DIAG_PASS)
	{
		*result = secondary;
		return result->status;
	}
	if (mismatch)
		return DIAG_FAIL;
	result->status = DIAG_PASS;
	return DIAG_PASS;

infrastructure:
	if (secondaryStarted)
	{
		DiagnosticResult cleanup = secondary;

		*secondaryPhase = DIAG_COMM_RESTORE;
		diagnosticSecondaryFinish(&cleanup);
	}
	if (mdStarted)
	{
		MARS_SYS_COMM0 = DIAG_COMM_RESTORE;
		diagnosticWait16(&MARS_SYS_COMM0, 0xFFFF, 0,
			DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining);
	}
	result->status = DIAG_INFRASTRUCTURE_ERROR;
	result->cpu = DIAG_TARGET_MASTER_SLAVE;
	result->phase = failurePhase;
	result->address = 0x20004020;
	result->actual = actual;
	result->remaining = remaining;
	return DIAG_INFRASTRUCTURE_ERROR;
}

int diagnosticsSecondaryEnter(void)
{
	diagnosticSecondaryAgent();
	return 1;
}

static int checkMarker(u32 address, u32 expected,
	DiagnosticResult *result)
{
	u32 actual = diagnosticRead32(address);

	if (actual == expected)
		return 1;
	result->status = DIAG_INFRASTRUCTURE_ERROR;
	result->cpu = DIAG_TARGET_MASTER;
	result->phase = DIAG_STATE_PREFLIGHT;
	result->expected = expected;
	result->actual = actual;
	result->address = address;
	return 0;
}

int diagnosticRuntimePreflight(DiagnosticContext *context,
	DiagnosticResult *result)
{
	u16 command;
	u32 actual;
	u32 remaining;

	runtimeEntered = 0;
	secondaryRequested = 0;
	secondaryEntered = 0;
	mdRequested = 0;
	mdEntered = 0;
	diagnosticRuntimeActive = 0;
	stateSaved = 0;
	if (!checkMarker(0x220FFFFC, 0x424B3030, result) ||
		!checkMarker(0x221FFFFC, 0x424B3031, result) ||
		!checkMarker(0x222FFFFC, 0x424B3130, result) ||
		!checkMarker(0x223FFFFC, 0x424B3131, result))
		return 0;

	context->segaCd = segaCDDetectedAtBoot != 0;
	context->mixerActive = sound_isInitialized() != 0;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, 0,
		DIAG_TIMEOUT_DMA, &command, &remaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->cpu = DIAG_TARGET_SLAVE;
		result->phase = DIAG_STATE_PREFLIGHT;
		result->actual = command;
		result->address = 0x20004024;
		result->remaining = remaining;
		return 0;
	}
	if ((SH2_DMA_CHCR0 & 1) && !(SH2_DMA_CHCR0 & 2) &&
		!diagnosticWait32(&SH2_DMA_CHCR0, 2, 2, DIAG_TIMEOUT_DMA,
			&actual, &remaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->cpu = DIAG_TARGET_MASTER;
		result->phase = DIAG_STATE_PREFLIGHT;
		result->actual = actual;
		result->address = 0xFFFFFF8C;
		result->remaining = remaining;
		return 0;
	}
	if (!context->mixerActive && (SH2_DMA_CHCR1 & 1) &&
		!(SH2_DMA_CHCR1 & 2) &&
		!diagnosticWait32(&SH2_DMA_CHCR1, 2, 2, DIAG_TIMEOUT_DMA,
			&actual, &remaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->cpu = DIAG_TARGET_MASTER;
		result->phase = DIAG_STATE_PREFLIGHT;
		result->actual = actual;
		result->address = 0xFFFFFF9C;
		result->remaining = remaining;
		return 0;
	}
	savedState.intmsk = MARS_SYS_INTMSK;
	savedState.dmactr = MARS_SYS_DMACTR;
	savedState.dmasar = MARS_SYS_DMASAR;
	savedState.dmadar = MARS_SYS_DMADAR;
	savedState.dmalen = MARS_SYS_DMALEN;
	savedState.pwmCtrl = MARS_PWM_CTRL;
	savedState.pwmCycle = MARS_PWM_CYCLE;
	savedState.vdpMode = MARS_VDP_DISPMODE;
	savedState.vdpShift = MARS_VDP_SHIFTREG;
	savedState.vdpFillLength = *(volatile u8 *)0x20004105;
	savedState.vdpFillAddress = MARS_VDP_FILADR;
	savedState.vdpFbctl = SH2_VDP_CONTROL;
	savedState.sciSmr = *(volatile u8 *)0xFFFFFE00;
	savedState.sciBrr = *(volatile u8 *)0xFFFFFE01;
	savedState.sciScr = *(volatile u8 *)0xFFFFFE02;
	savedState.sciTdr = *(volatile u8 *)0xFFFFFE03;
	savedState.sciSsr = *(volatile u8 *)0xFFFFFE04;
	savedState.drcr0 = SH2_DMA_DRCR0;
	savedState.drcr1 = SH2_DMA_DRCR1;
	savedState.dmaor = SH2_DMA_DMAOR;
	savedState.sar0 = SH2_DMA_SAR0;
	savedState.dar0 = SH2_DMA_DAR0;
	savedState.tcr0 = SH2_DMA_TCR0;
	savedState.chcr0 = SH2_DMA_CHCR0;
	savedState.sar1 = SH2_DMA_SAR1;
	savedState.dar1 = SH2_DMA_DAR1;
	savedState.tcr1 = SH2_DMA_TCR1;
	savedState.chcr1 = SH2_DMA_CHCR1;
	savedState.vbr = diagnosticReadVbr();
	savedState.sr = diagnosticReadSr();
	stateSaved = 1;
	result->status = DIAG_PASS;
	return 1;
}

int diagnosticRuntimeEnter(DiagnosticContext *context,
	DiagnosticResult *result)
{
	u16 command;
	u32 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, 0,
		DIAG_TIMEOUT_DMA, &command, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_QUIESCE, DIAG_STEP_WAIT_SLAVE_IDLE, 0x20004024,
			0, command, remaining);
	MARS_SYS_COMM4 = MARS_SEC_CMD_PWM_TEST_STOP;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, 0,
		DIAG_TIMEOUT_DMA, &command, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_QUIESCE, DIAG_STEP_STOP_PWM, 0x20004024,
			0, command, remaining);
	if (context->mixerActive)
	{
		MARS_SYS_COMM4 = 7;
		if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, 0,
			DIAG_TIMEOUT_DMA, &command, &remaining))
			return runtimeFailure(result, DIAG_TARGET_SLAVE,
				DIAG_STATE_QUIESCE, DIAG_STEP_STOP_MIXER, 0x20004024,
				0, command, remaining);
	}
	if ((SH2_DMA_CHCR1 & 1) && !(SH2_DMA_CHCR1 & 2) &&
		!diagnosticWait32(&SH2_DMA_CHCR1, 2, 2, DIAG_TIMEOUT_DMA,
			&actual, &remaining))
	{
		SH2_DMA_CHCR1 = 0;
		SH2_DMA_DMAOR = 0;
		if ((SH2_DMA_CHCR1 & 1) || (SH2_DMA_DMAOR & 1))
			return runtimeFailure(result, DIAG_TARGET_MASTER,
				DIAG_STATE_QUIESCE, DIAG_STEP_STOP_DMA, 0xFFFFFF9C,
				0, ((SH2_DMA_CHCR1 & 1) << 1) |
				(SH2_DMA_DMAOR & 1), remaining);
	}
	secondaryRequested = 1;
	MARS_SYS_COMM4 = DIAG_SEC_ENTER;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &command, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_QUIESCE, DIAG_STEP_ENTER_SLAVE, 0x20004024,
			DIAG_SEC_READY, command, remaining);
	if (MARS_SYS_COMM6 != DIAG_AGENT_VERSION)
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_QUIESCE, DIAG_STEP_SLAVE_VERSION, 0x20004026,
			DIAG_AGENT_VERSION, MARS_SYS_COMM6, remaining);
	secondaryEntered = 1;
	mdRequested = 1;
	if (!diagnosticMdCommand(DIAG_MD_ENTER, DIAG_AGENT_VERSION, result))
	{
		if (result->phase == 1)
			mdRequested = 0;
		return runtimeFailure(result, DIAG_TARGET_MD,
			DIAG_STATE_QUIESCE, DIAG_STEP_ENTER_MD_TRANSPORT,
			0x20004020, 0, result->actual, result->remaining);
	}
	if (result->status != DIAG_PASS)
		return runtimeFailure(result, DIAG_TARGET_MD,
			DIAG_STATE_QUIESCE, DIAG_STEP_ENTER_MD_STATUS, 0x20004022,
			DIAG_PASS, result->status, 0);
	mdEntered = 1;
	if (!diagnosticMdCommand(DIAG_MD_PING, DIAG_AGENT_VERSION, result))
		return runtimeFailure(result, DIAG_TARGET_MD,
			DIAG_STATE_QUIESCE, DIAG_STEP_PING_MD_TRANSPORT,
			0x20004020, 0, result->actual, result->remaining);
	if (MARS_SYS_COMM2 != DIAG_AGENT_VERSION)
		return runtimeFailure(result, DIAG_TARGET_MD,
			DIAG_STATE_QUIESCE, DIAG_STEP_PING_MD_VERSION, 0x20004022,
			DIAG_AGENT_VERSION, MARS_SYS_COMM2, 0);
	if (!secondaryPing(result))
	{
		result->phase = DIAG_STATE_QUIESCE;
		result->reserved = DIAG_STEP_PING_SLAVE;
		return 0;
	}
	runtimeEntered = 1;
	diagnosticRuntimeActive = 1;
	diagnosticPrimaryVblankCount = 0;
	diagnosticPrimaryHblankCount = 0;
	diagnosticPrimaryCommandCount = 0;
	diagnosticPrimaryPwmCount = 0;
	diagnosticPrimarySciCount = 0;
	diagnosticSecondaryVblankCount = 0;
	diagnosticSecondaryHblankCount = 0;
	diagnosticSecondaryCommandCount = 0;
	diagnosticSecondaryPwmCount = 0;
	diagnosticSecondarySciCount = 0;
	result->status = DIAG_PASS;
	return 1;
}

static int secondaryExit(DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_RESTORE, DIAG_STEP_EXIT_SLAVE, 0x20004024,
			DIAG_SEC_READY, actual, remaining);
	MARS_SYS_COMM4 = DIAG_SEC_EXIT;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_DONE,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_RESTORE, DIAG_STEP_EXIT_SLAVE, 0x20004024,
			DIAG_SEC_DONE, actual, remaining);
	if (MARS_SYS_COMM6 != DIAG_AGENT_VERSION)
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_RESTORE, DIAG_STEP_EXIT_SLAVE, 0x20004026,
			DIAG_AGENT_VERSION, MARS_SYS_COMM6, remaining);
	MARS_SYS_COMM4 = 0;
	return 1;
}

static int secondaryPing(DiagnosticResult *result)
{
	u16 actual;
	u32 remaining;

	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_RUN_TEST, DIAG_STEP_PING_SLAVE, 0x20004024,
			DIAG_SEC_READY, actual, remaining);
	MARS_SYS_COMM4 = DIAG_SEC_PING;
	if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, DIAG_SEC_READY,
		DIAG_TIMEOUT_CROSS_CPU, &actual, &remaining))
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_RUN_TEST, DIAG_STEP_PING_SLAVE, 0x20004024,
			DIAG_SEC_READY, actual, remaining);
	if (MARS_SYS_COMM6 != DIAG_AGENT_VERSION)
		return runtimeFailure(result, DIAG_TARGET_SLAVE,
			DIAG_STATE_RUN_TEST, DIAG_STEP_PING_SLAVE, 0x20004026,
			DIAG_AGENT_VERSION, MARS_SYS_COMM6, remaining);
	return 1;
}

int diagnosticRuntimeRestore(DiagnosticContext *context,
	DiagnosticResult *result)
{
	u8 failed = 0;
	u8 mdRestored = 0;
	u16 command;
	u32 remaining;
	DiagnosticResult candidate;
	candidate = *result;

	if (stateSaved)
	{
		SH2_DMA_CHCR0 = 0;
		SH2_DMA_CHCR1 = 0;
		SH2_DMA_DMAOR = 0;
		MARS_SYS_DMACTR = 0;
		MARS_PWM_CTRL = 0;
	}
	if (runtimeEntered)
	{
		candidate = *result;
		if (!diagnosticRuntimeHandshake(&candidate))
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_RESTORE_HANDSHAKE);
	}
	if (secondaryRequested || secondaryEntered || runtimeEntered)
	{
		candidate = *result;
		if (!secondaryExit(&candidate))
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_EXIT_SLAVE);
		else
		{
			secondaryRequested = 0;
			secondaryEntered = 0;
		}
	}
	if (mdRequested || mdEntered || runtimeEntered)
	{
		candidate = *result;
		if (!diagnosticMdCommand(DIAG_MD_RESTORE, 0, &candidate))
		{
			candidate.cpu = DIAG_TARGET_MD;
			candidate.address = 0x20004020;
			candidate.expected = 0;
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_RESTORE_MD_TRANSPORT);
		}
		else if (candidate.status != DIAG_PASS)
		{
			runtimeFailure(&candidate, DIAG_TARGET_MD,
				DIAG_STATE_RESTORE, DIAG_STEP_RESTORE_MD_STATUS,
				0x20004022, DIAG_PASS, candidate.status, 0);
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_RESTORE_MD_STATUS);
		}
		else
		{
			mdRestored = 1;
			mdEntered = 0;
		}
		candidate = *result;
		if (!diagnosticMdCommand(DIAG_MD_EXIT, DIAG_AGENT_VERSION,
			&candidate))
		{
			candidate.cpu = DIAG_TARGET_MD;
			candidate.address = 0x20004020;
			candidate.expected = 0;
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_EXIT_MD_TRANSPORT);
		}
		else if (candidate.status != DIAG_PASS)
		{
			runtimeFailure(&candidate, DIAG_TARGET_MD,
				DIAG_STATE_RESTORE, DIAG_STEP_EXIT_MD_STATUS,
				0x20004022, DIAG_PASS, candidate.status, 0);
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_EXIT_MD_STATUS);
		}
		else
			mdEntered = 0;
		if (mdRestored || candidate.status == DIAG_PASS)
			mdRequested = 0;
	}
	if (stateSaved)
	{
		SH2_DMA_SAR0 = savedState.sar0;
		SH2_DMA_DAR0 = savedState.dar0;
		SH2_DMA_TCR0 = savedState.tcr0;
		SH2_DMA_DRCR0 = savedState.drcr0;
		SH2_DMA_CHCR0 = savedState.chcr0;
		SH2_DMA_SAR1 = savedState.sar1;
		SH2_DMA_DAR1 = savedState.dar1;
		SH2_DMA_TCR1 = savedState.tcr1;
		SH2_DMA_DRCR1 = savedState.drcr1;
		SH2_DMA_CHCR1 = savedState.chcr1;
		SH2_DMA_DMAOR = savedState.dmaor;
		MARS_SYS_DMACTR = savedState.dmactr;
		MARS_SYS_DMASAR = savedState.dmasar;
		MARS_SYS_DMADAR = savedState.dmadar;
		MARS_SYS_DMALEN = savedState.dmalen;
		MARS_PWM_CYCLE = savedState.pwmCycle;
		MARS_PWM_CTRL = savedState.pwmCtrl;
		MARS_SYS_INTMSK = savedState.intmsk;
		MARS_VDP_SHIFTREG = savedState.vdpShift;
		*(volatile u8 *)0x20004105 = savedState.vdpFillLength;
		MARS_VDP_FILADR = savedState.vdpFillAddress;
		SH2_VDP_CONTROL = savedState.vdpFbctl & 1;
		MARS_VDP_DISPMODE = savedState.vdpMode;
		*(volatile u8 *)0xFFFFFE02 = 0;
		*(volatile u8 *)0xFFFFFE03 = savedState.sciTdr;
		*(volatile u8 *)0xFFFFFE01 = savedState.sciBrr;
		*(volatile u8 *)0xFFFFFE00 = savedState.sciSmr;
		*(volatile u8 *)0xFFFFFE04 = savedState.sciSsr;
		*(volatile u8 *)0xFFFFFE02 = savedState.sciScr;
		diagnosticWriteVbr(savedState.vbr);
		diagnosticWriteSr(savedState.sr);
	}
	diagnosticRuntimeActive = 0;
	marsVDP256Start();
	if (context->mixerActive && !secondaryRequested)
	{
		if (diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, 0,
			DIAG_TIMEOUT_DMA, &command, &remaining))
		{
			MARS_SYS_COMM4 = 8;
			if (!diagnosticWait16(&MARS_SYS_COMM4, 0xFFFF, 0,
				DIAG_TIMEOUT_DMA, &command, &remaining))
			{
				runtimeFailure(&candidate, DIAG_TARGET_SLAVE,
					DIAG_STATE_RESTORE, DIAG_STEP_START_MIXER_DONE,
					0x20004024, 0, command, remaining);
				rememberFailure(result, &failed, &candidate,
					DIAG_STATE_RESTORE, DIAG_STEP_START_MIXER_DONE);
			}
		}
		else
		{
			runtimeFailure(&candidate, DIAG_TARGET_SLAVE,
				DIAG_STATE_RESTORE, DIAG_STEP_START_MIXER_IDLE,
				0x20004024, 0, command, remaining);
			rememberFailure(result, &failed, &candidate,
				DIAG_STATE_RESTORE, DIAG_STEP_START_MIXER_IDLE);
		}
	}
	runtimeEntered = 0;
	secondaryRequested = 0;
	secondaryEntered = 0;
	mdRequested = 0;
	mdEntered = 0;
	stateSaved = 0;
	if (failed)
		return 0;
	result->status = DIAG_PASS;
	return 1;
}

int diagnosticRuntimeStatus(u16 catalog, DiagnosticResult *result)
{
	if (!runtimeEntered)
		return 0;
	if (!secondaryPing(result))
		return 0;
	if (!diagnosticMdCommand(DIAG_MD_STATUS, catalog, result))
		return 0;
	return result->status == DIAG_PASS;
}

int diagnosticRuntimeHandshake(DiagnosticResult *result)
{
	if (!secondaryPing(result))
		return 0;
	if (!diagnosticMdCommand(DIAG_MD_PING, DIAG_AGENT_VERSION, result))
		return runtimeFailure(result, DIAG_TARGET_MD,
			DIAG_STATE_RUN_TEST, DIAG_STEP_PING_MD_TRANSPORT,
			0x20004020, 0, result->actual, result->remaining);
	if (MARS_SYS_COMM2 != DIAG_AGENT_VERSION)
		return runtimeFailure(result, DIAG_TARGET_MD,
			DIAG_STATE_RUN_TEST, DIAG_STEP_PING_MD_VERSION, 0x20004022,
			DIAG_AGENT_VERSION, MARS_SYS_COMM2, 0);
	result->status = DIAG_PASS;
	return 1;
}

u16 diagnosticRuntimePad(void)
{
	DiagnosticResult result;
	u16 pad = MARS_SYS_COMM8;

	if (!runtimeEntered)
	{
		if ((pad & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			pad = MARS_SYS_COMM10;
		return pad;
	}
	result.status = DIAG_NOT_RUN;
	if (diagnosticMdCommand(DIAG_MD_PAD, 0, &result))
	{
		pad = MARS_SYS_COMM2;
		if ((pad & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE &&
			diagnosticMdCommand(DIAG_MD_PAD, 1, &result))
			pad = MARS_SYS_COMM2;
		return pad;
	}
	if ((pad & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		pad = MARS_SYS_COMM10;
	return pad;
}
