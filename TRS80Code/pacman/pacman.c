#include <stdio.h>
#include <stdlib.h>
#include "notedata.c"

#define LEFT_POS 0x82
#define RIGHT_POS 0x83

#define TRANSPARENT 0
#define BLACK 1
#define MEDIUMGREEN 2
#define LIGHTGREEN 3
#define DARKBLUE 4
#define LIGHTBLUE 5
#define DARKRED 6
#define CYAN 7
#define MEDIUMRED 8
#define LIGHTRED 9
#define DARKYELLOW 10
#define LIGHTYELLOW 11
#define DARKGREEN 12
#define MAGENTA 13
#define GRAY 14
#define WHITE 15

#define SETPATTERN(XXX,YYY) strcpy(buf,YYY); setCharPattern(XXX,buf);

#define PACMAN_SPRITENUM 0




__sfr __at 0x80 PORTX80;
__sfr __at 0x81 PORTX81;
__sfr __at 0x82 PORTX82;
__sfr __at 0x82 PORTX83;



// graphicsSetup
#define GRAPHICSMODE1 0
#define GRAPHICSMODE2 1
#define MULTICOLORMODE 2
#define TEXTMODE 3

#define TRUE 1
#define FALSE 0


typedef struct {
    unsigned char graphicsMode;
    unsigned char externalVideoEnabled;
    unsigned char interruptsEnabled;
    unsigned char sprites16x16Enabled;
    unsigned char spritesMagnified;
    unsigned char textForegroundColor;
    unsigned char backgroundColor;
    unsigned int NameTableAddr;
    unsigned int ColorTableAddr;
    unsigned int PatternTableAddr;
    unsigned int SpriteAttrTableAddr;
    unsigned int SpritePatternTableAddr;
} graphicsSetup;

typedef struct {
  int x;
  int y;
  unsigned char color;
  int xdir;
  int ydir;
} spriteAttr;



unsigned char bRunning = TRUE;
unsigned int anPos = 0;
int anDir = 1;
graphicsSetup g;
spriteAttr sprAttr[32];



/* *************************************************************
   | get the more-or-less random value of the R register       |
   *************************************************************
*/
unsigned char getRRegister() {
    #pragma asm
    ld a,r
    ld h,0
    ld l,a
    #pragma endasm
}



void setVDPRegister(unsigned char reg,unsigned char dat) {
    PORTX81 = dat;
    reg=reg+128;
    PORTX81 = reg;
}


void setVDPRAM(unsigned int addr, unsigned char dat) {
    static unsigned int addr_2;
    static unsigned int addr_1;
    
    addr_1 = (addr >> 8);
    addr_2 = addr_1;
    addr_2 = addr_2 + 64;

    addr_1 = (addr_1 << 8);
    addr_1 = addr - addr_1;

    PORTX81 = (unsigned char)addr_1;
    PORTX81 = (unsigned char)addr_2;
    PORTX80 = dat;
}

unsigned char getVDPRAM(unsigned int addr) {
    static unsigned int addr_2;
    static unsigned int addr_1;
    
    addr_2 = (addr >> 8);
    addr_1 = addr - (addr_2 << 8);

    PORTX81 = (unsigned char)addr_1;
    PORTX81 = (unsigned char)addr_2;
    return PORTX80;
}


void soundOut(unsigned char LeftOrRight, unsigned char dat) {
    if(LeftOrRight == LEFT_POS) 
        PORTX82 = dat;
    else
        PORTX83 = dat; 
}


unsigned char getJoystick(unsigned char LeftOrRight) {
    if(LeftOrRight == LEFT_POS)
        return PORTX83;
    else
        return PORTX82;
}


void setCharacterAt(unsigned char x, unsigned char y, unsigned char c) {
    unsigned int addr = g.NameTableAddr;

    if(g.graphicsMode == TEXTMODE)
      addr = addr + (y*40) + x;
    else
      addr = addr + (y<<5) + x;

    setVDPRAM(addr,c);
}


