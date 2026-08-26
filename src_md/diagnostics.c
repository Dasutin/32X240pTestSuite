#include <stdint.h>

#define COMM0 (*(volatile uint16_t *)0xA15120)
#define COMM2 (*(volatile uint16_t *)0xA15122)
#define COMM4 (*(volatile uint16_t *)0xA15124)
#define COMM6 (*(volatile uint16_t *)0xA15126)
#define COMM8 (*(volatile uint16_t *)0xA15128)
#define COMM10 (*(volatile uint16_t *)0xA1512A)
#define COMM12 (*(volatile uint16_t *)0xA1512C)
#define COMM14 (*(volatile uint16_t *)0xA1512E)
#define ADAPTER_CONTROL (*(volatile uint16_t *)0xA15100)
#define ROM_BANK (*(volatile uint8_t *)0xA15105)
#define DREQ_CONTROL (*(volatile uint16_t *)0xA15106)
#define DREQ_SOURCE (*(volatile uint32_t *)0xA15108)
#define DREQ_DESTINATION (*(volatile uint32_t *)0xA1510C)
#define DREQ_LENGTH (*(volatile uint16_t *)0xA15110)
#define DREQ_FIFO (*(volatile uint16_t *)0xA15112)
#define PWM_CONTROL (*(volatile uint16_t *)0xA15130)
#define PWM_CYCLE (*(volatile uint16_t *)0xA15132)
#define PWM_LEFT (*(volatile uint16_t *)0xA15134)
#define PWM_RIGHT (*(volatile uint16_t *)0xA15136)
#define PWM_MONO (*(volatile uint16_t *)0xA15138)
#define VDP_MODE (*(volatile uint16_t *)0xA15180)
#define VDP_SHIFT (*(volatile uint16_t *)0xA15182)
#define VDP_FILL_LENGTH_LOW (*(volatile uint8_t *)0xA15185)
#define VDP_FILL_ADDRESS (*(volatile uint16_t *)0xA15186)
#define VDP_FILL_DATA (*(volatile uint16_t *)0xA15188)
#define VDP_STATUS (*(volatile uint8_t *)0xA1518A)
#define VDP_CONTROL (*(volatile uint8_t *)0xA1518B)
#define SCD_RESET (*(volatile uint8_t *)0xA12001)
#define SCD_MEM_MODE (*(volatile uint16_t *)0xA12002)

#define DIAG_NOT_RUN 0
#define DIAG_PASS 1
#define DIAG_FAIL 2
#define DIAG_TIMEOUT 3
#define DIAG_MANUAL 4
#define DIAG_UNAVAILABLE 5
#define DIAG_INFRASTRUCTURE_ERROR 7

#define DIAG_MD_ENTER 0x1A01
#define DIAG_MD_EXIT 0x1A02
#define DIAG_MD_EXECUTE 0x1A03
#define DIAG_MD_PAD 0x1A04
#define DIAG_MD_SEGA_CD 0x1A05
#define DIAG_MD_RESTORE 0x1A06
#define DIAG_MD_COMMUNICATION 0x1A07
#define DIAG_MD_PING 0x1A08
#define DIAG_MD_STATUS 0x1A09
#define DIAG_MD_DETAILS 0x1A0A
#define DIAG_MD_PREPARE 0x1A0B
#define DIAG_AGENT_VERSION 0x0100
#define DIAG_COMM_READY 0xC101
#define DIAG_COMM_GO 0xC102
#define DIAG_COMM_ACK 0xC103
#define DIAG_COMM_DONE 0xC104
#define DIAG_COMM_RESTORE 0xC105
#define DIAG_DREQ_PATTERN 0xD400
#define DIAG_DREQ_ACK 0xD500
#define SEGA_CTRL_THREE 0x1000
#define SEGA_CTRL_NONE 0xF000
#define MD_TIMEOUT_REGISTER 0
#define MD_TIMEOUT_FIFO 1
#define MD_TIMEOUT_INTERRUPT 2
#define MD_TIMEOUT_CROSS_CPU 3
#define MD_TIMEOUT_DMA 4
#define MD_TIMEOUT_STIMULUS 5
#define MD_TIMEOUT_SEGA_CD 6
#define DIAGNOSTIC_WORD_RAM_ROM_BANK 1
#define DIAGNOSTIC_WORD_RAM_ROM_WINDOW 0x900000
#define DIAGNOSTIC_Z80_ROM_BANK 1
#define DIAGNOSTIC_Z80_ROM_WINDOW 0x910000
#define DIAGNOSTIC_Z80_PAYLOAD_SIZE 0x0200
#define DIAGNOSTIC_Z80_WAVEFORM 0x910116
#define DIAGNOSTIC_Z80_WAVEFORM_WORDS 44
#define DIAGNOSTIC_MD_ROM_BASE 0x00880000

typedef struct
{
	uint16_t status;
	uint16_t phase;
	uint32_t expected;
	uint32_t actual;
	uint32_t address;
	uint32_t remaining;
} MdResult;

typedef struct
{
	uint32_t start;
	uint16_t frames;
} MdDeadline;

typedef struct
{
	uint16_t adapter;
	uint8_t bank;
	uint16_t dreqControl;
	uint32_t dreqSource;
	uint32_t dreqDestination;
	uint16_t dreqLength;
	uint16_t pwmControl;
	uint16_t pwmCycle;
	uint16_t vdpMode;
	uint16_t vdpShift;
	uint16_t vdpFillLength;
	uint16_t vdpFillAddress;
	uint16_t vdpFbctl;
} MdSavedState;

volatile uint16_t diagnosticMdActive;
volatile uint32_t diagnosticMdHintCount;
volatile uint32_t diagnosticMdVblankCount;

extern volatile uint16_t controller_mode;
extern volatile uint16_t controller_force_reset;
extern int diagnosticMdRomBanks(void);
extern uint32_t diagnosticMdRamDreqProbe(void);
extern uint32_t diagnosticMdCrossDreqControl(void);
extern int diagnosticMdVdpDma(uint32_t source, uint32_t destination,
	uint32_t words);
extern int diagnosticMdWordRamVdpDma(void);
extern void diagnosticMdReloadFont(void);

static MdSavedState savedState;
static MdResult lastResult;
static uint16_t mdStatusDirty;
static uint16_t dreqRestorePending;
static uint8_t dreqRestoreControl;
static uint16_t dreqRestoreLength;
static uint32_t dreqRestoreSource;
static uint32_t dreqRestoreDestination;
static uint16_t wordRamRestorePending;
static uint16_t wordRamSavedMemMode;
static uint8_t wordRamSavedReset;
static uint8_t z80Saved[0x0F01];
static uint16_t z80ProgramActive;

static MdResult mdFail(uint32_t address, uint32_t expected,
	uint32_t actual);
static MdResult mdTimeout(uint32_t address, uint16_t phase,
	uint32_t remaining);
static uint32_t mdVdpCommand(uint16_t address, uint32_t base);

static void mdFinishDreqRestore(void)
{
	if (!dreqRestorePending)
		return;
	*(volatile uint8_t *)0xA15107 = 0;
	DREQ_SOURCE = dreqRestoreSource;
	DREQ_DESTINATION = dreqRestoreDestination;
	DREQ_LENGTH = dreqRestoreLength;
	*(volatile uint8_t *)0xA15107 = dreqRestoreControl;
	dreqRestorePending = 0;
}

