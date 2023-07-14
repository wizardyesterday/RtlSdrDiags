#!/bin/sh
#*****************************************************************************
# This build script creates an RtlSdr app.  It assumes that all the libraries
# have been built already.  If they have not been built type ./buildLibs.sh.
# Chris G. 07/05/2017
#
# All reference to transmit functionality has been removed.
# Chris G. 10/24/2020
#*****************************************************************************
Executable="app"

CcFiles="\
    app.cc"

Includes="\
    -I Filters \
    -I Filters/Int16 \
    -I AmDemodulator"
 
# Compile string.
Compile="g++ -g -O0 -o $Executable $Includes $CcFiles"

# Link options
LinkOptions="\
    -O0 \
    -L lib -lAmDemodulator \
    -lm"

# Build our application.
$Compile  $LinkOptions

# We're done.
exit 0