unsigned char getCharAt(unsigned char x, unsigned char y) {
    unsigned int addr = g.NameTableAddr;

    if(g.graphicsMode == TEXTMODE)
      addr = addr + (y*40) + x;
    else
      addr = addr + (y<<5) + x;

    return getVDPRAM(addr);
}


void setCharactersAt(unsigned char x, unsigned char y, char* s) {
    unsigned int addr = g.NameTableAddr;
    unsigned char c;

    if(g.graphicsMode == TEXTMODE)
      addr = addr + (y*40) + x;
    else
      addr = addr + (y<<5) + x;

    for(int i=0;i<strlen(s);i++) {
        c = *(s+i);
        setVDPRAM(addr+i, c);
    }
}


void setScreenColor(unsigned char textForegroundColor, unsigned char backgroundColor) {
    setVDPRegister(7,textForegroundColor << 4 | backgroundColor);
}


void setCharPattern(unsigned char pos, char* patt) {
    char duple[3];
    unsigned char d;
    unsigned int addr = pos*8 + g.PatternTableAddr;
    
    duple[2]=0x0;
    d = 0;
    for(unsigned char x=0;x<strlen(patt);x=x+2) {
        duple[0] = *(patt+x);
        duple[1] = *(patt+x+1);
        setVDPRAM(addr+d,(unsigned char) strtol(duple, NULL, 16));
        d++;
    }
}


unsigned char setCharacterGroupColor(unsigned char colorGroup, unsigned char foreground, unsigned char background) {
    unsigned char c;

    if(colorGroup > 31 || foreground > WHITE || background > WHITE)
        return 0; 

    c = ((foreground << 4) & 0xf0) + (background & 0x0f);
    setVDPRAM(g.ColorTableAddr + colorGroup, c);
}


void setPatterns() {
   unsigned int i;
   unsigned char buf[17];
   
   for(i=0;i<256;i++)
       setCharPattern(i,"0000000000000000");
   

                                        //     SHAPE  
   SETPATTERN('a',"8181818181818181");  //      | |   
                                        //      | |   

   SETPATTERN('b',"FF000000000000FF");  //      ___  
                                        //      ---   

   SETPATTERN('c',"3C42818181818181");  //      .--.
                                        //      |  |  

   SETPATTERN('d',"3F4080808080403F");  //       ___
                                        //      {___  

   SETPATTERN('e',"818181818181423C");  //      |  |
                                        //      '--'  

   SETPATTERN('f',"FC020101010102FC");  //       ___
                                        //       ___}
                                               
   SETPATTERN('g',"00000000000000FF");  //       ___


   SETPATTERN('h',"8080808080808080");  //       |
                                        //       |

   SETPATTERN('i',"FF00000000000000");  //       --
                                        //

   SETPATTERN('j',"0101010101010101");  //         |
                                        //         |

   SETPATTERN('k',"3F40808080808080");  //       ___
                                        //      |

   SETPATTERN('l',"FC02010101010101");  //       ___
                                        //          |

   SETPATTERN('m',"01010101010102FC");  //          |
                                        //       ___|

   SETPATTERN('n',"808080808080403F");  //       |
                                        //       |___

   SETPATTERN('o',"0000000000000001");  //          .


   SETPATTERN('p',"0000000000000080");  //         .


   SETPATTERN('q',"0100000000000000");  //          '

   
   SETPATTERN('r',"8000000000000000");  //         '

/*
   SETPATTERN('A',"788484FC84848400");
   SETPATTERN('B',"F88484F88484F800");
*/
   SETPATTERN('C',"7884808080847800");
/*
   SETPATTERN('D',"F88484848484F800");
*/
   SETPATTERN('E',"FC8080F88080FC00");
/*
   SETPATTERN('F',"FC8080F880808000");
   SETPATTERN('G',"788080BC84847800");
   SETPATTERN('H',"848484FC84848400");
   SETPATTERN('I',"FC3030303030FC00");
   SETPATTERN('J',"7C10101090907000");
   SETPATTERN('K',"8498A0C0A0988400");
   SETPATTERN('L',"808080808080FC00");
   SETPATTERN('M',"84CCB4B484848400");
   SETPATTERN('N',"84C4A4A4948C8400");
*/
   SETPATTERN('O',"7884848484847800");
/*
   SETPATTERN('P',"F88484F880808000");
   SETPATTERN('Q',"78848484948C7C00");
*/
   SETPATTERN('R',"F88488F088888400");
   SETPATTERN('S',"7C8080780404F800");
/*
   SETPATTERN('T',"FC30303030303000");
   SETPATTERN('U',"8484848484847800");
   SETPATTERN('V',"8484844848483000");
   SETPATTERN('W',"848484B4B4CC8400");
   SETPATTERN('X',"8484483048848400");
   SETPATTERN('Y',"8448483030303000");
   SETPATTERN('Z',"FC0810102040FC00");
   */
   SETPATTERN('0',"788C94B4A4C47800");
   SETPATTERN('1',"307030303030FC00");
   SETPATTERN('2',"788484186080FC00");
   SETPATTERN('3',"7884043804847800");
   SETPATTERN('4',"CCCCCCFC0C0C0C00");
   SETPATTERN('5',"FC80807804847800");
   SETPATTERN('6',"384080F884847800");
   SETPATTERN('7',"FC0C181830306000");
   SETPATTERN('8',"7884847884847800");
   SETPATTERN('9',"7884847C04087000");
   SETPATTERN(':',"0030300000303000");
   SETPATTERN('-',"0000007E00000000");
}

