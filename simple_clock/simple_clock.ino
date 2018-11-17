/*******************************************************************************

   Simple Arduino Clock without RTC
   Version 1.0

   Copyright (C) 2018 One Transistor <https://www.onetransistor.eu>

   Developed on Arduino Uno compatible board (ATmega328p) with LCD & keypad
   shield (16x2 HD44780 LCD and analog keypad).

   To edit date & time, push SELECT to cycle through hour, minutes, day, month
   and year. Then push UP key to increment selected variable (the one that is
   blinking).

   To adjust accuracy slightly alter the definition of ONESECOND below.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.

*******************************************************************************/

#include <LiquidCrystal.h>

#define START_YEAR  18 // initial year (18 means 2018)
#define START_WDAY  0 // week day of 01.01.18 (0 for Monday)
#define ONESECOND 1000 // milliseconds delay used for counting time

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

unsigned long int lastTime = 0;
unsigned long int editTime = 0;
unsigned long int keyTime = 0;
byte editMode = 0;
bool dispFlag = false;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte hh, mm, ss;
byte dd, MM, YY;
byte wd;

byte daysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char* wdays[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

void dispSec() {
  lcd.setCursor(10, 0);
  if (ss < 10) lcd.print('0');
  lcd.print(ss, DEC);
}

void dispMin() {
  lcd.setCursor(7, 0);
  if (mm < 10) lcd.print('0');
  lcd.print(mm, DEC);
}

void dispHour() {
  lcd.setCursor(4, 0);
  if (hh < 10) lcd.print('0');
  lcd.print(hh, DEC);
}

void dispDay() {
  lcd.setCursor(0, 1);
  if (dd < 9) lcd.print('0');
  lcd.print(dd + 1, DEC);

  lcd.setCursor(13, 1);
  lcd.print(wdays[wd]);
}

void dispMonth() {
  lcd.setCursor(3, 1);
  lcd.print(months[MM]);
}

void dispYear() {
  lcd.setCursor(7, 1);
  lcd.print(YY + 2000, DEC);
}

void dispAlarm(bool on) {
  lcd.setCursor(0, 0);
  on ? lcd.print("<<<") : lcd.print("   ");
  lcd.setCursor(13, 0);
  on ? lcd.print(">>>") : lcd.print("   ");
}

void dispAll() {
  dispMin();
  dispHour();
  dispDay();
  dispMonth();
  dispYear();
}

void clearMin() {
  lcd.setCursor(7, 0);
  lcd.print("  ");
}

void clearHour() {
  lcd.setCursor(4, 0);
  lcd.print("  ");
}

void clearDay() {
  lcd.setCursor(0, 1);
  lcd.print("  ");

  lcd.setCursor(13, 1);
  lcd.print("   ");
}

void clearMonth() {
  lcd.setCursor(3, 1);
  lcd.print("   ");
}

void clearYear() {
  lcd.setCursor(7, 1);
  lcd.print("    ");
}

void countTime() {
  // count seconds
  ss++;
  if (ss < 60) {
    dispSec();
    return;
  }
  ss = 0;
  dispSec();

  // count minutes
  mm++;
  if (mm < 60) {
    dispMin();
    return;
  }
  mm = 0;
  dispMin();

  // count hours
  hh++;
  if (hh < 24) {
    dispHour();
    return;
  }
  hh = 0;
  dispHour();

  dd++; wd++;
  if (wd == 7) wd = 0;
  if (dd < daysOfMonth[MM]) {
    dispDay();
    return;
  }
  dd = 0;
  dispDay();

  MM++;
  if (MM < 12) {
    dispMonth();
    return;
  }
  MM = 0;
  dispMonth();

  YY++;
  if ((YY % 4) == 0) daysOfMonth[1] = 29;
  else daysOfMonth[1] = 28;
  if ((YY % 100) == 0) daysOfMonth[1] = 28;
  dispYear();
}

byte readKeypad() {
  int adc = analogRead(A0);

  if (adc > 1000) return btnNONE; // this is the 1st option for speed reasons

  if (adc < 50) return btnRIGHT;
  if (adc < 250) return btnUP;
  if (adc < 450) return btnDOWN;
  if (adc < 650) return btnLEFT;
  if (adc < 850) return btnSELECT;

  return btnNONE;
}

void setup() {
  pinMode(10, INPUT);
  pinMode(A0, INPUT);
  lcd.begin(16, 2);

  lcd.setCursor(6, 0); lcd.print(':');
  lcd.setCursor(9, 0); lcd.print(':');
  lcd.setCursor(2, 1); lcd.print('-');
  lcd.setCursor(6, 1); lcd.print('-');
  lcd.setCursor(11, 1); lcd.print(',');

  ss = 0;
  mm = 0;
  hh = 0;
  dd = 0;
  MM = 0;
  YY = START_YEAR;
  wd = START_WDAY;

  dispSec();
  dispAll();

  delay(ONESECOND);
}

void loop() {
  unsigned long int currentTime = millis();
  if (currentTime >= lastTime + ONESECOND) {
    countTime();
    lastTime = currentTime;
  }

  if (currentTime >= editTime + 500) {
    if (editMode) {
      switch (editMode) {
        case 1:
          dispFlag ? dispHour() : clearHour();
          break;
        case 2:
          dispFlag ? dispMin() : clearMin();
          break;
        case 3:
          dispFlag ? dispDay() : clearDay();
          break;
        case 4:
          dispFlag ? dispMonth() : clearMonth();
          break;
        case 5:
          dispFlag ? dispYear() : clearYear();
          break;
      }

      dispFlag = 1 - dispFlag;
    }

    editTime = currentTime;
  }

  // scan keypad every 10 ms
  if (currentTime >= keyTime + 10) {
    byte key = readKeypad();

    // if using a pushbutton connected to a digital input pin modify "key == btnSELECT"
    if (key == btnSELECT) {
      dispAll(); // make sure everything is displayed
      editTime = 0; // start blinking as soon as possible
      editMode += 1;
      if (editMode > 5) {
        editMode = 0;
      }
    }

    // if using a pushbutton connected to a digital input pin modify "key == btnUP"
    if ((key == btnUP) && editMode) {
      switch (editMode) {
        case 1: hh = (hh + 1) % 24;
          break;
        case 2: mm = (mm + 1) % 60;
          break;
        case 3: dd = (dd + 1) % daysOfMonth[MM];
          wd = (wd + 1) % 7;
          if (dd == 0) wd = (wd + 35 - daysOfMonth[MM]) % 7;
          break;
        case 4: wd = (wd + daysOfMonth[MM]) % 7;
          MM = (MM + 1) % 12;
          if (MM == 0) {
            wd = (wd + 6) % 7;
            if ((YY % 4) == 0) wd = (wd + 5) % 7;
          }
          break;
        case 5:
          unsigned int days = 365;
          if ((YY % 4) == 0) days = 366;
          if ((YY % 100) == 0) days = 365;
          wd = (wd + days) % 7;
          YY = (YY + 1) % 56;
          if ((YY % 4) == 0) daysOfMonth[1] = 29;
          else daysOfMonth[1] = 28;
          if ((YY % 100) == 0) daysOfMonth[1] = 28;
          break;
      }
      dispAll();
    }

    keyTime = currentTime;
    if (key != btnNONE) keyTime += 200; // delay next possible key press by 200ms
  }
}
