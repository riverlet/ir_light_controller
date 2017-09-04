#include "Arduino.h"
//#include "EEPROM.h"
#include "storage.h"
//#define DOG 1

//------------------------------------------------------------------------------
// Include the IRremote library header
//
#include "../.piolibdeps/IRremote_ID4/IRremote.h"
/*
 * IRrecord: record and play back IR signals as a minimal
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input BUTTON_PIN; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#define STATUS_NORMAL 255
#define STATUS_CFG_POWER 0
#define STATUS_CFG_UP 1
#define STATUS_CFG_DOWN 2

#define KEY_POWER 0
#define KEY_UP 1
#define KEY_DOWN 2


#define BTN_IGNORE 0
#define BTN_LONG 2
#define BTN_SHOT 1

int RECV_PIN = 11;
int BUTTON_PIN = 12; //短按切换模式，长按保存当前模式
int STATUS_PIN = LED_BUILTIN;
int PWM_PIN = 9;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

int brightness[] = { 4, 8, 16, 32, 64, 96, 128, 160, 192, 255 };
int currentLevel = 2; // save in eeprom

byte powerOn = 0;
byte status = STATUS_NORMAL;

byte currentBtnState;
byte previousBtnState = HIGH;
unsigned long firstPressTime;
long millisHeld, lastPowerKeyMillis;

long prevFlash;

void flashOnce();
void run();

byte getKeyByStatus(byte status) {
    return status;
}


Storage storage;


byte checkBtn() {
    byte result = BTN_IGNORE;
    currentBtnState = digitalRead(BUTTON_PIN);

    // if the button state changes to pressed, remember the start time
    if (currentBtnState == LOW && previousBtnState == HIGH && (millis() - firstPressTime) > 200) {
        firstPressTime = millis();

    }

    millisHeld = (millis() - firstPressTime);


    // This if statement is a basic debouncing tool, the button must be pushed for at least
    // 100 milliseconds in a row for it to be considered as a push.
    if (millisHeld > 50) {

//        if (currentBtnState == LOW && millisHeld > prevMillisHeld) {
//            return BTN_SHOT;
//        }

        // check if the button was released since we last checked
        if (currentBtnState == HIGH && previousBtnState == LOW) {
            // HERE YOU WOULD ADD VARIOUS ACTIONS AND TIMES FOR YOUR OWN CODE
            // ===============================================================================

            // Button pressed for less than 1 second, one long LED blink
            if (millisHeld <= 1500) {
                result = BTN_SHOT;
            } else {
                result = BTN_LONG;
            }
        }
    }

    previousBtnState = currentBtnState;

    return result;
}

/**
 * 模式循环切换
 */
void switchStatus() {
    switch (status) {
        case STATUS_NORMAL:
            status = STATUS_CFG_POWER;
            break;
        case STATUS_CFG_POWER:
            status = STATUS_CFG_UP;
            break;
        case STATUS_CFG_UP:
            status = STATUS_CFG_DOWN;
            break;
        case STATUS_CFG_DOWN:
            status = STATUS_NORMAL;
            break;
        default:
            ;
    }
    storage.clearTemp();

}

/**
 * 保存当前模式数据
 */
void saveStatus() {
    if(status != STATUS_NORMAL) {
        //save receivedValues into eeprom;

        storage.saveTempAs(getKeyByStatus(status));

        digitalWrite(STATUS_PIN, HIGH);
        delay(1000);
        digitalWrite(STATUS_PIN, LOW);
    }
    switchStatus(); //自动进入下一个模式
}

void flashStatus() {
    if(status != STATUS_NORMAL && millis() - prevFlash > 1000) {
        switch (status) {
            case STATUS_CFG_POWER:
                flashOnce();
                break;
            case STATUS_CFG_UP:
                for (int i = 0; i < 2; i++) {
                    flashOnce();
                }
                break;
            case STATUS_CFG_DOWN:
                for (int i = 0; i < 3; i++) {
                    flashOnce();
                }
                break;
        }
        prevFlash = millis();
    }
}

void flashOnce() {
    digitalWrite(STATUS_PIN, HIGH);
    delay(50);
    digitalWrite(STATUS_PIN, LOW);
    delay(50);
}


void run() {
    if (irrecv.decode(&results)) {
        if(status == STATUS_NORMAL) {
            if(storage.getKey(&results) == KEY_POWER) { //power
                if(millis() - lastPowerKeyMillis > 1000) {
                    digitalWrite(STATUS_PIN, HIGH);
                    if(powerOn == 1) {
                        analogWrite(PWM_PIN, 0); //shut down
                        powerOn = 0;
                    } else {
                        powerOn = 1;
                        analogWrite(PWM_PIN, brightness[currentLevel]);

                    }
                    delay(100);
                    digitalWrite(STATUS_PIN, LOW);
                    lastPowerKeyMillis = millis();
                }

            } else if(storage.getKey(&results) == KEY_UP) { //up
                digitalWrite(STATUS_PIN, HIGH);
                int highestLevel = sizeof(brightness)/sizeof(int) - 1;
                if(powerOn == 0) {
                    currentLevel = highestLevel;
                } else {
                    if (currentLevel != highestLevel) {
                        ++currentLevel;
                    }
                }
                analogWrite(PWM_PIN, brightness[currentLevel]);
                powerOn = 1;
                delay(100);
                digitalWrite(STATUS_PIN, LOW);
            } else if(storage.getKey(&results) == KEY_DOWN) { //down
                digitalWrite(STATUS_PIN, HIGH);
                int lowestLevel = 0;
                if(powerOn == 0) {
                    currentLevel = lowestLevel;
                } else {
                    if (currentLevel != lowestLevel) {
                        --currentLevel;
                    }
                }
                analogWrite(PWM_PIN, brightness[currentLevel]);
                powerOn = 1;
                delay(100);
                digitalWrite(STATUS_PIN, LOW);
            }
            irrecv.resume(); // resume receiver
        } else {
            //每读到一个不同的值，则存入临时数组，最多8个。
            //保存的逻辑在按钮长按时处理

            if(storage.addToTemp(&results)) {
                digitalWrite(STATUS_PIN, HIGH);
                delay(500);
                digitalWrite(STATUS_PIN, LOW);
            }
            irrecv.resume(); // resume receiver

        }

    }
}

void setup()
{
#ifdef DOG
    Serial.begin(9600);
#endif
    storage.begin();
    irrecv.enableIRIn(); // Start the receiver
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(STATUS_PIN, OUTPUT);
    pinMode(PWM_PIN, OUTPUT);
}



void loop() {
    byte btn = checkBtn();
    switch (btn) {
        case BTN_SHOT:
            switchStatus();
            break;
        case BTN_LONG:
            saveStatus();
            break;
        default:
            ;
    }

    flashStatus();

    run();
}