void audioSilence() {
    for(unsigned int i=LEFT_POS;i<=RIGHT_POS;i++) {
        for(unsigned int j=255;j>247;j=j-2) {
            soundOut(i,j);
        }
    }
}



void setGraphicsMode() {
  unsigned char b;

  b = 0;
  if(g.graphicsMode == GRAPHICSMODE2) 
      b = b | 1;
 
  if(g.externalVideoEnabled == TRUE)
      b = b | 2;

  setVDPRegister(0,b);

  b = 192;
  if(g.interruptsEnabled == TRUE)
      b = b | 32;

  if(g.graphicsMode == TEXTMODE)
      b = b | 16;
  
  if(g.graphicsMode == MULTICOLORMODE) 
      b = b | 8;

  if(g.sprites16x16Enabled == TRUE)
      b = b | 2;

  if(g.spritesMagnified == TRUE) 
      b = b | 1;
  
  setVDPRegister(1, b);
  b = 0;

  setVDPRegister(2, 0xf);
  g.NameTableAddr = 0x3c00;

  setVDPRegister(3, 0x80);
  g.ColorTableAddr = 0x2000;

  setVDPRegister(4, 0x0);
  g.PatternTableAddr = 0x0000;

  setVDPRegister(5, 0x70);
  g.SpriteAttrTableAddr = 0x3800;

  setVDPRegister(6, 0x03);
  g.SpritePatternTableAddr = 0x1800;

  b = g.textForegroundColor << 4 | g.backgroundColor;
  setVDPRegister(7, b);
}


