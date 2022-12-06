//**************************************************************************
// file name: IqDataProcessor.cc
//**************************************************************************

#include <stdio.h>
#include "IqDataProcessor.h"

extern void nprintf(FILE *s,const char *formatPtr, ...);

/**************************************************************************

  Name: IqDataProcessor

  Purpose: The purpose of this function is to serve as the constructor
  of an IqDataProcessor object.

  Calling Sequence: IqDataProcessor()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
IqDataProcessor::IqDataProcessor(void)
{

  // Default to no demodulation of the signal
  demodulatorMode = None;

  // Let all signal exceed threshold.
  signalDetectThreshold = -200;

  // Instantiate a signal tracker.
  trackerPtr = new SignalTracker(signalDetectThreshold);

  // Default to no notification of signal state.p
  signalNotificationEnabled = false;

  // Default to no callback registered.
  signalCallbackPtr = NULL;

  // Default to no notification of signal state.p
  signalMagnitudeNotificationEnabled = false;

  // Default to no callback registered.
  signalMagnitudeCallbackPtr = NULL;

  return; 

} // IqDataProcessor

/**************************************************************************

  Name: ~IqDataProcessor

  Purpose: The purpose of this function is to serve as the destructor 
  of an IqDataProcessor object.

  Calling Sequence: ~IqDataProcessor()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
IqDataProcessor::~IqDataProcessor()
{

  // Release resources.
  delete trackerPtr;

  return; 

} // ~IqDataProcessor

/**************************************************************************

  Name: setAmDemodulator

  Purpose: The purpose of this function is to associate an instance of
  an AM demodulator object with this object.

  Calling Sequence: setAmDemodulator(demodulatorPtr)

  Inputs:

    demodulatorPtr - A pointer to an instance of an FmDemodulator object.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::setAmDemodulator(AmDemodulator *demodulatorPtr)
{

  // Save for future use.
  amDemodulatorPtr = demodulatorPtr;

  return;

} // setAmDemodulator

/**************************************************************************

  Name: setFmDemodulator

  Purpose: The purpose of this function is to associate an instance of
  an FM demodulator object with this object.

  Calling Sequence: setFmDemodulator(demodulatorPtr)

  Inputs:

    demodulatorPtr - A pointer to an instance of an FmDemodulator object.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::setFmDemodulator(FmDemodulator *demodulatorPtr)
{

  // Save for future use.
  fmDemodulatorPtr = demodulatorPtr;

  return;

} // setFmDemodulator

/**************************************************************************

  Name: setWbFmDemodulator

  Purpose: The purpose of this function is to associate an instance of
  a wideband FM demodulator object with this object.

  Calling Sequence: setWbFmDemodulator(demodulatorPtr)

  Inputs:

    demodulatorPtr - A pointer to an instance of a WbFmDemodulator object.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::setWbFmDemodulator(WbFmDemodulator *demodulatorPtr)
{

  // Save for future use.
  wbFmDemodulatorPtr = demodulatorPtr;

  return;

} // setWbFmDemodulator

/**************************************************************************

  Name: setSsbDemodulator

  Purpose: The purpose of this function is to associate an instance of
  a SSB demodulator object with this object.

  Calling Sequence: setSsbDemodulator(demodulatorPtr)

  Inputs:

    demodulatorPtr - A pointer to an instance of a SsbDemodulator object.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::setSsbDemodulator(SsbDemodulator *demodulatorPtr)
{

  // Save for future use.
  ssbDemodulatorPtr = demodulatorPtr;

  return;

} // setSsbDemodulator

/**************************************************************************

  Name: setDemodulatorMode

  Purpose: The purpose of this function is to set the demodulator mode.
  This mode dictates which demodulator should be used when processing IQ
  data samples.

  Calling Sequence: setDemodulatorMode(mode)

  Inputs:

    mode - The demodulator mode.  Valid values are None, Am, and Fm,
    WbFm, Lsb, and Usb.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::setDemodulatorMode(demodulatorType mode)
{

  // Save for future use.
  this->demodulatorMode = mode;

  switch (mode)
  {
    case Lsb:
    {
      // Set to demodulate lower sideband signals.
      ssbDemodulatorPtr->setLsbDemodulationMode();
      break;
    } // case

    case Usb:
    {
      // Set to demodulate upper sideband signals.
      ssbDemodulatorPtr->setUsbDemodulationMode();
      break;
    } // case

  } // switch

  return;

} // setDemodulatorMode

/**************************************************************************

  Name: setSignalDetectThreshold

  Purpose: The purpose of this function is to set the signal detect
  threshold.  A signal is considered as detected if a signal magnitude
  matches or exceeds the threshold value.

  Calling Sequence: success = setSignalDetectThreshold(threshold)

  Inputs:

    threshold - The signal detection threshold in decibels referenced
    to full scale.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::setSignalDetectThreshold(int32_t threshold)
{

  // Update for later use.
  this->signalDetectThreshold = threshold;

  // Notify the signal tracker of the new threshold.
  trackerPtr->setThreshold(threshold);

  return;

} // setSignalDetectThreshold

/**************************************************************************

  Name: enableSignalNotification

  Purpose: The purpose of this function is to allow notification when
  a new block of IQ data arrives.

  Calling Sequence: enableSignalNotification()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::enableSignalNotification(void)
{

  signalNotificationEnabled = true;

  return;

} // enableSignalNotification

/**************************************************************************

  Name: disableSignalNotification

  Purpose: The purpose of this function is to disallow notification when
  a new block of IQ data arrives.

  Calling Sequence: disableSignalNotification()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::disableSignalNotification(void)
{

  signalNotificationEnabled = false;

  return;

} // disableSignalNotification

/**************************************************************************

  Name: registerSignalStateCallback

  Purpose: The purpose of this function is to register a callback function
  that will be invoked whenever a block of IQ data is received.

  Calling Sequence: registerSignalStateCallback(callbackPtr,contextPtr)

  Inputs:

    callbackPtr - A pointer to a callback function

    contextPtr - A pointer to private data that will be passed to the
    callback function upon invocation.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::registerSignalStateCallback(
    void (*signalCallbackPtr)(bool signalPresent,void *contextPtr),
    void *contextPtr)
{

  // Save for later use.
  signalCallbackContextPtr = contextPtr;
  this->signalCallbackPtr = signalCallbackPtr;

} // registerSignalStateCallback

/**************************************************************************

  Name: enableSignalMagnitudeNotification

  Purpose: The purpose of this function is to allow notification of
  the average signal magnitude when a new block of IQ data arrives.

  Calling Sequence: enableSignalMagnitudeNotification()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::enableSignalMagnitudeNotification(void)
{

  signalMagnitudeNotificationEnabled = true;

  return;

} // enableSignalMagnitudeNotification


/**************************************************************************

  Name: disableSignalMagnitudeNotification

  Purpose: The purpose of this function is to disallow notification of
  signal magnitude data when a new block of IQ data arrives.

  Calling Sequence: disableSignalMagnitudeNotification()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::disableSignalMagnitudeNotification(void)
{

  signalMagnitudeNotificationEnabled = false;

  return;

} // disableSignalMagnitudeNotification

/**************************************************************************

  Name: registerSignalMagnitudeCallback

  Purpose: The purpose of this function is to register a callback function
  that will be invoked with signal magnitude information whenever a block
  of IQ data is received.

  Calling Sequence: registerSignalMagnitudeCallback(callbackPtr,contextPtr)

  Inputs:

    callbackPtr - A pointer to a callback function

    contextPtr - A pointer to private data that will be passed to the
    callback function upon invocation.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::registerSignalMagnitudeCallback(
    void (*callbackPtr)(uint32_t signalMagnitude,void *contextPtr),
    void *contextPtr)
{

  // Save for later use.
  signalMagnitudeCallbackContextPtr = contextPtr;
  this->signalMagnitudeCallbackPtr = callbackPtr;

} // registerSignalStateCallback

/**************************************************************************

  Name: downconvertByFsOver4

  Purpose: The purpose of this function is to downconvert an IQ
  data stream by Fs/4.  The equations for this function originated
  from "Understanding Digital Signal Processing, Third Edition" by
  Richard G. Lyons.  Section 13.1.2 Frequency Translation by -Fs/4,
  explains how this all works.  Equations (13-2) portray downconversion,
  and Equations (13-3) portray upconversion.

  Calling Sequence: downconvertByFsOver4(bufferPtr,byteCount)

  Inputs:

    bufferPtr - A pointer to signed IQ data.

    byteCount - The number of bytes contained in the buffer that is
    in the buffer.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::downconvertByFsOver4(
  int8_t *bufferPtr,
  uint32_t byteCount)
{
  uint32_t i;
  int8_t x, y;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This block of code translates the spectrum by -Fs/4.
  // Here's how it works.  The IQ samples are multiplied by
  // exp(j*PI/2) = {1,-j1,-1,j1,...}. Note that this sequence
  // repeats modulo 4.  Consider that z(n) = x(n) + jy(n).
  // The first 4 values of the translated spectrum are,
  //
  //    znew(0) =1[x(0) + jy(0)] = x(0) + jy(0).
  //    znew(1) = -j[(x(1) + jy(1)] = y(1) - jx(1).
  //    znew(2) = -1[x(2) + jy(2)] = -x(2) -jy(2).
  //    znew(3) = j[x(3) + jy(3)] = -y(3) + jx(3).
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  for (i = 0; i < byteCount; i += 8)
  {
    // znew(0) = x(0) + jy(0).
    // No processing needs to be done.

    // znew(1) = y(1) - jx(1).
    x = bufferPtr[i + 2];
    y = bufferPtr[i + 3];
    bufferPtr[i + 2] = y;
    bufferPtr[i + 3] = -x;  

    // znew(2) = -x(2) - jy(2).
    x = bufferPtr[i + 4];
    y = bufferPtr[i + 5];
    bufferPtr[i + 4] = -x;
    bufferPtr[i + 5] = -y;

    // znew(3) = -y(3) + jx(3).
    x = bufferPtr[i + 6];
    y = bufferPtr[i + 7];
    bufferPtr[i + 6] = -y;
    bufferPtr[i + 7] = x;
  } // for
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

} // downconvertByFsOver4

/**************************************************************************

  Name: upconvertByFsOver4

  Purpose: The purpose of this function is to upconvert an IQ
  data stream by Fs/4.  The equations for this function originated
  from "Understanding Digital Signal Processing, Third Edition" by
  Richard G. Lyons.  Section 13.1.2 Frequency Translation by Fs/4,
  explains how this all works.  Equations (13-2) portray downconversion,
  and Equations (13-3) portray upconversion.

  Calling Sequence: upconvertByFsOver4(bufferPtr,byteCount)

  Inputs:

    bufferPtr - A pointer to signed IQ data.

    byteCount - The number of bytes contained in the buffer that is
    in the buffer.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::upconvertByFsOver4(
  int8_t *bufferPtr,
  uint32_t byteCount)
{
  uint32_t i;
  int8_t x, y;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This block of code translates the spectrum by Fs/4.
  // Here's how it works.  The IQ samples are multiplied by
  // exp(j*PI/2) = {1,j1,-1,-j1,...}. Note that this sequence
  // repeats modulo 4.  Consider that z(n) = x(n) + jy(n).
  // The first 4 values of the translated spectrum are,
  //
  //    znew(0) =1[x(0) + jy(0)] = x(0) + jy(0).
  //    znew(1) = j[(x(1) + jy(1)] = -y(1) + jx(1).
  //    znew(2) = -1[x(2) + jy(2)] = -x(2) -jy(2).
  //    znew(3) = -j[x(3) + jy(3)] = y(3) - jx(3).
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  for (i = 0; i < byteCount; i += 8)
  {
    // znew(0) = x(0) + jy(0).
    // No processing needs to be done.

    // znew(1) = -y(1) + jx(1).
    x = bufferPtr[i + 2];
    y = bufferPtr[i + 3];
    bufferPtr[i + 2] = -y;
    bufferPtr[i + 3] = x;  

    // znew(2) = -x(2) - jy(2).
    x = bufferPtr[i + 4];
    y = bufferPtr[i + 5];
    bufferPtr[i + 4] = -x;
    bufferPtr[i + 5] = -y;

    // znew(3) = y(3) - jx(3).
    x = bufferPtr[i + 6];
    y = bufferPtr[i + 7];
    bufferPtr[i + 6] = y;
    bufferPtr[i + 7] = -x;
  } // for
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

} // upconvertByFsOver4

/**************************************************************************

  Name: acceptIqData

  Purpose: The purpose of this function is to queue data to be transmitted
  over the network.  The data consumer thread will dequeue the message
  and perform the forwarding of the data.

  Calling Sequence: acceptIqData(timeStamp,bufferPtr,byteCount);

  Inputs:

    timeStamp - A 32-bit timestamp associated with the message.

    bufferPtr - A pointer to IQ data.

    byteCount - The number of bytes contained in the buffer that is
    in the buffer.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::acceptIqData(unsigned long timeStamp,
                                   unsigned char *bufferPtr,
                                   unsigned long byteCount)
{
  int i;
  uint16_t signalPresenceIndicator;
  int8_t *signedBufferPtr;
  bool signalAllowed;
  uint32_t signalMagnitude;

  // Reference IQ samples as a set of signed values.
  signedBufferPtr = (int8_t *)bufferPtr;

  // Convert the IQ samples from offset binary to signed values.
  for (i = 0; i < byteCount; i++)
  {
    signedBufferPtr[i] -= 128;
  } // for

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // When the receive frequency is set by the Radio object, the
  // radio is tuned to the frequency with an offset of Fs/4.
  // This allows the 1/f noise to be translated away from the
  // desired frequency, once the signal is mixed back to
  // baseband in the digital domain.  Before frequency
  // translation, the desired signal resides at -Fs/4.  Once
  // upconverted, the signal will reside at baseband.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  upconvertByFsOver4(signedBufferPtr,byteCount);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Determine if a signal is available.
  signalPresenceIndicator = trackerPtr->run(signedBufferPtr,byteCount);

  switch (signalPresenceIndicator)
  {
    case SIGNALTRACKER_NOISE:
    {
      // We have no signal.
      signalAllowed = false;
      break;
    } // case

    case SIGNALTRACKER_STARTOFSIGNAL:
    case SIGNALTRACKER_SIGNALPRESENT:
    {
      // We have a signal.
      signalAllowed = true;
      break;
    } // case

    case SIGNALTRACKER_ENDOFSIGNAL:
    {
      // We like a squelch tail.
      signalAllowed = true;
     break;
    } // case

    default:
    {
      signalAllowed = false;
      break;
    } // case
  } // switch

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // In this context, signalAllowed is used as a signal presence
  // indicator.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  if ((signalNotificationEnabled) && (signalCallbackPtr != NULL))
  {
    // Notify the client client of new signal state information.
    signalCallbackPtr(signalAllowed,signalCallbackContextPtr);
  } // if
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Notify the client of signal magnitude information.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  if ((signalMagnitudeNotificationEnabled) &&
      (signalMagnitudeCallbackPtr != NULL))
  {
    // Retrieve the average magnitude of the last received IQ block.
    signalMagnitude = trackerPtr->getSignalMagnitude();

    // Notify the client client of new signal magnitude information.
    signalMagnitudeCallbackPtr(signalMagnitude,
                               signalMagnitudeCallbackContextPtr);
  } // if
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  if (signalAllowed)
  {
    switch (demodulatorMode)
    {
      case None:
      {
        break;
      } // case

      case Am:
      {
        // Demodulate as an AM signal.
        amDemodulatorPtr->acceptIqData(signedBufferPtr,byteCount);
        break;
      } // case

      case Fm:
      {
        // Demodulate as an FM signal.
        fmDemodulatorPtr->acceptIqData(signedBufferPtr,byteCount);
        break;
      } // case

      case WbFm:
      {
        // Demodulate as a wideband FM signal.
        wbFmDemodulatorPtr->acceptIqData(signedBufferPtr,byteCount);
        break;
      } // case

      case Lsb:
      case Usb:
      {
        // Demodulate as a single sideband signal.
        ssbDemodulatorPtr->acceptIqData(signedBufferPtr,byteCount);
        break;
      } // case

      default:
      {
        break;
      } // case
    } // switch
  } // if

  return;

} // acceptIqData

/**************************************************************************

  Name: displayInternalInformation

  Purpose: The purpose of this function is to display internal information
  in the IQ data processor.

  Calling Sequence: displayInternalInformation()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void IqDataProcessor::displayInternalInformation(void)
{

  nprintf(stderr,"\n--------------------------------------------\n");
  nprintf(stderr,"IQ Data Processor Internal Information\n");
  nprintf(stderr,"--------------------------------------------\n");

  nprintf(stderr,"Demodulator Mode         : ");

  switch (demodulatorMode)
  {
    case None:
    {
      nprintf(stderr,"None\n");
      break;
    } // case

    case Am:
    {
      nprintf(stderr,"AM\n");
      break;
    } // case

    case Fm:
    {
      nprintf(stderr,"FM\n");
      break;
    } // case

    case WbFm:
    {
      nprintf(stderr,"WBFM\n");
      break;
    } // case

    case Lsb:
    {
      nprintf(stderr,"LSB\n");
      break;
    } // case

    case Usb:
    {
      nprintf(stderr,"USB\n");
      break;
    } // case

    default:
    {
      break;
    } // case
  } // switch

  nprintf(stderr,"Signal Detect Threhold   : %d dBFs\n",signalDetectThreshold);

  return;

} // displayInternalInformation

