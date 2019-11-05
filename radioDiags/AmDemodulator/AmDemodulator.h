//**************************************************************************
// file name: AmDemodulator.h
//**************************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class implements a signal processing block known as an AM
// demodulator.  This class has a configurable demodulator gain.
///_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef __AMDEMODULATOR__
#define __AMDEMODULATOR__

#include <stdint.h>

#include "Decimator.h"
#include "IirFilter.h"

class AmDemodulator
{
  //***************************** operations **************************

  public:

  AmDemodulator(void);
  ~AmDemodulator(void);

  void resetDemodulator(void);
  void setDemodulatorGain(float gain);
  void acceptIqData(int8_t *bufferPtr,uint32_t bufferLength);
  void displayInternalInformation(void);

  private:

  //*******************************************************************
  // Utility functions.
  //*******************************************************************
  uint32_t reduceSampleRate(int8_t *bufferPtr,uint32_t bufferLength);
  uint32_t demodulateSignal(uint32_t bufferLength);
  uint32_t createPcmData(uint32_t bufferLength);
  void sendPcmData(uint32_t bufferLength);

  //*******************************************************************
  // Attributes.
  //*******************************************************************
  // This gain maps the envelope to an information signal.
  float demodulatorGain;

  // Decimated in-phase and quadrature data samples.
  float iData[4096];
  float qData[4096];

  // Demodulated data is the demodulator gain times delta theta.
  float demodulatedData[4096];

  // Demodulated data is converted into PCM data for listening.
  int16_t pcmData[512];

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // The first two decimators are used to lower the sample rate to 64000S/s
  // that is useful for AM demodulation.  Note that we have
  // tuner decimators for both, the I and the Q arm.  The third decimator
  // lowers the sample rate to 16000S/s. The fourth decimator lowers the
  // sample rate to that needed for 8000S/s PCM audio.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  Decimator *iTunerDecimatorPtr;
  Decimator *qTunerDecimatorPtr;
  Decimator *postDemodDecimatorPtr;
  Decimator *audioDecimatorPtr;

  IirFilter *dcRemovalFilterPtr;
};

#endif // __AMDEMODULATOR__
