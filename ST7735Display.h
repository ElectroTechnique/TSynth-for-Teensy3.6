#include "TeensyThreads.h"

// This Teensy3 native optimized version requires specific pins
#define sclk 20  // SCLK can also use pin 14  
#define mosi 21  // MOSI can also use pin 7
#define cs  2  // CS & DC can use pins 2, 6, 9, 10, 15, 20, 21, 22, 23
#define dc   3   //but certain pairs must NOT be used: 2+10, 6+9, 20+23, 21+22
#define rst  8   // RST can use any pin
#define DISPLAYDELAY 2
#define DISPLAYTIMEOUT 800

#include <ST7735_t3.h> // Hardware-specific library for T3.x

#include "Fonts/Org_01.h"
#include "Fonts/FreeSansBold18pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansOblique24pt7b.h"
#include "Fonts/FreeSansBoldOblique24pt7b.h"

ST7735_t3 tft = ST7735_t3(cs, dc, mosi, sclk, rst);

String currentParameter = "";
String currentValue = "";
String currentPgmNum = "";
String currentPatchName = "";
String newPatchName = "";
int paramType = PARAMETER;

boolean voiceOn[NO_OF_VOICES] = {false, false, false, false};

unsigned long timer = 0;

void startTimer() {
  if (state == PARAMETER) {
    timer = millis();
  }
}

void renderBootUpPage() {
  tft.fillScreen(ST7735_BLACK);
  tft.drawRect(17, 30, 46, 11, ST7735_WHITE);
  tft.fillRect(63, 30, 61, 11, ST7735_WHITE);
  tft.setCursor(20, 31);
  tft.setFont(&Org_01);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);
  tft.println("ELECTRO");
  tft.setTextColor(ST7735_BLACK);
  tft.setCursor(66, 37);
  tft.println("TECHNIQUE");
  tft.setTextColor(ST7735_YELLOW);
  tft.setFont(&FreeSansBoldOblique24pt7b);
  tft.setCursor(5, 82);
  tft.setTextSize(1);
  tft.println("T");
  tft.setFont(&FreeSansOblique24pt7b);
  tft.setCursor(30, 82);
  tft.println("Synth");
  tft.setTextColor(ST7735_RED);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(110, 100);
  tft.println("V1.0");
}

void renderCurrentPatchPage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setCursor(5, 53);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println(currentPgmNum);

  tft.drawRect(120, 28, 12, 12 , ST7735_BLUE);
  tft.drawRect(135, 28, 12, 12 , ST7735_BLUE);
  tft.drawRect(120, 43, 12, 12 , ST7735_BLUE);
  tft.drawRect(135, 43, 12, 12 , ST7735_BLUE);

  tft.setTextColor(ST7735_BLACK);
  tft.setFont(&Org_01);
  if (voiceOn[0]) {
    tft.fillRect(122, 30, 8, 8 , ST7735_BLUE);
    tft.setCursor(125, 36);
    tft.println("1");
  }
  if (voiceOn[1]) {
    tft.fillRect(137, 30, 8, 8 , ST7735_BLUE);
    tft.setCursor(138, 36);
    tft.println("2");
  }
  if (voiceOn[2]) {
    tft.fillRect(122, 45, 8, 8 , ST7735_BLUE);
    tft.setCursor(124, 51);
    tft.println("3");
  }
  if (voiceOn[3]) {
    tft.fillRect(137, 45, 8, 8 , ST7735_BLUE);
    tft.setCursor(138, 51);
    tft.println("4");
  }

  tft.drawFastHLine(10, 62, tft.width() - 20, ST7735_RED);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(1, 90);
  tft.setTextColor(ST7735_WHITE);
  tft.println(currentPatchName);
}

void renderPulseWidth(float value) {
  tft.drawFastVLine(110, 74, 21, ST7735_CYAN);
  tft.drawFastHLine(110, 74, 13 + (value * 13), ST7735_CYAN);
  tft.drawFastVLine(123 + (value * 13), 74, 20, ST7735_CYAN);
  tft.drawFastHLine(123 + (value * 13), 94, 13 - (value * 13), ST7735_CYAN);
  tft.drawFastVLine(136, 74, 21, ST7735_CYAN);
}

void renderVarTriangle(float value) {
  tft.drawLine(110, 94, 123 + (value * 13), 74, ST7735_CYAN);
  tft.drawLine(123 + (value * 13), 74, 136, 94, ST7735_CYAN);
}

void renderEnv(float att, float dec, float sus, float rel) {
  tft.drawLine(110, 94, 110 + (att * 13), 74, ST7735_CYAN);
  tft.drawLine(110 + (att * 13), 74.0, 110 + ((att  + dec) * 13), 94 - (sus * 20), ST7735_CYAN);
  tft.drawFastHLine(110 + ((att + dec) * 13), 94 - (sus * 20), 30 - ((att  + dec) * 13), ST7735_CYAN);
  tft.drawLine(139, 94 - (sus * 20), 139 + (rel * 13), 94, ST7735_CYAN);
}

