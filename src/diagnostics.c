#include "32x.h"
#include "32x_images.h"
#include "diagnostics.h"
#include "diagnostic_hw.h"
#include "help.h"
#include "hw_32x.h"
#include "shared_objects.h"
#include "string.h"

DiagnosticContext diagnosticContext;

static const u16 passValues[] =
{
	1, 2, 5, 10, 25, 50, 100, DIAGNOSTIC_CONTINUOUS
};

static const char *passLabels[] =
{
	"Passes: 1", "Passes: 2", "Passes: 5", "Passes: 10",
	"Passes: 25", "Passes: 50", "Passes: 100", "Passes: Continuous"
};

typedef struct
{
	const char *label;
	u16 first;
	u16 limit;
} DiagnosticRunSet;

static const DiagnosticRunSet runSets[] =
{
	{ "Start Full Diagnostics >", 0, 161 },
	{ "< Start Hardware Diagnostics >", 0, 134 },
	{ "< Start Audio Diagnostics >", 134, 156 },
	{ "< Start Video Diagnostics", 156, 161 }
};

#ifdef DEBUG
static const char *statusNames[] =
{
	"NOT RUN", "PASS", "FAIL", "TIMEOUT", "MANUAL", "UNAVAILABLE",
	"ABORTED", "INFRASTRUCTURE ERROR"
};
#endif

static u16 abortOldButton;

static void clearResult(DiagnosticResult *result)
{
	result->status = DIAG_NOT_RUN;
	result->cpu = 0;
	result->phase = 0;
	result->reserved = 0;
	result->catalog = 0;
	result->expected = 0;
	result->actual = 0;
	result->address = 0;
	result->remaining = 0;
}

static void resetContext(u16 passes, u16 first, u16 limit)
{
	int i;
	DiagnosticContext *context = &diagnosticContext;

	context->requestedPasses = passes;
	context->completedPasses = 0;
	context->currentPass = 0;
	context->currentTest = 0;
	context->currentGroup = 0;
	context->continuous = passes == DIAGNOSTIC_CONTINUOUS;
	context->aborted = 0;
	context->fatal = 0;
	context->segaCd = 0;
	context->mixerActive = 0;
	context->failureCount = 0;
	context->failureStart = 1;
	context->failureTotal = 0;
	context->state = DIAG_STATE_PREFLIGHT;
	abortOldButton = 0xFFFF;
	context->counts.passed = 0;
	context->counts.failed = 0;
	context->counts.manual = 0;
	context->counts.unavailable = 0;
	context->counts.notRun = 0;
	context->counts.timeout = 0;
	context->counts.aborted = 0;
	context->counts.infrastructure = 0;

	for (i = 0; i < DIAGNOSTIC_TEST_COUNT; i++)
	{
		clearResult(&context->results[i]);
		if (i < first || i >= limit)
		{
			context->results[i].catalog = diagnosticManifest[i].catalog;
			context->counts.notRun++;
		}
	}
	clearResult(&context->firstFailure);
	for (i = 0; i < DIAGNOSTIC_FAILURE_CAPACITY; i++)
		clearResult(&context->failures[i]);
}

static void copyShortName(char *destination, const char *source)
{
	int i;

	for (i = 0; i < 37 && source[i]; i++)
		destination[i] = source[i];
	destination[i] = 0;
}

