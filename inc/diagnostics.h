#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include "types.h"

#define DIAGNOSTIC_TEST_COUNT 161
#define DIAGNOSTIC_GROUP_COUNT 10
#define DIAGNOSTIC_FAILURE_CAPACITY 16
#define DIAGNOSTIC_CONTINUOUS 0xFFFF

typedef enum
{
	DIAG_GROUP_MD_SYSTEM,
	DIAG_GROUP_SH2_SYSTEM,
	DIAG_GROUP_CROSS_CPU,
	DIAG_GROUP_MD_VDP,
	DIAG_GROUP_SH2_VDP,
	DIAG_GROUP_FRAMEBUFFER,
	DIAG_GROUP_PALETTE,
	DIAG_GROUP_DMA,
	DIAG_GROUP_SOUND,
	DIAG_GROUP_GRAPHICS
} DiagnosticGroupId;

typedef enum
{
	DIAG_TARGET_MD,
	DIAG_TARGET_MASTER,
	DIAG_TARGET_SLAVE,
	DIAG_TARGET_MD_MASTER,
	DIAG_TARGET_MD_SLAVE,
	DIAG_TARGET_MASTER_SLAVE,
	DIAG_TARGET_SEGA_CD_MASTER,
	DIAG_TARGET_SEGA_CD_SLAVE,
	DIAG_TARGET_Z80,
	DIAG_TARGET_DISPLAY
} DiagnosticTarget;

typedef enum
{
	DIAG_OP_MD_SYSTEM = 1,
	DIAG_OP_SH2_SYSTEM,
	DIAG_OP_CROSS_CPU,
	DIAG_OP_MD_VDP,
	DIAG_OP_SH2_VDP,
	DIAG_OP_FRAMEBUFFER,
	DIAG_OP_PALETTE,
	DIAG_OP_DMA,
	DIAG_OP_SOUND,
	DIAG_OP_GRAPHICS
} DiagnosticOperation;

typedef enum
{
	DIAG_KIND_AUTOMATED,
	DIAG_KIND_AUDIBLE,
	DIAG_KIND_VISUAL,
	DIAG_KIND_UPGRADED
} DiagnosticResultKind;

typedef enum
{
	DIAG_TIMEOUT_REGISTER,
	DIAG_TIMEOUT_FIFO,
	DIAG_TIMEOUT_INTERRUPT,
	DIAG_TIMEOUT_CROSS_CPU,
	DIAG_TIMEOUT_DMA,
	DIAG_TIMEOUT_STIMULUS,
	DIAG_TIMEOUT_SEGA_CD
} DiagnosticTimeoutClass;

typedef enum
{
	DIAG_NOT_RUN,
	DIAG_PASS,
	DIAG_FAIL,
	DIAG_TIMEOUT,
	DIAG_MANUAL,
	DIAG_UNAVAILABLE,
	DIAG_ABORTED,
	DIAG_INFRASTRUCTURE_ERROR
} DiagnosticStatus;

typedef enum
{
	DIAG_STATE_NORMAL_UI,
	DIAG_STATE_OPTIONS,
	DIAG_STATE_PREFLIGHT,
	DIAG_STATE_QUIESCE,
	DIAG_STATE_RUN_GROUP,
	DIAG_STATE_RUN_TEST,
	DIAG_STATE_RESTORE,
	DIAG_STATE_RESULTS
} DiagnosticRunState;

enum
{
	DIAG_NEEDS_COMM = 1,
	DIAG_NEEDS_VDP = 2,
	DIAG_NEEDS_PWM = 4,
	DIAG_NEEDS_DMA = 8,
	DIAG_NEEDS_SEGA_CD = 16,
	DIAG_NEEDS_INTERRUPTS = 32
};

typedef struct
{
	u16 catalog;
	u8 group;
	u8 target;
	u8 operation;
	u8 kind;
	u8 timeout;
	u8 requirements;
	u8 cleanup;
	u8 safeContinue;
	const char *name;
} DiagnosticDescriptor;

typedef struct
{
	u8 status;
	u8 cpu;
	u8 phase;
	u8 reserved;
	u16 catalog;
	u32 expected;
	u32 actual;
	u32 address;
	u32 remaining;
} DiagnosticResult;

typedef struct
{
	const char *name;
	u16 first;
	u16 count;
} DiagnosticGroup;

typedef struct
{
	u32 passed;
	u32 failed;
	u32 manual;
	u32 unavailable;
	u32 notRun;
	u32 timeout;
	u32 aborted;
	u32 infrastructure;
} DiagnosticCounts;

typedef struct
{
	u16 requestedPasses;
	u16 completedPasses;
	u16 currentPass;
	u16 currentTest;
	u8 currentGroup;
	u8 continuous;
	u8 aborted;
	u8 fatal;
	u8 segaCd;
	u8 mixerActive;
	u8 failureCount;
	u8 failureStart;
	u16 failureTotal;
	u8 state;
	DiagnosticCounts counts;
	DiagnosticResult results[DIAGNOSTIC_TEST_COUNT];
	DiagnosticResult firstFailure;
	DiagnosticResult failures[DIAGNOSTIC_FAILURE_CAPACITY];
} DiagnosticContext;

extern const DiagnosticDescriptor diagnosticManifest[DIAGNOSTIC_TEST_COUNT];
extern const DiagnosticGroup diagnosticGroups[DIAGNOSTIC_GROUP_COUNT];
extern DiagnosticContext diagnosticContext;

void diagnosticsMenu(void);
int diagnosticsValidateManifest(void);
DiagnosticStatus diagnosticsExecute(const DiagnosticDescriptor *descriptor,
	DiagnosticResult *result);
int diagnosticsSecondaryEnter(void);

#endif
