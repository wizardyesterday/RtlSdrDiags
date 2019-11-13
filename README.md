# RtlSdrDiags
This code uses librtlsdr for a basis, and I have created an AM/FM/WBFM/SSB demodulator with this stuff.  The goal here was to keep it light weight so that it can run on a BeagleBone Black board.  There is no GUI, but instead, I have created a command interpreter that you access via netcat or telnet to port 20280.  You can change demodulaton modes on the fly, and you can change various radio parameters.
Let me mention a few things.  First of all, I did *not* write the librtlsdr code.  I've merely incorporated a snapshot of it into my code since it provides the foundation for my system, and I want people to be able to build the code in a standalone fashion.  Next up, I did *not* write any of the pfkutils code.  I incorporated a snapshot of Phil's code, and I specifically use his threadslinger code to provide a thread-to-thread messaging mechanism in my code.  Additionally, the libusb code is merely a copy of the header file so that I could perform experimental builds.  Finally, I didn't use makefiles, although, I don't really have anything against using make.  This occurred because I wanted to whip something up quick and dirty. The system grew though, but I still don't really have any regrets as to my decision.  The result is that the user need only have g++, and bash to build my code (and the other people's code which I have included in the system).
Here's how you build this stuff.

1. Clone the repository.
2. Navigate to the radioDiags directory.
3. Type "sh buildLibs.sh".
4. Type "buildRadioDiags.sh.

The executable will be in radioDiags/bin/radioDiags.

Now, the demodulated output is presented to stdout, and the format is 8000Samples/s, 16-bit, Little Endian PCM.  You could run the program in the following manner.

bin/radioDiags | aplay -f S16_LE -r 8000

Now that the program is running, you need to connect to the command and control link to port 20280 via netcat or telnet
(I prefer netcat).  A welcome message will appear, and if you type help, you will be presented with a help menu.  Okay, let's
say that you want to listen to a local station on the FM broadcast band.  At my location, I would type the following:

1. "start receiver"
2. "set rxfrequency 91500000"
3. "set demodmode 3"

This starts the receiver up, sets the frequency to 91.5MHz, and the demodulation mode to wideband FM.  If you type help, you'll
see how to set the other demodulator modes.  You can do other things also such as setting the demodulator gains.

What if you don't have a sound card on the computer for which the software is executing on?  For example, I use a BeagleBone
Black to run the SDR code (it's fast enough to do all the signal processing), and I use a 200MHz Pentium 2 computer to
play the sound?  Here's what I do.

On the BeagleBone Black, type the following (note that the -u argument to netcat
forces UDP usage.  If you don't have UDP support, don't use the -u argument):
1. "bin/radioDiags | netcat -u <IP address of host computer> 8000"
2. Perform steps t to 3 above to listen to your local FM station on 91.5MHz.
  
On the host computer (the one with the sound card), type the following:
1. "netcat -l -u -p 8000 | aplay -f S16_LE -r 8000"
2. On a separate shell, type "netcat <IP address of BeagleBone Black> 20280.
3. Perform steps 1 to 3 above to listen to your local FM station on 91.5Mhz.
  
Let it be noted that all demodulated output is 8000Samples/s PCM (for all demodulation modes).  I made this decision
because I wanted to be able to change demodulation modes on the fly.  Also note that steps 1 to 3 can be performed in
any order.  You can change any of the receiver parameters without having to stop and restart the receiver.  Yes, there is
a "stop receiver" command.

You'll also notice that you don't see the "PLL not locked" message when you execute my code.  It seems that alot of
software out there sets the sample rate *before* a valid frequency is entered into the system.  The frequency variable is
initialized to a value of zero, and this value is used when setting the sample rate.  The solution is to set the frequency
to something valid *before* you set the sample rate when developing code.

Now, on the BeagleBone Black, if you use the so-called async mode for USB transfers, you can see a segfault when you exit
the application.  This occurs due to the fact that the BeagleBone Black was distributed with an old version of libusb.  You
can either use a newer version of libusb to eliminate this problem if you so choose.  I chose a different approach.  I used
blocking I/O (you have that option with librtlsdr) and just have a separate reader thread to grab the incoming IQ data.  It
works well, and it works with older libusb stuff.  I'm a proponent of working with the lowest common denominator with
respect to software.

For those who are interested, here is what gets presented to the user when the help command is typed.  Note that I have
also captured what gets displayed when a connection is initially made to the command and control link to the diags
application (the welcome message).

\********************************************************************************  
## Start of help output.  
\********************************************************************************  
Welcome to System Diagnostics  
\>help  
  
set demodmode <mode: [0 (None) | 1 (AM) | 2 (FM)  
<                      | 3 (WBFM)] | 4 (LSB) | 5 (USB)>]  
set amdemodgain <gain>  
set fmdemodgain <gain>  
set wbfmdemodgain <gain>  
set ssbdemodgain <gain>  
set txgain <gain in dB>  
set rxgain <gain in dB>  
set txfrequency <frequency in Hertz>  
set rxfrequency <frequency in Hertz>  
set txbandwidth <bandwidth in Hertz>  
set rxbandwidth <bandwidth in Hertz>  
set rxsamplerate <samplerate in S/s>  
set rxwarp <warp in ppm>  
start transmitter  
stop transmitter  
start receiver  
stop receiver  
start frequencysweep <startfrequency> <stepsize> <count> <dwelltime>  
stop frequencysweep  
load iqfile <filename>  
get radioinfo  
get sweeperinfo  
exit system  
help  
Type <^B><enter> key sequence to repeat last command  
\>  
\********************************************************************************  
## End of help output.  
\********************************************************************************  