void setSpritePattern(unsigned char spriteNumber, char* patt) {
  char duple[3];
  unsigned char d;
  unsigned char x;
  unsigned int addr = g.SpritePatternTableAddr;
  duple[2] = 0x0;

  if(g.sprites16x16Enabled == TRUE) 
      addr = addr + (spriteNumber << 5);
  else
      addr = addr + (spriteNumber << 3);
  
  d=0;
  for(x=0;x<strlen(patt);x=x+2) {
        duple[0] = *(patt+x);
        duple[1] = *(patt+x+1);
        setVDPRAM(addr+d,(unsigned char)strtol(duple, NULL, 16));
        d++;
  }
}


 void setSpriteAttribute(unsigned char spriteNum, int x, unsigned int y, unsigned char color, unsigned char patternNumber) {
  unsigned int addr = g.SpriteAttrTableAddr + (spriteNum << 2);
  unsigned char vert;
  int horiz;
  unsigned char b5;

  b5 = color;
  vert = y ;

  if(x<0) {
    b5 = b5 | 0x80;
    x = x + 32;
  }
  horiz = x ;

  
  setVDPRAM(addr, vert);
  setVDPRAM(addr+1, (unsigned char)horiz);
  setVDPRAM(addr+2, patternNumber<<2);
  setVDPRAM(addr+3, b5);
}



void clearTRSScreen() {
  for(int i=0;i<48;i++)
    printf("\n");
}


void drawMaze() {
  unsigned char j;
  unsigned char k;


  for(j=0;j<32;j++)
     setCharacterGroupColor(j, DARKBLUE, BLACK);
  for(j=8;j<=11;j++)
     setCharacterGroupColor(j, DARKRED, BLACK);
  for(j=6;j<=7;j++)
     setCharacterGroupColor(j, WHITE, BLACK);


  for(k=0;k<24;k++)
      for(j=0;j<32;j++)
          setCharacterAt(j,k,' ');
  

   for(j=3;j<=9;j++) {
       setCharacterAt(0,j,'a');
       setCharacterAt(31,j,'a');
   }

   for(j=14;j<=22;j++) {
       setCharacterAt(0,j,'a');
       setCharacterAt(31,j,'a');    
   }

   for(j=0;j<32;j++) {
       setCharacterAt(j,1,'g');
       setCharacterAt(j,23,'g');
   }

   setCharactersAt(18,0,"SCORE: 9999999");

   for(j=5;j<=9;j++) {
       setCharacterAt(3, j, 'a');
       setCharacterAt(28, j, 'a');
   }

   for(j=14;j<=20;j++) {
       setCharacterAt(3, j, 'a');
       setCharacterAt(28, j, 'a');
   }

   for(j=5;j<=17;j++) {
       setCharacterAt(6, j, 'a');
       setCharacterAt(25, j, 'a');
   }

   for(j=8;j<=9;j++) {
       setCharacterAt(9, j, 'a');
       setCharacterAt(22,j, 'a');
   }

   setCharacterAt(9,14,'a');
   setCharacterAt(22,14,'a');

   for(j=10;j<22;j++) 
       setCharacterAt(j,4,'b');
   
   for(j=10;j<=13;j++) {
       setCharacterAt(j,7,'b');
       setCharacterAt(j+8,7,'b');
   }

   setCharacterAt(13, 10, 'i');
   setCharacterAt(14, 10, 'i');
   setCharacterAt(17, 10, 'i');
   setCharacterAt(18, 10, 'i');
   setCharacterAt(12, 11, 'h');
   setCharacterAt(19, 11, 'j');

   for(j=13;j<=18;j++) 
      setCharacterAt(j,12, 'g');

   setCharacterAt(10,15, 'b');
   setCharacterAt(21,15, 'b');
   setCharacterAt(15,15, 'b');
   setCharacterAt(16,15, 'b');

   for(j=10;j<=13;j++) {
    setCharacterAt(j,18, 'b');
    setCharacterAt(j+8,18, 'b');
   }

   for(j=7;j<=24;j++) 
    setCharacterAt(j,21,'b');

  setCharacterAt(0,2,'c');
  setCharacterAt(3,4,'c');
  setCharacterAt(6,4,'c');
  setCharacterAt(25,4,'c');
  setCharacterAt(28,4,'c');
  setCharacterAt(31,2,'c');
  setCharacterAt(0,13,'c');
  setCharacterAt(3,13,'c');
  setCharacterAt(9,13,'c');
  setCharacterAt(22,13,'c');
  setCharacterAt(28,13,'c');
  setCharacterAt(31,13,'c');

  setCharacterAt(0,10,'e');
  setCharacterAt(3,10,'e');
  setCharacterAt(9,10,'e');
  setCharacterAt(22,10,'e');
  setCharacterAt(28,10,'e');
  setCharacterAt(31,10,'e');
  setCharacterAt(0,23,'e');
  setCharacterAt(3,21,'e');
  setCharacterAt(6,18,'e');
  setCharacterAt(25,18,'e');
  setCharacterAt(31,23,'e');
  setCharacterAt(28,21,'e');

  setCharacterAt(9, 4, 'd');
  setCharacterAt(9, 18, 'd');
  setCharacterAt(6, 21, 'd');
  setCharacterAt(17,18, 'd');
  setCharacterAt(20,15, 'd');
  setCharacterAt(14,15, 'd');
  setCharacterAt(17,7,'d');

  setCharacterAt(22,4,'f');
  setCharacterAt(17,15,'f');
  setCharacterAt(14,18,'f');
  setCharacterAt(22,18,'f');
  setCharacterAt(25,21,'f');
  setCharacterAt(11,15,'f');
  setCharacterAt(14,7,'f');

  setCharacterAt(12,10,'k');
  setCharacterAt(9,7,'k');
  setCharacterAt(19,10,'l');
  setCharacterAt(22,7,'l');
  setCharacterAt(12,12,'n');
  setCharacterAt(9,15,'n');
  setCharacterAt(19,12,'m');
  setCharacterAt(22,15,'m');

  setCharacterAt(9,15,'n');
  setCharacterAt(22,15,'m');
}


