#include <stdio.h>
#include <stdlib.h>



__sfr __at 0xe0 UART_RECEIVE_HOLDING_REGISTER;
__sfr __at 0xe0 UART_TRANSMIT_HOLDING_REGISTER;
__sfr __at 0xe1 UART_INTERRUPT_ENABLE_REGISTER;
__sfr __at 0xe2 UART_INTERRUPT_STATUS_REGISTER;
__sfr __at 0xe3 UART_LINE_CONTROL_REGISTER;
__sfr __at 0xe4 UART_MODEM_CONTROL_REGISTER;
__sfr __at 0xe5 UART_LINE_STATUS_REGISTER;
__sfr __at 0xe6 UART_MODEM_STATUS_REGISTER;
__sfr __at 0xe7 UART_SCRATCH_PAD_REGISTER;


void hold(unsigned int x) {
    for(unsigned int y = 0; y<x;y++) {
      #pragma asm
      nop
      #pragma endasm
    }
}



void init_uart() {
  // disable interrupts
  UART_INTERRUPT_ENABLE_REGISTER = 0x00;
  
  // mask to set DLAB on
  UART_LINE_CONTROL_REGISTER = 0x80;

  // divisor = 12 (9600 bps)
  UART_RECEIVE_HOLDING_REGISTER = 6;

  // set MSB of the divisor
  UART_INTERRUPT_ENABLE_REGISTER = 0;

  // 8 bit, 1 stop bit, no parity, and clear DLAB
  UART_LINE_CONTROL_REGISTER = 0x03;
}


void sendByte(char x) {
    unsigned char control_reg;
    unsigned char b = 0x20;

    do {
        control_reg = UART_LINE_STATUS_REGISTER;
    }
    while(control_reg & b == 0);
    
    UART_TRANSMIT_HOLDING_REGISTER = x;
}

unsigned char getByte() {
    unsigned char control_reg;
    unsigned char b = 1;

    do {
        control_reg = UART_LINE_STATUS_REGISTER;
    }
    while(control_reg == 96);

    return UART_RECEIVE_HOLDING_REGISTER;
}

void sendString(char* s) {
  for(int i=0;i<strlen(s);i++) {
    sendByte(*(s+i));
  }
  sendByte(13);
}

void printLSResponse() {
   unsigned char c;
   do {
     c = getByte();
     if(c == ':')
         printf("\n  ");
     else
         printf("%c", c);
    }
    while(c != 13);  
}

int main()
{
  char buf[24];

  printf("INITIALIZING UART...\n");
  init_uart();

  printf("SENDING LS COMMAND...\n");
  sendString("ls");

  printf("LS RESPONSE:\n  ");
  printLSResponse();

  printf("\nSENDING KILL COMMAND...\n");
  sendString("kill");

  printf("TYPE SOMETHING, THEN PRESS ENTER");
  scanf("%s",buf);


  exit(-1);
}
