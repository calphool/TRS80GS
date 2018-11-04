#include <stdio.h>
#include <stdlib.h>


#pragma asm
    SECTION lowmem_ram
    org 0FBC2H
_workBuff:
    defs 400
_decodedData:
    defs 300
_d:
    defs 256,66
#pragma endasm



#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66


unsigned char control_reg;
unsigned char byt;

extern unsigned char workBuff[];
extern unsigned char decodedData[];
extern unsigned char d[];

unsigned int szDecodedData;
unsigned int timeout;
unsigned char sTimeOut[] = "\nTIMEOUT.";
unsigned char sCRLF[] = "\n  ";
unsigned char sOK[] = "OK";
 
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
//__sfr __at 0x82 PORTX82;
//__sfr __at 0x83 PORTX83;



void audioSilence() {
  #pragma asm
  push af
  ld  a,255
  out (0x82),a
  out (0x83),a
  ld  a,253
  out (0x82),a
  out (0x83),a
  ld  a,251
  out (0x82),a
  out (0x83),a
  ld  a,249
  out (0x82),a
  out (0x83),a
  pop af
  #pragma endasm
}

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



int base64decode (char *in, size_t inLen, unsigned char *out) { 
    char *end = in + inLen;
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;

    
    while (in < end) {
        unsigned char c = d[*in++];
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
        if ((len += 2) > 300) return 1;
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > 300) return 1;
        *(out++) = (buf >> 4) & 255;
    }

    szDecodedData = len;
    return 0;
}

void sendByte(char x) {
    byt = 0x20;
    timeout = 65534;

    do {
        timeout--;
    }
    while((UART_LINE_STATUS_REGISTER & byt) == 0 && timeout > 0);
    
    UART_TRANSMIT_HOLDING_REGISTER = x;
}

unsigned char getByte() {
    byt = 0x01;
    timeout = 65534;

    do {
      timeout--;
    }
    while((UART_LINE_STATUS_REGISTER & byt) == 0 && timeout > 0);

    return UART_RECEIVE_HOLDING_REGISTER;
}


void sendString(char* s) {
  static int i;
  static int j;

  j = strlen(s);

  for(i=0;i<j;i++) {
    sendByte(*(s+i));
    if(timeout == 0) {
      printf(sTimeOut);
      return;
    }
  }
  sendByte(13);
}

char* getString() {
  static int i;
  static char c;

  i = 0;
  c = 0;

  do {
    c = getByte();
    if(timeout == 0)
      return NULL;

    if(c != 13 && c != 10) {
      workBuff[i] = c;
      workBuff[i+1] = 0x0;
      i++;
    }
  }
  while(c != 13);

  return workBuff;
}

void printLSResponse() {
   static unsigned char c;
   static long chksum;

   printf(sCRLF);

   chksum = 0;
   do {
     c = getByte();
     chksum += c;
     
     if(c == ':') 
         printf(sCRLF);
     else
         putchar(c);
    }
    while(c != 13); 
    printf("\nCHKSUM: %ld\n", chksum); 
}


void drawChar(unsigned long offset, unsigned long count, unsigned char cc) {
  static unsigned long of;
  static unsigned long cnt;
  static unsigned char ccs;
  of = offset;
  cnt = count;
  ccs = cc;
  
  #pragma asm
        push   HL
        push   DE
        push   BC
        push   AF
        ld     HL,(_st_drawChar_of)       
        ld     DE,(_st_drawChar_of)
        INC    DE      
        ld     BC,(_st_drawChar_cnt)
        ld     a,(_st_drawChar_ccs)
        ld     (HL),a
        ldir 
        pop    AF
        pop    BC
        pop    DE
        pop    HL        
  #pragma endasm 
}

void cls() {
  drawChar((unsigned long)0x3c00, (unsigned long)0x3ff, (unsigned char)128);
}

void drawProgress(unsigned int xBytes) {
  drawChar((unsigned long)0x3f80, (unsigned long)64, (unsigned char)128);

  if(xBytes > 0) {
    drawChar((unsigned long)0x3f80, (unsigned long)xBytes, (unsigned char)153);
  }
}

int startsWith (char* base, char* str) {
    return (strstr(base, str) - base) == 0;
}


