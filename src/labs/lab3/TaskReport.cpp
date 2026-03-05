#include "labs/lab3/TaskReport.h"

#include <Arduino.h>
#include <stdio.h>

#include "services/PressData.h"

/* flag: set inside the timer ISR, consumed in loop() */
static volatile bool pendingReport = false;

/* ================================================ */

void TaskReport()
{
    pendingReport = true;
}

void TaskReportProcess()
{
    if (!pendingReport)
        return;
    pendingReport = false;

    /* snapshot + reset under a critical section */
    cli();
    PressStats snap = GetStats();
    ResetStats();
    sei();

    uint32_t avg = (snap.TotalPresses > 0)
                       ? (snap.TotalDurationMs / snap.TotalPresses)
                       : 0;

    printf("\n+=====================+\n");
    printf("|  Report (10 s)      |\n");
    printf("+---------------------+\n");
    printf("| Total  : %-6lu     |\n", snap.TotalPresses);
    printf("| Short  : %-6lu     |\n", snap.ShortPresses);
    printf("| Long   : %-6lu     |\n", snap.LongPresses);
    printf("| Avg    : %-4lu ms    |\n", avg);
    printf("+=====================+\n");
}