The output of the "get radioinfo" command appears next.

\********************************************************************************  
## Start of radioinfo output.  
\********************************************************************************  
\>get radioinfo  
  
\------------------------------------------------------  
Radio Internal Information  
\------------------------------------------------------  
Receive Enabled                     : No  
Receive Gain:                       : Auto  
Receive Frequency                   : 162550000 Hz  
Receive Bandwidth                   : 0 Hz  
Receive Sample Rate:                : 256000 S/s  
Receive Frequency Warp:             : 0 ppm  
Receive Timestamp                   : 0  
Receive Block Count                 : 0  
Transmit Enabled                    : No  
Transmitting Data                   : No  
Transmit Gain:                      : 0  
Transmit Frequency                  : 162550000 Hz  
Transmit Bandwidth                  : 10000000 Hz  
Transmit Sample Rate                : 256000 S/s  
Transmit Block Count                : 0  
  
\--------------------------------------------  
Data Consumer Internal Information  
\--------------------------------------------  
Last Timestamp           : 0  
Short Block Count        : 0  
  
\--------------------------------------------  
Data Provider Internal Information  
\--------------------------------------------  
Timestamp               : 0  
IQ File Name            :   
IQ Sample Buffer Length : 0  
IQ Sample Buffer Index  : 0  
  
\--------------------------------------------  
IQ Data Processor Internal Information  
\--------------------------------------------  
Demodulator Mode         : FM  
  
\--------------------------------------------  
AM Demodulator Internal Information  
\--------------------------------------------  
Demodulator Gain         : 300.000000  
  
\--------------------------------------------  
FM Demodulator Internal Information  
\--------------------------------------------  
Demodulator Gain         : 10185.916016  
  
\--------------------------------------------  
Wideband FM Demodulator Internal Information  
\--------------------------------------------  
Demodulator Gain         : 10185.916016  
  
\--------------------------------------------  
SSB Demodulator Internal Information  
\--------------------------------------------  
Demodulation Mode        : LSB  
Demodulator Gain         : 300.000000  
\>  
\********************************************************************************  
## End of radioinfo output.  
\********************************************************************************  
  
Anyway, if you have any questions, you can always catch me on freenode IRC.  I use the nick wizardyesterday or adhoc_rf_rocks.

Oh one last thing.  Anybody can use my software without grief.  I guess I'll have to put the GNU open source stuff at the
beginning of my files, and that will happen later.  Either way, feel free to use my software, sell it, or give it away.  I
believe in sharing what I create to benefit the Open Source Community.

