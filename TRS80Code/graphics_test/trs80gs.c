#include <stdio.h>
#include <stdlib.h>

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
  unsigned int x;
  unsigned int y;
  unsigned char color;
  int xdir;
  int ydir;
} spriteAttr;



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

   
   for(i=0;i<33;i++)
       setCharPattern(i,"0000000000000000");
     
   strcpy(buf, "A854A854A854A800");
   for(i=33;i<256;i++)  
       setCharPattern(i,buf);

/*
   SETPATTERN('!',"0C18183030006000");
   SETPATTERN(34, "00246C0000000000");
   SETPATTERN('#',"4848FC4848FC4800");
   SETPATTERN('$',"307CB07834F83000");
   SETPATTERN('%',"40A4481028548800");
   SETPATTERN('&',"2050206890906800");
   SETPATTERN(39, "0010100000000000");
   SETPATTERN('(',"1020202020201000");
   SETPATTERN(')',"2010101010102000");
   SETPATTERN('*',"885020F820508800");
   SETPATTERN('+',"202020F820202000");
   SETPATTERN(',',"0000000010102000");
   SETPATTERN('-',"000000F800000000"); 
   SETPATTERN('.',"0000000000303000");
   SETPATTERN('/',"0004081020408000");
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
   SETPATTERN(';',"0030300030301000");
   SETPATTERN('<',"0810204020100800");
   SETPATTERN('=',"0000FC00FC000000");
   SETPATTERN('>',"4020100810204000");
   SETPATTERN('?',"788C983030003000");
   SETPATTERN('@',"7884BCA4BC807C00");
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
   /*
   SETPATTERN(91, "3820202020203800");
   SETPATTERN(92, "0080402010080400");
   SETPATTERN(93, "7010101010107000");
   SETPATTERN(94, "2050880000000000");
   SETPATTERN(95, "000000000000FC00");
   SETPATTERN(96, "4060000000000000");
   SETPATTERN('a',"00003C4444443C00");
   SETPATTERN('b',"4040407844447800");
   SETPATTERN('c',"0000384040403800");
   SETPATTERN('d',"0808087888887800");
   SETPATTERN('e',"0000384870403800");
   SETPATTERN('f',"1028407040404000");
   SETPATTERN('g',"0000304838087000");
   SETPATTERN('h',"4040407048484800");
   SETPATTERN('i',"0020002020202000");
   SETPATTERN('j',"0010001010907000");
   SETPATTERN('k',"4040485060504800");
   SETPATTERN('l',"4040404040404000");
   SETPATTERN('m',"000000F0A8A8A800");
   SETPATTERN('n',"0000007048484800");
   SETPATTERN('o',"0000304848483000");
   SETPATTERN('p',"0000302830202000");
   SETPATTERN('q',"000060A060281000");
   SETPATTERN('r',"0000304840404000");
   SETPATTERN('s',"0000704070107000");
   SETPATTERN('t',"0000207020202000");
   SETPATTERN('u',"0000484848483800");
   SETPATTERN('v',"0000888850502000");
   SETPATTERN('w',"0000A8A8A8A85000");
   SETPATTERN('x',"0000505020505000");
   SETPATTERN('y',"0000282810107000");
   SETPATTERN('z',"0000780830407800");
   SETPATTERN(123,"3040408040403000");
   SETPATTERN(124,"3030303030303000");
   SETPATTERN(125,"3008080408083000");
   SETPATTERN(126,"000044A810000000");
*/
}

void audioSilence() {
    for(unsigned int i=LEFT_POS;i<=RIGHT_POS;i++) {
        for(unsigned int j=255;j>247;j=j-2) {
            soundOut(i,j);
        }
    }
}


/*
void setupGraphicsModeZero() {
    setVDPRegister(0,0);    // Graphics 0, No external video input
    setVDPRegister(1,192);  // 16K VRAM, disable interrupts, no magnification of sprites
    setVDPRegister(2,5);    // set up name   table at          0x1400
    setVDPRegister(3,128);  // set up color  table at          0x2000
    setVDPRegister(4,1);    // setup pattern table at          0x0800
    setVDPRegister(5,32);   // setup sprite attribute table at 0x1000
    setVDPRegister(6,0);    // setup sprite pattern table at   0x0000
    setVDPRegister(7,4);    // set background color 

    g.NameTableAddr = 0x1400;
    g.ColorTableAddr = 0x2000;
    g.PatternTableAddr = 0x0800;
    g.SpriteAttrTableAddr = 0x1000;
    g.SpritePatternTableAddr = 0x0000;
}
*/

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
  for(x=0;x<strlen(patt);x++) {
        duple[0] = *(patt+x);
        duple[1] = *(patt+x+1);
        setVDPRAM(addr+d,(unsigned char)strtol(duple, NULL, 16));
        d++;
  }
}


void setSpriteAttribute(unsigned char spriteNum, unsigned int x, unsigned int y, unsigned char color) {
  unsigned int addr = g.SpriteAttrTableAddr + (spriteNum << 2);
  unsigned char vert;
  unsigned char horiz;
  unsigned char name;
  unsigned char b5;

  b5 = color;
  vert = y - 32;
  horiz = x;
  if(horiz < 32)
    b5 = b5 | 0x80;
  else 
     horiz = horiz - 32;
  
  name = spriteNum;
  if(g.sprites16x16Enabled == TRUE)
      name = name << 2;  

  setVDPRAM(addr, vert);
  setVDPRAM(addr+1, horiz);
  setVDPRAM(addr+2, name);
  setVDPRAM(addr+3, b5);
}


