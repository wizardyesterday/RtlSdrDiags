//************************************************************************
// file name: FmDemodulator.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "FmDemodulator.h"

using namespace std;

// These coefficients are used for decimation by 4.
static float tunerDecimatorCoefficients[] =
{
  -0.0380330,
   0.0012216,
   0.0077779,
   0.0148656,
   0.0183034,
   0.0153173,
   0.0060915,
  -0.0055089,
  -0.0139925,
  -0.0143264,
  -0.0050115,
   0.0105109,
   0.0247893,
   0.0296140,
   0.0199914,
  -0.0025057,
  -0.0290992,
  -0.0463704,
  -0.0413861,
  -0.0073844,
   0.0523243,
   0.1242772,
   0.1888493,
   0.2269707,
   0.2269707,
   0.1888493,
   0.1242772,
   0.0523243,
  -0.0073844,
  -0.0413861,
  -0.0463704,
  -0.0290992,
  -0.0025057,
   0.0199914,
   0.0296140,
   0.0247893,
   0.0105109,
  -0.0050115,
  -0.0143264,
  -0.0139925,
  -0.0055089,
   0.0060915,
   0.0153173,
   0.0183034,
   0.0148656,
   0.0077779,
   0.0012216,
  -0.0380330  
};

// These coefficients are used for decimation by 4.
static float postDemodDecimatorCoefficients[] =
{
  -0.0266961,
  -0.0120813,
   0.0278621,
   0.1015842,
   0.1847865,
   0.2402400,
   0.2402400,
   0.1847865,
   0.1015842,
   0.0278621,
  -0.0120813,
  -0.0266961
};

// These coefficients are used for decimation by 2.
static float audioDecimatorCoefficients[] =
{
  -0.0011405,
   0.0183372,
   0.0030542,
  -0.0100052,
  -0.0059350,
   0.0115377,
   0.0109293,
  -0.0120883,
  -0.0175779,
   0.0110390,
   0.0262645,
  -0.0074772,
  -0.0377408,
  -0.0003152,
   0.0541009,
   0.0165897,
  -0.0829085,
  -0.0587608,
   0.1736804,
   0.4222137,
   0.4222137,
   0.1736804,
  -0.0587608,
  -0.0829085,
   0.0165897,
   0.0541009,
  -0.0003152,
  -0.0377408,
  -0.0074772,
   0.0262645,
   0.0110390,
  -0.0175779,
  -0.0120883,
   0.0109293,
   0.0115377,
  -0.0059350,
  -0.0100052,
   0.0030542,
   0.0183372,
  -0.0011405
};

extern void nprintf(FILE *s,const char *formatPtr, ...);

/*****************************************************************************

  Name: FmDemodulator

  Purpose: The purpose of this function is to serve as the contructor for
  an instance of an FmDemodulator.

  Calling Sequence: FmDemodulator()

  Inputs:

    None.

 Outputs:

    None.

*****************************************************************************/
FmDemodulator::FmDemodulator(void)
{
  int numberOfTunerDecimatorTaps;
  int numberOfPostDemodDecimatorTaps;
  int numberOfAudioDecimatorTaps;

  // Set to a nominal gain.
  demodulatorGain = 64000/(2 * M_PI);

  numberOfTunerDecimatorTaps =
      sizeof(tunerDecimatorCoefficients) / sizeof(float);

  numberOfPostDemodDecimatorTaps =
      sizeof(postDemodDecimatorCoefficients) / sizeof(float);

  numberOfAudioDecimatorTaps =
    sizeof(audioDecimatorCoefficients) / sizeof(float);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This pair of decimators reduce the sample rate from 256000S/s to
  // 64000S/s for use by the FM demodulator.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Allocate the decimator for the in-phase component.
  iTunerDecimatorPtr = new Decimator(numberOfTunerDecimatorTaps,
                                     tunerDecimatorCoefficients,
                                     4);

  // Allocate the decimator for the quadrature component.
  qTunerDecimatorPtr = new Decimator(numberOfTunerDecimatorTaps,
                                     tunerDecimatorCoefficients,
                                     4);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // The decimator reduces the sample rate of the demodulated data to
  // 16000S/s.  The output of this decimator is used by the next
  // decimator state.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Allocate the post demodulator decimator.
  postDemodDecimatorPtr = new Decimator(numberOfPostDemodDecimatorTaps,
                                        postDemodDecimatorCoefficients,
                                        4);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This decimator has an input sample rate of 16000S/s and an
  // output sample rate of 8000S/s.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Allocate the audio decimator.
  audioDecimatorPtr = new Decimator(numberOfAudioDecimatorTaps,
                                    audioDecimatorCoefficients,
                                    2);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Initial phase angle for d(theta)/dt computation.
  previousTheta = 0;

  return;

} // FmDemodulator

