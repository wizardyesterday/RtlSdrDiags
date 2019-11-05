#!/bin/sh
#*****************************************************************************
# This build script creates a static library that can be used for creating
# RtlSdr apps.  To run this script, type ./buildWbFmDemodulatorLib.sh.
# Chris G. 07/24/2017
#*****************************************************************************

CcFiles="\
    Filters/FirFilter.cc \
    Filters/IirFilter.cc \
    Filters/Decimator.cc \
    WbFmDemodulator/WbFmDemodulator.cc"

Includes="\
    -I Filters \
    -I WbFmDemodulator"
 
# Compile string.
Compile="g++ -O3 -c $Includes $CcFiles"

# Build our library.
$Compile

# Create the archive.
ar rcs lib/libWbFmDemodulator.a *.o

# Cleanup.
rm *.o

# We're done.
exit 0

