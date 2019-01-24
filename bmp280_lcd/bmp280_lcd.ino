/**
 * BMP280 sensor with ST7920 LCD display. More information at:
 * 
 * https://www.onetransistor.eu/2019/01/pressure-temperature-altitude-bmp280.html
 * https://ro.onetransistor.eu/2019/01/presiune-atmosferica-senzor-bmp280.html
 * 
 * Using BMx280MI library by Gregor Christandl
 **/
  
#include <Wire.h>
#include <BMx280MI.h>
#include <SPI.h>

#define LCD_RST 8
#define LCD_DATA                1       // Data bit
#define LCD_COMMAND             0       // Command bit
#define LCD_CLEAR_SCREEN        0x01    // Clear screen
#define LCD_ADDRESS_RESET       0x02    // The address counter is reset
#define LCD_BASIC_FUNCTION      0x30    // Basic instruction set
#define LCD_EXTEND_FUNCTION     0x34    // Extended instruction set

#define I2C_ADDRESS 0x76
#define SEALEVEL 1013.25

BMx280I2C bmp280(I2C_ADDRESS);

float pressure = 0, temperature = 0, altitude = 0;

// generated with http://dotmatrixtool.com, 16px by 16px, row major, big endian
uint8_t presIcon[32] = { 0x10, 0x40, 0x20, 0x8f, 0x41, 0x01, 0x41, 0x01, 0x41, 0x07, 0x41, 0x01, 0x20, 0x81, 0x10, 0x4f, 0x10, 0x41, 0x10, 0x41, 0x10, 0x47, 0xa2, 0x81, 0xc3, 0x01, 0xe3, 0x8f, 0x00, 0x00, 0x00, 0x00 };
uint8_t tempIcon[32] = { 0x01, 0x80, 0x02, 0x40, 0x02, 0x70, 0x02, 0x40, 0x02, 0x70, 0x02, 0x40, 0x02, 0xf0, 0x02, 0xc0, 0x04, 0xe0, 0x09, 0xf0, 0x0b, 0xf0, 0x0b, 0xf0, 0x09, 0xf0, 0x04, 0xe0, 0x03, 0xc0, 0x00, 0x00 };
uint8_t alttIcon[32] = { 0x00, 0x00, 0x00, 0x04, 0x00, 0x0e, 0x00, 0x1f, 0x00, 0x04, 0x08, 0x04, 0x18, 0x04, 0x14, 0x1f, 0x16, 0x0e, 0x23, 0x04, 0x21, 0x80, 0x7b, 0xc0, 0x4c, 0x60, 0x40, 0x30, 0xc0, 0x18, 0xff, 0xfc };


void ST7920_Write(boolean command, uint8_t lcdData) {
  SPI.beginTransaction(SPISettings(200000UL, MSBFIRST, SPI_MODE3));
  digitalWrite(10, HIGH);
  SPI.transfer(command ? 0xFA : 0xF8);
  SPI.transfer(lcdData & 0xF0);
  SPI.transfer((lcdData << 4) & 0xF0);
  digitalWrite(10, LOW);
  SPI.endTransaction();
}

void ST7920_Init() {
  digitalWrite(LCD_RST, LOW);
  delay(100);
  digitalWrite(LCD_RST, HIGH);

  ST7920_Write(LCD_COMMAND, LCD_BASIC_FUNCTION);
  ST7920_Write(LCD_COMMAND, LCD_CLEAR_SCREEN);
  ST7920_Write(LCD_COMMAND, 0x06);
  ST7920_Write(LCD_COMMAND, 0x0C);
}

void ST7920_setGraphicMode(boolean enabled = true) {
  ST7920_Write(LCD_COMMAND, enabled ? LCD_EXTEND_FUNCTION : LCD_BASIC_FUNCTION);
  if (enabled) ST7920_Write(LCD_COMMAND, LCD_EXTEND_FUNCTION | 0x02); // Graphic ON
}

