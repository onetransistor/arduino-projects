/*   Multiplexed 4 digit 7-segment display driver

     A potentiometer is used to adjust digit update interval
     from 1 to 250 milliseconds

     More information:
     https://www.onetransistor.eu/2018/12/drive-multiplexed-7segment-display.html
     https://www.onetransistor.eu/2018/12/wiring-of-4digit-7segment-display.html
     https://youtu.be/kiNet0rdaZU
*/

// invert these definitions for common cathode display
#define LED_ON    LOW
#define LED_OFF   HIGH

// the pin of segment A
// next segments MUST be wired to pin SEG_A+1, SEG_A+2 and so on
#define SEG_A 2
#define DIG_2 11
#define DIG_3 12
#define DIG_4 13

// the pin of digit 1
#define DIG_1 10

// segment encodings from G to A
// I'm using G to A because bitRead reads bits from rightmost bit
// source: https://en.wikipedia.org/wiki/Seven-segment_display#Displaying_letters
byte digits[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

char c_digit;
uint16_t number = 1240;
byte base = 10;

unsigned long lastRefresh = 0;
unsigned long numberIncTime = 0;
unsigned int interval;

void displayDigit(uint16_t digit, bool dp = false) {
  digit %= base;

  for (byte i = 0; i < 7; i++)
    digitalWrite(SEG_A + i, bitRead(digits[digit], i) ? LED_ON : LED_OFF);

  dp ? digitalWrite(SEG_A + 7, LED_ON) : digitalWrite(SEG_A + 7, LED_OFF);
}

void refreshDisplay() {
  displayDigit(number / pow(base, 3 - c_digit));
  for (byte i = 0; i < 4; i++)
      digitalWrite(DIG_1 + i, (i == c_digit) ? LED_ON : LED_OFF);
      
  c_digit++;
  if (c_digit == 4) c_digit = 0;
}

void setup() {
  for (byte i = 0; i < 8; i++)
    pinMode(SEG_A + i, OUTPUT);

  for (byte i = 0; i < 4; i++)
    pinMode(DIG_1 + i, OUTPUT);

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
