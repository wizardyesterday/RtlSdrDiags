//*************************************************************************
// File name: demod.cc
//*************************************************************************

//*************************************************************************
// This program provides demodulation of raw IQ data samples that are
// provided by my radioDiags app that executes on an rtl-sdr device.  The
// expected sample rate is 256000S/s, and the data format is 8-bit two's
// complement samples that arrive in the order: I,Q,I,Q,...
// Note that the data format is also compatible with the raw IQ samples
// provided by the HackRF.  This program uses the same demodulators that
// I use on my rtl-sdr code and my HackRF code.
//
// To run this program type,
// 
//     ./demodulator> -d [1 | 2 | 3 | 4 | 5] < inputfile > outputfile
///
// where,
//
//    d - The demodulation mode. Valid values are
//    1 - AM demodulation.
//    2 - FM demodulation.
//    3 - wideband FM demodulation.
//    4 - LSB demodulation.
//    5 - USB demodulation.
//
// For example, I type something, for default AM demodulation:
//  ./demod < someFile.iq | aplay -f s16_le -r 8000
//
// For FM demodulation, you would type:
//  ./demod -d 2 < somefile.iq | aplay -f s16_le -r 8000
//
// Keep in mind that if you had an SDR sending raw IQ data over a network
// connection, you could pipe the output of netcat to thedemod program
// and demodulate live data.
//*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>

#include "AmDemodulator.h"
#include "FmDemodulator.h"
#include "WbFmDemodulator.h"
#include "SsbDemodulator.h"

struct MyParameters
{
  int *demodulatorTypePtr;
};

int8_t inputBuffer[16384];

/*****************************************************************************

  Name: processPcmData

  Purpose: The purpose of this function is to serve as the callback
  function to accept PCM data from a demodulator.

  Calling Sequence: processPcmData(bufferPtr,bufferLength).

  Inputs:

    bufferPtr - A pointer to a buffer of PCM samples.

    bufferLength - The number of PCM samples in the buffer.

  Outputs:

    None.

*****************************************************************************/
static void processPcmData(int16_t *bufferPtr,uint32_t bufferLength)
{

  // Send the PCM samples to stdout for now.
  fwrite(bufferPtr,2,bufferLength,stdout);

  return;

} // processPcmData

/*****************************************************************************

  Name: nprintf

  Purpose: The purpose of this function is to provide network printing
  capability.

  Calling Sequence: nprint(s,formatPtr,arg1, arg2..)

  Inputs:

    s - A file pointer normally used by nprintf().  This is a dummy
    argument.

    formatPtr - A pointer to a printf()-style format string.

    arg1,arg2,... - The arguments that are to be printed.

  Outputs:

    None.

*****************************************************************************/
void nprintf(FILE *s,const char *formatPtr, ...)
{
  char buffer[2048];
  va_list args;

  // set up for argument retrieval  
  va_start(args,formatPtr);

  // store the formated data to a string
  vsprintf(buffer,formatPtr,args);

  // we're done with the args
  va_end(args);

  // display the information to the user
  fprintf(stderr,buffer);

} // nprintf

/*****************************************************************************

  Name: getUserArguments

  Purpose: The purpose of this function is to retrieve the user arguments
  that were passed to the program.  Any arguments that are specified are
  set to reasonable default values.

  Calling Sequence: exitProgram = getUserArguments(parameters)

  Inputs:

    parameters - A structure that contains pointers to the user parameters.

  Outputs:

    exitProgram - A flag that indicates whether or not the program should
    be exited.  A value of true indicates to exit the program, and a value
    of false indicates that the program should not be exited..

*****************************************************************************/
bool getUserArguments(int argc,char **argv,struct MyParameters parameters)
{
  bool exitProgram;
  bool done;
  int opt;

  // Default not to exit program.
  exitProgram = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default parameters.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default to AM demodulation.
  *parameters.demodulatorTypePtr = 1;

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"d:h");

    switch (opt)
    {
      case 'd':
      {
        *parameters.demodulatorTypePtr = atoi(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./demod -d [1-AM | 2-FM | 3-WBFM | 4-LSB | 5-USB]\n");

        // Indicate that program must be exited.
        exitProgram = true;
        break;
      } // case

      case -1:
      {
        // All options consumed, so bail out.
        done = true;
      } // case
    } // switch

  } // while
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (exitProgram);

} // getUserArguments

//***********************************************************
// Mainline code.
//***********************************************************
int main(int argc,char **argv)
{
  bool done;
  bool exitProgram;
  uint32_t i;
  uint32_t count;
  int demodulatorType;
  AmDemodulator *amDemodPtr;
  FmDemodulator *fmDemodPtr;
  WbFmDemodulator *wbFmDemodPtr;
  SsbDemodulator *ssbDemodPtr;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.demodulatorTypePtr = &demodulatorType;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Instantiate the demodulators.
  amDemodPtr = new AmDemodulator(processPcmData);
  fmDemodPtr = new FmDemodulator(processPcmData);
  wbFmDemodPtr = new WbFmDemodulator(processPcmData);
  ssbDemodPtr = new SsbDemodulator(processPcmData);

  switch (demodulatorType)
  {
    case 4:
    {
      ssbDemodPtr->setLsbDemodulationMode();
    } // case

    case 5:
    {
      ssbDemodPtr->setUsbDemodulationMode();
    } // case
  } // switch

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a 64 millisecond block of input samples.
    count = fread(inputBuffer,sizeof(int8_t),16384,stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      // Demodulate the data.
      switch (demodulatorType)
      {
        case 1: // AM demodulation.
        {
          amDemodPtr->acceptIqData(inputBuffer,count);
          break;
        } // case

        case 2: // FM demodulation.
        {
          fmDemodPtr->acceptIqData(inputBuffer,count);
          break;
        } // case

        case 3: // Wideband FM demodulation.
        {
          wbFmDemodPtr->acceptIqData(inputBuffer,count);
          break;
        } // case

        case 4: // LSB demodulation.
        case 5: // USB demodulation.
        {
          ssbDemodPtr->acceptIqData(inputBuffer,count);
          break;
        } // case

        default:
        {
          break;
        } // case

      } // switch

    } // else
  } // while

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Clean up resources.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  if (amDemodPtr != NULL)
  {
    delete amDemodPtr;
  } // if

  if (fmDemodPtr != NULL)
  {
    delete fmDemodPtr;
  } // if

  if (wbFmDemodPtr != NULL)
  {
    delete wbFmDemodPtr;
  } // if

  if (ssbDemodPtr != NULL)
  {
    delete ssbDemodPtr;
  } // if
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (0);

} // main
