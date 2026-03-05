/*
 *  TaskReport.h
 *  ------------
 *  Periodic statistics report — ISR sets a flag, main loop
 *  prints via printf.
 */

#pragma once

/*  Scheduler callback (ISR context) — just raises a flag.
 *  Period 10 000 ms, offset 50 ms.                          */
void TaskReport();

/*  Main-loop helper — if the flag is set, print the report
 *  and reset statistics.  No-op otherwise.                  */
void TaskReportProcess();