void handleLoadCmd(char* s) {
 static unsigned int addrToJump;

  unsigned int chk;
  unsigned int bytePerc;
  unsigned int address;
  unsigned int bytePercentage;
  unsigned long calcChk;
  char c[5];
  unsigned char temp;
  unsigned char blkCtr;

  c[4] = 0x0;


  blkCtr = 0;

  sendString(s);  // send the LOADCMD to server
  if(getString() == NULL) {    // get server's response
    printf(sTimeOut);
    return;
  }

  if(startsWith(workBuff,"ERROR") == 1) {
      puts(workBuff);
      return;
  }

  while(strncmp(workBuff,"GET_DONE",strlen(workBuff)) != 0) {
    blkCtr++;
    if(startsWith(workBuff,"OBJ ") == 1) {
      memmove(workBuff, workBuff+4, strlen(workBuff));
      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      address = strtoul(c, NULL, 16);
      memmove(workBuff, workBuff+5, strlen(workBuff));

      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      bytePercentage = strtoul(c, NULL, 16);
      memmove(workBuff, workBuff+5, strlen(workBuff));


      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      chk = strtoul(c, NULL, 16);
      memmove(workBuff, workBuff+5, strlen(workBuff));

      if(base64decode(workBuff, strlen(workBuff), &decodedData) != 0) {
        printf("PROBLEM WITH BASE64DECODE!\n");
      }

      calcChk = 0;
      for(int i=0;i<szDecodedData;i++) {
        temp = *(decodedData+i);
        calcChk = calcChk + (unsigned long)temp;
      }

      if(calcChk == chk) {
        drawProgress(bytePercentage);
        memcpy(address, decodedData,szDecodedData);
        sendString(sOK);
        if(getString() == NULL) {
          printf(sTimeOut);
          return;
        }
      }
      else {
        printf("  CHK MISMATCH: %d != %d\n",chk, calcChk);
        sendString("RESEND");
        if(getString() == NULL) {
          printf(sTimeOut);
          return;
        }
      }
    } 
    else if(startsWith(workBuff,"ENTPT ") == 1) {
      memmove(workBuff, workBuff+6, strlen(workBuff));
      c[0] = *(workBuff);
      c[1] = *(workBuff+1);
      c[2] = *(workBuff+2);
      c[3] = *(workBuff+3);
      address = strtoul(c, NULL, 16);
      sendString(sOK);
      if(getString() == NULL) {
          printf(sTimeOut);
          return;
      }

      printf("\nPROGRAM ENTRY POINT: %u\n", address);
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
      printf("UNKNOWN MSG FROM SERVER, %s\n", workBuff);
        sendString(sOK);
        if(getString() == NULL) {
          printf(sTimeOut);
          return;
        }
    }
  }

    printf(sCRLF);
}

void sendCommandAndDumpResult(char* s) {
    sendString(s);
    printLSResponse(); 
}


void init_base64Table_Loop(unsigned int i, unsigned int ix, unsigned int j) {
    unsigned int m;
    unsigned int n;

    n = j;
    for(m=i;m<=ix;m++) {
      *(d+n) = m;
      n++;
    }
}

void init_base64Table() {
/*
static const unsigned char d[] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,   // + 00
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,   // + 25
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,   // + 50
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,   // + 75
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,   // +100
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,   // +125
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,   // +150
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,   // +175
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,   // +200
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,   // +225
    66,66,66,66,66,66                                                             // +250
};
*/

  drawChar((unsigned long) &d, (unsigned long)256, (unsigned char)66);
  
  *(d+0x0a) = 64;
  *(d+0x2b) = 62;
  *(d+0x2f) = 63;
  
  init_base64Table_Loop(52,61,0x30);
  
  *(d+0x3d) = 65;

  init_base64Table_Loop(0,25,0x41);
  init_base64Table_Loop(26,51,0x61);
}


int main()
{
  char buf[128];
  int szBuf;
  char sTitle[] = "          TRS-80 MODEL 1 UART OS VYYYY-MM-DD-HH-MM-SS";

  cls();
  audioSilence();
  puts(sTitle);
  drawChar((unsigned long)0x3c40, (unsigned long)64, (unsigned char)131);
  init_base64Table();
  init_uart();

  fputs("\nCOMMAND?",stdout);
  while(strncmp(buf,"X",strlen(buf)) != 0) {
    fputs("\n>",stdout);
    fgets(buf,128,stdin);
    szBuf = strlen(buf);
    if (szBuf > 0 && buf[szBuf - 1] == '\n')
            buf[szBuf - 1] = '\0';

    if(strncmp(buf,"X",szBuf) == 0 || strncmp(buf,"Q",szBuf) == 0) {
      break;
    }
    else if (startsWith(buf,"GET") == 1) {
      handleLoadCmd(buf);
    }
    else if((startsWith(buf,"LS") == 1) || (startsWith(buf,"SETDELAY") == 1)) {
      sendCommandAndDumpResult(buf);
    }
    else if((startsWith(buf,"HELP") == 1) || (startsWith(buf,"?") == 1)) {
      printf("  CHOOSE:  HELP, LS, SETDELAY, GET, X\n");
    }
    else {
      printf("UNKNOWN CMD: %s\n",buf);
    }
  }

  printf("DONE. PRESS ENTER.\n");
  fgets(buf,128,stdin);

  #pragma asm
    jp    0
  #pragma endasm

  exit(-1);
}
