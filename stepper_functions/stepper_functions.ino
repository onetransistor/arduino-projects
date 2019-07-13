/*
 * Functions to generate driving pulses for stepper motors
 * All modes supported: wave drive, half drive, full drive
 * 
 * https://www.onetransistor.eu/2017/12/arduino-code-stepper-motors.html
 */

const uint8_t wavePulse[4] = { B00000001, B00000010, B00000100, B00001000 };
const uint8_t halfPulse[8] = { B00000001, B00000011, B00000010, B00000110,
                               B00000100, B00001100, B00001000, B00001001
                             };
const uint8_t fullPulse[4] = { B00000011, B00000110, B00001100, B00001001 };

uint8_t stepIndex = 0;

void pulseWrite(const uint8_t currentPulse) {
  uint8_t port = PORTB;
  port &= 0xF0; // clear bits 3:0, keep bits 7:4
  port |= currentPulse;
  PORTB = port;
}

void waveDrive(uint16_t numSteps, uint16_t stepDelayMs = 5, bool backwards = false) {
  uint8_t sequence = 0;
  numSteps += stepIndex;
  for (uint16_t i = stepIndex; i < numSteps; i++) {
    backwards ? sequence = wavePulse[3 - (i & 3)] : sequence = wavePulse[i & 3];
    pulseWrite(sequence);
    delay(stepDelayMs);
  }
  stepIndex = numSteps & 3;
}

void halfDrive(uint16_t numSteps, uint16_t stepDelayMs = 5, bool backwards = false) {
  uint8_t sequence = 0;
  numSteps += numSteps + stepIndex * 2;
  for (uint16_t i = (stepIndex * 2); i < numSteps; i++) {
    backwards ? sequence = halfPulse[7 - (i & 7)] : sequence = halfPulse[i & 7];
    pulseWrite(sequence);
    delay(stepDelayMs);
  }
  stepIndex = (numSteps / 2) & 3;
}

void fullDrive(uint16_t numSteps, uint16_t stepDelayMs = 5, bool backwards = false) {
  uint8_t sequence = 0;
  numSteps += stepIndex;
  for (uint16_t i = stepIndex; i < numSteps; i++) {
    backwards ? sequence = fullPulse[3 - (i & 3)] : sequence = fullPulse[i & 3];
    pulseWrite(sequence);
    delay(stepDelayMs);
  }
  stepIndex = numSteps & 3;
}

void setup() {
  DDRB |= 0x0F; // set pins D8-D11 as outputs
}

void loop() {
  waveDrive(200, 5, false);
  delay(1000);
  waveDrive(200, 5, true);
  delay(1000);
}
