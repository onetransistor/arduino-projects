/*
 * Test cu afisaj cu 7 segmente controlat de MAX7219
 * 
 * https://ro.onetransistor.eu/2020/07/afisare-valori-numerice-max7219.html
 * 
 */

#include "LedControl.h"

LedControl lc = LedControl(12, 11, 10, 1);

unsigned long dispUpdate = 0;
unsigned long testUpdate = 10000;

void startUp() {
  lc.setColumn(0, 1, 0xFF);
  delay(100);
  lc.setColumn(0, 2, 0xFF);
  delay(100);
  lc.setColumn(0, 3, 0xFF);
  delay(100);
  lc.setColumn(0, 4, 0xFF);
  delay(100);
  lc.setColumn(0, 5, 0xFF);
  delay(100);
  lc.setColumn(0, 6, 0xFF);
  delay(100);
  lc.setColumn(0, 7, 0xFF);
  delay(100);
  lc.setRow(0, 7, 0x00);
  lc.setRow(0, 0, 0x00);
  delay(100);
  lc.setRow(0, 6, 0x00);
  lc.setRow(0, 1, 0x00);
  delay(100);
  lc.setRow(0, 5, 0x00);
  lc.setRow(0, 2, 0x00);
  delay(100);
  lc.setRow(0, 4, 0x00);
  lc.setRow(0, 3, 0x00);
  delay(100);
}


void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  startUp();

  // numerotare segmente
  //segmentBit();
  //delay(60000);
}

void readVoltage() {
  int av = analogRead(A2) / 2.048;

  byte d;

  d = av % 10;
  lc.setDigit(0, 4, d, false);

  av /= 10;
  d = av % 10;
  lc.setDigit(0, 5, d, false);

  av /= 10;
  d = av % 10;
  lc.setDigit(0, 6, d, true);

  av /= 10;
  d = av % 10;
  if (av != 0) lc.setDigit(0, 7, d, false);
  else lc.setChar(0, 7, ' ', false);
}

void readCurrent() {
  int av = analogRead(A3) / 2.048;

  byte d;

  d = av % 10;
  lc.setDigit(0, 0, d, false);

  av /= 10;
  d = av % 10;
  lc.setDigit(0, 1, d, false);

  av /= 10;
  d = av % 10;
  lc.setDigit(0, 2, d, true);

  av /= 10;
  d = av % 10;
  if (av != 0) lc.setDigit(0, 3, d, false);
  else lc.setChar(0, 3, ' ', false);
}

void segmentBit() {
  lc.clearDisplay(0);
  lc.setRow(0, 7, 0x01); // B00000001
  lc.setRow(0, 6, 0x02); // B00000010
  lc.setRow(0, 5, 0x04); // B00000100
  lc.setRow(0, 4, 0x08); // B00001000
  lc.setRow(0, 3, 0x10); // B00010000
  lc.setRow(0, 2, 0x20); // B00100000
  lc.setRow(0, 1, 0x40); // B01000000
  lc.setRow(0, 0, 0x80); // B10000000
}

void displayError() {
  lc.clearDisplay(0);
  lc.setRow(0, 7, 0x4F);
  lc.setRow(0, 6, 0x05);
  lc.setRow(0, 5, 0x85);
  lc.setRow(0, 4, 0x00);
  lc.setRow(0, 3, 0x37);
  lc.setRow(0, 2, 0x4F);
  lc.setRow(0, 1, 0x77);
  lc.setRow(0, 0, 0x0F);
}

void loop() {
  unsigned long current = millis();

  // TEST: o data la 10 secunde apare mesajul de eroare
  if (testUpdate + 10000 < current) {
    displayError();
    testUpdate = current;
    dispUpdate = testUpdate + 3000; // blocheaza pentru 3 secunde afisarea valorilor numerice
  }

  // La fiecare 0,5 secunde se afiseaza valorile analogice
  if (dispUpdate + 500 < current) {
    readVoltage();
    readCurrent();
    dispUpdate = current;
  }
}
