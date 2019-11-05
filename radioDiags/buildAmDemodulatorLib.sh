#!/bin/sh
#*****************************************************************************
# This build script creates a static library that can be used for creating
# RtlSdr apps.  To run this script, type ./buildAmDemodulatorLib.sh.
# Chris G. 07/05/2017
#*****************************************************************************

CcFiles="\
    Filters/Decimator.cc \
    Filters/FirFilter.cc \
    Filters/IirFilter.cc \
    AmDemodulator/AmDemodulator.cc"

Includes="\
    -I Filters \
    -I AmDemodulator"
 
# Compile string.
Compile="g++ -O3 -c $Includes $CcFiles"

# Build our library.
$Compile

# Create the archive.
ar rcs lib/libAmDemodulator.a *.o

# Cleanup.
rm *.o

# We're done.
exit 0

