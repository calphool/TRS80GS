VIDEO    EQU    3C00H   ; location of video RAM in memory
         ORG    5200H   ; load this program into 0x5200 memory location (above BASIC and DOS)

; *****************************************************************
; * SNOWFALL - TRS-80                                             *
; * This program creates a snowfall scene.  It creates 128        *
; * snowflakes.  The X and Y coordinates are stored in two        *
; * parallel arrays (XS ans YS).  There are four "planes" of      *
; * of flakes -- four speeds, stored in two parallel arrays       *
; * called XINCS and YINCS.                                       *
; *                                                               *
; * FLOW:                                                         *
; *      10) Clear the screen                                     *  
; *      20) Set up random number seed value                      *
; *      30) Load up XS array with random values 0 - 127          *
; *      40) Load up YS array with random values 0 - 47           *
; *      50) Load up XINCS array with random values from 0 - 3    *
; *      60) Load up YINCS array with random values from 1 - 3    *
; *      70) Main loop                                            *
; *          A.  Clear any pixels from last iterations            *
; *          B.  Compute (XS[B],YS[B]) from XINCS[B] and YINCS[B] *
; *          C.  Draw pixel
; *****************************************************************
;
; ----------------------------------------
; |  Clear screen                        |
; ----------------------------------------  
ENTRY   LD     HL,VIDEO       ; put VIDEO RAM address into HL register
        LD     DE,VIDEO+1     ; put VIDEO RAM address +1 into DE 
        LD     BC,400H        ; put counter into BC 
        LD     (HL),10000000b ; put ascii 128 into video location
        LDIR                  ; run fast memory copy
;
; ----------------------------------------------------------------
; |  Setup random number seed                                    |
; ----------------------------------------------------------------
        CALL   RANDSD         ; set up the random number seed

;
; ----------------------------------------------------------------
; |  Load up XS array with random values from 0 - 127            |
; ----------------------------------------------------------------
        LD     B,128          ; do this loop 128 times
        LD     IX, XS         ; store the address of XS in IX
L1:
        CALL   RANDR          ; get random number for A
        SRA    A              ; divide A by 2 (0 - 127)
        AND    01111111b
        LD     (IX),A         ; store random number in address pointed by IX
        INC    IX             ; add 1 to the address
        DJNZ   L1             ; decrement B, then loop back to L1 until B is zero

;
; ----------------------------------------------------------------
; |  Load up YS array with random values from 0 - 47             |
; ----------------------------------------------------------------
        LD     B,128
        LD     IX, YS
L2:
        CALL   RANDR          ; get random number for A
        SRA    A              ; >> 1 (/2) (256 = 128)
        AND    01111111b      ; make sure to get rid of left most digit
        SRA    A              ; >> 1 (/2) (128 = 64)
        SRA    A              ; >> 1 (/2) (64  = 32)
        LD     C,A            ; put this in C temporarily
        CALL   RANDR          ; get another random number
        SRA    A              ; >> 1 (/2) (256 = 128)
        AND    01111111b      ; make sure to get rid of left most digit
        SRA    A              ; >> 1 (/2) (128 = 64)
        SRA    A              ; >> 1 (/2) (64 = 32)
        SRA    A              ; >> 1 (/2) (32 = 16)
        ADD    A,C            ; now A should contain a number from 0 - 47 (since a random number < 32 + a random number < 16 must be a random number < 48)
        LD     (IX),A         ; put 0 - 47 value to memory pointed to by IX
        INC    IX             ; add 1 to offset address to do next number (the YS memory array location)
        DJNZ   L2             ; subtract 1 from B and jump back to L2 if B > 0

;
; ----------------------------------------------------------------
; |  Load up XINCS array with random values from 0 - 3           |
; ----------------------------------------------------------------
        LD     B,4            ; set B to 4
        LD     IX, XINCS      ; set IX to the memory location of XINCS[0]
L3:
        CALL   RANDR          ; get random numer for A
        LD     C,3            ; set C to 3
        AND    C              ; bitwise AND random value with 3 (meaning random number 0 - 3)
        LD     (IX),A         ; put 1 - 3 value to memory pointed to by IX (XINCS[B])
        INC    IX             ; increment IX
        DJNZ   L3             ; subtract 1 from B and jump back to L3 if B > 0

;
; ----------------------------------------------------------------
; |  Load up YINCS array with random values from 1 - 3           |
; ----------------------------------------------------------------
        LD     B,4            ; set B to 4
        LD     IX, YINCS      ; set IX to memory location of YINCS[0]
L4:
        CALL   RANDR          ; get random number for A
        LD     C,2            ; set C to 2
        AND    C              ; AND A with 2, giving a random number from 0 - 2 in A
        INC    A              ; add 1 to A
        LD     (IX),A         ; put 1 - 3 value to memory pointed to by IX (YINCS[B])
        INC    IX             ; increment IX
        DJNZ   L4             ; subtract 1 from B and jump back to L4 if B > 0


;
; ----------------------------------------------------------------
; |  Main loop                                                   |
; ----------------------------------------------------------------
MAIN    

