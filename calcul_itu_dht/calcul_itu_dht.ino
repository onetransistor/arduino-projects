/*
 * Calculare indice temperatura umezeala cu senzor DHT22
 * https://ro.onetransistor.eu/2017/12/indice-confort-termic-dht11-arduino.html
 * Formula de calcul: http://pesd.ro/articole/nr.1/Teodoreanu.pdf
 */

#include <LiquidCrystal.h>
#include <DHT.h>

#define LED_ALBASTRU  11
#define LED_GALBEN    12
#define LED_ROSU      13

#define PIN_SENZOR     2

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // LCD conectat conform schemei shield-ului LCD
DHT senzor(PIN_SENZOR, DHT22);
float t, h;
int itu;

void setup() {
  //pinMode(10, INPUT); // necesar doar daca folosesti shield-ul cu LCD si butoane

  // configurare LED-uri
  pinMode(LED_ALBASTRU, OUTPUT);
  pinMode(LED_GALBEN, OUTPUT);
  pinMode(LED_ROSU, OUTPUT);

  digitalWrite(LED_ALBASTRU, LOW);
  digitalWrite(LED_GALBEN, LOW);
  digitalWrite(LED_ROSU, LOW);

  // configurare senzor
  senzor.begin();

  // configurare LCD
  lcd.begin(16, 2); // initializeaza LCD-ul cu 2 randuri de 16 caractere

  lcd.setCursor(0, 0);
  lcd.print(" Calculator ITU ");
  lcd.setCursor(0, 1);
  lcd.print("  Senzor DHT22  ");
  delay(2000);
  lcd.clear();
}

void loop() {
  t = senzor.readTemperature();
  h = senzor.readHumidity();
  float i = (t * 1.8 + 32) - (0.55 - 0.0055 * h) * ((t * 1.8 + 32) - 58);
  itu = int(i);

  // indicare prin LED-uri
  if (itu <= 65) {
    // confort, aprinde LED albastru
    digitalWrite(LED_ALBASTRU, HIGH);
    digitalWrite(LED_GALBEN, LOW);
    digitalWrite(LED_ROSU, LOW);
  }
  else if (itu <= 79) {
    // alerta, aprinde LED galben
    digitalWrite(LED_ALBASTRU, LOW);
    digitalWrite(LED_GALBEN, HIGH);
    digitalWrite(LED_ROSU, LOW);
  }
  else {
    // prag critic depasit, aprinde LED rosu
    digitalWrite(LED_ALBASTRU, LOW);
    digitalWrite(LED_GALBEN, LOW);
    digitalWrite(LED_ROSU, HIGH);
  }

  // afisare pe LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ITU = ");
  lcd.print(itu, DEC);
  lcd.print(F(" unitati"));
  lcd.setCursor(0, 1);
  lcd.print(t, 1);
  lcd.print(char(223));
  lcd.print('C');
  lcd.setCursor(13, 1);
  lcd.print(int(h), DEC);
  lcd.print('%');
 
  delay(2500);
}
