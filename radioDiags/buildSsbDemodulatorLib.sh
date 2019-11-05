#!/bin/sh
#*****************************************************************************
# This build script creates a static library that can be used for creating
# RtlSdr apps.  To run this script, type ./buildSsbDemodulatorLib.sh.
# Chris G. 07/05/2017
#*****************************************************************************

CcFiles="\
    Filters/Decimator.cc \
    Filters/FirFilter.cc \
    Filters/IirFilter.cc \
    SsbDemodulator/SsbDemodulator.cc"

Includes="\
    -I Filters \
    -I SsbDemodulator"
 
# Compile string.
Compile="g++ -O3 -c $Includes $CcFiles"

# Build our library.
$Compile

# Create the archive.
ar rcs lib/libSsbDemodulator.a *.o

# Cleanup.
rm *.o

# We're done.
exit 0

