#include <DCF77.h>

int DCF77Pin=11;
int blinkPin=13;
int seconds=0;
int previousSecond=0;
int minutes=0;
int hours=0;
DCF77 myDCF=DCF77(DCF77Pin);


void setup(void) 
{
  pinMode(blinkPin, OUTPUT);
  // open the serial port at 9600 bps:
  Serial.begin(9600);
}

void loop(void) 
{
  int DCFsignal = myDCF.scanSignal();
  if (DCFsignal) {
    digitalWrite(blinkPin, HIGH);
  } else {
    digitalWrite(blinkPin, LOW);
  }
  hours=myDCF.hh;
  minutes=myDCF.mm;
  seconds=myDCF.ss;
  if (seconds != previousSecond) 
    myDCF.serialDumpTime();
  delay(20);
  previousSecond = seconds;
}