void renderCurrentParameterPage() {
  switch (state) {
    case PARAMETER:
      tft.fillScreen(ST7735_BLACK);
      tft.setFont(&FreeSans12pt7b);
      tft.setCursor(0, 53);
      tft.setTextColor(ST7735_YELLOW);
      tft.setTextSize(1);
      tft.println(currentParameter);
      tft.drawFastHLine(10, 62, tft.width() - 20, ST7735_RED);
      tft.setCursor(1, 90);
      tft.setTextColor(ST7735_WHITE);
      tft.println(currentValue);
      switch (paramType) {
        case PULSE:
          //   renderPulseWidth(strtof(currentValue,NULL));
          break;
        case VAR_TRI:
          //     renderVarTriangle(strtof(currentValue,NULL));
          break;
        case FILTER_ENV:
          renderEnv(vcfAttack * 0.0001, vcfDecay * 0.0001, vcfSustain, vcfRelease * 0.0001);
          break;
        case AMP_ENV:
          renderEnv(vcaAttack * 0.0001, vcaDecay * 0.0001, vcaSustain, vcaRelease * 0.0001);
          break;
      }
      break;
  }
}

void renderDeletePatchPage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setCursor(5, 53);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println("Delete?");
  tft.drawFastHLine(10, 60, tft.width() - 20, ST7735_RED);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(0, 78);
  tft.setTextColor(ST7735_YELLOW);
  tft.println(patches.last().patchNo);
  tft.setCursor(35, 78);
  tft.setTextColor(ST7735_WHITE);
  tft.println(patches.last().patchName);
  tft.fillRect(0, 87, tft.width(), 21, 0xA000);
  tft.setCursor(0, 100);
  tft.setTextColor(ST7735_YELLOW);
  tft.println(patches.first().patchNo);
  tft.setCursor(35, 98);
  tft.setTextColor(ST7735_WHITE);
  tft.println(patches.first().patchName);
}

void renderSavePage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSansBold18pt7b);
  tft.setCursor(5, 53);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.println("Save?");
  tft.drawFastHLine(10, 60, tft.width() - 20, ST7735_RED);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(0, 78);
  tft.setTextColor(ST7735_YELLOW);
  tft.println(patches[patches.size() - 2].patchNo);
  tft.setCursor(35, 78);
  tft.setTextColor(ST7735_WHITE);
  tft.println(patches[patches.size() - 2].patchName);
  tft.fillRect(0, 87, tft.width(), 21, 0xA000);
  tft.setCursor(0, 100);
  tft.setTextColor(ST7735_YELLOW);
  tft.println(patches.last().patchNo);
  tft.setCursor(35, 100);
  tft.setTextColor(ST7735_WHITE);
  tft.println(patches.last().patchName);
}

void renderReinitialisePage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(5, 53);
  tft.println("Initialise to");
  tft.setCursor(5, 90);
  tft.println("panel setting");
}

void renderPatchNamingPage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(0, 53);
  tft.println("Rename Patch");
  tft.drawFastHLine(10, 62, tft.width() - 20, ST7735_RED);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(5, 90);
  tft.println(newPatchName);
}

void renderRecallPage() {
  tft.fillScreen(ST7735_BLACK);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(0, 45);
  tft.setTextColor(ST7735_YELLOW);
  tft.println(patches.last().patchNo);
  tft.setCursor(35, 45);
  tft.setTextColor(ST7735_WHITE);
  tft.println(patches.last().patchName);

  tft.fillRect(0, 56, tft.width(), 23, 0xA000);
  tft.setCursor(0, 72);
  tft.setTextColor(ST7735_YELLOW);
  tft.println(patches.first().patchNo);
  tft.setCursor(35, 72);
  tft.setTextColor(ST7735_WHITE);
  tft.println(patches.first().patchName);

  tft.setCursor(0, 98);
  tft.setTextColor(ST7735_YELLOW);
  patches.size() > 1 ? tft.println(patches[1].patchNo) : tft.println(patches.last().patchNo);
  tft.setCursor(35, 98);
  tft.setTextColor(ST7735_WHITE);
  patches.size() > 1 ? tft.println(patches[1].patchName) : tft.println(patches.last().patchName);
}

void showRenamingPage(String newName) {
  newPatchName = newName;
}

void showCurrentParameterPage(String param, String val, int pType) {
  currentParameter = param;
  currentValue = val;
  paramType = pType;
  startTimer();
}

void showCurrentParameterPage(String param, String val) {
  showCurrentParameterPage(param, val, PARAMETER);
}

void showPatchPage(String number, String patchName) {
  currentPgmNum = number;
  currentPatchName = patchName;
}

void displayThread() {
  threads.delay(2000);//Give bootup page chance to display
  while (1) {
    switch (state) {
      case PARAMETER:
        if ((millis() - timer) > DISPLAYTIMEOUT) {
          renderCurrentPatchPage();
        } else {
          renderCurrentParameterPage();
        }
        break;
      case RECALL:
        renderRecallPage();
        break;
      case SAVE:
        renderSavePage();
        break;
      case REINITIALISE:
        renderReinitialisePage();
        tft.updateScreen();//update before delay
        threads.delay(1000);
        state = PARAMETER;
        break;
      case PATCHNAMING:
        renderPatchNamingPage();
        break;
      case PATCH:
        renderCurrentPatchPage();
        break;
      case DELETE:
        renderDeletePatchPage();
        break;
    }

    tft.updateScreen();
    threads.delay(DISPLAYDELAY);
  }
}

void setupDisplay() {
  tft.useFrameBuffer(true);
  tft.initR(INITR_GREENTAB);
  tft.setRotation(3);
  tft.invertDisplay(true);
  renderBootUpPage();
  tft.updateScreen();
  threads.addThread(displayThread);
}
