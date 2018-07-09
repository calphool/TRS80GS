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

#define EAST_PACMAN_PAT_OFFSET 0
#define NORTH_PACMAN_PAT_OFFSET 3 
#define WEST_PACMAN_PAT_OFFSET 6
#define SOUTH_PACMAN_PAT_OFFSET 9

/* 0x82 = SOUND 1 when OUT */
/* 0x82 = JOY 1 when IN */
/* 0x83 = SOUND 2 when OUT */
/* 0x83 = JOY 2 when IN */

__sfr __at 0x80 PORTX80;
__sfr __at 0x81 PORTX81;
__sfr __at 0x82 PORTX82;
__sfr __at 0x83 PORTX83;



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

   SETPATTERN('A',"788484FC84848400");
   SETPATTERN('B',"F88484F88484F800");
   SETPATTERN('C',"7884808080847800");
   SETPATTERN('D',"F88484848484F800");
   SETPATTERN('E',"FC8080F88080FC00");
   SETPATTERN('F',"FC8080F880808000");
   SETPATTERN('G',"788080BC84847800");
   SETPATTERN('H',"848484FC84848400");
   SETPATTERN('I',"FC3030303030FC00");
   SETPATTERN('J',"7C10101090907000");
   SETPATTERN('K',"8498A0C0A0988400");
   SETPATTERN('L',"808080808080FC00");
   SETPATTERN('M',"84CCB4B484848400");
   SETPATTERN('N',"84C4A4A4948C8400");
   SETPATTERN('O',"7884848484847800");
   SETPATTERN('P',"F88484F880808000");
   SETPATTERN('Q',"78848484948C7C00");
   SETPATTERN('R',"F88488F088888400");
   SETPATTERN('S',"7C8080780404F800");
   SETPATTERN('T',"FC30303030303000");
   SETPATTERN('U',"8484848484847800");
   SETPATTERN('V',"8484844848483000");
   SETPATTERN('W',"848484B4B4CC8400");
   SETPATTERN('X',"8484483048848400");
   SETPATTERN('Y',"8448483030303000");
   SETPATTERN('Z',"FC0810102040FC00");
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
  for(int i=0;i<16;i++)
    printf("\n");
}


