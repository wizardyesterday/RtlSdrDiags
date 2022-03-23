This directory contains files that illustrate the transient response of the
R820T2 device when subjected to a register write the the VGA gain register.
I have two reasons why I am concerned about this behavior of the device.
1. The transient is not the most enjoyable to hear in demodulated audio.
2. The transient can cause instability in an AGC loop if one is not careful.

The transient becomes audible only under certain signal conditions, and
most of the time it will not be audible.  No instability has been observed
in either my my AGC systems due to the deband (settable) that is used and
the time constant (also settable) that is used.  I'll continue testing
this sytem with different stimulii to see if I can cause failure.

The files with a .raw extension are PCM data of the format: 16-bit signed,
little endian integers. These files can be played by your favorite raw
audio player such as mplayer.  To play a file, type:

  aplayer -f S16_LE -r 8000.

The files with a .pdf extension are plots of the transient response of the
R820T2 chip.

I have also copied a paper, by Harris and Smith, that describes one of the
AGC algorithms that I partially implemented.

***********************************************
03/23/2022
***********************************************
I believe that the transients occur when the IIC repeater is enabled (and
possibly disabed) in the Realtek 2832U chip. I performed two experiments:

1. Modify my set_if_gain() code in the tuner_r82xx.c file so that the VGA
gain is not updated.  That is, no IIC traffic did not occur.  After groking
the code in librtlsdr.c, I saw that the repeater was enabled and disabled
in the rtlsdr_set_tuner_if_gain() function.

2. I then rolled out my hacks in the tuner code and modified the librtlsdr.c
code so that the repeater was only enabled once in the
rtlsdr_set_tuner_if_gain() function, while allowing the tuner VGA gain
register to be updated.  The transients then disappeared.

I believe that when the IIC repeater is enabled, voltage/current spikes may
be getting picked up by the tuner chip and perhaps amplified.  Further
investigation needs to be performed.

