# RtlSdr
This code uses librtlsdr for a basis, and I have created an AM/FM/WBFM/SSB demodulator with this stuff.  The goal here was to keep it light weight so that it can run on a BeagleBone Black board.  There is no GUI, but instead, I have created a command interpreter that you access via netcat or telnet to port 20280.  You can change demodulaton modes on the fly, and you can change various radio parameters.
