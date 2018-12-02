/*   Multiplexed 4 digit 7-segment display driver
 *   
 *   A potentiometer is used to adjust digit update interval
 *   from 1 to 250 milliseconds
 *   
 *   More information:
 *   https://www.onetransistor.eu/2018/12/drive-multiplexed-7segment-display.html
 *   https://www.onetransistor.eu/2018/12/wiring-of-4digit-7segment-display.html
 *   https://youtu.be/kiNet0rdaZU
 */


// invert these definitions for common cathode display
#define LED_ON    LOW
#define LED_OFF   HIGH

// adjust this if you wire display in a different way
#define SEG_A 2
#define SEG_B 3
#define SEG_C 4
#define SEG_D 5
#define SEG_E 6
#define SEG_F 7
#define SEG_G 8
#define SEG_P 9

#define DIG_1 10
#define DIG_2 11
#define DIG_3 12
#define DIG_4 13

char c_digit;
uint16_t number = 1240;
byte base = 10;

unsigned long lastRefresh = 0;
unsigned long numberIncTime = 0;
unsigned int interval;

void displayDigit(uint16_t digit, bool dp = false) {
  digit %= base;

  switch (digit) {
    case 0:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_OFF);
      break;
    case 1:
      digitalWrite(SEG_A, LED_OFF);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_OFF);
      digitalWrite(SEG_E, LED_OFF);
      digitalWrite(SEG_F, LED_OFF);
      digitalWrite(SEG_G, LED_OFF);
      break;
    case 2:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_OFF);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_OFF);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 3:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_OFF);
      digitalWrite(SEG_F, LED_OFF);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 4:
      digitalWrite(SEG_A, LED_OFF);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_OFF);
      digitalWrite(SEG_E, LED_OFF);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 5:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_OFF);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_OFF);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 6:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_OFF);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 7:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_OFF);
      digitalWrite(SEG_E, LED_OFF);
      digitalWrite(SEG_F, LED_OFF);
      digitalWrite(SEG_G, LED_OFF);
      break;
    case 8:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 9:
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_OFF);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 10: // hex digit A
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_OFF);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 11: // hex digit B
      digitalWrite(SEG_A, LED_OFF);
      digitalWrite(SEG_B, LED_OFF);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 12: // hex digit C
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_OFF);
      digitalWrite(SEG_C, LED_OFF);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_OFF);
      break;
    case 13: // hex digit D
      digitalWrite(SEG_A, LED_OFF);
      digitalWrite(SEG_B, LED_ON);
      digitalWrite(SEG_C, LED_ON);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_OFF);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 14: // hex digit E
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_OFF);
      digitalWrite(SEG_C, LED_OFF);
      digitalWrite(SEG_D, LED_ON);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
    case 15: // hex digit F
      digitalWrite(SEG_A, LED_ON);
      digitalWrite(SEG_B, LED_OFF);
      digitalWrite(SEG_C, LED_OFF);
      digitalWrite(SEG_D, LED_OFF);
      digitalWrite(SEG_E, LED_ON);
      digitalWrite(SEG_F, LED_ON);
      digitalWrite(SEG_G, LED_ON);
      break;
  }

  dp ? digitalWrite(SEG_P, LED_ON) : digitalWrite(SEG_P, LED_OFF);
}

void refreshDisplay() {
  switch (c_digit) {
    case 3:
      digitalWrite(DIG_1, LED_OFF);
      digitalWrite(DIG_2, LED_OFF);
      digitalWrite(DIG_3, LED_OFF);
      digitalWrite(DIG_4, LED_ON);
      displayDigit(number);
      break;
    case 2:
      digitalWrite(DIG_1, LED_OFF);
      digitalWrite(DIG_2, LED_OFF);
      digitalWrite(DIG_3, LED_ON);
      digitalWrite(DIG_4, LED_OFF);
      displayDigit(number / base);
      break;
    case 1:
      digitalWrite(DIG_1, LED_OFF);
      digitalWrite(DIG_2, LED_ON);
      digitalWrite(DIG_3, LED_OFF);
      digitalWrite(DIG_4, LED_OFF);
      displayDigit(number / base / base);
      break;
    case 0:
      digitalWrite(DIG_1, LED_ON);
      digitalWrite(DIG_2, LED_OFF);
      digitalWrite(DIG_3, LED_OFF);
      digitalWrite(DIG_4, LED_OFF);
      displayDigit(number / base / base / base);
      break;
  }

  c_digit++;
  if (c_digit == 4) c_digit = 0;
}

void setup() {
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(SEG_P, OUTPUT);

  pinMode(DIG_1, OUTPUT);
  pinMode(DIG_2, OUTPUT);
  pinMode(DIG_3, OUTPUT);
  pinMode(DIG_4, OUTPUT);

  pinMode(A0, INPUT);

}

void loop() {
  unsigned long current = millis();

  if (current >= lastRefresh + interval) {
    refreshDisplay();
    lastRefresh = current;

    interval = analogRead(A0);
    interval /= 4;
  }

  if (current >= numberIncTime + 1000) {
    number++;
    numberIncTime = current;
  }
}
