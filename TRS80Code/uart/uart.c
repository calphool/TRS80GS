#include <stdio.h>
#include <stdlib.h>



unsigned char control_reg;
unsigned char b;


__sfr __at 0xe0 UART_RECEIVE_HOLDING_REGISTER;
__sfr __at 0xe0 UART_TRANSMIT_HOLDING_REGISTER;
__sfr __at 0xe0 UART_DIVISOR_LSB;
__sfr __at 0xe1 UART_DIVISOR_MSB;
__sfr __at 0xe1 UART_INTERRUPT_ENABLE_REGISTER;
__sfr __at 0xe2 UART_INTERRUPT_STATUS_REGISTER;
__sfr __at 0xe2 UART_FIFO_CONTROL_REGISTER;
__sfr __at 0xe3 UART_LINE_CONTROL_REGISTER;
__sfr __at 0xe4 UART_MODEM_CONTROL_REGISTER;
__sfr __at 0xe5 UART_LINE_STATUS_REGISTER;
__sfr __at 0xe6 UART_MODEM_STATUS_REGISTER;
__sfr __at 0xe7 UART_SCRATCH_PAD_REGISTER;



/*
void hold(unsigned int x) {
    for(unsigned int y = 0; y<x;y++) {
      #pragma asm
      nop
      #pragma endasm
    }
}
*/


void init_uart() {
  // disable interrupts
  UART_INTERRUPT_ENABLE_REGISTER = 0x00;
  
  // mask to set DLAB on
  UART_LINE_CONTROL_REGISTER = 0x80;

  /*
      Baud    Divisor
      1200    186
      1800    124
      2000    113
      2400     93
      3600     62
      4800     47
      7200     31
      9600     23
     19200     12
     38400      6
     57600      4
    115200      2
  */
  UART_DIVISOR_LSB = 4;

  // set MSB of the divisor
  UART_DIVISOR_MSB = 0;

  // 8 bit, 1 stop bit, no parity, and clear DLAB
  UART_LINE_CONTROL_REGISTER = 0x03;

  // turn on FIFO, 14 char depth, clear FIFO buffers
  UART_FIFO_CONTROL_REGISTER = 0xc7;

  // turn on auto flow control
  UART_MODEM_CONTROL_REGISTER = 0x22;
}


void sendByte(char x) {
    b = 0x20;

    do {
        #pragma asm
        nop
        #pragma endasm
    }
    while((UART_LINE_STATUS_REGISTER & b) == 0);
    
    UART_TRANSMIT_HOLDING_REGISTER = x;
}

unsigned char getByte() {
    b = 0x01;

    do {
        #pragma asm
        nop
        #pragma endasm
        //printf("%d (%d) ",UART_LINE_STATUS_REGISTER & b, UART_LINE_STATUS_REGISTER);
    }
    while((UART_LINE_STATUS_REGISTER & b) == 0);

    return UART_RECEIVE_HOLDING_REGISTER;
}

void sendString(char* s) {
  static int i;
  static int j;

  j = strlen(s);

  for(i=0;i<j;i++) {
    sendByte(*(s+i));
  }
  sendByte(13);
}

void printLSResponse() {
   static unsigned char c;
   static long chksum;

   chksum = 0;
   do {
     c = getByte();
     chksum += c;
     
     if(c == ':') {
         printf("\n  ");
     }
     else
         putchar(c);
    }
    while(c != 13); 
    printf("\nCHKSUM: %ld\n\n", chksum); 
}

void cls() {
  #pragma asm
        push HL
        push DE
        push BC
        ld     HL,0x3C00       
        ld     DE,0x3C01      
        ld     BC,0x3FF         
        ld     (HL),128 
        ldir         
        pop BC
        pop DE
        pop HL
  #pragma endasm
}

int main()
{
  char buf[255];

  cls();

  printf("INITIALIZING UART...\n");
  init_uart();


  while(strncmp(buf,"X") != 0) {
    printf(">");
    fgets(buf,255,stdin);
    if ((strlen(buf) > 0) && (buf[strlen (buf) - 1] == '\n'))
            buf[strlen (buf) - 1] = '\0';
          
    if(strncmp(buf,"X") != 0) {
      sendString(buf);
      printLSResponse();
      printf("\n");
    }

  }

  printf("Done.\n");

  exit(-1);
}
