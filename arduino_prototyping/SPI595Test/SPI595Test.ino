// This code runs on Arduino UNO R3

// *****************************************************************************************
// * TMS9118A test code
// *  
// * This code sets up a TMS9118A VDP chip and has it toggle through its background colors 
// * It uses two 8 bit shift registers (74HC595) to represent a data bus (lower 8 bits)
// * and the upper 3 bits of the other shift register represent MODE, CSR*, and CSW*
// * The VDP's RESET* is also tied to the lower bit (A5) of the Arduino through a switching 
// * transistor
// *****************************************************************************************


#include <SPI.h>

const int latchPin = A0;
const int clockPin = 13;
const int dataPin  = 11;
volatile unsigned int j;

void setup() {
  DDRC = B00111111;             // turn all the pins on PORTC (the analog pins) on

  pinMode(latchPin, OUTPUT);    // set latch as output mode
  pinMode(clockPin, OUTPUT);    // set clock as output mode
  pinMode(dataPin, OUTPUT);     // set data as output mode
  digitalWrite(latchPin, HIGH); // turn on the latch
  digitalWrite(clockPin, LOW);  // turn off the clock
  digitalWrite(dataPin, LOW);   // turn off any data


  // set up hardware SPI
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();

  PORTC = B00000000;           // invoke reset circuit
  delay(200);                  // wait for the reset
  PORTC = B00000001;           // toggle reset pin back up
  delay(200);

  // register 0
  writeRegister(0, B00000000);  // graphics mode 1, no external video

  // register 1
  writeRegister(1, B11000000); // 16k, Enable display, Disable interrupts, 8x8 sprites, magnification off

  // register 2
  writeRegister(2, B00000101); // address of name table in VRAM = 0x1400

  // register 3
  writeRegister(3, B10000000); // address of color table in VRAM = 0x2000

  // register 4  
  writeRegister(4, B00000001); // address of pattern table in VRAM = 0x0800

  // register 5 
  writeRegister(5, B00100000); // address of sprite attribute table in VRAM = 0x1000

  // register 6
  writeRegister(6, B00000000); // address of sprite pattern table in VRAM = 0x0000

  // register 7
  writeRegister(7, B00000011);// set background color
}

inline void writeRegister(byte reg, byte data) {
  writeData(data);
  writeData(reg | 128);
}

inline void writeData(unsigned int u) {
  u = u & 0b0001111111111111;
  j = u | 0b1010000000000000;
  // MODE high, CSW* LOW, CSR* HIGH, DATA=j

  PORTC = B00000001;   // set latch low (leaving reset high)
  SPI.transfer16(j);
  PORTC = B00111111;   // set latch high

  // MODE high, CSW* HIGH, CSR* HIGH, DATA=j
  j = u | 0b1110000000000000;

  PORTC = B00000001;   // set latch low (leaving reset high)
  SPI.transfer16(j);
  PORTC = B00111111;   // set latch high

  // MODE high, CSW* LOW, CSR* HIGH, DATA=j
  j = u | 0b1010000000000000;

  PORTC = B00000001;    // set latch low (leaving reset high)
  SPI.transfer16(j);
  PORTC = B00111111;    // set latch high
}

void loop() {
  for (int i = 1; i < 16; i++) {
    writeRegister(7,i);
    delay(1000);
  }
}
