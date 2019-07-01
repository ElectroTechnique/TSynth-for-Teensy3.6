// This Teensy3 native optimized version requires specific pins
#define sclk 20  // SCLK can also use pin 14  
#define mosi 21  // MOSI can also use pin 7
#define cs  2  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define dc   3   //but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define rst  1   // RST can use any pin

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
//#include <ST7735_t3.h> // Hardware-specific library
#include "Fonts/FreeSansBold18pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansOblique24pt7b.h"
#include "Fonts/FreeSansBoldOblique24pt7b.h"

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

//ST7735_t3 tft = ST7735_t3(cs, dc, mosi, sclk, rst);
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);
//Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

boolean updateDisplay = true;
long timer = 0;
const long DISPLAYTIMEOUT = 1500;

void startTimer() {
  timer = millis();
}

boolean timeOut() {
  if (timer == 0) return false;
  if ((millis() - timer) > DISPLAYTIMEOUT) {
    timer = 0;
    return true;
  }
  return false;
}

void testdrawtext(const char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t y = 0; y < tft.height(); y += 5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x = 0; x < tft.width(); x += 5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST7735_BLACK);
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color1);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = radius; x < tft.width(); x += radius * 2) {
    for (int16_t y = radius; y < tft.height(); y += radius * 2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = 0; x < tft.width() + radius; x += radius * 2) {
    for (int16_t y = 0; y < tft.height() + radius; y += radius * 2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testroundrects() {
  tft.fillScreen(ST7735_BLACK);
  int color = 100;
  int i;
  int t;
  for (t = 0 ; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = 127;
    int h = 159;
    for (i = 0 ; i <= 24; i += 1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1100;
    }
    color += 100;
  }
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
}

void renderCurrentPatchPage(int number, String patchName) {
  //  tft.fillScreen(ST7735_BLACK);
  //  tft.setFont(&FreeSansBold18pt7b);
  //  tft.setCursor(5, 55);
  //  tft.setTextColor(ST7735_YELLOW);
  //  tft.setTextSize(1);
  //  tft.println(number);
  //  tft.drawFastHLine(10, 58, tft.width() - 20, ST7735_YELLOW);
  //  tft.setFont(&FreeSans12pt7b);
  //  tft.setCursor(0, 90);
  //  tft.setTextColor(ST7735_WHITE);
  //  tft.println(patchName);
}

void renderPatchNamePage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(5, 50);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println("9");
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(40, 50);
  tft.setTextColor(ST7735_WHITE);
  tft.println("Moog Bass");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(5, 75);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println("12");
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(40, 75);
  tft.setTextColor(ST7735_WHITE);
  tft.println("Vangelis Str");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(5, 100);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println("127");
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(40, 100);
  tft.setTextColor(ST7735_WHITE);
  tft.println("Acid Lead 123");
}

void renderCurrentParameterPage(String parameter, String value) {
  if (updateDisplay) {
    startTimer();
    //  tft.fillScreen(ST7735_BLACK);
    //  tft.setFont(&FreeSans12pt7b);
    //  tft.setCursor(5, 55);
    //  tft.setTextColor(ST7735_YELLOW);
    //  tft.setTextSize(1);
    //  tft.println(parameter);
    //  tft.drawFastHLine(10, 58, tft.width() - 20, ST7735_YELLOW);
    //  tft.setFont(&FreeSans12pt7b);
    //  tft.setCursor(0, 90);
    //  tft.setTextColor(ST7735_WHITE);
    //  tft.println(value);
  }
}

void setupDisplay() {
  tft.initR(INITR_REDTAB);   // initialize a ST7735R chip, red tab
  tft.setRotation(3);
  tft.invertDisplay(true);
  tft.fillScreen(ST7735_BLACK);
  renderBootUpPage();
  //renderCurrentPatchPage();
}