void movePacman() {
  unsigned char c;
  int x = sprAttr[PACMAN_SPRITENUM].x;
  int y = sprAttr[PACMAN_SPRITENUM].y;

    sprAttr[PACMAN_SPRITENUM].x = x + sprAttr[PACMAN_SPRITENUM].xdir;
    sprAttr[PACMAN_SPRITENUM].y = y + sprAttr[PACMAN_SPRITENUM].ydir;
    if(x < -16)
      sprAttr[PACMAN_SPRITENUM].x = -16;
    if(x > 255)
      sprAttr[PACMAN_SPRITENUM].x = 255;

    if(y < 16)
      sprAttr[PACMAN_SPRITENUM].y = 16;
    if(y > 175)
      sprAttr[PACMAN_SPRITENUM].y = 176;
 
    anPos = anPos + anDir;
    if(anPos > 1 || anPos < 1) 
      anDir = -anDir;
   
    setSpriteAttribute(PACMAN_SPRITENUM, sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y, sprAttr[PACMAN_SPRITENUM].color,anPos);
}

unsigned char canGoLeftOrRight(unsigned char spritenum) {
    int x = sprAttr[spritenum].x;
    int y = sprAttr[spritenum].y;
    if(y == 16 && x >= 8 && x <= 232)
      return TRUE;
    if(y == 40 && x <= 184 && x >= 56)
      return TRUE;
    if(y == 64 && x >= 80 && x <= 160)
      return TRUE;
    if(y == 88) {
      if(x >= 0 && x <= 32)
        return TRUE;
      if(x >= 56 && x <= 80)
        return TRUE;
      if(x >= 160 && x <= 184)
        return TRUE;
      if(x >= 208 && x <= 240)
        return TRUE;
    }
    if(y == 104 && x >= 80 && x <= 160)
      return TRUE;
    if(y == 128 && x >= 56 && x <= 184)
      return TRUE;
    if(y == 152 && x >= 32 && x <= 208)
      return TRUE;
    if(y == 176 && x >= 8 && x <= 232)
      return TRUE;

    return FALSE;
}