static void drawProgress(const DiagnosticDescriptor *descriptor, u16 limit)
{
	char text[64];
	char name[40];
	DiagnosticContext *context = &diagnosticContext;

#ifdef DEBUG
	const DiagnosticGroup *group = &diagnosticGroups[descriptor->group];
	u16 within = descriptor->catalog - group->first;
	u32 notRun = context->counts.notRun + limit - descriptor->catalog + 1;
#endif

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;
	drawMainBG();
#ifdef DEBUG
	drawMenuTextwHighlight("32X DIAG RUNNING...", 84, 32,
		fontColorGreen, fontColorGreenHighlight);
	if (context->continuous)
		sprintf(text, "Pass %lu / Continuous", (u32)context->currentPass);
	else
		sprintf(text, "Pass %lu / %lu", (u32)context->currentPass,
		(u32)context->requestedPasses);
	drawMenuTextwHighlight(text, 40, 52, fontColorWhite,
		fontColorWhiteHighlight);
	drawMenuTextwHighlight(group->name, 40, 68, fontColorGreen,
		fontColorGreenHighlight);
	sprintf(text, "Group %lu / %lu", (u32)within, (u32)group->count);
	drawMenuTextwHighlight(text, 40, 80, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Catalog %03lu", (u32)descriptor->catalog);
	drawMenuTextwHighlight(text, 40, 96, fontColorWhite,
		fontColorWhiteHighlight);
	copyShortName(name, descriptor->name);
	drawMenuTextwHighlight(name, 40, 108, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Pass %lu  Fail %lu  Manual %lu",
		context->counts.passed, context->counts.failed,
		context->counts.manual);
	drawMenuTextwHighlight(text, 40, 132, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Unavailable %lu  Not run %lu",
		context->counts.unavailable, notRun);
	drawMenuTextwHighlight(text, 40, 144, fontColorWhite,
		fontColorWhiteHighlight);
	drawMenuTextwHighlight("Press B to abort between tests", 40, 176,
		fontColorGreen, fontColorGreenHighlight);
#else
	if (context->continuous)
		sprintf(text, "Pass: %lu / Continuous", (u32)context->currentPass);
	else
		sprintf(text, "Pass: %lu / %lu", (u32)context->currentPass,
			(u32)context->requestedPasses);
	drawMenuTextwHighlight(text, 40, 48, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Test ID: C%03lu", (u32)descriptor->catalog);
	drawMenuTextwHighlight(text, 40, 80, fontColorWhite,
		fontColorWhiteHighlight);
	copyShortName(name, descriptor->name);
	drawMenuTextwHighlight(name, 40, 92, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Tests remaining: %lu",
		(u32)(limit - descriptor->catalog + 1));
	drawMenuTextwHighlight(text, 40, 124, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Pass: %lu", context->counts.passed);
	drawMenuTextwHighlight(text, 40, 140, fontColorGreen,
		fontColorGreenHighlight);
	sprintf(text, "Fail: %lu", context->counts.failed +
		context->counts.timeout + context->counts.infrastructure);
	drawMenuTextwHighlight(text, 160, 140, fontColorRed,
		fontColorRedHighlight);
	drawMenuTextwHighlight("Press B to abort", 40, 180,
		fontColorGreen, fontColorGreenHighlight);
#endif
	Hw32xScreenFlip(1);
}

static void storeFailure(const DiagnosticResult *result)
{
	DiagnosticContext *context = &diagnosticContext;
	int index;

	if (!context->failureTotal)
	{
		context->firstFailure = *result;
		context->failures[0] = *result;
		context->failureCount = 1;
	}
	else if (context->failureCount < DIAGNOSTIC_FAILURE_CAPACITY)
	{
		context->failures[context->failureCount] = *result;
		context->failureCount++;
	}
	else
	{
		index = context->failureStart;
		context->failures[index] = *result;
		context->failureStart++;
		if (context->failureStart >= DIAGNOSTIC_FAILURE_CAPACITY)
			context->failureStart = 1;
	}
	context->failureTotal++;
}

static void recordResult(DiagnosticResult *result)
{
	DiagnosticCounts *counts = &diagnosticContext.counts;

	switch (result->status)
	{
		case DIAG_PASS:
			counts->passed++;
			break;
		case DIAG_FAIL:
			counts->failed++;
			storeFailure(result);
			break;
		case DIAG_TIMEOUT:
			counts->timeout++;
			storeFailure(result);
			break;
		case DIAG_MANUAL:
			counts->manual++;
			break;
		case DIAG_UNAVAILABLE:
			counts->unavailable++;
			break;
		case DIAG_ABORTED:
			counts->aborted++;
			break;
		case DIAG_INFRASTRUCTURE_ERROR:
			counts->infrastructure++;
			storeFailure(result);
			break;
		default:
			counts->notRun++;
			break;
	}
}

int diagnosticsValidateManifest(void)
{
	int group;
	int index = 0;

	for (group = 0; group < DIAGNOSTIC_GROUP_COUNT; group++)
	{
		int count;
		const DiagnosticGroup *entry = &diagnosticGroups[group];

		if (entry->first != index || entry->count == 0)
			return 0;
		for (count = 0; count < entry->count; count++, index++)
		{
			const DiagnosticDescriptor *descriptor =
				&diagnosticManifest[index];

			if (descriptor->catalog != index + 1 ||
				descriptor->group != group || !descriptor->name ||
				descriptor->target > DIAG_TARGET_DISPLAY ||
				descriptor->operation < DIAG_OP_MD_SYSTEM ||
				descriptor->operation > DIAG_OP_GRAPHICS ||
				descriptor->operation != group + 1 ||
				descriptor->kind > DIAG_KIND_UPGRADED ||
				descriptor->timeout > DIAG_TIMEOUT_SEGA_CD ||
				(descriptor->requirements & ~0x3F) ||
				(descriptor->cleanup & ~0x3F) ||
				descriptor->safeContinue > 1)
				return 0;
		}
	}

	return index == DIAGNOSTIC_TEST_COUNT;
}

static int pollAbort(void)
{
	u16 button = diagnosticRuntimePad();
	u16 pressed = button & ~abortOldButton;

	abortOldButton = button;
	return (pressed & SEGA_CTRL_B) != 0;
}

static void markRemainingNotRun(int start, int limit)
{
	int i;

	for (i = start; i < limit; i++)
	{
		DiagnosticResult *result = &diagnosticContext.results[i];

		if (result->catalog == diagnosticManifest[i].catalog &&
			result->status == DIAG_NOT_RUN)
			continue;
		clearResult(result);
		result->catalog = diagnosticManifest[i].catalog;
		diagnosticContext.counts.notRun++;
	}
}

static void markAborted(int index, int limit)
{
	DiagnosticResult *result;

	if (index >= DIAGNOSTIC_TEST_COUNT)
		return;
	result = &diagnosticContext.results[index];
	clearResult(result);
	result->catalog = diagnosticManifest[index].catalog;
	result->cpu = diagnosticManifest[index].target;
	result->status = DIAG_ABORTED;
	recordResult(result);
	markRemainingNotRun(index + 1, limit);
}

static void __attribute__((noinline)) runDiagnostics(u16 passes,
	u16 first, u16 limit)
{
	DiagnosticResult infrastructure;
	DiagnosticContext *context = &diagnosticContext;
	int stop = 0;

	resetContext(passes, first, limit);
	clearResult(&infrastructure);
	if (!diagnosticsValidateManifest())
	{
		infrastructure.status = DIAG_INFRASTRUCTURE_ERROR;
		infrastructure.phase = DIAG_STATE_PREFLIGHT;
		context->fatal = 1;
		recordResult(&infrastructure);
		markRemainingNotRun(first, limit);
		stop = 1;
	}
	if (!stop && !diagnosticRuntimePreflight(context, &infrastructure))
	{
		context->fatal = 1;
		recordResult(&infrastructure);
		markRemainingNotRun(first, limit);
		stop = 1;
	}
	if (!stop)
	{
		context->state = DIAG_STATE_QUIESCE;
		if (!diagnosticRuntimeEnter(context, &infrastructure))
		{
			context->fatal = 1;
			recordResult(&infrastructure);
			markRemainingNotRun(first, limit);
			stop = 1;
		}
	}
	if (!stop)
	{
		marsVDP256Start();
		initMainBG();
	}

	while (!stop && (context->continuous ||
		context->completedPasses < context->requestedPasses))
	{
		int index;

		context->currentPass = context->completedPasses + 1;
		for (index = first; index < limit; index++)
		{
			const DiagnosticDescriptor *descriptor =
				&diagnosticManifest[index];
			DiagnosticResult *result = &context->results[index];

			context->currentTest = descriptor->catalog;
			context->currentGroup = descriptor->group;
			context->state = DIAG_STATE_RUN_TEST;
			clearResult(result);
			result->catalog = descriptor->catalog;
			if (pollAbort())
			{
				context->aborted = 1;
				markAborted(index, limit);
				stop = 1;
				break;
			}
			clearResult(&infrastructure);
			infrastructure.catalog = descriptor->catalog;
			if (!diagnosticRuntimeStatus(descriptor->catalog,
				&infrastructure))
			{
				*result = infrastructure;
				result->status = DIAG_INFRASTRUCTURE_ERROR;
				recordResult(result);
				context->fatal = 1;
				markRemainingNotRun(index + 1, limit);
				stop = 1;
				break;
			}
			drawProgress(descriptor, limit);
			result->status = diagnosticsExecute(descriptor, result);
			if (!descriptor->safeContinue &&
				result->status != DIAG_PASS &&
				result->status != DIAG_MANUAL &&
				result->status != DIAG_UNAVAILABLE)
				result->status = DIAG_INFRASTRUCTURE_ERROR;
			recordResult(result);
			if (result->status == DIAG_INFRASTRUCTURE_ERROR)
			{
				context->fatal = 1;
				markRemainingNotRun(index + 1, limit);
				stop = 1;
				break;
			}
		}
		if (!stop)
		{
			clearResult(&infrastructure);
			if (!diagnosticRuntimeHandshake(&infrastructure))
			{
				context->fatal = 1;
				recordResult(&infrastructure);
				stop = 1;
			}
			else
				context->completedPasses++;
		}
	}

	context->state = DIAG_STATE_RESTORE;
	clearResult(&infrastructure);
	if (!diagnosticRuntimeRestore(context, &infrastructure))
	{
		context->fatal = 1;
		recordResult(&infrastructure);
	}
	context->state = DIAG_STATE_RESULTS;
}

static DiagnosticResult *failureAt(int detail)
{
	int failureIndex = detail;
	DiagnosticContext *context = &diagnosticContext;

	if (context->failureCount == DIAGNOSTIC_FAILURE_CAPACITY && detail)
		failureIndex = 1 + ((context->failureStart + detail - 2) %
			(DIAGNOSTIC_FAILURE_CAPACITY - 1));
	return &context->failures[failureIndex];
}

#ifndef DEBUG
static void copyFailureName(char *destination, const char *source)
{
	int i;

	for (i = 0; i < 25 && source[i]; i++)
		destination[i] = source[i];
	destination[i] = 0;
}
#endif

static void drawResults(int selection, int detail)
{
	char text[64];
	DiagnosticContext *context = &diagnosticContext;

#ifdef DEBUG
	u32 executions = context->counts.passed + context->counts.failed +
		context->counts.manual + context->counts.unavailable +
		context->counts.timeout + context->counts.aborted +
		context->counts.infrastructure;

	drawMainBG();
	drawMenuTextwHighlight("32X DIAGNOSTIC RESULTS", 72, 36,
		fontColorGreen, fontColorGreenHighlight);
	if (context->continuous)
		sprintf(text, "Requested Continuous  Completed %lu",
			(u32)context->completedPasses);
	else
		sprintf(text, "Requested %lu  Completed %lu",
			(u32)context->requestedPasses, (u32)context->completedPasses);
	drawMenuTextwHighlight(text, 40, 48, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Executions %lu", executions);
	drawMenuTextwHighlight(text, 40, 60, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Pass %lu  Fail %lu  Timeout %lu",
		context->counts.passed, context->counts.failed,
		context->counts.timeout);
	drawMenuTextwHighlight(text, 40, 76, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Manual %lu  Unavailable %lu",
		context->counts.manual, context->counts.unavailable);
	drawMenuTextwHighlight(text, 40, 88, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Not run %lu  Aborted %lu  Infra %lu",
		context->counts.notRun, context->counts.aborted,
		context->counts.infrastructure);
	drawMenuTextwHighlight(text, 40, 100, fontColorWhite,
		fontColorWhiteHighlight);
	if (context->fatal)
		drawMenuTextwHighlight("Stopped by infrastructure failure", 40, 112,
			fontColorRed, fontColorRedHighlight);
	else if (context->aborted)
		drawMenuTextwHighlight("Run aborted by user", 40, 112,
			fontColorRed, fontColorRedHighlight);
	else
		drawMenuTextwHighlight("Run completed", 40, 112,
			fontColorGreen, fontColorGreenHighlight);
	if (context->failureCount)
	{
		DiagnosticResult *failure = failureAt(detail);

		sprintf(text, "Failure %lu/%lu Total %lu C%03lu %s",
			(u32)(detail + 1), (u32)context->failureCount,
			(u32)context->failureTotal, (u32)failure->catalog,
			statusNames[failure->status]);
		drawMenuTextwHighlight(text, 40, 128, fontColorRed,
			fontColorRedHighlight);
		sprintf(text, "CPU %lu Phase %lu Step %lu", (u32)failure->cpu,
			(u32)failure->phase, (u32)failure->reserved);
		drawMenuTextwHighlight(text, 40, 140, fontColorWhite,
			fontColorWhiteHighlight);
		sprintf(text, "Addr %08lX Exp %08lX", failure->address,
			failure->expected);
		drawMenuTextwHighlight(text, 40, 152, fontColorWhite,
			fontColorWhiteHighlight);
		sprintf(text, "Act %08lX Rem %08lX", failure->actual,
			failure->remaining);
		drawMenuTextwHighlight(text, 40, 164, fontColorWhite,
			fontColorWhiteHighlight);
	}
	drawMenuTextwHighlight("Rerun", 40, 180,
		selection == 0 ? fontColorRed : fontColorWhite,
		selection == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
	drawMenuTextwHighlight("Back", 40, 192,
		selection == 1 ? fontColorRed : fontColorWhite,
		selection == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
#else
	u32 failed = context->failureTotal;
	u32 checked = context->counts.passed + failed;
	int first = detail * 5;
	int count = context->failureCount - first;
	int pages = (context->failureCount + 4) / 5;
	int i;

	if (count > 5)
		count = 5;
	drawMainBG();
	drawMenuTextwHighlight("32X DIAGNOSTIC RESULTS", 72, 36,
		fontColorGreen, fontColorGreenHighlight);
	sprintf(text, "Iterations: %lu", (u32)context->completedPasses);
	drawMenuTextwHighlight(text, 40, 48, fontColorWhite,
		fontColorWhiteHighlight);
	sprintf(text, "Pass: %lu/%lu", context->counts.passed, checked);
	drawMenuTextwHighlight(text, 40, 60, fontColorGreen,
		fontColorGreenHighlight);
	sprintf(text, "Failed: %lu", failed);
	drawMenuTextwHighlight(text, 176, 60, fontColorRed,
		fontColorRedHighlight);
	if (context->failureCount)
	{
		drawMenuTextwHighlight("Failed:", 40, 84, fontColorWhite,
			fontColorWhiteHighlight);
		for (i = 0; i < count; i++)
		{
			DiagnosticResult *failure = failureAt(first + i);
			char name[26];

			if (failure->catalog &&
				failure->catalog <= DIAGNOSTIC_TEST_COUNT)
				copyFailureName(name,
					diagnosticManifest[failure->catalog - 1].name);
			else
				copyFailureName(name, "Infrastructure");
			sprintf(text, "C%03lu:%s", (u32)failure->catalog, name);
			drawMenuTextwHighlight(text, 40, 92 + i * 10,
				fontColorRed, fontColorRedHighlight);
		}
	}
	if (pages > 1)
	{
		if (!detail)
			sprintf(text, "next>");
		else if (detail + 1 == pages)
			sprintf(text, "<prev");
		else
			sprintf(text, "<prev    next>");
		drawMenuTextwHighlight(text, 40, 140, fontColorWhite,
			fontColorWhiteHighlight);
	}
	drawMenuTextwHighlight("Rerun tests", 40, 160,
		selection == 0 ? fontColorRed : fontColorWhite,
		selection == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
	drawMenuTextwHighlight("Back", 40, 172,
		selection == 1 ? fontColorRed : fontColorWhite,
		selection == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
#endif
}

static int __attribute__((noinline)) resultsScreen(void)
{
	int selection = 0;
	int detail = 0;
	u16 oldButton = 0xFFFF;

	initMainBG();
	Hw32xScreenFlip(0);
	while (1)
	{
		u16 button;
		u16 pressed;

		Hw32xFlipWait();
		drawResults(selection, detail);
		button = diagnosticRuntimePad();
		pressed = button & ~oldButton;
		oldButton = button;
		if (pressed & (SEGA_CTRL_UP | SEGA_CTRL_DOWN))
			selection ^= 1;
#ifdef DEBUG
		if (diagnosticContext.failureCount &&
			(pressed & SEGA_CTRL_LEFT))
		{
			if (--detail < 0)
				detail = diagnosticContext.failureCount - 1;
		}
		if (diagnosticContext.failureCount &&
			(pressed & SEGA_CTRL_RIGHT))
		{
			detail++;
			if (detail >= diagnosticContext.failureCount)
				detail = 0;
		}
#else
		if ((pressed & SEGA_CTRL_LEFT) && detail > 0)
			detail--;
		if ((pressed & SEGA_CTRL_RIGHT) &&
			detail + 1 < (diagnosticContext.failureCount + 4) / 5)
			detail++;
#endif
		if (pressed & SEGA_CTRL_B)
			return 0;
		if (pressed & SEGA_CTRL_A)
			return selection == 0;
		Hw32xScreenFlip(0);
	}
}

void diagnosticsMenu(void)
{
	int passIndex = 0;
	int runSetIndex = 0;
	int selection = 1;
	u16 oldButton = 0xFFFF;

	while (1)
	{
		int start = 0;

		diagnosticContext.state = DIAG_STATE_OPTIONS;
		initMainBG();
		Hw32xScreenFlip(0);
		while (!start)
		{
			u16 button;
			u16 pressed;

			Hw32xFlipWait();
			drawMainBG();
			drawMenuTextwHighlight("32X DIAGNOSTIC TEST", 84, 34,
				fontColorGreen, fontColorGreenHighlight);
			drawMenuTextwHighlight(passLabels[passIndex], 40, 94,
				selection == 0 ? fontColorRed : fontColorWhite,
				selection == 0 ? fontColorRedHighlight :
					fontColorWhiteHighlight);
			drawMenuTextwHighlight(runSets[runSetIndex].label, 40, 102,
				selection == 1 ? fontColorRed : fontColorWhite,
				selection == 1 ? fontColorRedHighlight :
					fontColorWhiteHighlight);
			drawMenuTextwHighlight("Help", 40, 140,
				selection == 2 ? fontColorRed : fontColorWhite,
				selection == 2 ? fontColorRedHighlight :
					fontColorWhiteHighlight);
			drawMenuTextwHighlight("Back", 40, 148,
				selection == 3 ? fontColorRed : fontColorWhite,
				selection == 3 ? fontColorRedHighlight :
					fontColorWhiteHighlight);
			button = diagnosticRuntimePad();
			pressed = button & ~oldButton;
			oldButton = button;
			if (pressed & SEGA_CTRL_UP)
			{
				if (--selection < 0)
					selection = 3;
			}
			if (pressed & SEGA_CTRL_DOWN)
			{
				if (++selection > 3)
					selection = 0;
			}
			if (pressed & SEGA_CTRL_LEFT)
			{
				if (selection == 1 && runSetIndex > 0)
					runSetIndex--;
				else if (selection == 0 && --passIndex < 0)
					passIndex = 7;
			}
			if (pressed & SEGA_CTRL_RIGHT)
			{
				if (selection == 1 && runSetIndex < 3)
					runSetIndex++;
				else if (selection == 0 && ++passIndex > 7)
					passIndex = 0;
			}
			if (pressed & SEGA_CTRL_B)
				return;
			if (((pressed & SEGA_CTRL_A) && selection != 0) ||
				((pressed & SEGA_CTRL_START) && selection == 1))
			{
				if (selection == 3)
					return;
				if (selection == 2)
				{
					screenFadeOut(1);
					DrawHelp(HELP_DIAGNOSTICS);
					initMainBG();
					Hw32xScreenFlip(0);
					oldButton = 0xFFFF;
					continue;
				}
				start = 1;
			}
			Hw32xScreenFlip(0);
		}
		screenFadeOut(1);
		runDiagnostics(passValues[passIndex],
			runSets[runSetIndex].first, runSets[runSetIndex].limit);
		if (!resultsScreen())
			return;
		oldButton = 0xFFFF;
	}
}
