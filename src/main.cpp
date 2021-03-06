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
#define selectButtonMaxHoldCount 2

#define modeShowTime 0
#define modeEditTime 1
#define modeEditDate 2
#define modeEditBrightness 3

#define maxHourValue 24
#define maxMinuteValue 60
#define maxSecondValue 60

#define maxBrightness 15

// create am instance of the LED display library:
LedDisplay myDisplay = LedDisplay(dataPin, registerSelect, clockPin, enable, reset, displayLength);
GButton modeButton(modeButtonPin);
GButton selectButton(selectButtonPin);
DateTime now;

int brightness = 15;        // screen brightness

unsigned long lastBlinkMillis = 0;
bool printEmptyBlock = false;

int modeButtonHoldCount = 0;

int selectButtonHoldCount = 0;

char separator = ':';

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
    myDisplay.print(separator);
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
      myDisplay.print(separator);
    }
  } else {
    printBlock(value, needPrintSeparator);
  }
}

void printTime() {
  separator = ':';

  printBlock(now.hour(), true);
  printBlock(now.minute(), true);
  printBlock(now.second());
}

void printDate() {
  separator = '-';

  printBlock(now.day(), true);
  printBlock(now.month(), true);
  printBlock(String(now.year(), DEC).substring(2).toInt());

  delay(1000);
}

void printCharge() {
  myDisplay.print("Charge48");

  delay(1000);
}

void changeTime(int value) {
  switch (selectButtonHoldCount) {
    case 0:
      value = now.hour() + value;
      if (value >= maxHourValue) {
        value = 0; 
      } else if (value < 0) {
        value = maxHourValue;
      }
      
      rtc.adjust(DateTime(
        now.year(),
        now.month(),
        now.day(),
        value,
        now.minute(),
        now.second()
        )
      );
      break;
    case 1:
      value = now.minute() + value;
      if (value >= maxMinuteValue) {
        value = 0; 
      } else if (value < 0) {
        value = maxMinuteValue;
      }
      rtc.adjust(DateTime(
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        value,
        now.second()
        )
      );
      break;
    case 2:
      value = now.second() + value;
      if (value >= maxSecondValue) {
        value = 0; 
      } else if (value < 0) {
        value = maxSecondValue;
      }
      rtc.adjust(DateTime(
        now.year(),
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        value
        )
      );
      break;
  }
}

void changeDate(int value) {
  switch (selectButtonHoldCount) {
    case 0:
      rtc.adjust(DateTime(
        now.year() + value,
        now.month(),
        now.day(),
        now.hour(),
        now.minute(),
        now.second()
        )
      );
      break;
    case 1:
      rtc.adjust(DateTime(
        now.year(),
        now.month() + value,
        now.day(),
        now.hour(),
        now.minute(),
        now.second()
        )
      );
      break;
    case 2:
      rtc.adjust(DateTime(
        now.year(),
        now.month(),
        now.day() + value,
        now.hour(),
        now.minute(),
        now.second()
        )
      );
      break;
  }
}

void modeButtonClickActions() {
  switch (modeButtonHoldCount) {
    case modeShowTime:
      printDate();
      break;
    case modeEditTime:
      changeTime(-1);
      break;
    case modeEditDate:
      changeDate(-1);
      break;
    case modeEditBrightness:
      brightness--;
      if (brightness < 1) {
        brightness = maxBrightness;
      }
      break;
  }
}

void SelectButtonClickActions() {
  switch (modeButtonHoldCount) {
    case modeShowTime:
      printCharge();
      break;
    case modeEditTime:
      changeTime(1);
      break;
    case modeEditDate:
      changeDate(1);
      break;
    case modeEditBrightness:
      brightness++;
      if (brightness > maxBrightness) {
        brightness = 1;
      }
      break;
  }
}

void checkButtons() {

  if (modeButton.isHolded())  {
    modeButtonHoldCount++;
    selectButtonHoldCount = 0;

    if (modeButtonHoldCount > modeButtonMaxHoldCount) {
      modeButtonHoldCount = 0;
    }
  }

  if (modeButton.isClick() && !modeButton.isHold())  {
    modeButtonClickActions();
  }

  if (selectButton.isHolded())  {
    selectButtonHoldCount++;

    if (selectButtonHoldCount > selectButtonMaxHoldCount) {
      selectButtonHoldCount = 0;
    }
  }

  if (selectButton.isClick() && !selectButton.isHold())  {
    SelectButtonClickActions();
  }
}

void printEditTime() {
  separator = ':';
  switch (selectButtonHoldCount) {
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
  separator = '-';
  switch (selectButtonHoldCount) {
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
  myDisplay.setBrightness(brightness);
  myDisplay.home();
  checkButtons();
  
  now = rtc.now();

  printDataByMode();
}