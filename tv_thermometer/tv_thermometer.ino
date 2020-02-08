#include <TVout.h>
#include <utility/fontALL.h>

TVout TV;
float minTemp = 100, maxTemp = 0;
byte graph[117] = { 0 };

// Read thermistor and convert to Celsius degrees
// https://tkkrlab.nl/wiki/Arduino_KY-013_Temperature_sensor_module
double readThermistor(int RawADC) {
  double Temp;
  Temp = log(10000.0 * ((1024.0 / RawADC - 1)));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp )) * Temp );
  Temp = Temp - 273.15;            // Convert Kelvin to Celcius
  //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
  return Temp;
}

void draw_initial_screen() {
  TV.clear_screen();

  TV.set_cursor(19, 3);
  TV.select_font(font8x8);
  TV.print("Thermometer");

  TV.draw_rect(0, 0, 125, 14, 1, 2);

  TV.select_font(font6x8);
  TV.set_cursor(0, 20);
  TV.print("MIN");

  TV.set_cursor(43, 20);
  TV.print("Current");

  TV.set_cursor(109, 20);
  TV.print("MAX");

  TV.select_font(font4x6);
  TV.set_cursor(4, 86);
  TV.print("0");
  TV.set_cursor(0, 76);
  TV.print("10");
  TV.set_cursor(0, 66);
  TV.print("20");
  TV.set_cursor(0, 56);
  TV.print("30");
  TV.set_cursor(0, 46);
  TV.print("40");

  TV.draw_line(10, 46, 10, 88, 1);
  TV.draw_line(10, 88, 126, 88, 1);
  TV.set_pixel(9, 48, 1);
  TV.set_pixel(11, 48, 1);
  TV.set_pixel(9, 58, 1);
  TV.set_pixel(11, 58, 1);
  TV.set_pixel(9, 68, 1);
  TV.set_pixel(11, 68, 1);
  TV.set_pixel(9, 78, 1);
  TV.set_pixel(11, 78, 1);

  TV.set_cursor(67, 90);
  TV.select_font(font4x6);
  TV.print("2 min evolution");
}

void display_min(float mt) {
  TV.set_cursor(0, 33);
  TV.select_font(font6x8);
  if (mt < 10) TV.print(" ");
  TV.print(mt, 1);
}

void display_max(float mt) {
  TV.set_cursor(103, 33);
  TV.select_font(font6x8);
  if (mt < 10) TV.print(" ");
  TV.print(mt, 1);
}

void display_current(float t) {
  TV.set_cursor(42, 32);
  TV.select_font(font8x8);
  TV.print(t, 2);
  TV.draw_rect(40, 30, 44, 12, 1, -1);
}

void setup()  {
  TV.begin(PAL, 128, 96);
  draw_initial_screen();
  delay(1000);
}

void display_graph(float ct) {
  for (int i = 0; i < 116; i++) {
    graph[i] = graph[i + 1];
    TV.set_pixel(i + 10, 88 - graph[i], 1);
    if (graph[i + 1] > 0)
      TV.set_pixel(i + 11, 88 - graph[i + 1], 0);
  }
  graph[116] = (byte)ct;
  if (graph[116] > 40) graph[116] = 40;
}

void loop() {
  float currTemp = readThermistor(analogRead(A2));

  display_current(currTemp);
  display_graph(currTemp);

  if (currTemp < minTemp) {
    minTemp = currTemp;
    display_min(minTemp);
    TV.tone(1000, 200);
  }

  if (currTemp > maxTemp) {
    maxTemp = currTemp;
    display_max(maxTemp);
    TV.tone(1000, 200);
  }

  delay(1000);
}
