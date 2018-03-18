// http://www.toughdev.com/content/2013/11/experimenting-with-st7920-128x64-graphical-lcd-on-a-pic/
// https://github.com/zhongxu/avr.ST7920

#include <SPI.h>

#define LCD_RST 8

#define LCD_DATA                1       // Data bit
#define LCD_COMMAND             0       // Command bit
#define LCD_CLEAR_SCREEN        0x01    // Clear screen
#define LCD_ADDRESS_RESET       0x02    // The address counter is reset
#define LCD_BASIC_FUNCTION      0x30    // Basic instruction set
#define LCD_EXTEND_FUNCTION     0x34    // Extended instruction set

// Write a byte to ST7920 in SPI mode
void ST7920_Write(boolean command, byte lcdData) {
  SPI.beginTransaction(SPISettings(200000UL, MSBFIRST, SPI_MODE3));
  SPI.transfer(command ? 0xFA : 0xF8);
  SPI.transfer(lcdData & 0xF0);
  SPI.transfer((lcdData << 4) & 0xF0);
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

void setup() {
  pinMode(LCD_RST, OUTPUT);
  SPI.begin();

  ST7920_Init();
  ST7920_Write(LCD_COMMAND, 0x88);
  const char *text = "ST7920 Graphic LCD";
  for (int i = 0; i < 18; i++)
    ST7920_Write(LCD_DATA, *text++);

}

void loop() {

}

