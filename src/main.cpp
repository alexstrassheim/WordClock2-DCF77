#include <Arduino.h>

#include <DCF77.h>
#include <WClock.h>

int DCF77Pin=3;
int blinkPin=13;

int seconds=0;
int previousSecond=0;
int minutes=0;
int hours=0;

DCF77 myDCF(DCF77Pin);
WClock wclock;

void setup(void) {
    pinMode(blinkPin, OUTPUT);
    Serial.begin(9600);
    while(!Serial);
    myDCF.begin();
    wclock.begin();
    /* wclock.resetAndBlack(); */
    Serial.println("Start Clock");
    delay(3000);
}


void loop(void) {
    int DCFsignal = myDCF.scanSignal();
    if (DCFsignal) {
        digitalWrite(blinkPin, HIGH);
    } else {
        digitalWrite(blinkPin, LOW);
    }
    hours=myDCF.hh;
    minutes=myDCF.mm;
    seconds=myDCF.ss;
    /* wclock.resetAndBlack(); */
    wclock.showTime(hours, minutes, seconds);
    if (seconds != previousSecond) {
        myDCF.serialDumpTime();
    }
    previousSecond = seconds;

}
