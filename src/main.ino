// https://github.com/wouterdevinck/wordclock
// https://github.com/bagges/rgb_wordclock
#include <Arduino.h>

#include <DCF77.h>
#include <TimeLib.h>
#include "WClock.h"

#define DCF_PIN 12	         // Connection pin to DCF 77 device
#define DCF_INTERRUPT 12		 // Interrupt number associated with pin
#define WS2812B_PIN 10
#define NUM_LEDS 114
#define LDR_PIN 14

time_t prevDisplay = 0;          // when the digital clock was displayed
time_t time;
DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT,false);
// WClock wclock = WClock(WS2812B_PIN, NUM_LEDS);
WClock wclock;

int testHours = 0;
int testMinutes = 0;

const long oneSecondDelay = 1000;
long waitUntilRtc = 0;
long waitUntilLDR = 0;
boolean autoBrightnessEnabled = true;

void setup(void) {
    Serial.begin(9600);
    // while(!Serial);

    DCF.Start();
    setSyncInterval(30);
    setSyncProvider((getExternalTime)getDCFTime);

    Serial.println("Start Clock");
    wclock.begin();
    wclock.resetAndBlack();

    Serial.println("Waiting for DCF77 time ... ");
    Serial.println("It will take at least 2 minutes until a first update can be processed.");

    // while(timeStatus() == timeNotSet) {
    //   // wait until the time is set by the sync provider
    //   Serial.print(".");
    //   delay(2000);
    // }

}

///
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;
///

void loop(void) {
  //update the display only if the time has changed
  if( now() != prevDisplay) {
    prevDisplay = now();
    // digitalClockDisplay();
  }

  if(millis() >= waitUntilRtc) {
    // DEBUG_PRINT("doing clock logic");
    waitUntilRtc = millis();
    if(testMinutes != minute() || testHours != hour()) {
      testMinutes = minute();
      testHours = hour();
      // wclock.resetAndBlack();
      wclock.showTime(hour(), minute(), second());
    }
    waitUntilRtc += oneSecondDelay;
  }

  ///
  recvWithEndMarker();
  showNewData();
  ///

  doLDRLogic();
}

void doLDRLogic() {
  if(millis() >= waitUntilLDR && autoBrightnessEnabled) {
    /* DEBUG_PRINT("doing LDR logic"); */
    waitUntilLDR = millis();
    int ldrVal = map(analogRead(LDR_PIN), 0, 1023, 0, 200);
    /* FastLED.setBrightness(255-ldrVal); */
    /* FastLED.show(); */
    DEBUG_PRINT(ldrVal);
    wclock.setB(ldrVal);
    waitUntilLDR += oneSecondDelay;
  }
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  // if (Serial.available() > 0) {
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

int fir;
int sec;
String str = "12:12";
void showNewData() {
  if (newData == true) {
    Serial.print("This just in ... ");
    Serial.println(receivedChars);
     str = receivedChars;
    for (int i = 0; i < str.length(); i++) {
      if (str.substring(i, i+1) == " ") {
        fir = str.substring(0, i).toInt();
        sec = str.substring(i+1).toInt();
        break;
      }
    }
    Serial.println(fir);
    Serial.println(sec);
      wclock.showTime(fir, sec, 0);
    newData = false;
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.println("");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

unsigned long getDCFTime()
{
  time_t DCFtime = DCF.getTime();
  // Indicator that a time check is done
  if (DCFtime!=0) {
    Serial.print("X");
  }
  return DCFtime;
}
