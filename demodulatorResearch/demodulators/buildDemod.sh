#!/bin/sh
#*****************************************************************************
# This build script creates a demodulator app.  It assumes that all the
# libraries have been built already.  If they have not been built, type
# ./buildLibs.sh.
# Chris G. 07/14/2023
#*****************************************************************************
Executable="demod"

CcFiles="\
    demod.cc"

Includes="\
    -I Filters \
    -I Filters/Int16 \
    -I AmDemodulator \
    -I FmDemodulator \
    -I WbFmDemodulator \
    -I SsbDemodulator"
 
# Compile string.
Compile="g++ -g -O0 -o $Executable $Includes $CcFiles"

# Link options
LinkOptions="\
    -O0 \
    -L lib -lAmDemodulator \
    -L lib -lFmDemodulator \
    -L lib -lWbFmDemodulator \
    -L lib -lSsbDemodulator \
    -lm"

# Build our application.
$Compile  $LinkOptions

# We're done.
exit 0

