/*
 *  Lab 2 – Four-digit access-code checker
 *
 *  Hardware assumed:
 *    - 16x2 HD44780 LCD in 4-bit mode
 *    - 4x4 matrix keypad
 *    - Two LEDs (green = OK, red = FAIL)
 */

#include <Arduino.h>
#include <string.h>

#include "drivers/LcdDriver.h"
#include "drivers/LedDriver.h"
#include "drivers/LcdStream.h"
#include "drivers/KeypadDriver.h"
#include "services/StdioRedirect.h"

/* ======================================================================== *
 *  GPIO mapping                                                            *
 * ======================================================================== */

/* LEDs */
#define PIN_LED_OK   A4
#define PIN_LED_ERR  A5

/* LCD data / control */
#define PIN_LCD_RS   2
#define PIN_LCD_EN   3
#define PIN_LCD_D4   4
#define PIN_LCD_D5   5
#define PIN_LCD_D6   6
#define PIN_LCD_D7   7

/* LCD geometry */
#define SCREEN_W     16
#define SCREEN_H     2

/* Keypad row GPIOs */
#define PIN_KP_R0    8
#define PIN_KP_R1    9
#define PIN_KP_R2    10
#define PIN_KP_R3    11

/* Keypad column GPIOs (analog pins used as digital) */
#define PIN_KP_C0    A0
#define PIN_KP_C1    A1
#define PIN_KP_C2    A2
#define PIN_KP_C3    A3

/* ======================================================================== *
 *  Application parameters                                                  *
 * ======================================================================== */

static const char     SECRET[]       = "1234";
static const uint8_t  SECRET_LEN     = 4;
static const uint16_t MSG_DURATION   = 2000;   /* ms */

/* Shared LCD+keypad stream object */
static LcdStream _lcdKeypad;

/* ======================================================================== *
 *  Internal helpers                                                        *
 * ======================================================================== */

static bool _grabCodeAndCheck()
{
    char buf[SECRET_LEN + 1];

    LcdSetCursor(0, 1);

    for (uint8_t k = 0; k < SECRET_LEN; ++k)
    {
        buf[k] = (char)getchar();
        putchar('*');
    }
    buf[SECRET_LEN] = '\0';

    return (strncmp(buf, SECRET, SECRET_LEN) == 0);
}

/* ======================================================================== *
 *  Public entry points (called from main.cpp)                              *
 * ======================================================================== */

void SetupLab2()
{
    /* --- display --- */
    InitializeLcd(PIN_LCD_RS, PIN_LCD_EN,
                  PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7,
                  SCREEN_W, SCREEN_H);

    /* --- keypad --- */
    const uint8_t rp[KeypadRowCount] = { PIN_KP_R0, PIN_KP_R1,
                                          PIN_KP_R2, PIN_KP_R3 };
    const uint8_t cp[KeypadColCount] = { PIN_KP_C0, PIN_KP_C1,
                                          PIN_KP_C2, PIN_KP_C3 };
    InitializeKeypad(rp, cp);

    /* --- LEDs --- */
    InitializeLed(PIN_LED_OK);
    InitializeLed(PIN_LED_ERR);

    /* --- stdio through LCD/keypad --- */
    initStdio(&_lcdKeypad);

    /* splash screen */
    printf("Access Control");
    LcdSetCursor(0, 1);
    printf("Code: %u digits", SECRET_LEN);
    delay(MSG_DURATION);
}

void LoopLab2()
{
    /* turn both indicators off at the start of every cycle */
    SetLedState(PIN_LED_OK,  false);
    SetLedState(PIN_LED_ERR, false);

    LcdClear();
    LcdSetCursor(0, 0);
    printf("Enter Code:");

    bool granted = _grabCodeAndCheck();

    LcdClear();
    LcdSetCursor(0, 0);

    if (granted)
    {
        SetLedState(PIN_LED_OK, true);
        printf("Access Granted!");
    }
    else
    {
        SetLedState(PIN_LED_ERR, true);
        printf("Wrong Code!");
    }

    delay(MSG_DURATION);
}
