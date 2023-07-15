-------------------------------------------------
07/15/2023
-------------------------------------------------
This directory contains a command line demodulator tool that uses my SDR
demodulators for AM, FM, wideband FM, LSB, and USB.  This program is
called demod.cc.  Usage info can be displayed by typing: ./demod -h.

I also have provided Scilab programs that all the playing of the magnitude
of IQ samples from a data file.  The result is an animated display of the
signal in the time domain. I have a similar program for the playing of the
spectrum of a signal, although, this program needs some refinement.

Each sample of the data file is 8-bit 2's complement.  The ordering of the
data is: I1,Q1,I2,Q2....

I wrote these tools so that I could troubleshoot an occasional blanking of
demodulated audio when I was using my AM and SSB demodulators.  Originally,
I thought that I had arithmetic overflows in my decimator filters.  Once I
realized that this was not the case, I decided to examine the raw IQ data
samples.  To my surprise, I noticed that there seemed to be a "blanking" of
the IQ data.  These tools allowed me to rapidly find this issue.

I have two data files in this directory:
  f135_4.iq - A capture of IQ data at a frequency of 135.4MHz.
  yoyo.iq - A truncated version of the above data file.

To build the demodulator code, type the following:
  cd demodulators
  sh buildLibs.sh
  sh buildDemodulator.wh

A file, called demod, will be in the current directory.

I really should write makefiles for this app, before it grows tentacles.
I need to refresh my memory on using make, anyway, so now is a good time
to do that.

Chris G. 07/15/2023