unsigned char canGoUpOrDown(unsigned char spritenum) {
    int x = sprAttr[spritenum].x;
    int y = sprAttr[spritenum].y;

    if(x == 8 && y >= 16 && y <= 176)
      return TRUE;
    if(x == 32 && y >= 16 && y <= 176)
      return TRUE;
    if(x == 56 && y >= 16 && y <= 152)
      return TRUE;
    if(x == 80 && y >= 64 && y <= 104)
      return TRUE;
    if(x == 96 && y >= 104 && y <= 128)
      return TRUE;
    if(x == 120) {
      if(y >= 40 && y <= 72)
         return TRUE;
      if(y >= 128 && y <= 152)
         return TRUE;
    }
    if(x == 144 && y >= 104 && y <= 128)
      return TRUE;
    if(x == 184 && y >= 16 && y <= 152)
      return TRUE;
    if(x == 208 && y >= 16 && y <= 176)
      return TRUE;
    if(x == 232 && y >= 16 && y <= 176)
      return TRUE;

    return FALSE;
}


void checkControls() {
        unsigned char k = getJoystick(LEFT_POS);
        if(k == 191 && canGoLeftOrRight(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].xdir=2;
          sprAttr[PACMAN_SPRITENUM].ydir=0;
        }
        else
        if(k == 127 && canGoUpOrDown(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].ydir=-2;
          sprAttr[PACMAN_SPRITENUM].xdir=0;
        }
        else
        if(k == 223 && canGoLeftOrRight(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].xdir=-2;
          sprAttr[PACMAN_SPRITENUM].ydir=0;
        }
        else
        if(k == 239 && canGoUpOrDown(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].ydir=2;
          sprAttr[PACMAN_SPRITENUM].xdir=0;
        }
        else
        if(k == 247)
              bRunning = FALSE;
}


int main()
{
   int j;
   unsigned int k;
   unsigned int i;

   char buf[33];

   clearTRSScreen();

   printf("AUDIO OFF\n");
   audioSilence();

   printf("SETUP AUDIO\n");
   initNoteData();

   printf("SEEDING RNG\n");
   srand(getRRegister());

   printf("GRAPHICS MODE SETUP\n");

   g.graphicsMode = GRAPHICSMODE1;
   g.externalVideoEnabled = FALSE;
   g.interruptsEnabled = FALSE;
   g.spritesMagnified = FALSE;
   g.sprites16x16Enabled = TRUE;
   g.textForegroundColor = WHITE;
   g.backgroundColor = BLACK;
   setGraphicsMode();

   printf("SETUP TEXT PATS\n");
   setPatterns();

   printf("SETUP MAZE\n");
   drawMaze();

   printf("INIT SPRITE PATS\n");
   setSpritePattern(0,"0F1F3F7FFFFFFFFFFFFFFFFF7F3F1F0FF0F8FCBEFFFFFF00FFFFFFFFFEFCF8F0");  // PACMAN, mouth closed (right)
   setSpritePattern(1,"0F1F3F7FFFFFFFFFFFFFFFFF7F3F1F0FF0F8FCBEFFFEE000E0FEFFFFFEFCF8F0");  // PACMAN, mouth open 1 (right)
   setSpritePattern(2,"0F1F3F7FFFFFFFFFFFFFFFFF7F3F1F0FF0F8FCBEF8E0800080C0E0F8FEFCF8F0");  // PACMAN, mouth open 2 (right)

   sprAttr[PACMAN_SPRITENUM].x = 8;
   sprAttr[PACMAN_SPRITENUM].y = 16;
   sprAttr[PACMAN_SPRITENUM].color = DARKYELLOW;


   printf("ANIMATE SPRITE\n");
 
   
   while(bRunning == TRUE) {
        checkControls();
        movePacman();
        
        sprintf(buf, "%d:%d    ",sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y);
        setCharactersAt(0,0,buf);
   }

   printf("DONE!");
   exit(-1);
}