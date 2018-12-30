/*******************************************************************************
   Arduino capacitance meter with autoranging function
   Version 1.0
   Copyright (C) 2018 One Transistor <https://www.onetransistor.eu>

   More information at:
   https://www.onetransistor.eu/2018/12/autoranging-capacitance-meter-with-lcd.html

   Developed on Arduino Nano compatible board (ATmega328p) with LCD.

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

// where 1k resistor is connected
#define PIN_UF 10
// where 1M resistor is connected
#define PIN_NF 11
// where 220 ohm discharge resistor is connected
#define PIN_DIS 12

// set here correction factors (resistor value in M and k * 1000.0)
#define M_RESISTOR 1055.0 // the value of 1M resistor
#define K_RESISTOR 1042.5 // the value of 1k resistor

byte c_down[] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100,
  B00000
};

byte c_up[] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

int adjustment = 0;
float measurements[10] = { 0 };
byte measCount = 0;

#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void dischargeCapacitor() {
  // display discharge sign
  lcd.setCursor(15, 0);
  lcd.write(byte(0));

  // enable discharge resistor
  pinMode(PIN_DIS, OUTPUT);
  digitalWrite(PIN_DIS, LOW);

  // wait until complete discharge
  while (analogRead(A0) > 0)
    delay(100);

  // disconnect discharge resistor
  pinMode(PIN_DIS, INPUT);

  // clear sign
  lcd.setCursor(15, 0);
  lcd.write(' ');
}

long chargeCapacitor(byte resistorPin) {
  unsigned long startTime = 0;
  unsigned long stopTime = 0;

  // display charging sign
  lcd.setCursor(15, 0);
  lcd.write(byte(1));

  // set charging resistor
  pinMode(resistorPin, OUTPUT);
  digitalWrite(resistorPin, HIGH);

  // start measuring
  startTime = micros();
  while (analogRead(A0) < 646)
    ;
  stopTime = micros();

  long chargingTime = stopTime - startTime;

  // disconnect resistor
  pinMode(resistorPin, INPUT);

  // clear charging sign
  lcd.setCursor(15, 0);
  lcd.write(' ');

  return (chargingTime + adjustment);
}

void performMeasurement(float &capacity, char &unit) {
  long chTime = 0;

  capacity = 0;
  unit = 'u';

  // attempt to charge with the smaller resistor
  chTime = chargeCapacitor(PIN_UF);

  // if charging was too fast
  if (chTime < 1000) {
    // perform quick discharge
    pinMode(PIN_DIS, OUTPUT);
    digitalWrite(PIN_DIS, LOW);
    // capacitor must be smaller than 1uF
    // C < 1 uF, R = 330 => Tdis < 5 * 330 * 0.000001 = 1.65 ms
    // complete discharge occurs in less than 2 ms; let's wait 10
    delay(10);
    pinMode(PIN_DIS, INPUT);

    // try with the bigger resistor
    chTime = chargeCapacitor(PIN_NF);
    unit = 'n'; // switch to nanofarads

    if (chTime < 500) { // below 500 pF measurement is unreliable
      adjustment = 0 - chTime;
      return;
    }
  }

  if (unit == 'u')
    capacity = chTime / K_RESISTOR;
  else
    capacity = chTime / M_RESISTOR;
}

void calculateAverage() {
  byte i = 0; float capSum = 0;
  for (i; i < 10; i++)
    if (measurements[i] > 0)
      capSum += measurements[i];
    else
      break;

  lcd.setCursor(6, 1);
  if (i != 0) lcd.print(capSum / i, 2);
  else lcd.print("?          ");
}

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, c_down);
  lcd.createChar(1, c_up);

  lcd.setCursor(0, 0);
  lcd.print("   Capacitance  ");
  lcd.setCursor(0, 1);
  lcd.print("      meter     ");

  pinMode(PIN_NF, INPUT);
  pinMode(PIN_UF, INPUT);
  pinMode(PIN_DIS, INPUT);
  pinMode(A0, INPUT);

  delay(2000);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("C = ?");
  lcd.setCursor(0, 1);
  lcd.print("Avg = ?");
}

void loop() {
  float cap = 0;
  char units;

  performMeasurement(cap, units);

  lcd.setCursor(4, 0);
  if (cap) {
    // display measurement
    lcd.print(cap, 2);
    lcd.print(' ');
    lcd.print(units);
    lcd.print("F      ");

    // store it in array for averaging
    measurements[measCount] = cap;
    measCount++;
    if (measCount == 10) measCount = 0;

    // compute average and display it
    calculateAverage();
    lcd.print(' ');
    lcd.print(units);
    lcd.print("F      ");
  }
  else {
    lcd.print("?           ");

    lcd.setCursor(6, 1);
    lcd.print("?         ");

    // clear the measurements array
    for (measCount = 0; measCount < 10; measCount++)
      measurements[measCount] = 0;
    measCount = 0;
  }

  dischargeCapacitor();

  delay(500);
}
