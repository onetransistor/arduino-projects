/*
   Compute Heat Index with Arduino and DHT22 Sensor
   https://www.onetransistor.eu/2018/01/compute-heat-index-arduino-dht.html
   Formula: http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml included in
   Adafruit DHT Library https://github.com/adafruit/DHT-sensor-library
*/

#include <LiquidCrystal.h>
#include <DHT.h>

#define LED_GREEN   11
#define LED_YELLOW  12
#define LED_RED     13

#define SENSOR_PIN   2

#define CELSIUS_DEGREES

#ifdef CELSIUS_DEGREES
#define T_UNITS false
#define T_CHAR 'C'
#define THR_NORMAL 27
#define THR_CAUTION 32 // below 32C humiture
#define THR_CAUTION_EXTR 41 // below 41C humiture
#else
#define T_UNITS true
#define T_CHAR 'F'
#define THR_NORMAL 80
#define THR_CAUTION 90 // below 90F humiture
#define THR_CAUTION_EXTR 105 // below 105F humiture
#endif

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // LCD-keypad shield configuration
DHT sensor(SENSOR_PIN, DHT22);
float t, h, hi;

void setup() {
  //pinMode(10, INPUT); // only for LCD-keypad shield

  // LED configuration
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);

  // sensor configuration
  sensor.begin();

  // LCD configuration
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("   Heat index   ");
  lcd.setCursor(0, 1);
  lcd.print("  DHT22 Sensor  ");
  delay(100);

  // test the LEDs
  digitalWrite(LED_GREEN, HIGH);
  delay(300);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  delay(300);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  delay(300);
  digitalWrite(LED_RED, LOW);

  delay(1000);
  lcd.clear();
}

void loop() {
  t = sensor.readTemperature(T_UNITS);
  h = sensor.readHumidity();
  hi = sensor.computeHeatIndex(t, h, T_UNITS);

  // LED indication (see https://en.wikipedia.org/wiki/Heat_index#Effects_of_the_heat_index_(shade_values)
  if (hi < THR_NORMAL) {
    // no warning, all LEDs off
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
  }
  else if (hi < THR_CAUTION) {
    // caution, turn on green LED
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
  }
  else if (hi < THR_CAUTION_EXTR) {
    // extreme caution, turn on yellow LED
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_RED, LOW);
  }
  else {
    // (extreme) danger, turn on red LED
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, HIGH);
  }

  // display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humiture: ");
  lcd.print(hi, 1);
  lcd.print(char(223));
  lcd.print(T_CHAR);

  lcd.setCursor(0, 1);
  lcd.print(t, 1);
  lcd.print(char(223));
  lcd.print(T_CHAR);
  lcd.setCursor(13, 1);
  lcd.print(int(h), DEC);
  lcd.print('%');

  delay(2500);
}
