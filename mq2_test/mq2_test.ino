#include <DHT.h>

const float VCC = 5.0;                     // supply voltage and ADC reference
const float RL = 4700.0;                   // load resistor value
const float Ro_Winsen = 16595.6;           // presumed sensor resistance in clean air, at 20C and 55%RH
const float Ro_Hanwei = Ro_Winsen / 9.58;  // presumed sensor resistance in 1000 ppm H2, at 20C and 65%RH

int VRLadc = 0;   // ADC value (0..1023)
float VRL = 0.0;  // computed voltage at ADC input
float Rs = 0.0;   // sensor resistance
float t, h;

DHT dht(2, DHT22);

float correctionFactorTH_Hanwei(float temp, float hum) {
  float scale = -0.00699088200923904 * hum + 4.01118163062014;
  float slope = 0.00000984021398498172 * hum - 0.303183822378689;

  // don't forget to convert temperature to Fahrenheit
  return scale * pow((1.8 * temp + 32), slope);
}

int alcoholPPM_Hanwei(float Rs_real, float Ro_standard, float temp, float hum) {
  const float slope = -0.372095603150543;
  const float scale = 20.9733672651754;
  int ppm;

  Rs_real = Rs_real / correctionFactorTH_Hanwei(temp, hum);
  ppm = (int)pow(Rs_real / (Ro_standard * scale), (1 / slope));
  if (ppm < 100) ppm = 0; // lowest detectable alcohol concentration is 100 ppm
  return ppm;
}

float correctionFactorTH_Winsen(float temp, float hum) {
  const float aaH = 0.0000669005032586238;
  const float baH = -0.0159747107540879;
  const float abH = -0.00748672213648967;
  const float bbH = 1.78146466558454;

  return (aaH * hum * temp) + (baH * temp) + (abH * hum) + bbH;
}

int alcoholPPM_Winsen(float Rs_real, float Ro_standard, float temp, float hum) {
  const float slope = -0.552464627410705;
  const float scale = 10.2889924963885;
  int ppm;

  Rs_real = Rs_real / correctionFactorTH_Winsen(temp, hum);
  ppm = (int)pow(Rs_real / (Ro_standard * scale), (1 / slope));
  if (ppm < 300) ppm = 0; // lowest detectable gas concentration is 300 ppm
  return ppm;
}

void setup() {
  analogReference(EXTERNAL);
  pinMode(A0, INPUT);
  dht.begin();
  Serial.begin(115200);
  delay(5000);
}

void loop() {
  VRLadc = analogRead(A0);
  VRL = VRLadc * VCC / 1023.0;
  Serial.print("VRL:");
  Serial.print(VRL, 2);

  Rs = (VCC - VRL) * RL / VRL;
  Serial.print(" Rs:");
  Serial.print(Rs, 1);

  t = dht.readTemperature();
  h = dht.readHumidity();
  Serial.print(" T:");
  Serial.print(t, 1);
  Serial.print(" H:");
  Serial.print(h, 0);

  Serial.print(" ppm_W:");
  Serial.print(alcoholPPM_Winsen(Rs, Ro_Winsen, t, h));

  Serial.print(" ppm_H:");
  Serial.print(alcoholPPM_Hanwei(Rs, Ro_Hanwei, t, h));
  Serial.println();

  delay(1000);
}
