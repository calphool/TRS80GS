#include <stdio.h>
#include <stdlib.h>


#define DEBUG 1
#undef DEBUG


#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66


unsigned char control_reg;
unsigned char b;
unsigned char workBuff[400];
#define OUTLEN 300
unsigned char decodedData[OUTLEN];
unsigned int szDecodedData;


__sfr __at 0xe0 UART_RECEIVE_HOLDING_REGISTER;
__sfr __at 0xe0 UART_TRANSMIT_HOLDING_REGISTER;
__sfr __at 0xe0 UART_DIVISOR_LSB;
__sfr __at 0xe1 UART_DIVISOR_MSB;
__sfr __at 0xe1 UART_INTERRUPT_ENABLE_REGISTER;
//__sfr __at 0xe2 UART_INTERRUPT_STATUS_REGISTER;
__sfr __at 0xe2 UART_FIFO_CONTROL_REGISTER;
__sfr __at 0xe3 UART_LINE_CONTROL_REGISTER;
__sfr __at 0xe4 UART_MODEM_CONTROL_REGISTER;
__sfr __at 0xe5 UART_LINE_STATUS_REGISTER;
//__sfr __at 0xe6 UART_MODEM_STATUS_REGISTER;
//__sfr __at 0xe7 UART_SCRATCH_PAD_REGISTER;



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


static const unsigned char d[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

int base64decode (char *in, size_t inLen, unsigned char *out) { 
    char *end = in + inLen;
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    //printf("  BASE64DECODE(*,%u,*) ",inLen);
    
    while (in < end) {
        unsigned char c = d[*in++];
        //printf(" %u",(unsigned char)c);
        switch (c) {
        case WHITESPACE: continue;   /* skip whitespace */
        case INVALID:    return 1;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4) {
                len+=3;
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0; iter = 0;

            }   
        }
    }
    if (iter == 3) {
        if ((len += 2) > OUTLEN) return 1;
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > OUTLEN) return 1;
        *(out++) = (buf >> 4) & 255;
    }

    //printf(" (ITER=%d,LEN=%d)\n",iter,len);

    szDecodedData = len;
    return 0;
}

void sendByte(char x) {
    b = 0x20;

    do {
        #pragma asm
        nop
        #pragma endasm
        //printf("%d ", UART_LINE_STATUS_REGISTER);
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
    }
    while((UART_LINE_STATUS_REGISTER & b) == 0);

    return UART_RECEIVE_HOLDING_REGISTER;
}


void sendString(char* s) {
  static int i;
  static int j;

  //printf("IN SENDSTRING: %s\n", s);
  j = strlen(s);

  for(i=0;i<j;i++) {
    sendByte(*(s+i));
  }
  sendByte(13);
}

char* getString() {
  static int i;
  static char c;

  i = 0;
  c = 0;
  //printf("IN GETSTRING\n");

  while(c != 13) {
    c = getByte();
    if(c != 13 && c != 10) {
      workBuff[i] = c;
      workBuff[i+1] = 0x0;
      i++;
    }
  }

  return workBuff;
}

void printLSResponse() {
   static unsigned char c;
   static long chksum;

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
 static unsigned int addrToJump;

  unsigned int chk;
  unsigned int address;
  unsigned long calcChk;
  char c[5];
  unsigned char temp;

  c[4] = 0x0;

  //printf("in HANDLELOADCMD: %s",s);
  
  sendString(s);  // send the LOADCMD to server
  getString();    // get server's response

  if(startsWith(workBuff,"ERROR") == 1) {
      puts(workBuff);
      return;
  }

  while(strncmp(workBuff,"LOADCMD_DONE",strlen(workBuff)) != 0) {
    if(startsWith(workBuff,"OBJ ") == 1) {
      //printf("  %s\n",workBuff);
      //printf("  MOVNG PAST OBJ\n");
      memmove(workBuff, workBuff+4, strlen(workBuff));
      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      //printf("  GETTING ADDRESS\n");
      address = strtoul(c, NULL, 16);
      //printf("  ADDR = %X\n", address);
      //printf("  MOVING PAST ADDR\n");
      memmove(workBuff, workBuff+5, strlen(workBuff));
      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      //printf("  GETTING CHECKSM\n");
      chk = strtoul(c, NULL, 16);
      //printf("  CHKSUM = %d\n", chk);
      //printf("  MOVING PAST CHECKSM\n");
      memmove(workBuff, workBuff+5, strlen(workBuff));
      //printf("  INVOKING BASE64DECODE\n");
      if(base64decode(workBuff, strlen(workBuff), &decodedData) != 0) {
        printf("Problem with base64decode!\n");
      }
      //printf("  DECODED DATA SIZE %d\n", szDecodedData);
      calcChk = 0;
      for(int i=0;i<szDecodedData;i++) {
        temp = *(decodedData+i);
        calcChk = calcChk + (unsigned long)temp;
        //if(szDecodedData < 50)
          //printf("(%d) %ld ",temp, calcChk);
      }

      //printf("\n  CALCULATED CHECKSM %d\n", calcChk);

      if(calcChk == chk) {
        // load this data into memory
        printf("%c",191);
        memcpy(address, decodedData,szDecodedData);
        sendString("OK");
        getString();
      }
      else {
        printf("  CHK MISMATCH: %d != %d\n",chk, calcChk);
        sendString("RESEND");
        getString();
      }
    } 
    else if(startsWith(workBuff,"ENTPT ") == 1) {
      memmove(workBuff, workBuff+6, strlen(workBuff));
      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      address = strtoul(c, NULL, 16);
      sendString("OK");
      getString();
      printf("\nPROGRAM ENTRY POINT: %ud\n", address);
      printf("ENTER 'G' TO RUN AT ENTRY POINT, OR 'C' TO CANCEL\n");
      fgets(workBuff,255,stdin);
      if(startsWith(workBuff,"G") == 1) {
          addrToJump = address;  // sets HL
          #pragma asm
            jp    (hl)
          #pragma endasm
      }
      else {
        break;
      }
    }
    else {
      printf("UNKNOWN MESSAGE FROM SERVER, %s\n", workBuff);
        sendString("OK");
        getString();
    }
  }

    printf("\n");
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
  char buf[128];
  int szBuf;

  cls();

  puts("TRS-80 MODEL 1 UART OS VYYYY-MM-DD-HH-MM-SS");
  puts("===========================================");
  init_uart();

  fputs("\nCOMMAND?",stdout);
  while(strncmp(buf,"X",strlen(buf)) != 0) {
    fputs("\n>",stdout);
    fgets(buf,128,stdin);
    szBuf = strlen(buf);
    if (szBuf > 0 && buf[szBuf - 1] == '\n')
            buf[szBuf - 1] = '\0';

    if(strncmp(buf,"X",szBuf) == 0 || strncmp(buf,"QUIT",szBuf) == 0) {
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

  #pragma asm
    jp    0
  #pragma endasm


  exit(-1);
}
