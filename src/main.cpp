#include <Arduino.h>
#include <LedDisplay.h>
#include <Wire.h>
#include <RTClib.h>
#include <GyverButton.h>

RTC_DS3231 rtc;

// Define pins for the LED display. 
// You can change these, just re-wire your board:
#define dataPin 2              // connects to the display's data in
#define registerSelect 3       // the display's register select pin 
#define clockPin 4             // the display's clock pin
#define enable 5               // the display's chip enable pin
#define reset 6               // the display's reset pin
#define modeButtonPin 8
#define selectButtonPin 9

#define displayLength 8        // number of characters in the display

#define modeButtonMaxHoldCount 3
#define modeButtonMaxClickCount 2

#define modeShowTime 0
#define modeEditTime 1
#define modeEditDate 2
#define modeEditBrightness 3

// create am instance of the LED display library:
LedDisplay myDisplay = LedDisplay(dataPin, registerSelect, clockPin, enable, reset, displayLength);
GButton modeButton(modeButtonPin);
GButton selectButton(selectButtonPin);
DateTime now;

int brightness = 15;        // screen brightness

unsigned long lastBlinkMillis = 0;
bool printEmptyBlock = false;

int modeButtonHoldCount = 0;
int modeButtonClickCount = 0;

void setup() {
  // initialize the display library:
  myDisplay.begin();
  // set the brightness of the display:
  myDisplay.setBrightness(brightness);

  modeButton.setTickMode(AUTO);
  selectButton.setTickMode(AUTO);

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
  if (value < 10) {
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

    if (needPrintSeparator) {
      myDisplay.print(':');
    }
  } else {
    printBlock(value, needPrintSeparator);
  }
}

void printTime() {
  printBlock(now.hour(), true);
  printBlock(now.minute(), true);
  printBlock(now.second());
}

void printDate() {
  printBlock(now.day(), true);
  printBlock(now.month(), true);
  printBlock(String(now.year(), DEC).substring(2).toInt());
}

void checkButtons() {

  if (modeButton.isHolded())  {
    modeButtonHoldCount++;
    modeButtonClickCount = 0;

    if (modeButtonHoldCount > modeButtonMaxHoldCount) {
      modeButtonHoldCount = 0;
    }

    Serial.println(modeButtonHoldCount);
  }

  if (modeButton.isClick())  {
    modeButtonClickCount++;

    if (modeButtonClickCount > modeButtonMaxClickCount) {
      modeButtonClickCount = 0;
    }
    
  }
  
}

void printEditTime() {
  switch (modeButtonClickCount) {
  case 0:
    printBlinkBlock(now.hour(), true);
    printBlock(now.minute(), true);
    printBlock(now.second());
    break;
  case 1:
    printBlock(now.hour(), true);
    printBlinkBlock(now.minute(), true);
    printBlock(now.second());
    break;
  case 2:
    printBlock(now.hour(), true);
    printBlock(now.minute(), true);
    printBlinkBlock(now.second());
    break;
  }
}

void printEditDate() {
  switch (modeButtonClickCount) {
  case 0:
    printBlinkBlock(now.day(), true);
    printBlock(now.month(), true);
    printBlock(String(now.year(), DEC).substring(2).toInt());
    break;
  case 1:
    printBlock(now.day(), true);
    printBlinkBlock(now.month(), true);
    printBlock(String(now.year(), DEC).substring(2).toInt());
    break;
  case 2:
    printBlock(now.day(), true);
    printBlock(now.month(), true);
    printBlinkBlock(String(now.year(), DEC).substring(2).toInt());
    break;
  }
}

void printDataByMode() {
  switch (modeButtonHoldCount) {
  case modeShowTime:
    printTime();
    break;
  case modeEditTime:
    printEditTime();
    break;
  case modeEditDate:
    printEditDate();
    break;
  case modeEditBrightness:
    printBlinkBlock(brightness);
    myDisplay.print(' ');
    myDisplay.print(' ');
    myDisplay.print(' ');
    myDisplay.print(' ');
    myDisplay.print(' ');
    myDisplay.print(' ');
    break;
  default:
    printTime();
    break;
  }
}

void loop() {
  myDisplay.home();
  checkButtons();
  
  now = rtc.now();

  printDataByMode();
}