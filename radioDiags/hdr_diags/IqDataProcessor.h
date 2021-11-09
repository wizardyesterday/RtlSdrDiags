//**********************************************************************
// file name: IqDataProcessor.h
//**********************************************************************

#ifndef _IQDATAPROCESSOR_H_
#define _IQDATAPROCESSOR_H_

#include <stdint.h>
#include "AmDemodulator.h"
#include "FmDemodulator.h"
#include "WbFmDemodulator.h"
#include "SsbDemodulator.h"
#include "SignalTracker.h"

class IqDataProcessor
{
  public:

  enum demodulatorType {None=0, Am=1, Fm=2, WbFm = 3, Lsb = 4, Usb = 5};

  IqDataProcessor(void);
  ~IqDataProcessor(void);

  void setDemodulatorMode(demodulatorType mode);
  void setAmDemodulator(AmDemodulator *demodulatorPtr);
  void setFmDemodulator(FmDemodulator *demodulatorPtr);
  void setWbFmDemodulator(WbFmDemodulator *demodulatorPtr);
  void setSsbDemodulator(SsbDemodulator *demodulatorPtr);
  void setSignalDetectThreshold(uint32_t threshold);

  void acceptIqData(unsigned long timeStamp,
                    unsigned char *bufferPtr,
                    unsigned long byteCount);

  void displayInternalInformation(void);

private:

  // Attributes.

  // The demodulator operating mode.
  demodulatorType demodulatorMode;

  // This is used for the squelch system.
  uint32_t signalDetectThreshold;

  // Demodulator support.
  AmDemodulator *amDemodulatorPtr;
  FmDemodulator *fmDemodulatorPtr;
  WbFmDemodulator *wbFmDemodulatorPtr;
  SsbDemodulator *ssbDemodulatorPtr;

  // Squelch support.
  SignalTracker *trackerPtr;

};

#endif // _IQDATAPROCESSOR_H_
