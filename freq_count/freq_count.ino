// Arduino frequency counter from a few Hz up to 6 MHz
// One Transistor, 2018
// https://www.onetransistor.eu/
// 
// Based on:
//  * Frequency Counter sketch by Nick Gammon (CC BY 3.0 AU) 
//    http://www.gammon.com.au/timers
//  * FreqCounter library by Martin Nawrath (LGPL 2.1)
//    http://interface.khm.de/index.php/lab/interfaces-advanced/arduino-frequency-counter-library/

// set sampling period here (in milliseconds):
unsigned int samplingPeriod = 200;

// Timer 1 overflows counter
volatile unsigned long overflow1;

void init_Timer1() {
  overflow1 = 0; // reset overflow counter

  // Set control registers (see datasheet)
  TCCR1A = 0; // normal mode of operation
  TCCR1B = bit(CS12) | bit(CS11) | bit(CS10); // use external clock source

  TCNT1 = 0; // set current timer value to 0

  TIMSK1 = bit(TOIE1); // enable interrupt on overflow
}

ISR(TIMER1_OVF_vect) {
  overflow1++; // increment overflow counter
}

// Timer 2 overflows counter
volatile unsigned int overflow2;

void init_Timer2() {
  overflow2 = 0; // reset overflow counter

  GTCCR = bit(PSRASY); // reset prescalers

  // Set control registers (see datasheet)
  TCCR2A = bit(WGM21); // CTC mode
  TCCR2B = bit(CS22) | bit(CS20); // prescaler set to 1/128, "ticks" at 125 kHz
  OCR2A = 124; // counts from 0 to 124, then fire interrupt and reset;

  TCNT2 = 0; // set current timer value to 0

  TIMSK2 = bit(OCIE2A); // enable interrupt
}

// interrupt happens at each 125 counts / 125 kHz = 0.001 seconds = 1 ms
ISR(TIMER2_COMPA_vect) {
  if (++overflow2 < samplingPeriod) // add an overflow and check if it's ready
    return; // still sampling

  unsigned long totalSamples = (overflow1 << 16) + TCNT1;  
    
  float freqHz = totalSamples * 1000.0 / samplingPeriod;

  Serial.print("Frequency: ");
  Serial.print((unsigned long)freqHz);
  Serial.println("Hz");

  // reset timers
  TCNT1 = 0; overflow1 = 0;
  TCNT2 = 0; overflow2 = 0;
}

void setup() {
  // enable serial output
  Serial.begin(115200);
  Serial.println("Arduino Frequency Counter");
  Serial.println();

  // Disable Timer0; millis() will no longer work
  TCCR0A = 0; TCCR0B = 0;

  // start timer 1 (count frequency)
  init_Timer1();
  init_Timer2();
}

void loop() {
  // nothing here; interrupts perform everything
  // you can add user input that changes sampling period
}
