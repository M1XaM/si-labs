#pragma once
#include <Arduino.h>

// Task descriptor used by the sequential bare-metal scheduler.
struct Task_t {
    void (*Func)(void);  // task function pointer
    int  Rec;            // recurrence period in ms (timer ticks)
    int  Offset;         // initial delay offset in ms (ticks)
    int  RecCnt;         // countdown counter (managed by scheduler)
};

const uint8_t MaxSchedulerTasks = 8;

// Configures Timer1 for a 1 ms tick ISR. Call before sei().
void SchedulerInit();

// Registers a task. RecCnt is initialised from Offset.
// Tasks fire in registration order within the same tick.
void SchedulerRegister(Task_t* task);
