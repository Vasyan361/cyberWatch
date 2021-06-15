#include <Arduino.h>
#include <LedDisplay.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// Define pins for the LED display. 
// You can change these, just re-wire your board:
#define dataPin 2              // connects to the display's data in
#define registerSelect 3       // the display's register select pin 
#define clockPin 4             // the display's clock pin
#define enable 5               // the display's chip enable pin
#define reset 6               // the display's reset pin

#define displayLength 8        // number of characters in the display

// create am instance of the LED display library:
LedDisplay myDisplay = LedDisplay(dataPin, registerSelect, clockPin, enable, reset, displayLength);

int brightness = 15;        // screen brightness

unsigned long lastBlinkMillis = 0;
bool printEmptyBlock = false; 


void setup() {
  // initialize the display library:
  myDisplay.begin();
  // set the brightness of the display:
  myDisplay.setBrightness(brightness);

  Serial.begin(9600);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void printBlock(int value, bool needPrintSeparator = false) {
  if (value < 10)
  {
    myDisplay.print(0, DEC);
  }
  myDisplay.print(value, DEC);

  if (needPrintSeparator) {
    myDisplay.print(':');
  }
}

void printBlinkBlock(int value, bool needPrintSeparator = false) {
  if (millis() - lastBlinkMillis >= 500){
    printEmptyBlock = !printEmptyBlock;
    lastBlinkMillis = millis();
  }

  if (printEmptyBlock)
  {
    myDisplay.print(' ');
    myDisplay.print(' ');

    if (needPrintSeparator)
    {
      myDisplay.print(':');
    }
  } else {
    printBlock(value, needPrintSeparator);
  }
}

void loop() {
  myDisplay.home();
  
  DateTime now = rtc.now();

  printBlock(now.hour(), true);
  printBlock(now.minute(), true);
  printBlock(now.second());
}