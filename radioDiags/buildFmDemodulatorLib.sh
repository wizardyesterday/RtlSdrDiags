#!/bin/sh
#*****************************************************************************
# This build script creates a static library that can be used for creating
# RtlSdr apps.  To run this script, type ./buildFmDemodulatorLib.sh.
# Chris G. 06/29/2017
#*****************************************************************************

CcFiles="\
    Filters/Decimator.cc \
    FmDemodulator/FmDemodulator.cc"

Includes="\
    -I Filters \
    -I FmDemodulator"
 
# Compile string.
Compile="g++ -O3 -c $Includes $CcFiles"

# Build our library.
$Compile

# Create the archive.
ar rcs lib/libFmDemodulator.a *.o

# Cleanup.
rm *.o

# We're done.
exit 0

