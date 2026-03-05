#include <Arduino.h>
#include <avr/interrupt.h>

#include "drivers/ButtonDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/SerialStream.h"

#include "services/Scheduler.h"
#include "services/StdioRedirect.h"

#include "labs/lab3/TaskButtonMeasure.h"
#include "labs/lab3/TaskPressStats.h"
#include "labs/lab3/TaskReport.h"

/* ---------- hardware pin mapping ---------- */
static const uint8_t PIN_BTN     =  6;
static const uint8_t PIN_LED_G   = 12;
static const uint8_t PIN_LED_R   = 11;
static const uint8_t PIN_LED_Y   = 10;

/* ---------- serial I/O handle ---------- */
static SerialStream serialIo;

/* ---------- scheduler task table ----------
 * Measure and Stats share period / offset so they fire in the
 * same tick: the producer (registered first) runs before the
 * consumer.  Report fires once every 10 seconds.
 */
static Task_t tblMeasure = { TaskButtonMeasure, 20,    0,  0 };
static Task_t tblStats   = { TaskPressStats,    20,    0,  0 };
static Task_t tblReport  = { TaskReport,        10000, 50, 0 };

/* ========================================== */

void SetupLab3()
{
    /* 1. serial + stdio */
    serialIo.begin(9600);
    initStdio(&serialIo);

    /* 2. peripheral init */
    InitializeButton(PIN_BTN);
    InitializeLed(PIN_LED_G);
    InitializeLed(PIN_LED_R);
    InitializeLed(PIN_LED_Y);

    /* 3. task-local init */
    TaskButtonMeasureInit(PIN_BTN, PIN_LED_G, PIN_LED_R);
    TaskPressStatsInit(PIN_LED_Y);

    /* 4. scheduler — interrupts off during registration */
    cli();
    SchedulerInit();
    SchedulerRegister(&tblMeasure);
    SchedulerRegister(&tblStats);
    SchedulerRegister(&tblReport);
    sei();

    /* 5. startup banner */
    printf("Lab 3: Button Monitor\n");
    printf("Short press < 500 ms -> green\n");
    printf("Long  press >= 500ms -> red\n");
    printf("Report every 10 s via STDIO\n");
}

void LoopLab3()
{
    /* printf output is deferred to main context (safe for Serial). */
    TaskReportProcess();
}