void updateSprites() {
  unsigned int i;

  for(i=0;i<32;i++) {
    sprAttr[i].x = sprAttr[i].x + sprAttr[i].xdir;
    if(sprAttr[i].x > 288) sprAttr[i].x = 0;
    if(sprAttr[i].x < 0) sprAttr[i].x = 288;
    sprAttr[i].y = sprAttr[i].y + sprAttr[i].ydir;
    if(sprAttr[i].y > 288) sprAttr[i].y = 0;
    if(sprAttr[i].y < 0) sprAttr[i].y = 288;
    setSpriteAttribute(i, sprAttr[i].x, sprAttr[i].y, sprAttr[i].color);
  }
}


void clearTRSScreen() {
  for(int i=0;i<48;i++)
    printf("\n");
}

unsigned char getKeyboard() {
    void *addr;
    unsigned char val;

    addr = 0x3801;

    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return 'G';
      if(val >= 64)
        return 'F';
      if(val >= 32)
        return 'E';
      if(val >= 16)
        return 'D';
      if(val >= 8)
        return 'C';
      if(val >= 4)
        return 'B';
      if(val >= 2)
        return 'A';
      return '@';
    }
    addr = 0x3802;
    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return 'O';
      if(val >= 64)
        return 'N';
      if(val >= 32)
        return 'M';
      if(val >= 16)
        return 'L';
      if(val >= 8)
        return 'K';
      if(val >= 4)
        return 'J';
      if(val >= 2)
        return 'I';
      return 'H';
    }
    addr = 0x3804;
    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return 'W';
      if(val >= 64)
        return 'V';
      if(val >= 32)
        return 'U';
      if(val >= 16)
        return 'T';
      if(val >= 8)
        return 'S';
      if(val >= 4)
        return 'R';
      if(val >= 2)
        return 'Q';
      return 'P';
    }
    addr = 0x3808;
    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return 0;
      if(val >= 64)
        return 0;
      if(val >= 32)
        return 0;
      if(val >= 16)
        return 0;
      if(val >= 8)
        return ',';
      if(val >= 4)
        return 'Z';
      if(val >= 2)
        return 'Y';
      return 'X';
    }
    addr = 0x3810;
    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return '7';
      if(val >= 64)
        return '6';
      if(val >= 32)
        return '5';
      if(val >= 16)
        return '4';
      if(val >= 8)
        return '3';
      if(val >= 4)
        return '2';
      if(val >= 2)
        return '1';
      return '0';
    }
    addr = 0x3820;
    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return '/';
      if(val >= 64)
        return '.';
      if(val >= 32)
        return '-';
      if(val >= 16)
        return ',';
      if(val >= 8)
        return ';';
      if(val >= 4)
        return ':';
      if(val >= 2)
        return '9';
      return '8';
    }
    addr = 0x3840;
    val = *((char*)addr);
    if(val > 0) {
      if(val >= 128)
        return ' ';
      if(val >= 64)
        return 94;
      if(val >= 32)
        return 93;
      if(val >= 16)
        return 91;
      if(val >= 8)
        return 92;
      if(val >= 4)
        return 96;
      if(val >= 2)
        return 10;
      return 13;
    }
    return 0;
}

int main()
{
   unsigned int j;
   unsigned int k;
   unsigned int i;

   clearTRSScreen();

   printf("INITIALIZING GRAPHICS CONTROL STRUCTURE...\n");
   //memset(&g, 0x0, sizeof(graphicsSetup));

   printf("TURNING OFF AUDIO...\n");
   audioSilence();

   printf("SEEDING RANDOMNESS...\n");
   srand(getRRegister());

   printf("SETTING UP GRAPHICS MODE...\n");

   g.graphicsMode = GRAPHICSMODE1;
   g.externalVideoEnabled = FALSE;
   g.interruptsEnabled = FALSE;
   g.spritesMagnified = FALSE;
   g.sprites16x16Enabled = FALSE;
   g.textForegroundColor = WHITE;
   g.backgroundColor = DARKBLUE;
   setGraphicsMode();
  // setupGraphicsModeZero();

   printf("LOADING SCREEN WITH CHARACTERS...\n");
   j=0;
   k=0;
   for(i=0;i<255;i++) {
       setCharacterAt(j,k,i);
       setCharacterAt(j,k+8,i);
       setCharacterAt(j,k+16,i);
       j++;
       if(j>31) {
           k++;
           j=0;
       }    
   }

   printf("SETTING TEXT PATTERNS...\n");
   setPatterns();

   printf("INITIALIZING SPRITE PATTERNS...\n");
   for(i=0;i<32;i++) 
     setSpritePattern(i,"3C42A9A985D9423C");

   printf("INITIALIZING SPRITE ATTRIBUTES...\n");
   for(int i=0;i<32;i++) {
     sprAttr[i].x = rand()%288;
     sprAttr[i].y = rand()%288;
     sprAttr[i].color = rand()%15+1;
     sprAttr[i].xdir = rand()%10 - 5;
     sprAttr[i].ydir = rand()%10 - 5;
     setSpriteAttribute(i, sprAttr[i].x, sprAttr[i].y, sprAttr[i].color);
   }
   
   printf("MOVING SPRITES AND COLORS...\n");
 
   k=0;
   while(k==0) {
       k = getKeyboard();
       updateSprites();
       for(j=0;j<32;j++)
           setCharacterGroupColor(j, rand()%15+1, rand()%15+1);
   }
   clearTRSScreen();
   printf("DONE!");
   exit(-1);
}