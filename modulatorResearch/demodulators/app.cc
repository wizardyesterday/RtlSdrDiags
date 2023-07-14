#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "AmDemodulator.h"

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

//***********************************************************
// Mainline code.
//***********************************************************
int main(int argc,char **argv)
{
  bool done;
  uint32_t count;
  uint32_t i;
  AmDemodulator *demodPtr;

  demodPtr = new AmDemodulator(processPcmData);

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
      demodPtr->acceptIqData(inputBuffer,count);
    } // else
  } // while

  // Clean up resources.
  if (demodPtr != NULL)
  {
    delete demodPtr;
  } // if

  return (0);

} // main
