#ifndef DIAGNOSTIC_HW_H
#define DIAGNOSTIC_HW_H

#include "diagnostics.h"

#define DIAG_AGENT_VERSION 0x0100
#define DIAG_MD_COMMAND_BASE 0x1A00
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
#define DIAG_SEC_ENTER 0xD100
#define DIAG_SEC_PING 0xD101
#define DIAG_SEC_READY 0xD1F0
#define DIAG_SEC_DONE 0xD1F1
#define DIAG_SEC_ERROR 0xD1FF
#define DIAG_SEC_EXECUTE 0xD200
#define DIAG_SEC_DMA_MASK 0xD201
#define DIAG_SEC_DMA_RESTORE 0xD202
#define DIAG_SEC_EXIT 0xD300
#define DIAG_COMM_READY 0xC101
#define DIAG_COMM_GO 0xC102
#define DIAG_COMM_ACK 0xC103
#define DIAG_COMM_DONE 0xC104
#define DIAG_COMM_RESTORE 0xC105
#define DIAG_CROSS_READY 0xC201
#define DIAG_DREQ_PATTERN 0xD400
#define DIAG_DREQ_ACK 0xD500

#define DIAG_SCRATCH_GENERAL ((volatile u16 *)0x26010000)
#define DIAG_SCRATCH_DMA ((volatile u16 *)0x26020000)
#define DIAG_SCRATCH_SECONDARY ((volatile u16 *)0x26030000)

typedef struct
{
	u32 remaining;
	u16 sample;
	u16 reserved;
} DiagnosticDeadline;

u8 diagnosticRead8(u32 address);
u16 diagnosticRead16(u32 address);
u32 diagnosticRead32(u32 address);
void diagnosticWrite8(u32 address, u8 value);
void diagnosticWrite16(u32 address, u16 value);
void diagnosticWrite32(u32 address, u32 value);
int diagnosticWait16(volatile u16 *address, u16 mask, u16 expected,
	u8 timeoutClass, u16 *actual, u32 *remaining);
int diagnosticWait32(volatile u32 *address, u32 mask, u32 expected,
	u8 timeoutClass, u32 *actual, u32 *remaining);
void diagnosticDeadlineStart(DiagnosticDeadline *deadline,
	u8 timeoutClass);
int diagnosticDeadlineActive(DiagnosticDeadline *deadline);
u32 diagnosticDeadlineRemaining(const DiagnosticDeadline *deadline);
u32 diagnosticProbeSr(u32 value);
u32 diagnosticReadSr(void);
void diagnosticWriteSr(u32 value);
u32 diagnosticCacheSuspend(void);
void diagnosticCacheRestore(u32 state);
u32 diagnosticReadVbr(void);
void diagnosticWriteVbr(u32 value);
int diagnosticHblankModes(volatile u16 *modes, u32 count,
	u32 *remaining);
int diagnosticMdCommand(u16 command, u16 argument,
	DiagnosticResult *result);
int diagnosticMdDmaBegin(u16 catalog, DiagnosticResult *result);
int diagnosticMdDmaFinish(u16 catalog, DiagnosticResult *result);
int diagnosticSecondaryBegin(u16 catalog, DiagnosticResult *result);
int diagnosticSecondaryFinish(DiagnosticResult *result);
int diagnosticSecondaryCommand(u16 catalog, DiagnosticResult *result);
int diagnosticSecondaryDmaIsolation(int enable,
	DiagnosticResult *result);
DiagnosticStatus diagnosticCommunicationTransaction(u16 catalog,
	DiagnosticResult *result);
void diagnosticSecondaryAgent(void);
int diagnosticRuntimePreflight(DiagnosticContext *context,
	DiagnosticResult *result);
int diagnosticRuntimeEnter(DiagnosticContext *context,
	DiagnosticResult *result);
int diagnosticRuntimeRestore(DiagnosticContext *context,
	DiagnosticResult *result);
int diagnosticRuntimeStatus(u16 catalog, DiagnosticResult *result);
int diagnosticRuntimeHandshake(DiagnosticResult *result);
u16 diagnosticRuntimePad(void);

#endif