void drawMaze() {
  unsigned char j;
  unsigned char k;

  for(j=0;j<32;j++)
     setCharacterGroupColor(j, DARKBLUE, BLACK);  // set all characters to blue on black
  for(j=8;j<=11;j++)
     setCharacterGroupColor(j, DARKRED, BLACK);   // set chars 64 - 95 to red on black 
  for(j=6;j<=7;j++)
     setCharacterGroupColor(j, WHITE, BLACK);     // set chars 48 - 63 to white on black


  // clear screen
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



unsigned char canGoNorth(unsigned char spriteNum) {
    int x = sprAttr[spriteNum].x;
    int y = sprAttr[spriteNum].y;
    

    if(x == 8 && y > 16 )
      return TRUE;

    if(x == 32 && y > 16)
      return TRUE;

    if(x == 56 && y < 150)
      return TRUE;

    if(x == 184 && y < 150)
      return TRUE;

    if(x == 208 && y > 16 )
      return TRUE;

    if(x == 232 && y > 16)
      return TRUE;

    if(x == 120 && y >= 38 && y <= 62)
      return TRUE;

    if(x == 80 && y >= 62 && y <= 102)
      return TRUE;

    if(x == 160 && y >= 62 && y <= 102)
      return TRUE;

    if(x == 96 && y >= 102 && y <= 126)
      return TRUE;

    if(x == 144 && y >= 102 && y <= 126)
      return TRUE;

    if(x == 120 && y >= 128 && y <= 150)
      return TRUE;

    if(y <= 16)
      return FALSE;

    return FALSE;
}

unsigned char canGoEast(unsigned char spriteNum) {
    int x = sprAttr[spriteNum].x;
    int y = sprAttr[spriteNum].y;

    if(x <= 230)
      return TRUE;

    return FALSE;
}

unsigned char canGoWest(unsigned char spriteNum) {
    int x = sprAttr[spriteNum].x;
    int y = sprAttr[spriteNum].y;

    if(x >= 10)
        return TRUE;

    return FALSE;
}

unsigned char canGoSouth(unsigned char spriteNum) {
    int x = sprAttr[spriteNum].x;
    int y = sprAttr[spriteNum].y;


    if(x == 8 && y <= 172)
      return TRUE;

    if(x == 32 && y <= 172)
      return TRUE;

    if(x == 56 && y <= 150)
      return TRUE;

    if(x == 184 && y <= 150)
      return TRUE;

    if(x == 208 && y <= 172)
      return TRUE;

    if(x == 232 && y <= 172)
      return TRUE;

    if(x == 120 && y >= 38 && y <= 62)
      return TRUE;

    if(x == 80 && y >= 62 && y <= 102)
      return TRUE;

    if(x == 160 && y >= 62 && y <= 102)
      return TRUE;

    if(x == 96 && y >= 102 && y <= 126)
      return TRUE;

    if(x == 144 && y >= 102 && y <= 126)
      return TRUE;

    if(x == 120 && y >= 128 && y <= 150)
      return TRUE;

    if(y > 172)
      return FALSE;

    return FALSE;
}


void movePacman() {
    unsigned char c;
    int x = sprAttr[PACMAN_SPRITENUM].x;
    int y = sprAttr[PACMAN_SPRITENUM].y;
    int xd = sprAttr[PACMAN_SPRITENUM].xdir;
    int yd = sprAttr[PACMAN_SPRITENUM].ydir;

    if((xd < 0 && canGoWest(PACMAN_SPRITENUM)) || (xd > 0 && canGoEast(PACMAN_SPRITENUM)))
        sprAttr[PACMAN_SPRITENUM].x = x + xd;
    if((yd < 0 && canGoNorth(PACMAN_SPRITENUM)) || (yd > 0 && canGoSouth(PACMAN_SPRITENUM)))
        sprAttr[PACMAN_SPRITENUM].y = y + yd;

    if(x < -8)
        sprAttr[PACMAN_SPRITENUM].x = -8;
    if(x > 248)
        sprAttr[PACMAN_SPRITENUM].x = 248;

    if(y < 16)
        sprAttr[PACMAN_SPRITENUM].y = 16;
    if(y > 174)
        sprAttr[PACMAN_SPRITENUM].y = 174;
   
    anPos = anPos + anDir;
    if(anPos > 1 || anPos < 1) 
      anDir = -anDir;
   
    if(yd < 0)
        setSpriteAttribute(PACMAN_SPRITENUM, sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y, sprAttr[PACMAN_SPRITENUM].color,anPos + NORTH_PACMAN_PAT_OFFSET);
    else 
        if(yd > 0)
            setSpriteAttribute(PACMAN_SPRITENUM, sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y, sprAttr[PACMAN_SPRITENUM].color,anPos + SOUTH_PACMAN_PAT_OFFSET);
        else
            if(xd < 0)
                setSpriteAttribute(PACMAN_SPRITENUM, sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y, sprAttr[PACMAN_SPRITENUM].color,anPos + WEST_PACMAN_PAT_OFFSET);
            else
                setSpriteAttribute(PACMAN_SPRITENUM, sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y, sprAttr[PACMAN_SPRITENUM].color,anPos + EAST_PACMAN_PAT_OFFSET);
}


#define J_N 239
#define J_NE 111
#define J_E 127
#define J_SE 95
#define J_S 223
#define J_SW 159
#define J_W 191
#define J_NW 175
#define J_BUTTON 247
void checkControls() {
        unsigned char k = getJoystick(LEFT_POS);
        if(k == J_E && canGoEast(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].xdir=2;
          sprAttr[PACMAN_SPRITENUM].ydir=0;
        }
        else
        if(k == J_N && canGoNorth(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].ydir=-2;
          sprAttr[PACMAN_SPRITENUM].xdir=0;
        }
        else
        if(k == J_W && canGoWest(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].xdir=-2;
          sprAttr[PACMAN_SPRITENUM].ydir=0;
        }
        else
        if(k == J_S && canGoSouth(PACMAN_SPRITENUM)) {
          sprAttr[PACMAN_SPRITENUM].ydir=2;
          sprAttr[PACMAN_SPRITENUM].xdir=0;
        }
        else
        if(k == J_BUTTON)
              bRunning = FALSE;
}


void wait(unsigned int x) {
    for(unsigned int y = 0; y<x;y++) {
      #pragma asm
      nop
      #pragma endasm
    }
}


void audioSilence() {
  soundOut(LEFT_POS,255); soundOut(RIGHT_POS,255);
  soundOut(LEFT_POS,253); soundOut(RIGHT_POS,253);
  soundOut(LEFT_POS,251); soundOut(RIGHT_POS,251);
  soundOut(LEFT_POS,249); soundOut(RIGHT_POS,249);
}


void volumeup() {
  soundOut(LEFT_POS,9);  soundOut(RIGHT_POS,9);
  //soundOut(LEFT_POS,11); soundOut(RIGHT_POS,11);
  //soundOut(LEFT_POS,13); soundOut(RIGHT_POS,13);
  //soundOut(LEFT_POS,15); soundOut(RIGHT_POS,15);
}


#define EIGHTHNOTE 1000
#define playLeft(NOTE) PORTX82 = notes[NOTE].b1; PORTX82 = notes[NOTE].b2;
#define playRight(NOTE) PORTX83 = notes[NOTE].b1; PORTX83 = notes[NOTE].b2;
void introMusic() {
  volumeup();

  playLeft(C0);                    playRight( C2);                  wait(EIGHTHNOTE);
                                   playRight( C3);                  wait(EIGHTHNOTE);
  playLeft(C1);                    playRight( G2);                  wait(EIGHTHNOTE);
                                   playRight( E2);                  wait(EIGHTHNOTE);
  playLeft(C0);                    playRight( C3);                  wait(EIGHTHNOTE);
                                   playRight( G2);                  wait(EIGHTHNOTE);
  playLeft(C1);                    playRight( E2);                  wait(EIGHTHNOTE);
                                                                    wait(EIGHTHNOTE);

  playLeft(CS0);                   playRight( CS2);                 wait(EIGHTHNOTE);
                                   playRight( CS3);                 wait(EIGHTHNOTE);
  playLeft(CS1);                   playRight( GS2);                 wait(EIGHTHNOTE);
                                   playRight( F2);                  wait(EIGHTHNOTE);
  playLeft(CS0);                   playRight( CS3);                 wait(EIGHTHNOTE);
                                   playRight( GS2);                 wait(EIGHTHNOTE);
  playLeft(CS1);                   playRight( F2);                  wait(EIGHTHNOTE);
                                                                    wait(EIGHTHNOTE);

  playLeft(C0);                    playRight( C2);                  wait(EIGHTHNOTE);
                                   playRight( C3);                  wait(EIGHTHNOTE);
  playLeft(C1);                    playRight( G2);                  wait(EIGHTHNOTE);
                                   playRight( E2);                  wait(EIGHTHNOTE);
  playLeft(C0);                    playRight( C3);                  wait(EIGHTHNOTE);
                                   playRight( G2);                  wait(EIGHTHNOTE);
  playLeft(C1);                    playRight( E2);                  wait(EIGHTHNOTE);
                                                                    wait(EIGHTHNOTE);

  playLeft(G0);                    playRight( F2);                  wait(EIGHTHNOTE);
                                   playRight( FS2);                 wait(EIGHTHNOTE);
                                   playRight( G2);                  wait(EIGHTHNOTE/2);
                                                                    audioSilence(); wait(EIGHTHNOTE/32); volumeup();
  playLeft(A1);                    playRight( G2);                  wait(EIGHTHNOTE);
                                   playRight( GS2);                 wait(EIGHTHNOTE);
                                   playRight( A3);                  wait(EIGHTHNOTE/2);
                                                                    audioSilence(); wait(EIGHTHNOTE/32); volumeup();
  playLeft(B1);                    playRight( A3);                  wait(EIGHTHNOTE);
                                   playRight( AS3);                 wait(EIGHTHNOTE);
                                   playRight( B3);                  wait(EIGHTHNOTE/2);
                                                                    audioSilence(); wait(EIGHTHNOTE/32); volumeup();
  playLeft(C1);                    playRight( C3);                  wait(EIGHTHNOTE);
                                                                    wait(EIGHTHNOTE);
                                                                    wait(EIGHTHNOTE);
                                                                    wait(EIGHTHNOTE);

  audioSilence();
}


int main()
{
   int j;
   unsigned int k;
   unsigned int i;

   char buf[33];

   bRunning = TRUE;

   clearTRSScreen();

   printf(".---------------------------------------.\n");
   printf("(                PACMAN!!               )\n");
   printf("(           YYYY-MM-DD-HH-MM-SS         )\n");
   printf("(              CKSUM               )\n");
   printf("'---------------------------------------'\n\n");
   printf("THIS PROGRAM REQUIRES THE TRS-80 GRAPHICS ");
   printf("AND SOUND CARD V2.0.\n\n");

   printf("AUDIO OFF... ");
   audioSilence();

   printf("SETUP AUDIO...\n");
   initNoteData();

   printf("SEEDING RANDOM NUMBER GENERATOR... ");
   srand(getRRegister());

   printf("GRAPHICS MODE SETUP...\n");

   g.graphicsMode = GRAPHICSMODE1;
   g.externalVideoEnabled = FALSE;
   g.interruptsEnabled = FALSE;
   g.spritesMagnified = FALSE;
   g.sprites16x16Enabled = TRUE;
   g.textForegroundColor = WHITE;
   g.backgroundColor = BLACK;
   setGraphicsMode();

   printf("HIDE PACMAN IF ON SCREEN... ");
   sprAttr[PACMAN_SPRITENUM].x = 8;
   sprAttr[PACMAN_SPRITENUM].y = 16;
   sprAttr[PACMAN_SPRITENUM].color = BLACK;
   setSpriteAttribute(PACMAN_SPRITENUM, sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y, sprAttr[PACMAN_SPRITENUM].color,anPos);


   printf("SETUP TEXT PATTERNS...\n");
   setPatterns();

   printf("SETUP MAZE... ");
   drawMaze();

   printf("INIT SPRITE PATTERNS...\n");

   //071F3F7F7FFFFFFFFFFFFF7F7F3F1F07E0F8FCDEFEFFFF80FFFFFFFEFEFCF8E0   <- closed mouth east face
   //061E3E7E7EEEFEFFFFFFFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0   <- closed mouth north face
   //071F3F7B7FFFFF01FFFFFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0   <- closed mouth west face
   //071F3F7B7FFFFF01FFFFFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0   <- closed mouth south face

   //071F3F7F7FFFFFFFFFFFFF7F7F3F1F07E0F8FCDEFFFCE080E0FCFFFEFEFCF8E0   <- open mouth east (1)
   //08183C7C7CEEFEFFFFFFFF7F7F3F1F0720387C7E7EFFFFFFFFFFFFFEFEFCF8E0   <- open mouth north (1)
   //071F3F7BFF3F0701073FFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0   <- open mouth west (1)
   //071F3F7F7FFFFFFFFFFEEE7C7C3C1808E0F8FCFEFEFFFFFFFFFFFF7E7E7C3820   <- open mouth south (1)

   //071F3F7F7FFFFFFFFFFFFF7F7F3F1F07E0F8FCDEF8E080000080E0F8FEFCF8E0   <- open mouth east (2)
   //0010307878ECFCFEFFFFFF7F7F3F1F0700080C1E1E3F3F7FFFFFFFFEFEFCF8E0   <- open mouth north (2)
   //071F3F7B1F0701000001071F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0   <- open mouth west (2)
   //071F3F7F7FFFFFFFFEFCEC7878301000E0F8FCFEFEFFFFFF7F3F3F1E1E0C0800   <- open mouth south (2)

   setSpritePattern(0, "071F3F7F7FFFFFFFFFFFFF7F7F3F1F07E0F8FCDEFEFFFF80FFFFFFFEFEFCF8E0"); // PACMAN, mouth closed (east)
   setSpritePattern(1, "071F3F7F7FFFFFFFFFFFFF7F7F3F1F07E0F8FCDEFFFCE080E0FCFFFEFEFCF8E0"); // PACMAN, mouth open 1 (east)
   setSpritePattern(2, "071F3F7F7FFFFFFFFFFFFF7F7F3F1F07E0F8FCDEF8E080000080E0F8FEFCF8E0"); // PACMAN, mouth open 2 (east)
   
   setSpritePattern(3, "061E3E7E7EEEFEFFFFFFFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0"); // PACMAN, mouth closed (north)
   setSpritePattern(4, "08183C7C7CEEFEFFFFFFFF7F7F3F1F0720387C7E7EFFFFFFFFFFFFFEFEFCF8E0"); // PACMAN, mouth open 1 (north)
   setSpritePattern(5, "0010307878ECFCFEFFFFFF7F7F3F1F0700080C1E1E3F3F7FFFFFFFFEFEFCF8E0"); // PACMAN, mouth open 2 (north)
   
   setSpritePattern(6, "071F3F7B7FFFFF01FFFFFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0"); // PACMAN, mouth closed (west)
   setSpritePattern(7, "071F3F7BFF3F0701073FFF7F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0"); // PACMAN, mouth open 1 (west)
   setSpritePattern(8, "071F3F7B1F0701000001071F7F3F1F07E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0"); // PACMAN, mouth open 2 (west)
   
   setSpritePattern(9, "071F3F7F7FFFFFFFFFFEEE7E7E3E1E06E0F8FCFEFEFFFFFFFFFFFFFEFEFCF8E0"); // PACMAN, mouth closed (south)
   setSpritePattern(10,"071F3F7F7FFFFFFFFFFEEE7C7C3C1808E0F8FCFEFEFFFFFFFFFFFF7E7E7C3820"); // PACMAN, mouth open 1 (south)
   setSpritePattern(11,"071F3F7F7FFFFFFFFEFCEC7878301000E0F8FCFEFEFFFFFF7F3F3F1E1E0C0800"); // PACMAN, mouth open 2 (south)
   
   setSpritePattern(12,"070F0F1D193A383F3F3F3F3F3F3F1D08F0F8F8DCCCAE8EFEFEFEFEFEFEFEDC88"); // GHOST (1)
   setSpritePattern(13,"0F1F1F3F3375717F7F7F7F7F7F7F6E24E0F0F0F8985C1CFCFCFCFCFCFCFCEC48"); // GHOST (2)
   
   sprAttr[PACMAN_SPRITENUM].color = DARKYELLOW;

   printf("PLAY INTRO MUSIC... ");
   introMusic();

   printf("ANIMATE SPRITES...\n");  
   while(bRunning == TRUE) {
        checkControls();
        movePacman();
        
        sprintf(buf, "%d:%d    ",sprAttr[PACMAN_SPRITENUM].x, sprAttr[PACMAN_SPRITENUM].y);
        setCharactersAt(0,0,buf);
   }

   printf("DONE!");
   exit(-1);
}