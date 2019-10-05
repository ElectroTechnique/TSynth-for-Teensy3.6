#include "TeensyThreads.h"

// This Teensy3 native optimized version requires specific pins
#define sclk 20  // SCLK can also use pin 14  
#define mosi 21  // MOSI can also use pin 7
#define cs  2  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define dc   3   //but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define rst  8   // RST can use any pin
#define DISPLAYDELAY 10

#include <ST7735_t3.h> // Hardware-specific library for T3.x

#include "Fonts/FreeSansBold18pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansOblique24pt7b.h"
#include "Fonts/FreeSansBoldOblique24pt7b.h"

ST7735_t3 tft = ST7735_t3(cs, dc, mosi, sclk, rst);

volatile long DISPLAYTIMEOUT = 600;
volatile boolean newParameter = true;
volatile boolean newValue = true;

String currentParameter = "parameter";
String currentValue = "value";
String currentPgmNum = "0";
String currentPatchName = "patchname";

long timer = 0;

void startTimer() {
  timer = millis();
}

volatile boolean timeOut() {
  if (timer == 0) return false;
  if ((millis() - timer) > DISPLAYTIMEOUT) {
    timer = 0;
    return true;
  }
  return false;
}

void renderBootUpPage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSansBoldOblique24pt7b);
  tft.setCursor(5, 75);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println("T");
  tft.setFont(&FreeSansOblique24pt7b);
  tft.setCursor(28, 75);
  tft.println("Synth");
  tft.setTextColor(ST7735_RED);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(110, 95);
  tft.println("V0.92");
}

void renderCurrentPatchPage() {
  if (newParameter) {
    tft.fillRect(0, 0, tft.width(), 62, ST7735_BLACK);
    tft.setFont(&FreeSansBold18pt7b);
    tft.setCursor(5, 53);
    tft.setTextColor(ST7735_YELLOW);
    tft.setTextSize(1);
    tft.println(currentPgmNum);
    tft.drawFastHLine(10, 62, tft.width() - 20, ST7735_YELLOW);
    newParameter = false;
  }
  if (newValue) {
    tft.fillRect(0, 63, tft.width(), 45, ST7735_BLACK);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(1, 90);
    tft.setTextColor(ST7735_WHITE);
    tft.println(currentPatchName);
    newValue = false;
  }
}


void renderCurrentParameterPage() {
  if (newParameter) {
    tft.fillRect(0, 0, tft.width(), 62, ST7735_BLACK);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(5, 53);
    tft.setTextColor(ST7735_YELLOW);
    tft.setTextSize(1);
    tft.println(currentParameter);
    tft.drawFastHLine(10, 62, tft.width() - 20, ST7735_YELLOW);
  }
  if (newValue) {
    tft.fillRect(0, 63, tft.width(), 45, ST7735_BLACK);
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(1, 90);
    tft.setTextColor(ST7735_WHITE);
    tft.println(currentValue);
  }
}

void renderPatchNamePage() {
  //  tft.fillRect(ST7735_BLACK);
  //  tft.setFont(&FreeSans9pt7b);
  //  tft.setCursor(5, 50);
  //  tft.setTextColor(ST7735_YELLOW);
  //  tft.setTextSize(1);
  //  tft.println("9");
  //  tft.setFont(&FreeSans9pt7b);
  //  tft.setCursor(40, 50);
  //  tft.setTextColor(ST7735_WHITE);
  //  tft.println("Moog Bass");
}

void showCurrentParameterPage(String param, String val, int time) {
  DISPLAYTIMEOUT = time;
  if (currentParameter == param) {
    newParameter = false;
  } else {
    currentParameter = param;
    newParameter = true;
    startTimer();
  }

  if (currentValue == val) {
    newValue = false;
  } else {
    currentValue = val;
    newValue = true;
    startTimer();
  }
}

void showCurrentParameterPage(String param, String val) {
  showCurrentParameterPage(param, val, 600);
}

void showCurrentPatchPage(String number, String patchName) {
  if (currentPgmNum == number ) {
    newParameter = false;
  } else {
    currentPgmNum = number;
    newParameter = true;
  }
  if (currentPatchName == patchName) {
    newValue = false;
  } else {
    currentPatchName = patchName;
    newValue = true;
  }
}

void displayThread() {
  threads.delay(2000);//Give bootup page chance to display
  while (1) {
    if (!timeOut()) {
      renderCurrentParameterPage();
    } else {
      currentParameter = "";
      currentValue = "";
      newParameter = true;
      newValue = true;
      renderCurrentPatchPage();
    }
    //tft.updateScreen();
    threads.delay(DISPLAYDELAY);
  }
}

void setupDisplay() {
 // tft.useFrameBuffer(true);
  tft.initR(INITR_GREENTAB);
  tft.fillScreen(ST7735_WHITE);//Display is inverted
  tft.setRotation(3);
  tft.invertDisplay(true);
  renderBootUpPage();
  threads.addThread(displayThread);
}
