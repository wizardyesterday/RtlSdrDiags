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

  void acceptIqData(unsigned long timeStamp,
                    unsigned char *bufferPtr,
                    unsigned long byteCount);

  void displayInternalInformation(void);

private:

  // Attributes.

  // The demodulator operating mode.
  demodulatorType demodulatorMode;

  // Demodulator support.
  AmDemodulator *amDemodulatorPtr;
  FmDemodulator *fmDemodulatorPtr;
  WbFmDemodulator *wbFmDemodulatorPtr;
  SsbDemodulator *ssbDemodulatorPtr;

};

#endif // _IQDATAPROCESSOR_H_