/*****************************************************************************

  Name: ~FmDemodulator

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of an FmDemodulator.

  Calling Sequence: ~FmDemodulator()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
FmDemodulator::~FmDemodulator(void)
{

  // Release resources.
  delete iTunerDecimatorPtr;
  delete qTunerDecimatorPtr;
  delete postDemodDecimatorPtr;
  delete audioDecimatorPtr;

  return;

} // ~FmDemodulator

/*****************************************************************************

  Name: resetDemodulator

  Purpose: The purpose of this function is to reset the demodulator to its
  initial condition.

  Calling Sequence: resetDemodulator()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void FmDemodulator::resetDemodulator(void)
{

  // Reset all filter structures to their initial conditions.
  iTunerDecimatorPtr->resetFilterState();
  qTunerDecimatorPtr->resetFilterState();
  postDemodDecimatorPtr->resetFilterState();
  audioDecimatorPtr->resetFilterState();

  // Initial phase angle for d(theta)/dt computation.
  previousTheta = 0;

  return;

} // resetDemodulator

/*****************************************************************************

  Name: setDemodulatorGain

  Purpose: The purpose of this function is to set the gain of the
  demodulator.  This gain is used to convert the phase differences of the
  signal values into numbers that are later converted into PCM values.

  Calling Sequence: setDemodulatorGain(gain)

  Inputs:

    gain - The demodulator gain.

  Outputs:

    None.

*****************************************************************************/
void FmDemodulator::setDemodulatorGain(float gain)
{

  this->demodulatorGain = gain;

  return;

} // setDemodulatorGain

/*****************************************************************************

  Name: acceptIqData

  Purpose: The purpose of this function is to perform the high level
  processing that is associated with the demodulation of a signal.

  Calling Sequence: acceptIqData(bufferPtr,bufferLength)

  Inputs:

    bufferPtr - A pointer to IQ data.

    bufferLength - The number of bytes contained in the buffer that is
    in the buffer.

  Outputs:

    None.

*****************************************************************************/
void FmDemodulator::acceptIqData(int8_t *bufferPtr,uint32_t bufferLength)
{
  uint32_t sampleCount;

  // Decimate the signal to the demodulator sample rate.
  sampleCount = reduceSampleRate(bufferPtr,bufferLength);

  // Demodulate the decimated signal.
  sampleCount = demodulateSignal(sampleCount);

  // Create the PCM data stream.
  sampleCount = createPcmData(sampleCount);

  // Send the PCM data to the information sink.
  sendPcmData(sampleCount);

  return;

} // acceptIqData

/*****************************************************************************

  Name: reduceSampleRate

  Purpose: The purpose of this function is to accept interleaved IQ data
  and decimate the data by a factor of 4.  The decimated data will be stored
  in the iData[] and qData[] arrays.

  Calling Sequence: sampleCount = reduceSampleRate(bufferPtr,bufferLength)

  Inputs:

    bufferPtr - A pointer to IQ data.

    bufferLength - The number of bytes contained in the buffer.

  Outputs:

    sampleCount - The number of samples stored in the iData[] and qData[]
    arrays.

*****************************************************************************/
uint32_t FmDemodulator::reduceSampleRate(
    int8_t *bufferPtr,
    uint32_t bufferLength)
{
  uint32_t i;
  uint32_t outputBufferIndex;
  bool sampleAvailable;
  float sample;

  // Set to reference the beginning of the in-phase output buffer.
  outputBufferIndex = 0;

  // Decimate the in-phase samples.
  for (i = 0; i < bufferLength; i += 2)
  {
    sampleAvailable = iTunerDecimatorPtr->decimate((float)bufferPtr[i],
                                                   &sample);
    if (sampleAvailable)
    {
      // Store the decimated sample.
      iData[outputBufferIndex] = sample;

      // Reference the next storage location.
      outputBufferIndex++;
    } // if
  } // for

  // Set to reference the beginning of the quadrature output buffer.
  outputBufferIndex = 0;

  // Decimate the quadrature samples.
  for (i = 1; i < (bufferLength + 1); i += 2)
  {
    sampleAvailable = qTunerDecimatorPtr->decimate((float)bufferPtr[i],
                                                   &sample);
    if (sampleAvailable)
    {
      // Store the decimated sample.
      qData[outputBufferIndex] = sample;

      // Reference the next storage location.
      outputBufferIndex++;
    } // if
  } // for

  return (outputBufferIndex);

} // reduceSampleRate

