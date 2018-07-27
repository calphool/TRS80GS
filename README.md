# TRS80GS
Storage area for my TRS-80 Model 1 Graphics, Sound, Gaming, and Communication cartidge

![Version 3 of PCB](/img/Darth_Video.png?raw=true "Version 4 of PCB")


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

**Be Aware**:  ~~I haven't built or tested my card yet (as of March 2018).  So it *probably* doesn't work as is.  We'll see as soon as 
I get finished testing the components individually with an Arduino as a kind of "hardware emulator".  I don't want to risk a 
"40 year old museum piece" while I figure out how to get the "glue" working properly.~~  I *have* now built a working prototype.  
You'll find the eaglecad and such in the rev2 folder (though you'd be better off using the rev4 version).  There's also some source code to test it once you get the board built.  

**Update**:  The photo is of the Rev4 version of this card.  It's fully working, and better than the rev2 and rev3.

Theory of Operation
===================

~~If you examine the *original* schematic for the board, you can see that it's got a bunch of decoding logic, and some SPDT dip switches.  The theory here 
is that I'm attaching the decoding logic to the lower half of [the expansion port's address bus](http://www.classiccmp.org/cpmarchives/trs80/mirrors/kjsl/www.kjsl.com/trs80/mod1intern.html) 
and using the OUT* and IN* pins to decide what we're doing.  In this way, the port address numbers will be adjustable.  
I'm hard wiring bit zero as a toggle bit though, so really you're adjusting the 7 most significant bits, and bit zero toggles 
between related devices or related pins on a device.~~

Strike all that... I pivoted on the design by rev2.  We got rid of silly things like glue logic chips and SPDT dip switches, and simply added 
a [CPLD](https://www.microchip.com/wwwproducts/en/ATF22V10C) that manages the address decoding and control signal generation.  The code for the CPLD is now in the repo.  So you 
"adjust" the addresses in code now, not with SPDT dip switches (what was I thinking?)  You program the CPLD with a Stager 4800 programmer, and a very particular version of 
its software (VSpeed 5.0).  Details are in the .PLD file in the repo.

Rev3 includes some addtions.  First, it fixes the sound problem that the rev2 board had so we get both sound chips working (all 6 audio channels and 2 noise channels).  Second, it includes
a [32k SRAM](https://www.mouser.com/ProductDetail/511-M48Z35Y70PC1).  The original expansion system for the TRS-80 used DRAMs, but SRAM actually ends up being much better, especially when you use a non-volatile one like I have.  The Rev3 board 
also cleans up the joystick bodges that I had to do on the Rev2 board.  Rev3 works fully, and I've been building a Pacman game on it.

~~Rev4 is currently (July 2018) being built by our Chinese friends at allpcb.com, and it~~ is an upgraded version of Rev3 with one major improvement -- a [UART](https://en.wikipedia.org/wiki/16550_UART).  I 
~~*should* be~~ am able to connect a [FTDI cable](http://www.ftdichip.com/Products/Cables/USBTTLSerial.htm) via USB from my Mac to this UART and I should be able to transfer binary data back and forth.  
Initially I'll just use it to move my Pacman code back and forth, but in truth with a fully functional UART, I could build a kind of network interface and create something loosely equivalent to a LAN share, 
which would be sort of cool.  In fact, I *could* probably build a SLIP interface and do stuff like surf the web or use FTP from a 40 year old TRS-80 Model 1, but we'll save all that software work for a later date.

The only trouble with having a UART is that now I have *two* interrupts to deal with, so you won't be able to simultaneously use the VPD's interrupts *and* the UART's interrupts.  I've included
a jumper to allow you to switch which one you're working with.  I *suppose* I could add a latch behind an address so that you could tell which interrupt was being invoked, but honestly I 
can't imagine why we need to be both loading data *and* altering the screen at the same time, so Rev4 will use a jumper.  I think I'll start out without coding any interrupt stuff and see what I can get away with
using plain old polling.  It'll be slower, but simpler.  Once I get it working, then I'll look into trickier stuff like interrupt handling in z88dk.

Rev4 is now here and "working" though it took a little bit of hacking to get it up and running.  The original design for 
Rev4 included a separate crystal for the UART (1.8432mhz), but that proved problematic.  It caused the VDP crystal to 
act flaky due to interference.  So, I decided to try removing its crystal and all its support caps and resistors, and just 
drive the XTAL in for the UART from the CPUCLK output of the VDP.  It's the wrong speed (3.5mhz instead of 1.8432mhz), but 
luckily the UART has a divider value that you plug into a register during setup that allows it to use any chip slower than
16mhz.  I coded a couple of little test programs, one for the MAC (the "server") and one for the TRS-80 (the "client").  I
was able to transfer data back and forth, but I'm reaching out to folks on reddit to see if there's a better way to code 
it, since I have an arbitrary 8 millisecond wait on each byte transferred because I don't understand hardware flow control.

TMS9118A
--------
This little VDP chip has an interesting history.  It didn't disappear with the TI-99/4A.  It got used in the Colecovision, and
then its descendants got picked up and used in the MSX computer systems.  Later, it evolved into the graphics and sound chip
used in the Sega Master System and Sega Game Gear.  It has a bank of attached DRAM memory (16k), and you access this memory through
an 8 bit data bus and a few bus flags (MODE, CSR*, and CSW*).  It also has CAS and RAS signals to keep the DRAM running.  So, *technically*, in addition to getting decent graphics and 
sound with this card, we're also giving the TRS-80 Model 1 an additional 16k of RAM, though it's only accessible through a 
tiny keyhole of two IO ports, and much of it is used up by the VDP chip itself (depending on what mode you set it to).

~~At the time of this writing (March 2018), I've managed to bodge up a working TMS9118A on a breadboard, and drive it with an 
Arduino.  I've uploaded the Arduino code for that, and an incredibly frightening breadboard photo that looks like something out of 
the movie Aliens.  The code doesn't do anything too spectacular, but just changes the background color, which means that 
I've successfully managed to figure out how to drive the WRITE side of the chip (writing to what the VDP calls "control 
registers", which are how you set up almost everything in the VDP).  I've also included a small video showing it doing that.~~

We're way past that stage now.  The TMS9118 works like a dream, and I've been building a Pacman clone against it, using the z88dk C compiler.  So far I'm fitting inside my nice 32k RAM addition
just fine.  There's a lot of fluff in my code, and I'm pretty sure I could get it crammed down at least 25% if I worked at it.  Right now I've got an animated pacman that can move around inside
the maze, and I've got the intro music working.  Ghosts and dots coming soon....

SN76489
-------
~~I haven't done much with the sound chips yet, but hopefully they'll be a bit easier to manage than the VDP.  Bit 0 of the 
address bus will toggle between them.  Stereo baby!~~

Sound didn't work quite right in the rev2 board design.  There's a "Ready" line on the SN76489 that needed to be tied to the WAIT line on the TRS-80 interface.  Why?  Because the SN76489 doesn't
like receiving a second byte when it's still processing the first one.  I *could* write all my code to put a delay inbetween each byte send, but that's kind of lame and non-deterministic.  The
WAIT line was originally conceived so that the TRS-80 could interface to "slow" peripherals, and although the sound chip isn't *super* slow, it seemed the way to go was to tie that READY line 
to it.  Unfortunately on the rev2 board that meant that I couldn't use *both* sound chips, because I didn't have a free OR gate anywhere to run the two Ready lines through.  So, I pulled one of them
and just wired the other one up on the rev2 board.  It worked as expected.  This has all been cleaned up in the rev3 and rev4 boards.

74LS244
-------
This is managing the joystick switches.  It takes 5 switches to handle up, down, left, right, and trigger.  ~~I don't have any
kind of debounce circuit in there, so I'm guessing that'll be a problem.  We'll see when we get there.~~

Joystick circuits worked more or less flawlessly, though I had stuff wired backward on the PCB for rev2.  I had to bodge some jumper wires to get them working because of that, but that's all cleaned 
up in Rev3 and Rev4.  I originally was toying with the idea of building some kind of one-way parallel interface that used the joystick ports, but the more I thought about it the less I liked it and
decided to add the UART instead.


ExtVid
------
I don't really know if I'll do this, but it's sort of an intriguing idea.  Apparently the TMS9118A has a "video input" pin, 
believe it or not.  In principle at least, I could feed the video out of the TRS-80 to the video-input of the TMS9118A, and it
will *combine* the video together, with the TRS-80's video feed at the bottom of the stack of planes that the TMS9118A creates (which, 
by the way is how it manages sprites -- 32 of them as separate planes stacked on top of each other).  If I do this, and it 
works, it could create some interesting possibilities.  You'd still have the "feel" of a TRS-80 Model 1 with its nasty, clunky
pseudographics, but then you could overlay color and sprites over the top of it.  I guess we'll see when we get there.  The 
current (March 2018) schematics don't implement this idea.

(July 2018) I haven't done anything with ExtVid yet.  I've come to realize that it's unlikely to just work out of the box, because the 
RESET line on the TMS9118A does double duty as some kind of video sync signal.  So I'd probably have to come up with a sync separator 
circuit for the ExtVid line, then amplify that signal up to whatever ungodly voltage causes the RESET to act like a SYNC signal.  I probably should just
hook up a video signal to see how bad it tears as it moves down the screen -- maybe it's still usable without all that extra circuitry -- just haven't 
tried it yet.
