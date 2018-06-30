// Basic code for LCD-Keypad v1.1 shield for Arduino
// Based on: https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009)
// Detailed instuctions [in Romanian language]: https://ro.onetransistor.eu/2018/06/shield-lcd-butoane-arduino.html

#include <LiquidCrystal.h>

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

unsigned long keyTime = 0;

void turnOn() {
  pinMode(10, INPUT);
}

void turnOff() {
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
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

void keypadAction() {
  unsigned long currentTime = millis();
  if (currentTime - keyTime < 200) return; // less than 200 ms passed; quit

  byte key = readKeypad();
  delay(100);
  keyTime = currentTime; // store last call time

  if (key != readKeypad()) return; // key was not pressed for sufficient time

  // perform actions

  if (key == btnNONE) return; // no key is pressed; quit
  lcd.setCursor(0, 1);
  switch (key)
  {
    case btnRIGHT:
      {
        lcd.print("RIGHT ");
        break;
      }
    case btnLEFT:
      {
        lcd.print("LEFT   ");
        break;
      }
    case btnUP:
      {
        lcd.print("UP    ");
        turnOn();
        break;
      }
    case btnDOWN:
      {
        lcd.print("DOWN  ");
        break;
      }
    case btnSELECT:
      {
        lcd.print("SELECT");
        break;
      }
  }
}

void setup() {
  lcd.begin(16, 2);
  lcd.print("Keypad Shield");
}

void loop() {
  keypadAction(); // place user code in the switch() statement of this function (above)
}

