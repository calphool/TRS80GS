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
  pinMode(latchPin, OUTPUT);    // set latch as output mode
  pinMode(clockPin, OUTPUT);    // set clock as output mode
  pinMode(dataPin, OUTPUT);     // set data as output mode
  DDRC = B00111111;             // turn all the pins on PORTC (the analog pins) on
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
  writeData(B00000000);        // graphics mode 1, no external video
  writeData(B10000000);        // command for register 0

  // register 1
  writeData(B11000000);        // 16k, Enable display, Disable interrupts, 8x8 sprites, magnification off
  writeData(B10000001);        // command for register 1

  // register 2
  writeData(B00000101);        // address of name table in VRAM = 0x1400
  writeData(B10000010);        // command for register 2

  // register 3
  writeData(B10000000);        // address of color table in VRAM = 0x2000
  writeData(B10000011);        // command for register 3

  // register 4 
  writeData(B00000001);        // address of pattern table in VRAM = 0x0800
  writeData(B10000100);        // command for register 4

  // register 5  
  writeData(B00100000);        // address of sprite attribute table in VRAM = 0x1000
  writeData(B10000101);        // command for register 5

  // register 6
  writeData(B00000000);        // address of sprite pattern table in VRAM = 0x0000
  writeData(B10000110);        // command for register 6

  // register 7
  writeData(B00000011);        // set background color 
  writeData(B10000111);        // command for register 7
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
    writeData(i);            // set background color 
    writeData(B10000111);    // register 7
    delay(500);
  }
}