void ST7920_displayString(uint8_t row, const char *p, uint8_t offset = 0) {
  uint8_t addr;
  switch (row) {
    case 1: addr = 0x90; break;
    case 2: addr = 0x88; break;
    case 3: addr = 0x98; break;
    default: addr = 0x80;
  }

  addr += offset;

  ST7920_Write(LCD_COMMAND, addr);
  uint8_t length = strlen(p);
  for (uint8_t i = 0; i < length; i++)
    ST7920_Write(LCD_DATA, *p++);
}

void ST7920_ClearGraphicMem() {
  for (uint8_t x = 0; x < 16; x++)
    for (uint8_t y = 0; y < 32; y++) {
      ST7920_Write(LCD_COMMAND, 0x80 | y);
      ST7920_Write(LCD_COMMAND, 0x80 | x);
      ST7920_Write(LCD_DATA, 0x00);
      ST7920_Write(LCD_DATA, 0x00);
    }
}

void ST7920_displayIcon16(uint8_t row, uint8_t *icon) {
  uint8_t y_address = 0x80;
  uint8_t x_address = 0x80;
  if (row >= 2) x_address = 0x88;
  if (row % 2 == 1) y_address += 16;

  for (uint8_t i = 0; i < 16; i++) {
    ST7920_Write(LCD_COMMAND, y_address + i);
    ST7920_Write(LCD_COMMAND, x_address);
    ST7920_Write(LCD_DATA, icon[i * 2]);
    ST7920_Write(LCD_DATA, icon[i * 2 + 1]);
  }
}

void setup() {
  // display init
  pinMode(LCD_RST, OUTPUT);
  SPI.begin();
  pinMode(10, OUTPUT); // CS pin
  digitalWrite(10, LOW); // inactive
  delay(100);
  ST7920_Init();

  ST7920_setGraphicMode(true);
  ST7920_ClearGraphicMem();

  // highlight title
  for (uint8_t y = 0; y < 15; y++)
    for (uint8_t x = 0; x < 8; x++) {
      ST7920_Write(LCD_COMMAND, 0x80 | y);
      ST7920_Write(LCD_COMMAND, 0x80 | x);
      ST7920_Write(LCD_DATA, 0xFF);
      ST7920_Write(LCD_DATA, 0xFF);
    }

  ST7920_displayIcon16(1, presIcon);
  ST7920_displayIcon16(2, tempIcon);
  ST7920_displayIcon16(3, alttIcon);

  ST7920_setGraphicMode(false);
  ST7920_displayString(0, "     BMP280     ");

  // sensor
  Wire.begin();
  delay(200);

  if (!bmp280.begin()) {
    ST7920_displayString(2, "     Error!     ");
    while (1);
  }

  bmp280.resetToDefaults();
  bmp280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
  bmp280.writeOversamplingTemperature(BMx280MI::OSRS_T_x02);
  bmp280.writeFilterSetting(BMx280MI::FILTER_x16);
  bmp280.writePowerMode(BMx280MI::BMx280_MODE_NORMAL);
  bmp280.writeStandbyTime(BMx280MI::T_SB_5);
}

void loop() {
  String ps, ts, as;
  delay(1000);

  if (!bmp280.measure()) return;

  do {
    delay(100);
  } while (!bmp280.hasValue());

  pressure = bmp280.getPressure();
  temperature = bmp280.getTemperature();

  // barometric formula in BMP180 datasheet
  altitude = 44330 * (1.0 - pow((pressure / 100) / SEALEVEL, 0.1903));

  ps = String(' ') + String(pressure * 0.00750061683, 1) + " mmHg";
  ts = String(' ') + String(temperature, 2) + " deg.C";
  as = String(' ') + String(altitude, 1) + " m est.";

  ST7920_displayString(1, ps.c_str(), 1);
  ST7920_displayString(2, ts.c_str(), 1);
  ST7920_displayString(3, as.c_str(), 1);
}
