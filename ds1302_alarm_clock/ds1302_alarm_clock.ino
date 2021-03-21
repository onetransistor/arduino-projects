/*******************************************************************************
   Arduino Clock with alarm function and DS1302 RTC module
   Version 1.0
   Copyright (C) 2018 One Transistor <https://www.onetransistor.eu>

   More information at:
   https://www.onetransistor.eu/2018/12/alarm-clock-with-ds1302-rtc.html

   Developed on Arduino Uno compatible board (ATmega328p) with LCD, DS1302
   module, active buzzer and 4 push buttons.

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

#define RTC_CLK 8 // clock
#define RTC_DAT 9 // data
#define RTC_RST 10 // enable

#define BTN_AL  A0
#define BTN_DN  A1
#define BTN_UP  A2
#define BTN_SET A3

#define AL_BUZZ 11

#include <LiquidCrystal.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

unsigned long timeUpdate = 0;
unsigned long timeButton = 0;
unsigned long timeAlarm = 0;
bool buttonPress = false;
byte alarmOn = 0;
bool alarmSw = 1;
byte setMode = 0;
byte setAlarm = 0;

byte daysOfMonth[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char* months[13] = {"---", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char* wdays[8] = {"---", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

byte hh, mm, ss; // hour, minute, second
byte dd, MM, yy; // day, month, year
byte dw; // day of the week
byte ae, ah, am; // alarm: enabled, hour, minute

static byte bcd2bin (byte val) {
  return val - 6 * (val >> 4);
}

static byte bin2bcd (byte val) {
  return val + 6 * (val / 10);
}

// DS1302 communication from https://github.com/NeiroNx/RTCLib
void rtc_write(byte val) {
  pinMode(RTC_DAT, OUTPUT);
  shiftOut(RTC_DAT, RTC_CLK, LSBFIRST, val);
}

void rtc_write_reg(byte addr, byte val) {
  digitalWrite(RTC_RST, HIGH);
  rtc_write(addr);
  rtc_write(val);
  digitalWrite(RTC_RST, LOW);
}

byte rtc_read() {
  pinMode(RTC_DAT, INPUT);
  byte value = 0;
  for (byte i = 0; i < 8; i++) {
    value |= (digitalRead(RTC_DAT) << i);
    digitalWrite(RTC_CLK, HIGH);
    digitalWrite(RTC_CLK, LOW);
  }
  return value;
}

byte rtc_read_reg(byte addr) {
  digitalWrite(RTC_RST, HIGH);
  rtc_write(addr);
  byte val = rtc_read();
  digitalWrite(RTC_RST, LOW);
  return val;
}

void rtc_read_time() {
  digitalWrite(RTC_RST, HIGH);
  rtc_write(0xBF); // burst read mode

  byte ss_reg = rtc_read();
  if (bitRead(ss_reg, 7)) {// clock is halted
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC not running");
    lcd.setCursor(0, 1);
    lcd.print("Please set time");
    return; // TO DO
  }

  ss = bcd2bin(ss_reg & 0x7F);
  mm = bcd2bin(rtc_read());
  hh = bcd2bin(rtc_read());
  dd = bcd2bin(rtc_read());
  MM = bcd2bin(rtc_read());
  dw = bcd2bin(rtc_read());
  yy = bcd2bin(rtc_read());
  digitalWrite(RTC_RST, LOW);
}

void initTimeDisplay() {
  lcd.clear();
  lcd.setCursor(6, 0); lcd.print(':');
  lcd.setCursor(9, 0); lcd.print(':');
  lcd.setCursor(2, 1); lcd.print('-');
  lcd.setCursor(6, 1); lcd.print('-');
  lcd.setCursor(11, 1); lcd.print(',');
}

void updateDisplay() {
  rtc_read_time();

  lcd.setCursor(7, 0);
  if (mm < 10) lcd.print('0');
  lcd.print(mm, DEC);

  lcd.setCursor(4, 0);
  if (hh < 10) lcd.print('0');
  lcd.print(hh, DEC);

  lcd.setCursor(0, 1);
  if (dd < 9) lcd.print('0');
  lcd.print(dd, DEC);

  lcd.setCursor(13, 1);
  lcd.print(wdays[dw]);

  lcd.setCursor(3, 1);
  lcd.print(months[MM]);

  lcd.setCursor(7, 1);
  lcd.print(yy + 2000, DEC);

  lcd.setCursor(15, 0);
  ae ? lcd.print('*') : lcd.print(' ');
}

void setVariable() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set");
  lcd.setCursor(0, 1);
  lcd.print("< >");

  rtc_write_reg(0x8E, 0x00);

  switch (setMode) {
    case 1: // year
      lcd.setCursor(4, 0);
      lcd.print("Year");
      lcd.setCursor(6, 1);
      lcd.print(yy + 2000, DEC);
      break;
    case 2: // month
      rtc_write_reg(0x8C, bin2bcd(yy));

      lcd.setCursor(4, 0);
      lcd.print("Month");
      lcd.setCursor(6, 1);
      lcd.print(months[MM]);
      break;
    case 3: // day
      rtc_write_reg(0x88, bin2bcd(MM));

      lcd.setCursor(4, 0);
      lcd.print("Day");
      lcd.setCursor(6, 1);
      if (dd < 10) lcd.print('0');
      lcd.print(dd, DEC);
      break;
    case 4: // day of week
      rtc_write_reg(0x86, bin2bcd(dd));

      lcd.setCursor(4, 0);
      lcd.print("Weekday");
      lcd.setCursor(6, 1);
      lcd.print(wdays[dw]);
      break;
    case 5: // hour
      rtc_write_reg(0x8A, bin2bcd(dw));

      lcd.setCursor(4, 0);
      lcd.print("Hour");
      lcd.setCursor(6, 1);
      if (hh < 10) lcd.print('0');
      lcd.print(hh, DEC);
      break;
    case 6: // minutes
      rtc_write_reg(0x84, bin2bcd(hh));

      lcd.setCursor(4, 0);
      lcd.print("Minute");
      lcd.setCursor(6, 1);
      if (mm < 10) lcd.print('0');
      lcd.print(mm, DEC);
      break;
    case 7:
      rtc_write_reg(0x82, bin2bcd(mm));
      rtc_write_reg(0x80, 0x00); // make sure clock is not halted and reset seconds
    default:
      setMode = 0;
      rtc_write_reg(0x8E, 0x80); // protect writes
      initTimeDisplay();
      updateDisplay();
  }
}

void adjustVariable(char a = 1) {
  lcd.setCursor(6, 1);
  lcd.print("    ");
  lcd.setCursor(6, 1);
  char t;

  switch (setMode) {
    case 1: // year
      t = yy; t += a;
      if ((t >= 0) && (t < 100)) yy = t;
      lcd.print(yy + 2000, DEC);
      break;
    case 2: // month
      t = MM; t += a;
      if ((t > 0) && (t <= 12)) MM = t;
      lcd.print(months[MM]);
      break;
    case 3: // day
      t = dd; t += a;
      if ((t > 0) && (t <= daysOfMonth[MM])) dd = t;
      if (dd < 10) lcd.print('0');
      lcd.print(dd, DEC);
      break;
    case 4: // day of week
      t = dw; t += a;
      if ((t > 0) && (t <= 7)) dw = t;
      lcd.print(wdays[dw]);
      break;
    case 5: // hour
      t = hh; t += a;
      if ((t >= 0) && (t < 24)) hh = t;
      if (hh < 10) lcd.print('0');
      lcd.print(hh, DEC);
      break;
    case 6: // minutes
      t = mm; t += a;
      if ((t >= 0) && (t < 60)) mm = t;
      if (mm < 10) lcd.print('0');
      lcd.print(mm, DEC);
      break;
  }

  switch (setAlarm) {
    case 1: // enable
      ae = 1 - ae;
      ae ? lcd.print("Yes") : lcd.print("No ");
      break;
    case 2: // hour
      t = ah; t += a;
      if ((t >= 0) && (t < 24)) ah = t;
      if (ah < 10) lcd.print('0');
      lcd.print(ah, DEC);
      break;
    case 3: // minutes
      t = am; t += a;
      if ((t >= 0) && (t < 60)) am = t;
      if (am < 10) lcd.print('0');
      lcd.print(am, DEC);
      break;
  }
}

void setAlVariable() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm");
  lcd.setCursor(0, 1);
  lcd.print("< >");

  rtc_write_reg(0x8E, 0x00);

  switch (setAlarm) {
    case 1: // alarm enabled
      lcd.setCursor(6, 0);
      lcd.print("Enabled");
      lcd.setCursor(6, 1);
      ae ? lcd.print("Yes") : lcd.print("No ");
      break;
    case 2: // alarm hour
      rtc_write_reg(0xC0, ae);
      if (ae == 0) { // is disabled, do not ask for ah and am
        setAlarm = 0;
        rtc_write_reg(0x8E, 0x80); // protect writes
        initTimeDisplay();
        updateDisplay();
        return;
      }

      lcd.setCursor(6, 0);
      lcd.print("Hour");
      lcd.setCursor(6, 1);
      if (ah < 10) lcd.print('0');
      lcd.print(ah, DEC);
      break;
    case 3: // alarm minute
      rtc_write_reg(0xC2, ah);

      lcd.setCursor(6, 0);
      lcd.print("Minute");
      lcd.setCursor(6, 1);
      if (am < 10) lcd.print('0');
      lcd.print(am, DEC);
      break;
    case 4:
      rtc_write_reg(0xC4, am);
    default:
      setAlarm = 0;
      rtc_write_reg(0x8E, 0x80); // protect writes
      initTimeDisplay();
      updateDisplay();
  }
}

void setup() {
  pinMode(RTC_RST, OUTPUT);
  pinMode(RTC_CLK, OUTPUT);
  pinMode(RTC_DAT, INPUT);
  digitalWrite(RTC_RST, LOW);

  pinMode(BTN_AL, INPUT_PULLUP);
  pinMode(BTN_DN, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_SET, INPUT_PULLUP);

  pinMode(AL_BUZZ, OUTPUT);
  digitalWrite(AL_BUZZ, LOW);

  delay(1000);

  lcd.begin(16, 2);

  // get alarm info
  ae = rtc_read_reg(0xC1);
  ah = rtc_read_reg(0xC3);
  am = rtc_read_reg(0xC5);

  if ((ah > 23) || (am > 59)) {
    ae = 0;
    ah = 0;
    am = 0;
  }

  initTimeDisplay();
  updateDisplay();
}

void loop() {
  unsigned long current = millis();

  if ((current >= timeUpdate + 1000) && (setMode + setAlarm == 0)) {
    if (ss == 0) {
      updateDisplay();
      if (ae && (hh == ah) && (mm == am)) alarmOn = true;
      else alarmOn = false;
    }

    // display seconds
    lcd.setCursor(10, 0);
    if (ss < 10) lcd.print('0');
    lcd.print(ss, DEC);

    // increment seconds
    ss++;
    if (ss == 60) ss = 0;

    timeUpdate = current;
  }

  if (current >= timeButton + 10) {
    if ((digitalRead(BTN_SET) == LOW) && (setAlarm == 0)) {
      buttonPress = true;
      setMode++;
      setVariable();
    }

    if ((digitalRead(BTN_UP) == LOW) && (setMode || setAlarm)) {
      buttonPress = true;
      adjustVariable(1);
    }

    if ((digitalRead(BTN_DN) == LOW) && (setMode || setAlarm)) {
      buttonPress = true;
      adjustVariable(-1);
    }

    if ((digitalRead(BTN_AL) == LOW) && (setMode == 0)) {
      buttonPress = true;
      if (alarmOn) {
        alarmOn = 0;
        digitalWrite(AL_BUZZ, LOW);
        lcd.setCursor(0, 0); lcd.print("   ");
        lcd.setCursor(13, 0); lcd.print("   ");
      }
      else {
        setAlarm++;
        setAlVariable();
      }
    }

    timeButton = current;
    if (buttonPress) {
      timeButton += 200; // delay next possible key press by 200 ms
      buttonPress = false;
    }
  }

  if (current >= timeAlarm + 300) {
    if (alarmOn) {
      digitalWrite(AL_BUZZ, alarmSw);
      lcd.setCursor(0, 0);
      alarmSw ? lcd.print("<<<") : lcd.print("   ");
      lcd.setCursor(13, 0);
      alarmSw ? lcd.print(">>>") : lcd.print("   ");
      alarmSw = 1 - alarmSw;
      timeAlarm = current;
    }
    else {
      digitalWrite(AL_BUZZ, LOW);
      timeAlarm = current;
    }
  }
}
