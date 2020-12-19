#define NAND_RB 2 // Read busy
#define NAND_R  3 // Read Enable
#define NAND_E  4 // Chip Enable
#define NAND_CL 5 // Command latch
#define NAND_AL 6 // Address latch
#define NAND_W  7 // Write

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

void setup() {
  Serial.begin(115200);
  Serial.println("NAND Test");

  pinMode(NAND_RB, INPUT_PULLUP);
  pinMode(NAND_R, OUTPUT);
  pinMode(NAND_E, OUTPUT);
  pinMode(NAND_CL, OUTPUT);
  pinMode(NAND_AL, OUTPUT);
  pinMode(NAND_W, OUTPUT);

  nandIdleBus();

  nandEnable(true);
  nandReadSignature();
  nandReadStatus();
  nandEnable(false);
}

void loop() {
  
}