;
; ----------------------------------------------------------------
; |  Iterate over current positions and "unset" them so          |
; |  that they disappear.                                        |
; ----------------------------------------------------------------
        LD     B,128          ; Loop over all 128 snowflake positions
L5:            
        LD     IX,XS          ; put the address of XS array into IX
        LD     D,0            ; D = 0
        LD     E,B            ; E = B (put loop counter into a 16 bit register)
        ADD    IX,DE          ; DE to IX, making IX point to an array instance
        LD     C,(IX)         ; C now contains the X position for a flake

        LD     IX,YS          ; put the address of YS array into IX
        LD     D,0            ; D = 0
        LD     E,B            ; E = B (put loop counter into a 16 bit register)
        ADD    IX,DE          ; DE to IX, making IX point to an array instance
        LD     D,(IX)         ; D now contains the Y position for a flake
        LD     E,C            ; move C (current X position) to E
        PUSH   BC             ; push BC onto the stack so we can loop inside UNSETGR
        CALL   UNSETGR        ; call routine to unset (black out) a pixel
        POP    BC             ; pop BC back from the stack

;
; ----------------------------------------------------------------
; |  Put current main loop iterator into A register              |
; |  Jump to specific locations based on where we are in the     |
; |  main loop (this is how we form the 4 planes of flakes       |
; ----------------------------------------------------------------
        LD     A,B
        CP     32             ; B < 32?
        JP     C,LS32         ; yes, plane 4
        CP     64             ; B < 64?
        JP     C,LS64         ; yes, plane 3
        CP     96             ; B < 96?
        JP     C,LS96         ; yes, plane 2
                              
LS128:                        ;      plane 1
        LD     IX, XINCS      ; Load address of XINCS into IX
        LD     D,0            ; D = 0
        LD     E,0            ; E = 0
        ADD    IX, DE         ; IX now contains address of XINCS[0]
        LD     IY, XS         ; Load XS array address into IY
        LD     D,0            ; D = 0
        LD     E,B            ; E = main loop counter (flake number)
        ADD    IY, DE         ; IY now contains address of XS[B]
        LD     A,(IX)         ; Load XINCS[0] into A
        LD     C,(IY)         ; Load XS[B] into C
        ADD    A,C            ; A now contains value we want (XINCS[0]), add it to XS[B]
        CP     128            ; Is flake X position > 128 now?
        JP     NC,TOBIG1      ; Yes, go do something about it
        JP     OK1            ; Nope, it's fine, jump over this stuff 
TOBIG1: SUB    128            ; Since it was too big
OK1:    LD     (IY),A         ; Put newly computed XS[B] back into memory

        LD     IX, YINCS      ; Load address of YINCS into IX
        LD     D,0            ; D = 0
        LD     E,0            ; E = 0
        ADD    IX, DE         ; IX now contains address of YINCS[0]
        LD     IY, YS         ; Load YS array address into IY
        LD     D,0            ; D = 0
        LD     E,B            ; E = main loop counter (flake number)
        ADD    IY, DE         ; IY now contains address of YS[B]
        LD     A,(IX)         ; Load YINCS[0] into A
        LD     C,(IY)         ; Load YS[B] into C
        ADD    A,C            ; A now contains value we want (YINCS[0]), add it to YS[B]
        CP     48             ; Is flake X position > 48 now?
        JP     NC,TOBIG2      ; Yes, go do something about it
        JP     OK2            ; Nope, it's fine, jump over this stuff 
TOBIG2: SUB    48             ; Since it was too big
OK2:    LD     (IY),A         ; Put newly computed YS[B] back into memory
        JP     JOIN           ; Okay, we've set the XS[B] and YS[B] to their new values

                              ; this block is the same as the above, just plane 2 (DE set to 1)
LS96:   LD     IX, XINCS
        LD     D,0
        LD     E,1
        ADD    IX, DE  
        LD     IY, XS
        LD     D,0
        LD     E,B
        ADD    IY, DE   
        LD     A,(IX)
        LD     C,(IY)
        ADD    A,C    
        CP     128
        JP     NC,TOBIG3
        JP     OK3
TOBIG3: SUB    128 
OK3:    LD     (IY),A

        LD     IX, YINCS
        LD     D,0
        LD     E,1
        ADD    IX, DE   
        LD     IY, YS
        LD     D,0
        LD     E,B
        ADD    IY, DE 
        LD     A,(IX)
        LD     C,(IY)
        ADD    A,C  
        CP     48
        JP     NC,TOBIG4
        JP     OK4
TOBIG4: SUB    48 
OK4:    LD     (IY),A                      
        JP     JOIN

                              ; this block is the same as the above, just plane 3 (DE set to 2)
LS64:   LD     IX, XINCS
        LD     D,0
        LD     E,2
        ADD    IX, DE   
        LD     IY, XS
        LD     D,0
        LD     E,B
        ADD    IY, DE  
        LD     A,(IX)
        LD     C,(IY)
        ADD    A,C      
        CP     128
        JP     NC,TOBIG5
        JP     OK5        
TOBIG5: SUB    128 
OK5:    LD     (IY),A
       
        LD     IX, YINCS
        LD     D,0
        LD     E,2
        ADD    IX, DE  
        LD     IY, YS
        LD     D,0
        LD     E,B
        ADD    IY, DE   
        LD     A,(IX)
        LD     C,(IY)
        ADD    A,C    
        CP     48
        JP     NC,TOBIG6
        JP     OK6
TOBIG6: SUB    48 
OK6:    LD     (IY),A                      
        JP     JOIN

; ----------------------------------------------------------------
; | This little nasty thing works around a limitation of the Z80 |
; |                                                              |
; | This is a hacky way to get from the bottom of the main loop  |
; | back to the top of the main loop (maximum distance thing     |
; ----------------------------------------------------------------
JB5:                     ; branch back to L5 (top of the main loop)
        JP     L5

                         ; this block is the same as the above, just plane 4 (DE set to 3)        
LS32:   LD     IX, XINCS
        LD     D,0
        LD     E,3
        ADD    IX, DE   
        LD     IY, XS
        LD     D,0
        LD     E,B
        ADD    IY, DE   
        LD     A,(IX)
        LD     C,(IY)
        ADD    A,C      
        CP     128
        JP     NC,TOBIG7
        JP     OK7
TOBIG7: SUB    128 
OK7:    LD     (IY),A

        LD     IX, YINCS
        LD     D,0
        LD     E,3
        ADD    IX, DE   
        LD     IY, YS
        LD     D,0
        LD     E,B
        ADD    IY, DE   
        LD     A,(IX)
        LD     C,(IY)
        ADD    A,C     
        CP     48
        JP     NC,TOBIG8
        JP     OK8
TOBIG8: SUB    48 
OK8:    LD     (IY),A        

;
; ----------------------------------------------------------------
; |  Draw pixel at XS[B] and YS[B]                               |
; ----------------------------------------------------------------
JOIN:                    ; XS[B] and YS[B] have been updated, get ready to call SETGR to plot flake
        LD     IX,XS
        LD     D,0
        LD     E,B
        ADD    IX,DE
        LD     C,(IX)    ; C now contains the X position

        LD     IX,YS
        LD     D,0
        LD     E,B
        ADD    IX,DE
        LD     D,(IX)    ; D now contains the Y position

        LD     E,C       ; E = C (x position)
        PUSH   BC        ; push BC so that B can be used in SETGR
        CALL SETGR       ; plot the pixel at (E,D)
        POP    BC        ; pop BC back so B contains main loop iterator

        DJNZ   JB5       ; decrement B, if it's greater than zero, use the trampoline
                         ; routine to bounce back to the top of the loop

        JP     MAIN      ; start over after plotting all 128 flakes        

        
        
; **********************************************************
; *  Graphics routines                                     *
; *  Set (E,D) before calling SETGR, UNSETGR, or TEST      *
; *  X coordinate in E register                            *
; *  Y coordinate in D register                            *
; *  Note, all registers are destroyed by this routine     *
; *  Make sure you PUSH and POP the registers before       *
; *  invoking any of these routines                        *
; **********************************************************
SETGR   LD  A,0C6H
        JR  TEST10
UNSETGR LD  A,86H
        JR  TEST10
TEST    LD  A,46H
TEST10  LD  (INST+1),A
ADDRES  LD  A,D
        LD  B,0FFH
LOOP    INC B
        SUB 3
        JP  P,LOOP
        ADD A,3
        SLA A
        LD  C,A
        LD  L,B
        LD  H,0
        LD  B,6
LOOP1   ADD HL,HL
        DJNZ LOOP1
        LD  D,0
        SRL E
        JR  NC,CONT
        INC C
CONT    ADD HL,DE
        LD  DE,VIDEO
        ADD HL,DE
        SLA C
        SLA C
        SLA C
        LD  A,(INST+1)
        ADD A,C
        LD  (INST+1),A
INST    DEFB    0CBH
        DEFB    0
        SET     7,(HL)
        RET


; * generates a random number from 0 - 255 (best used as a seed value, because it's not
; * especially random
RANDSD   PUSH    HL
         LD      A,R
         LD      L,A
         AND     63
         LD      H,A
         LD      A,(HL)
         POP     HL
         LD      (seed),A
         RET

; * Generates pretty decent random number between 0 - 255, and as long as the seed routine has been called
RANDR    PUSH   BC
         ld a, (seed)
         ld b, a 
         rrca ; multiply by 32
         rrca
         rrca
         xor   1fH 
         add a, b
         sbc a, 255 ; carry
         ld (seed), a
         POP    BC
         ret

; ###############################################################
; # DATA SECTION                                                #
; ###############################################################
XS       DS   128  ; 128 X locations
YS       DS   128  ; 128 Y locations
XINCS    DS   4    ; 4 x increment values 
YINCS    DS   4    ; 4 y increment values 
SEED     DS   1    ; seed value for random numbers

        END
