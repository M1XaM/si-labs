#include <Arduino.h>
#include "drivers/LedDriver.h"
#include "drivers/SerialStream.h"
#include "services/StdioRedirect.h"

const int BuiltInLed = 13;

SerialStream serialStream;

void setup() {
    serialStream.begin(9600);
    initStdio(&serialStream);

    InitializeLed(BuiltInLed);

    printf("System Initialized...\n");
    printf("Commands: led on | led off\n");
}

void scanUserAction() {
    printf("\nEnter command: ");
    
    while (!serialStream.available()) {
        delay(10);
    }
    
    delay(50);
    
    int c;
    while (serialStream.available()) {
        c = getchar();
        if (c != '\n' && c != '\r') {
            ungetc(c, stdin);  
            break;
        }
    }
    
    char action[7] = {0};  
    
    int result = scanf("led %6s", action);
    printf("Debug: scanf result = %d, action = '%s'\n", result, action);
    
    if (result == 1) {
        if (strcmp(action, "on") == 0) {
            SetLedState(BuiltInLed, true);
            printf("MSG: LED state changed to ON\n");
        }
        else if (strcmp(action, "off") == 0) {
            SetLedState(BuiltInLed, false);
            printf("MSG: LED state changed to OFF\n");
        }
        else {
            printf("ERROR: Unknown action '%s'\n", action);
        }
    } else {
        printf("ERROR: Invalid command format\n");
    }
    
    while (serialStream.available()) {
        getchar();
    }
    
    delay(100);
}

void loop() {
    scanUserAction();
}