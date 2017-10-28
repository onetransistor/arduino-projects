/*  
 *  Arduino powered light/dark activated switch
 *  Copyright (C) 2017 One Transistor <https://www.onetransistor.eu>
 *  Licensed under GNU General Public License v3
 */  

#include <EEPROM.h>

// Pins for photoresistor, relay and encoder
const int sensorPin = A3; // A3
const int relayPin = D4; // D4
const int encoderA = D2; // D2
const int encoderB = D5; // D5
const int modeSwitchPin = D3; // D3

// Change that will be considered significant in read value
const int valueThreshold = 20;
const int rotaryEncoderStep = 10;

int sensorValue = 0;
volatile unsigned int sensorThreshold = 512;
volatile byte modeSwitch = 0; // 0 = always off, 1 = always on, 2 and 3 = dark or light activated switch

void setup() {
  pinMode(sensorPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  pinMode(encoderA, INPUT);
  pinMode(encoderB, INPUT);
  pinMode(modeSwitchPin, INPUT_PULLUP);

  attachInterrupt(0, readEncoder, CHANGE);
  attachInterrupt(1, readMode, LOW);

  modeSwitch = EEPROM.read(1);
  if (modeSwitch > 3) modeSwitch = 2;

  sensorThreshold = (EEPROM.read(2) << 8) + EEPROM.read(3);
  if (sensorThreshold > 1023) sensorThreshold = 512;

  // uncomment these lines for debugging
  //Serial.begin(9600);
  //Serial.println("Light/dark activated switch");
}

void loop() {
  while (modeSwitch < 2) ;

  // sensor is read twice in 0.5 seconds
  // if there is no significant change in sensor value
  // it is decided whether to activate the relay or not
  int sensorValueInstant1;
  do {
    sensorValueInstant1 = analogRead(sensorPin); // read sensor
    delay(500); // wait 0.5 seconds
    int sensorValueInstant2 = analogRead(sensorPin); // read sensor again

    // if there is no significant change in read values
    // it means there is a constant change of light intensity
    if (max(sensorValueInstant1, sensorValueInstant2) - min(sensorValueInstant1, sensorValueInstant2) < valueThreshold)
      sensorValue = sensorValueInstant1; // store sensor value

    delay(500);
  } while (sensorValue != sensorValueInstant1);

  // uncomment these lines for debugging
  //Serial.print("Photoresistor: ");
  //Serial.print(sensorValue / 10.24, 1);
  //Serial.println(" %");

  // if value is over or below threshold
  // depending on mode setting, turn on or off the relay
  if ((sensorValue < sensorThreshold) && (modeSwitch > 1))
    digitalWrite(relayPin, (modeSwitch == 2) ? HIGH : LOW);
  else
    digitalWrite(relayPin, (modeSwitch == 3) ? HIGH : LOW);
}

// https://playground.arduino.cc/Main/RotaryEncoders
void readEncoder() {
  if (digitalRead(encoderA) == digitalRead(encoderB))
  {
    if (sensorThreshold < 1024) sensorThreshold += rotaryEncoderStep;
  }
  else {
    if (sensorThreshold > 0) sensorThreshold -= rotaryEncoderStep;
  }

  EEPROM.update(2, sensorThreshold >> 8);
  EEPROM.update(3, sensorThreshold & 0xFF);

  // uncomment these lines for debugging
  //Serial.print("New threshold: ");
  //Serial.print(sensorThreshold / 10.24, 1);
  //Serial.println(" %");
}

// "debouncing" procedure from
// http://forum.arduino.cc/index.php?topic=45000.0
void readMode() {
  static unsigned long lastPush = 0;
  unsigned long currentPush = millis();

  if (currentPush - lastPush > 200) {
    modeSwitch++;
    if (modeSwitch > 3)
      modeSwitch = 0;
  }
  lastPush = currentPush;

  if (modeSwitch < 2)
    digitalWrite(relayPin, (modeSwitch == 1) ? HIGH : LOW);

  EEPROM.update(1, modeSwitch);

  // uncomment these lines for debugging
  //Serial.print("Sensor mode: ");
  //Serial.println(modeSwitch, DEC);
}

