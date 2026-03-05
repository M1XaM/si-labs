#include "services/Scheduler.h"
#include <avr/interrupt.h>

static Task_t* Tasks[MaxSchedulerTasks];
static uint8_t TaskCount = 0;

// Timer1 CTC at 16 MHz, prescaler 8 → 2 MHz tick.
// OCR1A = 1999 → compare match every 2000 counts = 1 ms.
void SchedulerInit() {
    TaskCount = 0;
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1A  = F_CPU/ 8 / 1000 - 1; // 16 000 000 / 8 / 1000 - 1 = 1999
    TIMSK1 = (1 << OCIE1A);
}

void SchedulerRegister(Task_t* task) {
    if (TaskCount < MaxSchedulerTasks) {
        task->RecCnt = task->Offset;
        Tasks[TaskCount++] = task;
    }
}

// Guard against FreeRTOS, which defines its own TIMER1_COMPA_vect.
#ifndef USE_FREERTOS
ISR(TIMER1_COMPA_vect) {
    for (uint8_t i = 0; i < TaskCount; i++) {
        if (--Tasks[i]->RecCnt <= 0) {
            Tasks[i]->RecCnt = Tasks[i]->Rec;
            Tasks[i]->Func();
        }
    }
}
#endif
