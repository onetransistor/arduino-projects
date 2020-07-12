/*
 * DC motor with optical encoder feedback PID controller
 * 
 * https://www.onetransistor.eu/2020/07/dc-motor-pid-control-optical-encoder.html
 */

#include <LiquidCrystal.h>
#include <PID_v1.h>

/* Keypad buttons */
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

/* Pin definitions */
#define encoderA 2
#define encoderB 12

#define motorDirL 3
#define motorDirR 11

/* Strip lines per inch */
#define stripLPI 150.0

/* LCD and Keypad variables */
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
unsigned long keyTime = 0;
unsigned long dispRefresh = 0;
volatile long encoderPos = 0;

/* PID controller */
double kp = 5, ki = 0.1, kd = 0.05;
double initialPos = 0, /* where we are */
       pwmFactor = 0, /* the "amount" of action we need to take (pulse width here) */
       desiredPos = 0; /* where we want to be */
PID motorPID(&initialPos, &pwmFactor, &desiredPos, kp, ki, kd, DIRECT);

/* Position variables */
int positionStep[] = { 5, 50, 150, 300 };
int positionIdx = 0;
int currentPos = 0;


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
  delay(10);
  keyTime = currentTime; // store last call time

  if (key != readKeypad()) return; // key was not pressed for sufficient time

  // perform actions
  if (key == btnNONE) return; // no key is pressed; quit
  switch (key)
  {
    case btnRIGHT:
      {
        currentPos += positionStep[positionIdx];
        setPosition(currentPos);
        break;
      }
    case btnLEFT:
      {
        currentPos -= positionStep[positionIdx];
        setPosition(currentPos);
        break;
      }
    case btnUP:
      {
        if (positionIdx < 3) positionIdx++;
        lcd.setCursor(7, 0);
        lcd.print(positionStep[positionIdx] / 150.0, 2);
        lcd.print(" in");
        break;
      }
    case btnDOWN:
      {
        if (positionIdx > 0) positionIdx--;
        lcd.setCursor(7, 0);
        lcd.print(positionStep[positionIdx] / 150.0, 2);
        lcd.print(" in");
        break;
      }
    case btnSELECT:
      {
        break;
      }
  }
}

void processEncoder() {
  if (PINB & 0x10) encoderPos++;
  else encoderPos--;
}

void moveMotor(int pwm) {
  if (pwm > 0) {
    pwm = pwm + 55;
    analogWrite(motorDirR, pwm);
    digitalWrite(motorDirL, LOW);
  }
  else {
    pwm = abs(pwm) + 55;
    analogWrite(motorDirL, pwm);
    digitalWrite(motorDirR, LOW);
  }
}

void setPosition(int posLine) {
  desiredPos = posLine;
}

void motorAction() {
  initialPos = encoderPos;
  if (desiredPos == initialPos) {
    // do nothing
    digitalWrite(motorDirL, LOW);
    digitalWrite(motorDirR, LOW);
    return;
  }

  motorPID.Compute();
  moveMotor(pwmFactor);
}

void setup() {
  /* Configure encoder input */
  pinMode (encoderA, INPUT);
  pinMode (encoderB, INPUT);
  attachInterrupt(0, processEncoder, FALLING);

  /* Configure motor control pins */
  pinMode (motorDirL, OUTPUT);
  pinMode (motorDirR, OUTPUT);
  digitalWrite(motorDirL, LOW); // disable motor
  digitalWrite(motorDirR, LOW); // disable motor
  TCCR2B = TCCR2B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz

  /* Start LCD display */
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Step <         >");
  lcd.setCursor(7, 0);
  lcd.print(positionStep[positionIdx] / 150.0, 2);
  lcd.print(" in");
  lcd.setCursor(0, 1);
  lcd.print("Real: ");

  /* Configure PID controller */
  motorPID.SetMode(AUTOMATIC);
  motorPID.SetSampleTime(10);
  motorPID.SetOutputLimits(-30, 30);
}

void loop() {
  motorAction(); // compute PID output
  keypadAction();

  unsigned long c = millis();

  if (c > dispRefresh + 1000) {
    float dist_in = encoderPos / stripLPI;

    lcd.setCursor(6, 1);
    lcd.print(dist_in, 2);
    lcd.print(" in    ");

    dispRefresh = c;
  }
}