/*****************************************************************************

  Name: demodulateSignal

  Purpose: The purpose of this function is to demodulate an FM signal that
  is stored in the iData[] and qData[] arrays. The signal processing that
  is performed is now described.  First, the phase angle of the signal is
  computed by the equation: theta(n) = atan[Q(n) / i(n)].  Next, the
  phase angle of the previous IQ sample is subtracted from theta(n).  This
  approximates the derivative of the phase angle with respect to time.
  Note that omega(n) = dtheta(n)/dn represents the instantaneous frequency
  of the signal.  Due to the branch cut of the atan() function at PI
  radians, processing is performed to ensure that the phase difference
  satisfies the equation, -PI < dtheta < PI.  Finally, the demodulated
  signal is multiplied by the demodulator gain, and the result is a
  scaled demodulated signal.

  Calling Sequence: sampleCount = demodulateSignal(bufferLength)

  Inputs:

    bufferLength - The number of entries contained iData[] and qData buffers.

  Outputs:

    sampleCount - The number of samples stored in the demodulatedData[]
    array.

*****************************************************************************/
uint32_t FmDemodulator::demodulateSignal(uint32_t bufferLength)
{
  uint32_t i;
  float theta;
  float deltaTheta;

  for (i = 0; i < bufferLength; i++)
  {
    // Kludge to prevent 0/0 condition for the atan2() function.
    qData[i] = qData[i] + 1e-10;

    // Compute phase angle.
    theta = atan2((double)qData[i],(double)iData[i]);

    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // Compute d(theta)/dt.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    deltaTheta = theta - previousTheta;

    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // We want -M_PI <= deltaTheta <= M_PI.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    while (deltaTheta > M_PI)
    {
      deltaTheta -= (2 * M_PI);
    } // while

    while (deltaTheta < (-M_PI))
    {
      deltaTheta += (2 * M_PI);
    } // while
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    // Store the demodulated data.
    demodulatedData[i] = demodulatorGain * deltaTheta;

    // Update our last phase angle for the next iteration.
    previousTheta = theta;

  } // for

  return (bufferLength);

} // demodulateSignal

/*****************************************************************************

  Name: createPcmData

  Purpose: The purpose of this function is to convert the signal, contained
  in demodulatedData[], into 16-bit little endian PCM data.

  Calling Sequence: createPcmData(bufferLength)

  Inputs:

    bufferLength - The number of entries contained in the demodulatedData[]
    buffer.

  Outputs:

    sampleCount - The number of samples stored in the pcmData[]
    array.

*****************************************************************************/
uint32_t FmDemodulator::createPcmData(uint32_t bufferLength)
{
  uint32_t i;
  uint32_t outputBufferIndex;
  bool sampleAvailable;
  float sample;

  // Reference the beginning of the PCM output buffer.
  outputBufferIndex = 0;

  for (i = 0; i < bufferLength; i++)
  {
    // Perform the first stage of decimation.
    sampleAvailable = postDemodDecimatorPtr->decimate(demodulatedData[i],
                                                      &sample);
    if (sampleAvailable)
    {
      // Perform the second stage of decimation.
      sampleAvailable = audioDecimatorPtr->decimate(sample,&sample);

      if (sampleAvailable)
      {
        // Store PCM sample.
        pcmData[outputBufferIndex] = (int16_t)sample;

        // Reference the next storage location.
        outputBufferIndex++;
      } // if
    } // if

  } // for

  return (outputBufferIndex);

} // createPcmData

/*****************************************************************************

  Name: sendPcmData

  Purpose: The purpose of this function is to send the PCM data to the
  appropriate information sink.

  Calling Sequence: sendPcmData(bufferLength)

  Inputs:

    bufferLength - The number of entries contained in the pcmData[] buffer.

  Outputs:

    None.

*****************************************************************************/
void FmDemodulator::sendPcmData(uint32_t bufferLength)
{
  uint32_t i;

  // Send the PCM samples to stdout for now.
  fwrite(pcmData,2,bufferLength,stdout);

  return;

} // sendPcmData

/**************************************************************************

  Name: displayInternalInformation

  Purpose: The purpose of this function is to display internal information
  in the FM demodulator.

  Calling Sequence: displayInternalInformation()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void FmDemodulator::displayInternalInformation(void)
{

  nprintf(stderr,"\n--------------------------------------------\n");
  nprintf(stderr,"FM Demodulator Internal Information\n");
  nprintf(stderr,"--------------------------------------------\n");

  nprintf(stderr,"Demodulator Gain         : %f\n",demodulatorGain);

  return;

} // displayInternalInformation


