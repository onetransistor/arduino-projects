/*
 * Test sketch for the printer sliding unit with
 * DC motor and optical encoder
 * 
 * https://www.onetransistor.eu/2020/07/dc-motor-pid-control-optical-encoder.html
 */

#include <LiquidCrystal.h>

#define encoderA 2
#define encoderB 12

// motor control pins must be PWM
#define motorDirL 3
#define motorDirR 11

/* Strip lines per inch */
#define stripLPI 150.0

// LCD wiring is for the LCD-Keypad shield
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
volatile long encoderPos = 0;
float lastKnownPos = 0;

void processEncoder() {
  // direct port reading is faster than digitalRead(encoderB)
  if (PINB & 0x10) encoderPos++;
  else encoderPos--;
}

void moveMotor(int pwm) {
  if (pwm > 0) {
    analogWrite(motorDirR, pwm);
    digitalWrite(motorDirL, LOW);
  }
  else {
    pwm = abs(pwm);
    analogWrite(motorDirL, pwm);
    digitalWrite(motorDirR, LOW);
  }
}

void testMotorPWM() {
  bool once = true;

  lcd.print("PWM Testing...");
  Serial.println("PWM_Factor Speed_[mm/s]");

  for (int p = 0; p < 256; p++) {
    unsigned long tm_now = millis();
    long enc = encoderPos;

    // move alternatively back and forth
    if (p % 2 == 0) moveMotor(0 - p);
    else moveMotor(p);

    // attempt to move motor 1/3 inches
    while ((millis() < tm_now + 500) && (abs(enc - encoderPos) <= (stripLPI / 3))) ;

    // get both time required to do this and the movement magnitude
    tm_now = millis() - tm_now;
    enc = abs(enc - encoderPos);

    // if significant movement occurred (1/30 inches) display current PWM
    if (once && (enc >= (stripLPI / 30))) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Move start: ");
      lcd.print(p, DEC);
      once = false;
    }

    // compute speed and print it to serial port
    float d_mm = enc / stripLPI * 25.4;
    float speed_mmps = d_mm / tm_now * 1000.0;

    Serial.print(p, DEC); Serial.print(' ');
    Serial.print(speed_mmps, 2); Serial.println();

    // wait for inertial movement
    delay(500);

    // get actual position and stop the test if it moves too fast
    enc = abs(enc - encoderPos);
    // if more than 3 inches movement occurred stop the test
    if (enc > 3 * stripLPI) {
      lcd.setCursor(0, 1);
      lcd.print("Test stop:  ");
      lcd.print(p, DEC);
      Serial.println("Test end");
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(10, INPUT); // for the LCD shield
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("PWM Test");

  // configure optical encoder
  pinMode (encoderA, INPUT);
  pinMode (encoderB, INPUT);
  attachInterrupt(0, processEncoder, FALLING);

  /* Configure motor control pins */
  pinMode (motorDirL, OUTPUT);
  pinMode (motorDirR, OUTPUT);
  digitalWrite(motorDirL, LOW); // disable motor
  digitalWrite(motorDirR, LOW); // disable motor
  TCCR2B = TCCR2B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz

  delay(1000);
  lcd.clear();

  testMotorPWM();

  digitalWrite(motorDirL, LOW);
  digitalWrite(motorDirR, LOW);

  delay(2000);
  lastKnownPos = encoderPos / stripLPI * 25.4;
}

void loop() {
  // check if movement occurred and display distance
  float currentPos = encoderPos / stripLPI * 25.4;
  if (currentPos != lastKnownPos) {
    lcd.setCursor(0, 1);
    lcd.print(currentPos);
    lcd.print(" mm        ");
    lastKnownPos = currentPos;
  }
}
