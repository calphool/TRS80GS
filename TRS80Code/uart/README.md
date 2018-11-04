Getting UART code working for TRS80GS
-------------------------------------

Make sure you've followed this power-up sequence:

    A) Turn on TRS80GS card
    B) Power on TRS-80
    C) Plug in FTDI cable to Mac USB
    D) Start uart_server executable

Also, don't use the Reset button on the TRS-80.  It creates chaos.  Use the power button.


The UART file upload/download system works in two parts.  First there is a TRS-80 binary module
that loads into the SRAM of the TRS80GS card.  Steps 1 - 6 must be completed any time you 
change the uart.c module.

The second part is the Mac server component.  It runs on your Mac (although it *might* work on a)
PC that has GCC, I've not tried it there, and I wouldn't be surprised if the serial code didn't 
work the same way).  It basically listens for commands from the UART chip and responds appropriately.




Step 1.  Compile uart.c using cmp.sh, which produces a .cmd and a .wav file 

	:~/z88dk/cmp.sh uart 55000

	Note: the 55000 is the address you want the uart client code to load to.  Make sure that the 
	.cmd file fits inside the space you specify.  The address space for the Z80 is 
	0x0000 to 0xffff.  If your .cmd file is 7k, then you would use 0xffff-(7*1024), or
	58368 (0xE400).  You can use a lower value that that (and probably should), but that would be 
	the maximum.  Depending on the details of how the compiler optimizes, that may not be low 
	enough.  I generally subtract an addtional 3000 so that I don't have to worry about it.


Step 2.  Load the uart.wav file into Audacity and amplify it 8 db.  (You must allow clipping.)

Step 3.  Turn on your TRS-80 Model 1.  When you see the MEM SIZE? prompt, type 18000
         Then connect your TRS80 cassette line to your Mac or PC.

Step 4.  On the TRS-80 type:

         SYSTEM

         *? A

Step 5.  Press PLAY on Audacity

     Note: if you are using an official TRS-80 cassette cable, you'll be using the black 
     plug into your Mac or PC.  Also, note that as the data loads, you should see a couple of 
     asterisks in the top right side of the screen, with the right one flashing off and on
     as the data loads.

Step 6.  Once the data is loaded type:

         *? /55000

     Note that the number you type should match what you specified in step 1.  The UART 
     program should appear on screen and ask you for a command.  You can type ? to see 
     the list of commands it supports.

Step 7.  Compile the Mac server component.
     :~/TRS80GS/OSXCode$ gcc uart_server.c

Step 8.  Execute the Mac server code.
     :~/TRS80GS/OSXCode$ ./uart_server

     Note: you should see something like:

         Starting...
         Getting devices...
         Searching for non-bluetooth tty devices...
             ...ignoring bluetooth device: tty.Bluetooth-Incoming-Port ...

         Found 1 valid serial device(s).
         Defaulting to serial device: tty.usbserial-AI055XFC
         Opening...
         Waiting for command.


Step 9.  On the TRS-80 command prompt, type:

         >LS

     Note: you should see a list of files that are in the same directory as the uart_server. 
     If you have no .CMD files, you need to find some and put them in that directory and try again.

Step 10.  To load a .CMD file, at the TRS-80 type:

         >GET LASERDEF.CMD

     Note:  You will see a progress bar as the .CMD file is loaded into memory and the console on
            the Mac uart_server app will show a percentage loaded, like this:

		     Get Command requested...
		        GET LASERDEF.CMD
		        Reading Object block, addr 0x8000, length = 128 (0 percent complete)
				Reading Object block, addr 0x8080, length = 128 (1 percent complete)
				Reading Object block, addr 0x8100, length = 128 (2 percent complete)
				Reading Object block, addr 0x8180, length = 128 (3 percent complete)
				Reading Object block, addr 0x8200, length = 128 (5 percent complete)
				Reading Object block, addr 0x8280, length = 128 (6 percent complete)
				Reading Object block, addr 0x8300, length = 128 (7 percent complete)
				Reading Object block, addr 0x8380, length = 128 (9 percent complete)
				Reading Object block, addr 0x8400, length = 128 (10 percent complete)
				Reading Object block, addr 0x8480, length = 128 (11 percent complete)
				Reading Object block, addr 0x8500, length = 128 (13 percent complete)
				Reading Object block, addr 0x8580, length = 128 (14 percent complete)


    After it finishes loading, you should see:

        PROGRAM ENTRY POINT: xxxxxx
        ENTER 'G' TO RUN AT ENTRY POINT, or 'C' TO CANCEL

    Normally you want to type G at this point.  If you know what you're doing, you could type C to 
    get back to the command prompt to load something else into memory.


Step 11.  Once you're done with whatever you loaded, you can restart your TRS-80 and the code for the 
          uart client should still be memory resident (as long as whatever you loaded didn't corrupt it).

          So, just start your TRS-80 again, use 18000 for your memory size, type SYSTEM and then:

          *? /55000

          to launch the UART client to load something different.  If the UART client gets corrupted, 
          you will need to run steps 4 - 6 again.  However, it is rare for a .CMD to use memory outside 
          of where it was loaded, so this should rarely happen.