static void mdDeadlineStart(MdDeadline *deadline, uint16_t timeoutClass)
{
	static const uint8_t frames[] = { 1, 4, 2, 8, 20, 60, 180 };

	if (timeoutClass > MD_TIMEOUT_SEGA_CD)
		timeoutClass = MD_TIMEOUT_CROSS_CPU;
	deadline->start = diagnosticMdVblankCount;
	deadline->frames = frames[timeoutClass];
}

static void __attribute__((noinline)) mdDeadlineStartFrames(
	MdDeadline *deadline, uint16_t frames)
{
	deadline->start = diagnosticMdVblankCount;
	deadline->frames = frames;
}

static int mdDeadlineActive(const MdDeadline *deadline)
{
	return diagnosticMdVblankCount - deadline->start < deadline->frames;
}

static uint32_t mdDeadlineRemaining(const MdDeadline *deadline)
{
	uint32_t elapsed = diagnosticMdVblankCount - deadline->start;

	return elapsed < deadline->frames ? deadline->frames - elapsed : 0;
}

static void __attribute__((noinline)) mdWaitFrames(uint16_t frames)
{
	MdDeadline deadline;

	mdDeadlineStartFrames(&deadline, frames);
	while (mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
}

static const uint16_t mdVdpDmaPattern[] =
{
	0x1357, 0x2468, 0xAAAA, 0x5555, 0x0000, 0xFFFF, 0x0F0F, 0xF0F0,
	0x1234, 0x5678, 0x9ABC, 0xDEF0, 0x8001, 0x4002, 0x2004, 0x1008
};

static void mdPlaneText(const char *text, uint16_t x, uint16_t y)
{
	volatile uint16_t *data = (volatile uint16_t *)0xC00000;
	volatile uint32_t *control = (volatile uint32_t *)0xC00004;
	uint16_t address = 0xE000 + (((y << 6) | x) << 1);

	*control = mdVdpCommand(address, 0x40000000);
	while (*text)
	{
		uint8_t character = (uint8_t)*text++;

		*data = 0x4000 | (character >= 0x20 ? character - 0x20 : 0);
	}
}

static void mdGenesisColorBars(void)
{
	volatile uint16_t *data = (volatile uint16_t *)0xC00000;
	volatile uint32_t *control = (volatile uint32_t *)0xC00004;
	const uint16_t colors[8] =
	{
		0x0000, 0x0E00, 0x000E, 0x0E0E,
		0x00E0, 0x0EE0, 0x00EE, 0x0EEE
	};
	int bar;
	int index;
	int y;

	for (bar = 0; bar < 8; bar++)
	{
		*control = mdVdpCommand((uint16_t)(((bar >> 1) * 16 +
			1 + (bar & 1)) * 2), 0xC0000000);
		*data = colors[bar];
	}
	*control = mdVdpCommand(0x7E00, 0x40000000);
	for (index = 0; index < 16; index++)
		*data = 0x1111;
	for (index = 0; index < 16; index++)
		*data = 0x2222;
	for (y = 4; y < 24; y++)
	{
		*control = mdVdpCommand(0xE000 + (y << 7), 0x40000000);
		for (index = 0; index < 40; index++)
		{
			bar = index / 5;
			*data = ((bar >> 1) << 13) | 0x03F0 | (bar & 1);
		}
	}
	mdStatusDirty = 1;
}

static void mdStatusScreen(uint16_t catalog)
{
	volatile uint16_t *data = (volatile uint16_t *)0xC00000;
	volatile uint32_t *control = (volatile uint32_t *)0xC00004;
	char number[] = "Catalog 000 / 161";
	int index;

	if (!catalog || mdStatusDirty)
	{
		if (mdStatusDirty)
			diagnosticMdReloadFont();
		*control = mdVdpCommand(0xE000, 0x40000000);
		for (index = 0; index < 64 * 32; index++)
			*data = 0;
		mdStatusDirty = 0;
	}
	if (catalog == 161)
	{
		mdGenesisColorBars();
		return;
	}
	number[8] = '0' + (catalog / 100) % 10;
	number[9] = '0' + (catalog / 10) % 10;
	number[10] = '0' + catalog % 10;
	mdPlaneText(number, 11, 12);
}

static MdResult mdPass(void)
{
	MdResult result;

	result.status = DIAG_PASS;
	result.phase = 0;
	result.expected = 0;
	result.actual = 0;
	result.address = 0;
	result.remaining = 0;
	return result;
}

static uint16_t mdCommPattern(uint16_t catalog, uint16_t index)
{
	return (uint16_t)(0x5000 | ((catalog & 0x00FF) << 3) | index);
}

static MdResult mdCommunicationTransaction(uint16_t catalog)
{
	volatile uint16_t *words = (volatile uint16_t *)0xA15120;
	MdResult result = mdPass();
	MdDeadline deadline;
	int index;

	COMM0 = DIAG_COMM_READY;
	mdDeadlineStart(&deadline, MD_TIMEOUT_CROSS_CPU);
	while (COMM0 != DIAG_COMM_GO && mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (COMM0 != DIAG_COMM_GO)
		return mdTimeout(0xA15120, 1,
			mdDeadlineRemaining(&deadline));
	COMM0 = DIAG_COMM_ACK;
	mdDeadlineStart(&deadline, MD_TIMEOUT_CROSS_CPU);
	while (COMM0 == DIAG_COMM_ACK && mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (COMM0 == DIAG_COMM_ACK)
		return mdTimeout(0xA15120, 2,
			mdDeadlineRemaining(&deadline));
	if (catalog == 1 || catalog == 39 || catalog == 40)
	{
		if (COMM0 != DIAG_COMM_GO)
			return mdTimeout(0xA15120, 3,
				mdDeadlineRemaining(&deadline));
		for (index = 1; index < 8; index++)
			words[index] = mdCommPattern(catalog, index);
		words[0] = mdCommPattern(catalog, 0);
	}
	else if (catalog == 16)
	{
		mdDeadlineStart(&deadline, MD_TIMEOUT_CROSS_CPU);
		while (words[0] != mdCommPattern(catalog, 0) &&
			mdDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		if (words[0] != mdCommPattern(catalog, 0))
			result = mdTimeout(0xA15120, 4,
				mdDeadlineRemaining(&deadline));
		else
		{
			for (index = 0; index < 8; index++)
			{
				uint16_t expected = mdCommPattern(catalog, index);
				uint16_t actual = words[index];

				if (actual != expected)
				{
					result = mdFail(0xA15120 + index * 2,
						expected, actual);
					break;
				}
			}
		}
		COMM0 = DIAG_COMM_DONE;
	}
	mdDeadlineStart(&deadline, MD_TIMEOUT_CROSS_CPU);
	while (COMM0 != DIAG_COMM_RESTORE && mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (COMM0 != DIAG_COMM_RESTORE)
		result = mdTimeout(0xA15120, 5,
			mdDeadlineRemaining(&deadline));
	return result;
}

static MdResult mdFail(uint32_t address, uint32_t expected,
	uint32_t actual)
{
	MdResult result;

	result.status = DIAG_FAIL;
	result.phase = 0;
	result.expected = expected;
	result.actual = actual;
	result.address = address;
	result.remaining = 0;
	return result;
}

static MdResult mdTimeout(uint32_t address, uint16_t phase,
	uint32_t remaining)
{
	MdResult result = mdFail(address, 1, 0);

	if (address == 0xA15107)
		result.actual = *(volatile uint8_t *)0xA15107;
	else
		result.actual = *(volatile uint16_t *)address;
	result.status = DIAG_TIMEOUT;
	result.phase = phase;
	result.remaining = remaining;
	return result;
}

static int mdSetFm(uint8_t expected, uint8_t *actual,
	uint32_t *remaining)
{
	volatile uint8_t *reg = (volatile uint8_t *)0xA15100;
	MdDeadline deadline;
	uint8_t value;

	mdDeadlineStart(&deadline, MD_TIMEOUT_REGISTER);
	do
	{
		value = *reg;
		*reg = (value & 0x7F) | (expected & 0x80);
		value = *reg;
	} while ((value & 0x80) != (expected & 0x80) &&
		mdDeadlineActive(&deadline));
	if (actual)
		*actual = value;
	if (remaining)
		*remaining = mdDeadlineRemaining(&deadline);
	return (value & 0x80) == (expected & 0x80);
}

static MdResult mdFmTransition(uint8_t expected, uint16_t phase,
	uint16_t status)
{
	uint8_t actual;
	uint32_t remaining;

	if (mdSetFm(expected, &actual, &remaining))
		return mdPass();
	{
		MdResult result = mdFail(0xA15100, expected & 0x80,
			actual & 0x80);

		result.status = status;
		result.phase = phase;
		result.remaining = remaining;
		return result;
	}
}

static MdResult mdRwFm(void)
{
	static const uint8_t patterns[] = { 0, 0x80, 0x80, 0 };
	volatile uint8_t *reg = (volatile uint8_t *)0xA15100;
	uint8_t saved = *reg;
	int i;

	for (i = 0; i < 4; i++)
	{
		MdResult result = mdFmTransition(patterns[i], 1, DIAG_FAIL);

		if (result.status != DIAG_PASS)
		{
			MdResult restored = mdFmTransition(saved, 2,
				DIAG_INFRASTRUCTURE_ERROR);

			return restored.status == DIAG_PASS ? result : restored;
		}
	}
	return mdFmTransition(saved, 2, DIAG_INFRASTRUCTURE_ERROR);
}

static MdResult mdVdpAcquire(uint8_t *saved)
{
	*saved = *(volatile uint8_t *)0xA15100;
	return mdFmTransition(0, 1, DIAG_INFRASTRUCTURE_ERROR);
}

static MdResult mdVdpRelease(uint8_t saved)
{
	return mdFmTransition(saved, 2, DIAG_INFRASTRUCTURE_ERROR);
}

static MdResult mdRw8(uint32_t address, uint8_t mask)
{
	static const uint8_t patterns[] = { 0, 0xFF, 0xAA, 0x55 };
	volatile uint8_t *reg = (volatile uint8_t *)address;
	uint8_t saved = *reg;
	int i;

	for (i = 0; i < 4; i++)
	{
		uint8_t expected = (saved & ~mask) | (patterns[i] & mask);
		uint8_t actual;

		*reg = expected;
		__asm__ volatile ("" ::: "memory");
		actual = *reg;
		if ((actual & mask) != (expected & mask))
		{
			*reg = saved;
			return mdFail(address, expected & mask, actual & mask);
		}
	}
	*reg = saved;
	return mdPass();
}

static MdResult mdRw16(uint32_t address, uint16_t mask)
{
	static const uint16_t patterns[] = { 0, 0xFFFF, 0xAAAA, 0x5555 };
	volatile uint16_t *reg = (volatile uint16_t *)address;
	uint16_t saved = *reg;
	int i;

	for (i = 0; i < 4; i++)
	{
		uint16_t expected = (saved & ~mask) | (patterns[i] & mask);
		uint16_t actual;

		*reg = expected;
		__asm__ volatile ("" ::: "memory");
		actual = *reg;
		if ((actual & mask) != (expected & mask))
		{
			*reg = saved;
			return mdFail(address, expected & mask, actual & mask);
		}
	}
	*reg = saved;
	return mdPass();
}

static MdResult mdRw32(uint32_t address, uint32_t mask)
{
	volatile uint32_t *reg = (volatile uint32_t *)address;
	uint32_t saved = *reg;
	uint32_t expected = 0x06020000;
	uint32_t actual;

	*reg = expected;
	actual = *reg;
	*reg = saved;
	if ((actual & mask) != (expected & mask))
		return mdFail(address, expected & mask, actual & mask);
	return mdPass();
}

static MdResult mdCommunication(void)
{
	volatile uint16_t *words = (volatile uint16_t *)0xA15120;
	uint16_t saved[8];
	int i;

	for (i = 0; i < 8; i++)
		saved[i] = words[i];
	for (i = 0; i < 8; i++)
	{
		uint16_t expected = (uint16_t)(0xA55A ^ (i * 0x1111));

		words[i] = expected;
		if (words[i] != expected)
		{
			uint16_t actual = words[i];
			int restore;

			for (restore = 0; restore < 8; restore++)
				words[restore] = saved[restore];
			return mdFail(0xA15120 + i * 2, expected, actual);
		}
	}
	for (i = 0; i < 8; i++)
		words[i] = saved[i];
	return mdPass();
}

static MdResult mdPwmFifo(uint16_t catalog)
{
	volatile uint16_t *fifo = &PWM_MONO;
	uint16_t savedControl = PWM_CONTROL;
	uint16_t savedCycle = PWM_CYCLE;
	MdDeadline deadline;
	uint16_t route = 5;

	if (catalog == 13)
	{
		fifo = &PWM_LEFT;
		route = 1;
	}
	else if (catalog == 14)
	{
		fifo = &PWM_RIGHT;
		route = 4;
	}
	PWM_CONTROL = 0;
	PWM_CYCLE = 0x0800;
	PWM_CONTROL = route;
	mdWaitFrames(30);
	mdDeadlineStart(&deadline, MD_TIMEOUT_FIFO);
	while ((*fifo & 0x8000) && mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (*fifo & 0x8000)
	{
		MdResult result = mdTimeout((uint32_t)fifo, 1,
			mdDeadlineRemaining(&deadline));

		PWM_CONTROL = 0;
		PWM_CYCLE = savedCycle;
		PWM_CONTROL = savedControl;
		return result;
	}
	*fifo = 0;
	*fifo = 0;
	*fifo = 0;
	PWM_CONTROL = 0;
	PWM_CYCLE = savedCycle;
	PWM_CONTROL = savedControl;
	return mdPass();
}

static MdResult mdSystem(uint16_t catalog)
{
	switch (catalog)
	{
		case 1:
			return mdCommunication();
		case 2:
			return mdRwFm();
		case 3:
			return mdRw8(0xA15103, 0x03);
		case 4:
		{
			int failure = diagnosticMdRomBanks();

			if (failure)
			{
				static const uint32_t markers[] =
				{
					0x424B3030, 0x424B3031, 0x424B3130, 0x424B3131
				};
				uint8_t savedBank = ROM_BANK;
				uint32_t actual;

				ROM_BANK = failure - 1;
				actual = *(volatile uint32_t *)0x9FFFFC;
				ROM_BANK = savedBank;
				return mdFail(0x9FFFFC, markers[failure - 1], actual);
			}
			return mdPass();
		}
		case 5:
		{
			uint32_t failure = diagnosticMdRamDreqProbe();

			if (failure)
			{
				MdResult result = mdFail(0xA15106,
					(failure >> 8) & 0xFF, failure & 0xFF);

				result.phase = failure >> 16;
				return result;
			}
			return mdPass();
		}
		case 6:
			return mdRw32(0xA15108, 0x00FFFFFF);
		case 7:
			return mdRw32(0xA1510C, 0x00FFFFFF);
		case 8:
			return mdRw16(0xA15110, 0xFFFC);
		case 9:
			return mdRw8(0xA1511B, 0x01);
		case 10:
		{
			uint32_t before = diagnosticMdHintCount;
			MdDeadline deadline;
			volatile uint16_t *vdp = (volatile uint16_t *)0xC00004;

			*vdp = 0x8A01;
			*vdp = 0x8014;
			mdDeadlineStart(&deadline, MD_TIMEOUT_INTERRUPT);
			while (diagnosticMdHintCount == before &&
				mdDeadlineActive(&deadline))
				__asm__ volatile ("" ::: "memory");
			*vdp = 0x8004;
			if (diagnosticMdHintCount == before)
				return mdTimeout(0xC00004, 1,
					mdDeadlineRemaining(&deadline));
			return mdPass();
		}
		case 11:
			return mdRw16(0xA15130, 0x000F);
		case 12:
			return mdRw16(0xA15132, 0x0FFF);
		case 13:
		case 14:
		case 15:
			return mdPwmFifo(catalog);
		default:
			return mdFail(catalog, 1, 0);
	}
}

static MdResult mdWaitStatus(uint8_t mask)
{
	MdDeadline deadline;
	uint8_t actual;

	mdDeadlineStart(&deadline, MD_TIMEOUT_DMA);
	while (!((actual = VDP_STATUS) & mask) &&
		mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (!(actual & mask))
	{
		MdResult result = mdTimeout(0xA1518A, 1,
			mdDeadlineRemaining(&deadline));

		result.expected = mask;
		result.actual = actual;
		return result;
	}
	mdDeadlineStart(&deadline, MD_TIMEOUT_DMA);
	while (((actual = VDP_STATUS) & mask) &&
		mdDeadlineActive(&deadline))
		__asm__ volatile ("" ::: "memory");
	if (actual & mask)
	{
		MdResult result = mdTimeout(0xA1518A, 2,
			mdDeadlineRemaining(&deadline));

		result.expected = 0;
		result.actual = actual;
		return result;
	}
	return mdPass();
}

static MdResult mdVdp(uint16_t catalog)
{
	switch (catalog)
	{
		case 59:
			return mdRw16(0xA15180, 0x00C3);
		case 60:
			return mdRw16(0xA15182, 0x0001);
		case 61:
			return mdRw8(0xA15185, 0xFF);
		case 62:
			return mdRw16(0xA15186, 0xFFFF);
		case 63:
			return mdWaitStatus(0x80);
		case 64:
			return mdWaitStatus(0x40);
		case 65:
			return mdWaitStatus(0x20);
		case 66:
		{
			uint16_t before = VDP_CONTROL & 1;
			MdDeadline deadline;
			int changed;

			VDP_CONTROL = before ^ 1;
			mdDeadlineStart(&deadline, MD_TIMEOUT_DMA);
			while ((VDP_CONTROL & 1) == before &&
				mdDeadlineActive(&deadline))
				__asm__ volatile ("" ::: "memory");
			changed = (VDP_CONTROL & 1) != before;
			VDP_CONTROL = before;
			if (!changed)
			{
				MdResult result = mdTimeout(0xA1518A, 1,
					mdDeadlineRemaining(&deadline));

				mdDeadlineStart(&deadline, MD_TIMEOUT_INTERRUPT);
				while ((VDP_CONTROL & 1) != before &&
					mdDeadlineActive(&deadline))
					__asm__ volatile ("" ::: "memory");
				if ((VDP_CONTROL & 1) != before)
					result.status = DIAG_INFRASTRUCTURE_ERROR;
				return result;
			}
			mdDeadlineStart(&deadline, MD_TIMEOUT_INTERRUPT);
			while ((VDP_CONTROL & 1) != before &&
				mdDeadlineActive(&deadline))
				__asm__ volatile ("" ::: "memory");
			if ((VDP_CONTROL & 1) != before)
			{
				MdResult result = mdFail(0xA1518A, before,
					VDP_CONTROL & 1);

				result.status = DIAG_INFRASTRUCTURE_ERROR;
				return result;
			}
			return mdPass();
		}
		default:
			return mdFail(catalog, 1, 0);
	}
}

static int mdFramebufferOne(uint16_t catalog)
{
	return catalog == 84 || catalog == 90 || catalog == 92 ||
		catalog == 102 || catalog == 104;
}

static int mdSelectFramebuffer(uint16_t framebuffer, uint32_t *remaining)
{
	MdDeadline deadline;
	uint8_t actual = VDP_CONTROL & 1;
	uint8_t expected = framebuffer & 1;

	if (actual == expected)
	{
		if (remaining)
			*remaining = 0;
		return 1;
	}
	mdDeadlineStart(&deadline, MD_TIMEOUT_DMA);
	VDP_CONTROL = expected;
	do
		actual = VDP_CONTROL & 1;
	while (actual != expected && mdDeadlineActive(&deadline));
	if (remaining)
		*remaining = mdDeadlineRemaining(&deadline);
	return actual == expected;
}

static int mdWaitAutofill(uint32_t *remaining)
{
	MdDeadline deadline;
	uint8_t actual;

	mdDeadlineStart(&deadline, MD_TIMEOUT_DMA);
	do
		actual = VDP_CONTROL;
	while ((actual & 2) && mdDeadlineActive(&deadline));
	if (remaining)
		*remaining = mdDeadlineRemaining(&deadline);
	return !(actual & 2);
}

static void mdFramebufferFinish(MdResult *result, uint16_t framebuffer,
	uint16_t fillLength, uint16_t fillAddress)
{
	uint32_t remaining;

	VDP_FILL_LENGTH_LOW = fillLength;
	VDP_FILL_ADDRESS = fillAddress;
	if (!mdSelectFramebuffer(framebuffer, &remaining))
	{
		result->status = DIAG_INFRASTRUCTURE_ERROR;
		result->expected = framebuffer & 1;
		result->actual = VDP_CONTROL & 1;
		result->address = 0xA1518A;
		result->phase = 2;
		result->remaining = remaining;
	}
}

static MdResult mdFramebuffer(uint16_t catalog)
{
	volatile uint16_t *words = (volatile uint16_t *)0x840200;
	volatile uint8_t *bytes = (volatile uint8_t *)0x840200;
	volatile uint16_t *overwrite = (volatile uint16_t *)0x860200;
	uint16_t saved[16];
	uint16_t savedFb = VDP_CONTROL & 1;
	uint16_t savedFillLength = VDP_FILL_LENGTH_LOW;
	uint16_t savedFillAddress = VDP_FILL_ADDRESS;
	uint32_t remaining;
	int i;

	if (!mdWaitAutofill(&remaining))
	{
		MdResult result = mdTimeout(0xA1518A, 1, remaining);

		result.status = DIAG_INFRASTRUCTURE_ERROR;
		return result;
	}
	if (!mdSelectFramebuffer(mdFramebufferOne(catalog) ^ 1, &remaining))
	{
		MdResult result = mdTimeout(0xA1518A, 1, remaining);

		mdFramebufferFinish(&result, savedFb, savedFillLength,
			savedFillAddress);
		return result;
	}
	for (i = 0; i < 16; i++)
		saved[i] = words[i];
	if (catalog == 91 || catalog == 92)
	{
		for (i = 0; i < 16; i++)
			bytes[i] = 0x90 + i;
		for (i = 0; i < 16; i++)
		{
			if (bytes[i] != (uint8_t)(0x90 + i))
			{
				MdResult result = mdFail(0x840200 + i,
					0x90 + i, bytes[i]);
				for (i = 0; i < 16; i++)
					words[i] = saved[i];
				mdFramebufferFinish(&result, savedFb, savedFillLength,
					savedFillAddress);
				return result;
			}
		}
	}
	else if (catalog == 89 || catalog == 90)
	{
		for (i = 0; i < 16; i++)
		{
			words[i] = 0x5500 | i;
			overwrite[i] = 0x00AA;
			if ((words[i] & 0x00FF) != 0x00AA)
			{
				MdResult result = mdFail(0x860200 + i * 2,
					0x00AA, words[i] & 0x00FF);
				for (i = 0; i < 16; i++)
					words[i] = saved[i];
				mdFramebufferFinish(&result, savedFb, savedFillLength,
					savedFillAddress);
				return result;
			}
		}
	}
	else if (catalog >= 101)
	{
		uint16_t fillAddress;

		VDP_FILL_LENGTH_LOW = 15;
		VDP_FILL_ADDRESS = 0x0100;
		VDP_FILL_DATA = 0x73C1;
		if (!mdWaitAutofill(&remaining))
		{
			MdResult result = mdTimeout(0xA1518A, 2, remaining);

			result.status = DIAG_INFRASTRUCTURE_ERROR;
			return result;
		}
		fillAddress = VDP_FILL_ADDRESS;
		if (catalog == 103 || catalog == 104)
		{
			if ((fillAddress & 0x00FF) != 0x0010)
			{
				MdResult result = mdFail(0xA15186, 0x0010,
					fillAddress & 0x00FF);
				for (i = 0; i < 16; i++)
					words[i] = saved[i];
				mdFramebufferFinish(&result, savedFb, savedFillLength,
					savedFillAddress);
				return result;
			}
		}
		else
		{
			for (i = 0; i < 16; i++)
			{
				if (words[i] != 0x73C1)
				{
					MdResult result = mdFail(0x840200 + i * 2,
						0x73C1, words[i]);
					for (i = 0; i < 16; i++)
						words[i] = saved[i];
					mdFramebufferFinish(&result, savedFb, savedFillLength,
						savedFillAddress);
					return result;
				}
			}
		}
	}
	else
	{
		for (i = 0; i < 16; i++)
		{
			words[i] = 0x5A00 | i;
			if (words[i] != (uint16_t)(0x5A00 | i))
			{
				MdResult result = mdFail(0x840200 + i * 2,
					0x5A00 | i, words[i]);
				for (i = 0; i < 16; i++)
					words[i] = saved[i];
				mdFramebufferFinish(&result, savedFb, savedFillLength,
					savedFillAddress);
				return result;
			}
		}
	}
	for (i = 0; i < 16; i++)
		words[i] = saved[i];
	{
		MdResult result = mdPass();

		mdFramebufferFinish(&result, savedFb, savedFillLength,
			savedFillAddress);
		return result;
	}
}

static MdResult mdPalette(uint16_t catalog)
{
	volatile uint16_t *palette = (volatile uint16_t *)0xA15200;
	uint16_t saved[16];
	uint16_t mode = VDP_MODE;
	uint16_t fillLength = VDP_FILL_LENGTH_LOW;
	uint16_t fillAddress = VDP_FILL_ADDRESS;
	int i;

	for (i = 0; i < 16; i++)
		saved[i] = palette[i];
	if (catalog == 113)
		VDP_MODE = mode & ~3;
	if (catalog == 115)
	{
		VDP_FILL_LENGTH_LOW = 0xFF;
		VDP_FILL_ADDRESS = 0x7000;
		VDP_FILL_DATA = 0x4D44;
	}
	for (i = 0; i < 16; i++)
	{
		uint16_t expected = 0x8000 | (i << 6) | i;

		palette[i] = expected;
		if (palette[i] != expected)
		{
			MdResult result = mdFail(0xA15200 + i * 2,
				expected, palette[i]);
			for (i = 0; i < 16; i++)
				palette[i] = saved[i];
			VDP_FILL_LENGTH_LOW = fillLength;
			VDP_FILL_ADDRESS = fillAddress;
			VDP_MODE = mode;
			return result;
		}
	}
	for (i = 0; i < 16; i++)
		palette[i] = saved[i];
	if (catalog == 115)
	{
		uint32_t remaining;

		if (!mdWaitAutofill(&remaining))
		{
			{
				MdResult result = mdTimeout(0xA1518A, 1, remaining);

				result.status = DIAG_INFRASTRUCTURE_ERROR;
				return result;
			}
		}
		VDP_FILL_LENGTH_LOW = fillLength;
		VDP_FILL_ADDRESS = fillAddress;
	}
	VDP_MODE = mode;
	return mdPass();
}

static uint32_t mdVdpCommand(uint16_t address, uint32_t base)
{
	return base | ((uint32_t)(address & 0x3FFF) << 16) |
		((address & 0xC000) >> 14);
}

static MdResult mdRomToVramDma(void)
{
	volatile uint16_t *data = (volatile uint16_t *)0xC00000;
	volatile uint32_t *control = (volatile uint32_t *)0xC00004;
	uint16_t saved[16];
	uint16_t destination = 0xA000;
	int i;

	*control = mdVdpCommand(destination, 0);
	for (i = 0; i < 16; i++)
		saved[i] = *data;
	if (diagnosticMdVdpDma((uint32_t)mdVdpDmaPattern -
		DIAGNOSTIC_MD_ROM_BASE,
		destination, 16))
	{
		MdResult result = mdTimeout(0xC00004, 1, 0);

		*control = mdVdpCommand(destination, 0x40000000);
		for (i = 0; i < 16; i++)
			*data = saved[i];
		return result;
	}
	*control = mdVdpCommand(destination, 0);
	for (i = 0; i < 16; i++)
	{
		uint16_t actual = *data;

		if (actual != mdVdpDmaPattern[i])
		{
			MdResult result = mdFail(0xC00000,
				mdVdpDmaPattern[i], actual);
			*control = mdVdpCommand(destination, 0x40000000);
			for (i = 0; i < 16; i++)
				*data = saved[i];
			return result;
		}
	}
	*control = mdVdpCommand(destination, 0x40000000);
	for (i = 0; i < 16; i++)
		*data = saved[i];
	return mdPass();
}

static int segaCdPresent(void)
{
	volatile uint8_t *locations[] =
	{
		(volatile uint8_t *)0x41586D,
		(volatile uint8_t *)0x41606D,
		(volatile uint8_t *)0x41AD6D
	};
	int i;

	for (i = 0; i < 3; i++)
	{
		if (locations[i][0] == 'S' && locations[i][1] == 'E' &&
			locations[i][2] == 'G' && locations[i][3] == 'A')
			return 1;
	}
	return 0;
}

static MdResult mdFinishWordRamRestore(void)
{
	if (!wordRamRestorePending)
		return mdPass();
	SCD_MEM_MODE = wordRamSavedMemMode;
	SCD_RESET = wordRamSavedReset;
	wordRamRestorePending = 0;
	return mdPass();
}

static MdResult mdWordRamPrepare(void)
{
	volatile uint16_t *wordRam = (volatile uint16_t *)0x600000;
	const uint16_t *source =
		(const uint16_t *)DIAGNOSTIC_WORD_RAM_ROM_WINDOW;
	MdDeadline deadline;
	uint8_t savedBank;
	uint8_t adapter;
	MdResult result;
	MdResult released;
	uint32_t i;

	if (!segaCdPresent())
	{
		result = mdPass();
		result.status = DIAG_UNAVAILABLE;
		return result;
	}
	mdFinishDreqRestore();
	result = mdFinishWordRamRestore();
	if (result.status != DIAG_PASS)
		return result;
	wordRamSavedReset = SCD_RESET;
	SCD_RESET = wordRamSavedReset | 2;
	mdDeadlineStart(&deadline, MD_TIMEOUT_SEGA_CD);
	while (!(SCD_RESET & 2) && mdDeadlineActive(&deadline))
		SCD_RESET = wordRamSavedReset | 2;
	if (!(SCD_RESET & 2))
	{
		SCD_RESET = wordRamSavedReset;
		return mdTimeout(0xA12001, 2,
			mdDeadlineRemaining(&deadline));
	}
	result = mdVdpAcquire(&adapter);
	if (result.status != DIAG_PASS)
	{
		SCD_RESET = wordRamSavedReset;
		return result;
	}
	savedBank = ROM_BANK;
	ROM_BANK = DIAGNOSTIC_WORD_RAM_ROM_BANK;
	wordRamSavedMemMode = SCD_MEM_MODE;
	SCD_MEM_MODE = 0;
	for (i = 0; i < 0x8000; i++)
		wordRam[i] = source[i];
	for (i = 0; i < 0x8000; i++)
	{
		if (wordRam[i] != source[i])
		{
			result = mdFail((uint32_t)&wordRam[i], source[i], wordRam[i]);
			SCD_MEM_MODE = wordRamSavedMemMode;
			SCD_RESET = wordRamSavedReset;
			ROM_BANK = savedBank;
			released = mdVdpRelease(adapter);
			return released.status == DIAG_PASS ? result : released;
		}
	}
	ROM_BANK = savedBank;
	dreqRestoreControl = *(volatile uint8_t *)0xA15107;
	dreqRestoreSource = DREQ_SOURCE;
	dreqRestoreDestination = DREQ_DESTINATION;
	dreqRestoreLength = DREQ_LENGTH;
	*(volatile uint8_t *)0xA15107 = 0;
	DREQ_LENGTH = 0x8000;
	dreqRestorePending = 1;
	wordRamRestorePending = 1;
	released = mdVdpRelease(adapter);
	return released.status == DIAG_PASS ? mdPass() : released;
}

static MdResult mdWordRamCapture(void)
{
	MdResult result;

	if (!wordRamRestorePending || !dreqRestorePending)
		return mdFail(0xA15110, 0x8000, DREQ_LENGTH);
	DREQ_SOURCE = 0x00600000;
	*(volatile uint8_t *)0xA15107 = 6;
	if (diagnosticMdWordRamVdpDma())
		result = mdTimeout(0xA15107, 2, DREQ_LENGTH);
	else
		result = mdPass();
	mdStatusDirty = 1;
	return result;
}

static uint16_t mdDmaPattern(uint16_t index)
{
	return (uint16_t)(0x5AA5 ^ (index * 0x1111));
}

static MdResult mdDreqFeed(void)
{
	uint8_t savedControl = *(volatile uint8_t *)0xA15107;
	uint16_t savedLength = DREQ_LENGTH;
	MdDeadline deadline;
	uint16_t index;

	*(volatile uint8_t *)0xA15107 = 0;
	DREQ_LENGTH = 64;
	*(volatile uint8_t *)0xA15107 = 4;
	for (index = 0; index < 64; index++)
	{
		mdDeadlineStart(&deadline, MD_TIMEOUT_DMA);
		while ((*(volatile uint8_t *)0xA15107 & 0x80) &&
			mdDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		if (*(volatile uint8_t *)0xA15107 & 0x80)
		{
			*(volatile uint8_t *)0xA15107 = 0;
			DREQ_LENGTH = savedLength;
			*(volatile uint8_t *)0xA15107 = savedControl;
			return mdTimeout(0xA15107, 1,
				mdDeadlineRemaining(&deadline));
		}
		DREQ_FIFO = mdDmaPattern(index);
	}
	*(volatile uint8_t *)0xA15107 = 0;
	DREQ_LENGTH = savedLength;
	*(volatile uint8_t *)0xA15107 = savedControl;
	return mdPass();
}

static void z80Bus(int request)
{
	*(volatile uint16_t *)0xA11100 = request ? 0x0100 : 0;
}

static void mdPwmDrain(void)
{
	uint16_t limit = 0xFFFF;
	uint16_t left;
	uint16_t right;

	PWM_CONTROL = 0;
	PWM_CYCLE = 0x0412;
	PWM_CONTROL = 5;
	left = PWM_LEFT;
	right = PWM_RIGHT;
	while ((!(left & 0x4000) || !(right & 0x4000)) && --limit)
	{
		left = PWM_LEFT;
		right = PWM_RIGHT;
	}
	PWM_CONTROL = 0;
}

static int mdZ80Acquire(MdResult *result)
{
	volatile uint8_t *busRequest = (volatile uint8_t *)0xA11100;
	uint16_t remaining = 0xFFFF;

	z80Bus(1);
	while ((*busRequest & 1) && --remaining)
		__asm__ volatile ("" ::: "memory");
	if (!(*busRequest & 1))
		return 1;
	result->status = DIAG_INFRASTRUCTURE_ERROR;
	result->address = 0xA11100;
	z80Bus(0);
	return 0;
}

static MdResult mdZ80Finish(void)
{
	volatile uint8_t *z80 = (volatile uint8_t *)0xA00000;
	MdResult result = mdPass();
	uint32_t i;

	if (!z80ProgramActive)
		return result;
	if (!mdZ80Acquire(&result))
		return result;
	*(volatile uint16_t *)0xA11200 = 0;
	for (i = 0; i < 0x0F01; i++)
		z80[i] = z80Saved[i];
	*(volatile uint16_t *)0xA11200 = 0x0100;
	z80Bus(0);
	z80ProgramActive = 0;
	mdPwmDrain();
	return result;
}

static MdResult mdZ80Play(uint16_t catalog)
{
	volatile uint8_t *z80 = (volatile uint8_t *)0xA00000;
	const uint8_t *source = (const uint8_t *)DIAGNOSTIC_Z80_ROM_WINDOW;
	MdResult result = mdPass();
	uint8_t savedBank;
	uint32_t i;

	if (!z80ProgramActive)
		mdPwmDrain();
	mdWaitFrames(2);
	if (!z80ProgramActive)
	{
		*(volatile uint16_t *)0xA11200 = 0x0100;
		if (!mdZ80Acquire(&result))
			return result;
		for (i = 0; i < 0x0F01; i++)
			z80Saved[i] = z80[i];
		savedBank = ROM_BANK;
		ROM_BANK = DIAGNOSTIC_Z80_ROM_BANK;
		for (i = 0; i < DIAGNOSTIC_Z80_PAYLOAD_SIZE; i++)
			z80[i] = source[i];
		ROM_BANK = savedBank;
		z80[0x0F00] = 0;
		*(volatile uint16_t *)0xA11200 = 0;
		z80Bus(0);
		*(volatile uint16_t *)0xA11200 = 0x0100;
		z80ProgramActive = 1;
		mdWaitFrames(10);
	}
	if (!mdZ80Acquire(&result))
		return result;
	z80[0x0F00] = catalog - 153;
	z80Bus(0);
	mdWaitFrames(60);
	if (!mdZ80Acquire(&result))
		return result;
	z80[0x0F00] = 0;
	z80Bus(0);
	if (catalog == 156)
	{
		result = mdZ80Finish();
		if (result.status != DIAG_PASS)
			return result;
	}
	result.status = DIAG_MANUAL;
	return result;
}

static MdResult mdSound(uint16_t catalog)
{
	volatile uint16_t *fifo = &PWM_MONO;
	const uint16_t *source = (const uint16_t *)DIAGNOSTIC_Z80_WAVEFORM;
	uint16_t samples[DIAGNOSTIC_Z80_WAVEFORM_WORDS];
	uint16_t route = 5;
	uint16_t savedControl = PWM_CONTROL;
	uint16_t savedCycle = PWM_CYCLE;
	uint8_t savedBank;
	uint32_t start;
	uint16_t i;
	uint16_t duration = 60;
	uint16_t sampleIndex = 0;
	int switched = 0;

	if (catalog >= 154)
		return mdZ80Play(catalog);
	if (catalog == 136)
	{
		fifo = &PWM_LEFT;
		route = 1;
	}
	else if (catalog == 137)
	{
		fifo = &PWM_RIGHT;
		route = 4;
	}
	else if (catalog == 138)
	{
		fifo = &PWM_LEFT;
		route = 1;
		duration = 120;
	}
	else if (catalog == 139)
	{
		fifo = &PWM_RIGHT;
		route = 4;
		duration = 120;
	}
	savedBank = ROM_BANK;
	ROM_BANK = DIAGNOSTIC_Z80_ROM_BANK;
	for (i = 0; i < DIAGNOSTIC_Z80_WAVEFORM_WORDS; i++)
		samples[i] = source[i];
	ROM_BANK = savedBank;
	mdPwmDrain();
	mdWaitFrames(2);
	PWM_CONTROL = 0;
	PWM_MONO = 0;
	PWM_MONO = 0;
	PWM_MONO = 0;
	PWM_CYCLE = 0x0412;
	PWM_CONTROL = route;
	start = diagnosticMdVblankCount;
	while (diagnosticMdVblankCount - start < duration)
	{
		MdDeadline deadline;
		uint16_t sample = samples[sampleIndex];
		uint32_t elapsed = diagnosticMdVblankCount - start;

		mdDeadlineStart(&deadline, MD_TIMEOUT_FIFO);
		while ((*fifo & 0x8000) && mdDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		if (*fifo & 0x8000)
		{
			PWM_CONTROL = 0;
			PWM_CYCLE = savedCycle;
			PWM_CONTROL = savedControl;
			return mdTimeout((uint32_t)fifo, 1,
				mdDeadlineRemaining(&deadline));
		}
		*fifo = sample;
		if (++sampleIndex >= DIAGNOSTIC_Z80_WAVEFORM_WORDS)
			sampleIndex = 0;
		if (!switched && elapsed >= 60 && catalog == 138)
		{
			PWM_CONTROL = 8;
			switched = 1;
		}
		else if (!switched && elapsed >= 60 && catalog == 139)
		{
			PWM_CONTROL = 2;
			switched = 1;
		}
	}
	{
		MdDeadline deadline;

		mdDeadlineStart(&deadline, MD_TIMEOUT_FIFO);
		while (!(*fifo & 0x4000) && mdDeadlineActive(&deadline))
			__asm__ volatile ("" ::: "memory");
		if (!(*fifo & 0x4000))
		{
			PWM_CONTROL = 0;
			PWM_CYCLE = savedCycle;
			PWM_CONTROL = savedControl;
			return mdTimeout((uint32_t)fifo, 2,
				mdDeadlineRemaining(&deadline));
		}
	}
	PWM_CONTROL = 0;
	mdPwmDrain();
	PWM_CYCLE = savedCycle;
	PWM_CONTROL = savedControl;
	{
		MdResult result = mdPass();

		result.status = DIAG_MANUAL;
		return result;
	}
}

static MdResult mdCross(uint16_t catalog)
{
	if (catalog == 39 || catalog == 40)
		return mdCommunication();
	if (catalog == 41 || catalog == 42)
	{
		volatile uint8_t *reg = (volatile uint8_t *)0xA15100;
		uint8_t expected = *reg ^ 0x80;

		return mdFmTransition(expected, 1, DIAG_FAIL);
	}
	if (catalog == 43 || catalog == 44)
	{
		uint32_t failure = diagnosticMdCrossDreqControl();

		if (failure)
		{
			MdResult result = mdFail(0xA15107,
				(failure >> 8) & 0xFF, failure & 0xFF);

			result.phase = failure >> 16;
			return result;
		}
		return mdPass();
	}
	if (catalog == 45 || catalog == 46)
	{
		volatile uint32_t *reg = (volatile uint32_t *)0xA15108;
		uint32_t actual;

		*reg = 0x06020000;
		actual = *reg;
		return (actual & 0x00FFFFFF) == 0x00020000 ? mdPass() :
			mdFail(0xA15108, 0x00020000, actual & 0x00FFFFFF);
	}
	if (catalog == 47 || catalog == 48)
	{
		volatile uint32_t *reg = (volatile uint32_t *)0xA1510C;
		uint32_t actual;

		*reg = 0x06020000;
		actual = *reg;
		return (actual & 0x00FFFFFF) == 0x00020000 ? mdPass() :
			mdFail(0xA1510C, 0x00020000, actual & 0x00FFFFFF);
	}
	if (catalog == 49 || catalog == 50)
	{
		volatile uint16_t *reg = (volatile uint16_t *)0xA15110;
		uint16_t expected = (*reg & 0x0003) | 0xA55C;
		uint16_t actual;

		*reg = expected;
		actual = *reg;
		return (actual & 0xFFFC) == (expected & 0xFFFC) ? mdPass() :
			mdFail(0xA15110, expected & 0xFFFC, actual & 0xFFFC);
	}
	if (catalog >= 51 && catalog <= 58)
	{
		uint8_t saved = *(volatile uint8_t *)0xA15103;
		uint8_t bit = (catalog & 1) ? 1 : 2;

		*(volatile uint8_t *)0xA15103 = saved | bit;
		*(volatile uint8_t *)0xA15103 = saved;
		return mdPass();
	}
	return mdFail(catalog, 1, 0);
}

static MdResult mdVdpOwnedExecute(uint16_t catalog)
{
	uint8_t saved;
	MdResult result = mdVdpAcquire(&saved);
	MdResult released;

	if (result.status != DIAG_PASS)
		return result;
	if (catalog >= 59 && catalog <= 66)
		result = mdVdp(catalog);
	else if ((catalog >= 83 && catalog <= 84) ||
		(catalog >= 89 && catalog <= 92) ||
		(catalog >= 101 && catalog <= 104))
		result = mdFramebuffer(catalog);
	else if (catalog >= 113 && catalog <= 115)
		result = mdPalette(catalog);
	else
		result = mdFail(catalog, 1, 0);
	released = mdVdpRelease(saved);
	return released.status == DIAG_PASS ? result : released;
}

static MdResult mdExecute(uint16_t catalog)
{
	if (catalog == 124 || catalog == 125)
		return mdWordRamCapture();
	if (catalog <= 15)
		return mdSystem(catalog);
	if (catalog >= 39 && catalog <= 58)
		return mdCross(catalog);
	if ((catalog >= 59 && catalog <= 66) ||
		(catalog >= 83 && catalog <= 84) ||
		(catalog >= 89 && catalog <= 92) ||
		(catalog >= 101 && catalog <= 104) ||
		(catalog >= 113 && catalog <= 115))
		return mdVdpOwnedExecute(catalog);
	if (catalog == 122 || catalog == 123)
		return mdDreqFeed();
	if (catalog == 126)
		return mdRomToVramDma();
	if ((catalog >= 135 && catalog <= 139) ||
		(catalog >= 154 && catalog <= 156))
		return mdSound(catalog);
	return mdPass();
}

static uint16_t mdPad(uint16_t port)
{
	volatile uint8_t *data = (volatile uint8_t *)(port ? 0xA10005 : 0xA10003);
	volatile uint8_t *control = (volatile uint8_t *)(port ? 0xA1000B : 0xA10009);
	uint8_t high;
	uint8_t low;
	uint16_t value;

	*control = 0x40;
	*data = 0x40;
	high = *data;
	*data = 0;
	low = *data;
	*data = 0x40;
	if (low & 0x0C)
		return SEGA_CTRL_NONE;
	value = (~high & 0x3F) | ((~low & 0x30) << 2);
	return SEGA_CTRL_THREE | value;
}

static MdResult saveRuntime(void)
{
	uint8_t adapter;
	MdResult result;

	savedState.adapter = ADAPTER_CONTROL;
	savedState.bank = ROM_BANK;
	savedState.dreqControl = DREQ_CONTROL;
	savedState.dreqSource = DREQ_SOURCE;
	savedState.dreqDestination = DREQ_DESTINATION;
	savedState.dreqLength = DREQ_LENGTH;
	savedState.pwmControl = PWM_CONTROL;
	savedState.pwmCycle = PWM_CYCLE;
	result = mdVdpAcquire(&adapter);
	if (result.status != DIAG_PASS)
		return result;
	savedState.vdpMode = VDP_MODE;
	savedState.vdpShift = VDP_SHIFT;
	savedState.vdpFillLength = VDP_FILL_LENGTH_LOW;
	savedState.vdpFillAddress = VDP_FILL_ADDRESS;
	savedState.vdpFbctl = VDP_CONTROL;
	return mdVdpRelease(adapter);
}

static MdResult restoreRuntime(void)
{
	volatile uint16_t *data = (volatile uint16_t *)0xC00000;
	volatile uint32_t *control = (volatile uint32_t *)0xC00004;
	uint8_t adapter;
	MdResult result;
	MdResult restored;
	int index;

	result = mdZ80Finish();
	restored = mdFinishWordRamRestore();
	if (result.status == DIAG_PASS)
		result = restored;
	mdFinishDreqRestore();
	PWM_CONTROL = 0;
	DREQ_CONTROL = 0;
	ROM_BANK = savedState.bank;
	DREQ_SOURCE = savedState.dreqSource;
	DREQ_DESTINATION = savedState.dreqDestination;
	DREQ_LENGTH = savedState.dreqLength;
	DREQ_CONTROL = savedState.dreqControl;
	PWM_CYCLE = savedState.pwmCycle;
	PWM_CONTROL = savedState.pwmControl;
	restored = mdVdpAcquire(&adapter);
	if (restored.status == DIAG_PASS)
	{
		VDP_SHIFT = savedState.vdpShift;
		VDP_FILL_LENGTH_LOW = savedState.vdpFillLength;
		VDP_FILL_ADDRESS = savedState.vdpFillAddress;
		VDP_CONTROL = savedState.vdpFbctl & 1;
		VDP_MODE = savedState.vdpMode;
	}
	else
		result = restored;
	ADAPTER_CONTROL = savedState.adapter;
	restored = mdVdpRelease(savedState.adapter >> 8);
	if (restored.status != DIAG_PASS)
		result = restored;
	if (mdStatusDirty)
	{
		diagnosticMdReloadFont();
		mdStatusDirty = 0;
	}
	*control = mdVdpCommand(0xE000, 0x40000000);
	for (index = 0; index < 64 * 32; index++)
		*data = 0;
	return result;
}

uint16_t diagnosticMdDispatch(uint16_t command)
{
	uint16_t argument = COMM2;
	MdResult result = mdPass();

	if (command == DIAG_MD_ENTER)
	{
		diagnosticMdVblankCount = 0;
		diagnosticMdActive = 1;
		result = saveRuntime();
		lastResult = result;
		if (result.status != DIAG_PASS)
		{
			diagnosticMdActive = 0;
			controller_force_reset = 1;
			return result.status;
		}
		lastResult = mdPass();
		mdStatusScreen(0);
		return DIAG_PASS;
	}
	if (command == DIAG_MD_EXIT)
	{
		result = mdZ80Finish();
		if (result.status != DIAG_PASS)
			return result.status;
		mdFinishDreqRestore();
		result = mdFinishWordRamRestore();
		diagnosticMdActive = 0;
		controller_force_reset = 1;
		return result.status;
	}
	if (command == DIAG_MD_RESTORE)
	{
		diagnosticMdVblankCount = 0;
		diagnosticMdActive = 1;
		lastResult = restoreRuntime();
		diagnosticMdActive = 0;
		controller_force_reset = 1;
		return lastResult.status;
	}
	if (command == DIAG_MD_PAD)
		return mdPad(argument);
	if (command == DIAG_MD_PING)
		return DIAG_AGENT_VERSION;
	if (command == DIAG_MD_STATUS)
	{
		mdFinishDreqRestore();
		lastResult = mdFinishWordRamRestore();
		if (lastResult.status == DIAG_PASS)
			mdStatusScreen(argument);
		return lastResult.status;
	}
	if (command == DIAG_MD_DETAILS)
	{
		COMM8 = lastResult.address >> 16;
		COMM10 = lastResult.address;
		COMM12 = lastResult.remaining >> 16;
		COMM14 = lastResult.remaining;
		return lastResult.phase;
	}
	if (command == DIAG_MD_SEGA_CD)
		return segaCdPresent();
	if (command == DIAG_MD_COMMUNICATION)
		result = mdCommunicationTransaction(argument);
	if (command == DIAG_MD_PREPARE)
		result = mdWordRamPrepare();
	else if (command == DIAG_MD_EXECUTE)
		result = mdExecute(argument);
	else if (command != DIAG_MD_COMMUNICATION && command != DIAG_MD_PREPARE)
		result = mdFail(command, DIAG_MD_EXECUTE, command);
	lastResult = result;
	COMM8 = result.expected >> 16;
	COMM10 = result.expected;
	COMM12 = result.actual >> 16;
	COMM14 = result.actual;
	return result.status;
}
