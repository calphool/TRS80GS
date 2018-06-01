# TRS80GS
Storage area for my TRS-80 Model 1 Graphics, Sound, and Gaming cartidge

![Photo of prototype PCB](/img/TRS80GS/img/TRS-80 Model 1 GraphicsSoundJoystick Card Prototype - Imgur.jpg?raw=true "Prototype PCB")


My friend's first computer was a 1977 TRS-80 Model 1.  I'm a few years younger than him, so my first computer was a TI-99/4A, 
followed soon after by an Atari 8-bit (800XL I believe).

For some reason my friend kept trying to assert that his *Trash-80* was in some way superior to all home computers of the era, 
and of course this couldn't stand.  I decided to see what all his fussing was about.  I learned some Z80 assembler, 
started hacking a TRS-80 emulator, and eventually got somewhat comfortable with Z80 assembler (I still prefer 6502 assembler).  I 
tried to do some graphics stuff, but *man*, TRS-80 Model 1's *barely* have the ability to do anything you'd call "graphics." 
I did a couple of snow-field type demos, but it looks more like slowly falling bricks on some low-gravity planet than snow.

The TRS-80 uses these goofy ["pseudographics" characters (2 by 3 grids)](https://www.classic-computers.org.nz/system-80/manuals_tm3_page33.jpg), 
and you have to do everything by selecting the right "character" in order to plot these huge blocky pixels.  Your resolution is effectively 128 x 48, and 
it's a pain to work with, because you're actually manipulating a 64 x 16 text grid.  I wrote some assembler code so that I could at least *address* 
the TRS-80 Model 1 screen *as if* it had 128 x 48 resolution, which I've included in this repo (see the snow.asm code, it's the FSETGR routine).

Eventually the day came that I thought "Ok, I'm going to try to write a game for this nasty old thing."  I *really* wanted 
graphics, sound, and a joystick, but alas that wasn't really in the cards for an unmodified TRS-80 Model 1.  So I began 
imagining what it would be like to augment a TRS-80 Model 1 so that it was *at least* as capable as *my* first computer, the 
TI.  I found a couple of dodgy designs for such devices from the era, but they were hand drawn, hard to read, and loaded with 
components that are no longer available.  So, I decided to design my own.

This repo is a gathering place for my work so far.  I've designed a cartridge that will plug into the TRS-80 Model 1 expansion 
port.  It is built around a TMS9118A video display processor (a close relative of the one used in the TI-99/4A and Colecovision).
It also includes two SN76489 sound chips (also from TI-99/4A, but *better* because we have two of them -- six voice harmony baby!)
I've also included a 74LS244 on the bus attached to an Atari 2600 joystick port (DB9), which in theory will give me the 
ability to poll for joystick condition.  That may end up being a bad design, since it's not using any interrupts, but I 
figured I'd start simple and if I need to implement interrupts I'll have to dig into how the TRS-80 deals with them.

**Be Aware**:  I haven't built or tested my card yet (as of March 2018).  So it *probably* doesn't work as is.  We'll see as soon as 
I get finished testing the components individually with an Arduino as a kind of "hardware emulator".  I don't want to risk a 
"40 year old museum piece" while I figure out how to get the "glue" working properly.

Theory of Operation
===================

If you examine the schematic for the board, you can see that it's got a bunch of decoding logic, and some SPDT dip switches.  The theory here 
is that I'm attaching the decoding logic to the lower half of [the expansion port's address bus](http://www.classiccmp.org/cpmarchives/trs80/mirrors/kjsl/www.kjsl.com/trs80/mod1intern.html) 
and using the OUT* and IN* pins to decide what we're doing.  In this way, the port address numbers will be adjustable.  
I'm hard wiring bit zero as a toggle bit though, so really you're adjusting the 7 most significant bits, and bit zero toggles 
between related devices or related pins on a device.

TMS9118A
--------
This little VDP chip has an interesting history.  It didn't disappear with the TI-99/4A.  It got used in the Colecovision, and
then its descendants got picked up and used in the MSX computer systems.  Later, it evolved into the graphics and sound chip
used in the Sega Master System and Sega Game Gear.  It has a bank of attached DRAM memory (16k), and you access this memory through
an 8 bit data bus and a few bus flags (MODE, CSR*, and CSW*).  It also has CAS and RAS signals to keep the DRAM running.  So, *technically*, in addition to getting decent graphics and 
sound with this card, we're also giving the TRS-80 Model 1 an additional 16k of RAM, though it's only accessible through a 
tiny keyhole of two IO ports, and much of it is used up by the VDP chip itself (depending on what mode you set it to).

At the time of this writing (March 2018), I've managed to bodge up a working TMS9118A on a breadboard, and drive it with an 
Arduino.  I've uploaded the Arduino code for that, and an incredibly frightening breadboard photo that looks like something out of 
the movie Aliens.  The code doesn't do anything too spectacular, but just changes the background color, which means that 
I've successfully managed to figure out how to drive the WRITE side of the chip (writing to what the VDP calls "control 
registers", which are how you set up almost everything in the VDP).  I've also included a small video showing it doing that.

SN76489
-------
I haven't done much with the sound chips yet, but hopefully they'll be a bit easier to manage than the VDP.  Bit 0 of the 
address bus will toggle between them.  Stereo baby!

74LS244
-------
This is managing the joystick switches.  It takes 5 switches to handle up, down, left, right, and trigger.  I don't have any
kind of debounce circuit in there, so I'm guessing that'll be a problem.  We'll see when we get there.  

ExtVid
------
I don't really know if I'll do this, but it's sort of an intriguing idea.  Apparently the TMS9118A has a "video input" pin, 
believe it or not.  In principle at least, I could feed the video out of the TRS-80 to the video-input of the TMS9118A, and it
will *combine* the video together, with the TRS-80's video feed at the bottom of the stack of planes that the TMS9118A creates (which, 
by the way is how it manages sprites -- 32 of them as separate planes stacked on top of each other).  If I do this, and it 
works, it could create some interesting possibilities.  You'd still have the "feel" of a TRS-80 Model 1 with its nasty, clunky
pseudographics, but then you could overlay color and sprites over the top of it.  I guess we'll see when we get there.  The 
current (March 2018) schematics don't implement this idea.

