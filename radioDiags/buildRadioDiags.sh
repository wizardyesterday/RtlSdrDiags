#!/bin/sh
#*****************************************************************************
# This build script creates an RtlSdr app.  It assumes that all the libraries
# have been built already.  If they have not been built type ./buildLibs.sh.
# Chris G. 07/05/2017
#
# All reference to transmit functionality has been removed.
# Chris G. 10/24/2020
#*****************************************************************************
Executable="bin/radioDiags"

CcFiles="\
    src_diags/radioApp.cc \
    src_diags/DataConsumer.cc \
    src_diags/FrequencySweeper.cc \
    src_diags/IqDataProcessor.cc \
    src_diags/Radio.cc \
    src_diags/console.cc \
    src_diags/diagUi.cc"

Includes="\
    -I include \
    -I src/convenience \
    -I pfkUtils \
    -I hdr_diags \
    -I Filters \
    -I AmDemodulator \
    -I FmDemodulator \
    -I WbFmDemodulator \
    -I SsbDemodulator"
 
# Compile string.
Compile="g++ -O3 -o $Executable $Includes $CcFiles"

# Link options
LinkOptions="\
    -O3 \
    -L lib -lrtlsdr \
    -L lib -lmsgq \
    -L lib -lAmDemodulator \
    -L lib -lFmDemodulator \
    -L lib -lWbFmDemodulator \
    -L lib -lSsbDemodulator \
    -lusb-1.0 \
    -lm \
    -lpthread \
    -lrt"

# Build our application.
$Compile  $LinkOptions

# We're done.
exit 0

