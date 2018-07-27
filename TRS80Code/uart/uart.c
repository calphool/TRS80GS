#include <stdio.h>
#include <stdlib.h>


#define DEBUG 1
#undef DEBUG

unsigned char control_reg;
unsigned char b;
unsigned char workBuff[300];


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

    #ifdef DEBUG
      puts("  ENTER-SENDSTRING(),");
    #endif


  j = strlen(s);

  for(i=0;i<j;i++) {
    sendByte(*(s+i));
  }
  sendByte(13);

    #ifdef DEBUG
      puts("  EXIT-SENDSTRING(),");
    #endif
}

char* getString() {
  static int i;
  static char c;

    #ifdef DEBUG
      puts("  ENTER-GETSTRING(),");
    #endif


  i = 0;
  c = 0;

  while(c != 13) {
    c = getByte();
    if(c != 13 && c != 10) {
      workBuff[i] = c;
      workBuff[i+1] = 0x0;
      i++;
    }
  }

    #ifdef DEBUG
      puts("  EXIT-GETSTRING(),");
    #endif

  return workBuff;
}

void printLSResponse() {
   static unsigned char c;
   static long chksum;

    #ifdef DEBUG
      puts("  ENTER-PRINTLSRESPONSE(),");
    #endif

   printf("\n  ");

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
    printf("\nCHKSUM: %ld\n", chksum); 


    #ifdef DEBUG
      puts("  EXIT-PRINTLSRESPONSE(),");
    #endif
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

int startsWith (char* base, char* str) {
    return (strstr(base, str) - base) == 0;
}

void handleLoadCmd(char* s) {
    #ifdef DEBUG
      puts("  ENTER-HANDLELOADCMD(),");
    #endif


  sendString(s);
  getString();

  if(strncmp(workBuff,"NO_FILE_SPECIFIED",strlen(workBuff)) == 0 ||
     strncmp(workBuff,"FILE_NOT_CMD_FORMAT",strlen(workBuff)) == 0 ||
     strncmp(workBuff,"UNABLE_TO_OPEN_FILE",strlen(workBuff)) == 0) {
      puts(workBuff);
      return;
  }

  while(strncmp(workBuff,"LOADCMD_DONE",strlen(workBuff)) != 0) {
    puts(workBuff);
    sendString("OK");
    getString();
  }

    #ifdef DEBUG
      puts("  EXIT-HANDLELOADCMD(),");
    #endif
}

void sendCommandAndDumpResult(char* s) {
    #ifdef DEBUG
      puts("  ENTER-SENDCOMMANDANDDUMPRESULT(),");
    #endif
    sendString(s);
    printLSResponse(); 
    #ifdef DEBUG     
      puts("  EXIT-SENDCOMMANDANDDUMPRESULT(),");
    #endif
}

int main()
{
  char buf[255];

  cls();

  printf("INITIALIZING UART...\n");
  init_uart();

  printf("\nCOMMAND?");
  while(strncmp(buf,"X",strlen(buf)) != 0) {
    printf("\n>");
    fgets(buf,255,stdin);
    if ((strlen(buf) > 0) && (buf[strlen (buf) - 1] == '\n'))
            buf[strlen (buf) - 1] = '\0';

    if(strncmp(buf,"X",strlen(buf)) == 0) {
      break;
    }
    else if (startsWith(buf,"LOADCMD") == 1) {
      handleLoadCmd(buf);
    }
    else if(startsWith(buf,"LS") == 1) {
      sendCommandAndDumpResult(buf);
    }
    else if(startsWith(buf,"KILL") == 1) {
      sendCommandAndDumpResult(buf);  
    }
    else if(startsWith(buf,"SETDELAY") == 1) {
      sendCommandAndDumpResult(buf);
    }
    else {
      printf("UNKNOWN COMMAND: %s\n",buf);
    }

  }

  printf("DONE.\n");

  exit(-1);
}
