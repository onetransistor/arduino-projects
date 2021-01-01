/*********************************************************************

    Read NAND flash data with Arduino Pro Mini
    Schematic and more information at:
    https://www.onetransistor.eu/2020/11/arduino-parallel-8x-nand-flash.html
    https://www.onetransistor.eu/2020/12/read-nand-device-signature-arduino.html
    https://www.onetransistor.eu/2021/01/dump-data-from-nand-flash-with-arduino.html

*********************************************************************/

#define NAND_RB 2 // Read busy
#define NAND_R  3 // Read Enable
#define NAND_E  4 // Chip Enable
#define NAND_CL 5 // Command latch
#define NAND_AL 6 // Address latch
#define NAND_W  7 // Write

bool ss; // spare select
int16_t bs; // block index

void portModeOutput() {
  DDRC |= 0x3F;
  DDRB |= 0x03;
}

void portModeInput() {
  DDRC &= ~0x3F;
  DDRB &= ~0x03;
}

void portWrite(uint8_t by) {
  PORTC = (PORTC & 0xC0) | (by & 0x3F);
  PORTB = (PORTB & 0xFC) | (by >> 6);
}

uint8_t portRead() {
  return (PINC & 0x3F) | (((PINB & 0x03) << 6));
}

void nandIdleBus() {
  digitalWrite(NAND_R, HIGH);
  digitalWrite(NAND_W, HIGH);
  digitalWrite(NAND_CL, LOW);
  digitalWrite(NAND_AL, LOW);
  digitalWrite(NAND_E, HIGH);
}

void nandEnable(bool e) {
  digitalWrite(NAND_E, !e);
}

void nandSendCommand(uint8_t cmd, bool last = true) {
  digitalWrite(NAND_CL, HIGH);
  digitalWrite(NAND_W, LOW);
  portWrite(cmd);
  digitalWrite(NAND_W, HIGH); // this triggers data capture
  if (last) digitalWrite(NAND_CL, LOW);
}

void nandSendAddress(uint8_t column, uint8_t page, uint16_t block) {
  digitalWrite(NAND_AL, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite(column);
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite((page & 0x1F) | ((block & 0x07) << 5));
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite((block >> 3) & 0xFF);
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite((block >> 11) & 0x03);
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_AL, LOW);
}

void nandReadSignature() {
  uint8_t sm = 0x00, sd = 0x00;

  portModeOutput();
  nandSendCommand(0x90);
  digitalWrite(NAND_AL, HIGH);
  digitalWrite(NAND_W, LOW);
  portWrite(0x00); // address 0x00
  digitalWrite(NAND_W, HIGH); // this triggers data capture
  digitalWrite(NAND_AL, LOW);

  portModeInput();
  digitalWrite(NAND_R, LOW);
  sm = portRead();
  digitalWrite(NAND_R, HIGH);
  digitalWrite(NAND_R, LOW);
  sd = portRead();
  digitalWrite(NAND_R, HIGH);

  Serial.print("NAND manufacturer: ");
  Serial.println(sm, HEX);
  Serial.print("NAND device ID:    ");
  Serial.println(sd, HEX);
}

void nandReadStatus() {
  uint8_t st = 0x00;

  portModeOutput();
  nandSendCommand(0x70);

  portModeInput();
  digitalWrite(NAND_R, LOW);
  st = portRead();
  digitalWrite(NAND_R, HIGH);

  Serial.print("NAND status:       ");
  Serial.println(st, HEX);
}

void nandReadPageArea(uint16_t areaSize = 256) {
  uint8_t d[areaSize];
  uint8_t cd = 0;

  for (uint16_t i = 0; i < areaSize; i++) {
    digitalWrite(NAND_R, LOW);
    d[i] = portRead();
    digitalWrite(NAND_R, HIGH);
  }

  for (uint16_t i = 0; i < areaSize; i++) {
    if (i % 16 == 0) Serial.println();

    if (d[i] < 0x10) Serial.print('0');
    Serial.print(d[i], HEX);
    Serial.print(' ');
  }
}

void nandReadBlock(uint16_t block) {
  for (uint8_t i = 0; i < 32; i++) {
    portModeOutput();
    nandSendCommand(0x00);
    nandSendAddress(0x00, i, block);
    portModeInput();

    while (digitalRead(NAND_RB) == LOW) ;
    nandReadPageArea();

    portModeOutput();
    nandSendCommand(0x01);
    nandSendAddress(0x00, i, block);
    portModeInput();

    while (digitalRead(NAND_RB) == LOW) ;
    nandReadPageArea();
  }
}

void nandReadBlockSpare(uint16_t block) {
  for (uint8_t i = 0; i < 32; i++) {
    portModeOutput();
    nandSendCommand(0x50);
    nandSendAddress(0x00, i, block);
    portModeInput();

    while (digitalRead(NAND_RB) == LOW) ;
    nandReadPageArea(16);
  }
}

void printBlock(uint16_t block, bool spare = false) {
  Serial.print("NAND Block ");
  Serial.print(block, DEC);
  Serial.println(" Dump:");

  unsigned long total_t = 0;
  unsigned long start_t = millis();

  // first block = 0x0000 (0)
  // last block  = 0x0FFF (4095)
  if (spare) nandReadBlockSpare(block);
  else nandReadBlock(block);

  total_t = millis() - start_t;

  Serial.println();
  Serial.println();
  Serial.print("Read time: ");
  Serial.print(total_t);
  Serial.println(" milliseconds");

  Serial.print("Speed: ");
  Serial.print(32 * 512 * 1000.0 / total_t / 1024, 2);
  Serial.println(" kilobytes/second");
}

void setup() {
  Serial.begin(57600);

  pinMode(NAND_RB, INPUT_PULLUP);
  pinMode(NAND_R, OUTPUT);
  pinMode(NAND_E, OUTPUT);
  pinMode(NAND_CL, OUTPUT);
  pinMode(NAND_AL, OUTPUT);
  pinMode(NAND_W, OUTPUT);

  nandIdleBus();

  Serial.println("NAND Test (https://www.onetransistor.eu)");
  nandEnable(true);
  nandReadSignature();
  nandReadStatus();

  Serial.println("Please set \"No line ending\" in Serial Monitor");
  Serial.println("Send m/s for main/square memory area.");
  Serial.println("Send block address to read [0..4095].");
  delay(10);
}

void loop() {
  if (Serial.available() > 0) {
    if (Serial.peek() == 'm') {
      ss = false;
      while (Serial.read() != -1); // to clear buffer
      Serial.println("Set read mode: main area.");
    } else if (Serial.peek() == 's') {
      ss = true;
      while (Serial.read() != -1); // to clear buffer
      Serial.println("Set read mode: spare area.");
    } else {
      bs = Serial.parseInt();
      if ((bs < 0) || (bs >= 4096)) {
        Serial.println("Valid blocks are: 0..4095.");
      } else {
        printBlock(bs, ss);
      }
    }
  }
}
