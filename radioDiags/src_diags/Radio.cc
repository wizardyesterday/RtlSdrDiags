//**********************************************************************
// file name: Radio.cc
//**********************************************************************

#include <stdio.h>
#include <unistd.h>

#include "Radio.h"

extern "C"
{
  #include "rtl-sdr.h"
  #include "convenience.h"
}

#define RECEIVE_BUFFER_SIZE (32768)
#define TRANSMIT_BUFFER_SIZE (32768)

#define RECEIVE_AUTO_GAIN (99999)

extern void nprintf(FILE *s,const char *formatPtr, ...);

/**************************************************************************

  Name: Radio

  Purpose: The purpose of this function is to serve as the constructor
  of a Radio object.

  Calling Sequence: Radio(deviceNumber,txSampleRate,rxSampleRate)

  Inputs:

    deviceNumber - The device number of interest.  Just use 0.

    txSampleRate - The sample rate of the baseband portion of the
    transmitter in samples per second.

    rxSampleRate - The sample rate of the baseband portion of the
    receiver in samples per second.

  Outputs:

    None.

**************************************************************************/
Radio::Radio(int deviceNumber,uint32_t txSampleRate,uint32_t rxSampleRate)
{ 
  int i;
  bool success;

  // Save for later use.
  this->transmitSampleRate = txSampleRate;
  this->receiveSampleRate = rxSampleRate;

  // Save for later use.
  this->deviceNumber = deviceNumber;

  // Indicate that there is no association with a device.
  devicePtr = 0;

  // Ensure that we don't have any dangling pointers.
  receiveDataProcessorPtr = 0;
  dataConsumerPtr = 0;
  dataProviderPtr = 0;

  // Start with a clean slate.
  receiveTimeStamp = 0;
  receiveBlockCount = 0;
  transmitBlockCount = 0;

  // Indicate that the radio is not requested to stop.
  requestReceiverToStop = false;

  // Initialize radio lock.
  pthread_mutex_init(&radioLock,0);

  // Set up the receiver with a default configuration.
  success = setupReceiver();

  if (success)
  {
    // Set up the transmitter with a default configuration.
    success = setupTransmitter();

    if (!success)
    {
      fprintf(stderr,"Could not initialize transmitter.\n");
    } // if
  } // if
  else
  {
    fprintf(stderr,"Could not initialize receiver.\n");
  } // else

  if (success)
  {
    // Allocate the receive buffer that is to be used for IQ samples.
    receiveBufferPtr = (uint8_t *)calloc(RECEIVE_BUFFER_SIZE,sizeof(uint8_t));

    // Instantiate the data provider object. The transmit method will
    // need this.
    dataProviderPtr = new DataProvider(); 

    // Instantiate the IQ data processor object.  The data consumer object
    // will need this.
    receiveDataProcessorPtr = new IqDataProcessor();

    // Instantiate the data consumer object.  eventConsumerProcedure method
    // will need this.
    dataConsumerPtr = new DataConsumer(receiveDataProcessorPtr);

    // Instantiate the AM demodulator.
    amDemodulatorPtr = new AmDemodulator;

    // Associate the AM demodulator with the Iq data processor.
    receiveDataProcessorPtr->setAmDemodulator(amDemodulatorPtr);

    // Instantiate the FM demodulator.
    fmDemodulatorPtr = new FmDemodulator;

    // Associate the FM demodulator with the Iq data processor.
    receiveDataProcessorPtr->setFmDemodulator(fmDemodulatorPtr);

    // Instantiate the wideband FM demodulator.
    wbFmDemodulatorPtr = new WbFmDemodulator;

    // Associate the FM demodulator with the Iq data processor.
    receiveDataProcessorPtr->setWbFmDemodulator(wbFmDemodulatorPtr);

    // Instantiate the SSB demodulator.
    ssbDemodulatorPtr = new SsbDemodulator;

    // Associate the SSB demodulator with the Iq data processor.
    receiveDataProcessorPtr->setSsbDemodulator(ssbDemodulatorPtr);

    // Set the demodulation mode.
    receiveDataProcessorPtr->setDemodulatorMode(IqDataProcessor::Fm);

    // Create the event consumer thread.
    pthread_create(&eventConsumerThread,0,
                   (void *(*)(void *))eventConsumerProcedure,this);
  } // if
 
  return;
 
} // Radio

/**************************************************************************

  Name: ~Radio

  Purpose: The purpose of this function is to serve as the destructor
  of a Radio object.

  Calling Sequence: Radio()

  Inputs:

    None.

  Outputs:

    None.


**************************************************************************/
Radio::~Radio(void)
{

  // Notify the event consumer thread that it is time to exit.
  timeToExit = true;

  // Wait for thread to terminate.
  pthread_join(eventConsumerThread,0);

  // We're done with the receiver.
  tearDownReceiver();

  // We're done with the transmitter.
  tearDownTransmitter();

  //-------------------------------------
  // Release resources in this order.
  //-------------------------------------
  if (dataConsumerPtr != 0)
  {
    delete dataConsumerPtr;
  } // if

  if (receiveDataProcessorPtr != 0)
  {
    delete receiveDataProcessorPtr;
  } // if

  if (dataProviderPtr != 0)
  {
    delete dataProviderPtr;
  } // if

  if (amDemodulatorPtr != 0)
  {
    delete amDemodulatorPtr;
  } // if

  if (fmDemodulatorPtr != 0)
  {
    delete fmDemodulatorPtr;
  } // if

  if (wbFmDemodulatorPtr != 0)
  {
    delete wbFmDemodulatorPtr;
  } // if

  if (ssbDemodulatorPtr != 0)
  {
    delete ssbDemodulatorPtr;
  } // if

  if (receiveBufferPtr != 0)
  {
    free(receiveBufferPtr);
  } // if
  //-------------------------------------

  // Destroy radio lock.
  pthread_mutex_destroy(&radioLock);

  return;
 
} // ~Radio

/**************************************************************************

  Name: setupReceiver

  Purpose: The purpose of this function is to set up the receiver with
  a default configuration.

  Calling Sequence: success = setupReceiver()

  Inputs:

    None.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setupReceiver(void)
{
  bool success;
  int error;

  // Default to success.
  success = true;

  // Indicate that the receive process is disabled.
  receiveEnabled = false;

  // Default to 100MHz.
  receiveFrequency = 100000000LL;

  // Use automatic bandwidth selection.
  receiveBandwidth = 0;

  // Default to automatic gain mode.
  receiveGainInDb = RECEIVE_AUTO_GAIN;

  // Default to no frequency error.
  receiveWarpInPartsPerMillion = 0;

  return (success);
 
} // setupReceiver

/**************************************************************************

  Name: setupTransmitter

  Purpose: The purpose of this function is to set up the transmitter with
  a default configuration.

  Calling Sequence: success = setupTransmitter()

  Inputs:

    None.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setupTransmitter(void)
{
  bool success;
  int error;

  // Default to success.
  success = true;

  // Indicate that the transmit process is disabled.
  transmitEnabled = false;

  // Indicate that the system is not transmitting modulated data.
  transmittingData = false;

  // Default to 100MHz.
  transmitFrequency = 100000000LL;

  // Use a transmit filter bandwidth of 10MHz
  transmitBandwidth = 10000000;

  // Minimize the transmit gain.
  transmitGainInDb = 0;

  // set the transmit frequency.
  // Set the transmit sample rate.
  // set the transmi bandwidth.
  // set the transmit gain.

  return (success);
 
} // setupTransmitter

/**************************************************************************

  Name: tearDownReceiver

  Purpose: The purpose of this function is to tear down the receiver.

  Calling Sequence: tearDownReceiver()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::tearDownReceiver(void)
{

  // Stop all data flow.
  stopReceiver();

  return;
  
} // tearDownReceiver

/**************************************************************************

  Name: tearDownTransmitter

  Purpose: The purpose of this function is to tear down the transmitter.

  Calling Sequence: tearDownTransmitter()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::tearDownTransmitter(void)
{

  // Indicate that the transmit process is disabled.
  transmitEnabled = false;

  return;
  
} // tearDownTransmitter

/**************************************************************************

  Name: startReceiver

  Purpose: The purpose of this function is to enable the receiver.  The
  underlying system software will enable the receive path for the RFICs
  of interest, configure the hardware to enable data transfer for the 
  receive path, and enable receive event generation.

  Calling Sequence: startReceiver()

  Inputs:

    None.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::startReceiver(void)
{
  bool success;
  bool status;
  int deviceIndex;
  char deviceNumberString[20];
  int error;
  int errorCount;

  // Default to success
  success = true;

  // Indicate that no errors occurred.
  errorCount = 0;

  if (!receiveEnabled)
  {
    // Format our parameter.
    snprintf(deviceNumberString,sizeof(deviceNumberString),"%d",deviceNumber);

    // See if our radio device is available.
    deviceIndex = verbose_device_search(deviceNumberString);

    if (deviceIndex >= 0)
    {
      // Start up the radio device.         
      error = rtlsdr_open((rtlsdr_dev_t **)&devicePtr,(uint32_t)deviceIndex);

      if (error == 0)
      {
        // Set the receive frequency.
        status = setReceiveFrequency(receiveFrequency);

        if (!status)
        {
          // Indicate that one more error occurred.
          errorCount++;
        } // if

        // Set the receive sample rate.
        status = setReceiveSampleRate(receiveSampleRate);

        if (!status)
        {
          // Indicate that one more error occurred.
          errorCount++;
        } // if

        // Set the receive bandwidth.
        status = setReceiveBandwidth(receiveBandwidth);

        if (!status)
        {
          // Indicate that one more error occurred.
          errorCount++;
        } // if

        // Set the receive gain.
        status = setReceiveGainInDb(receiveGainInDb);

        if (!status)
        {
          // Indicate that one more error occurred.
          errorCount++;
        } // if

        // Set the ppm error.
        status = setReceiveWarpInPartsPerMillion(receiveWarpInPartsPerMillion);

        if (!status)
        {
          // Indicate that one more error occurred.
          errorCount++;
        } // if

        if (errorCount == 0)
        {  
          // Ensure that the data consumer will accept data.
          dataConsumerPtr->start();

          // Reset the endpoint to ensure that data stream is synchronized.
          verbose_reset_buffer((rtlsdr_dev_t *)devicePtr);

          // Ensure that the system will accept any arriving data.
          receiveEnabled = true;
        } // if
        else
        {
          // Errors occurred, so let's close the radio.
          error = rtlsdr_close((rtlsdr_dev_t *)devicePtr);

          // Indicate that the radio hardware is not available.
          devicePtr = 0;

          // Indicate failure.
          success = false;
        } // else
      } // if
      else
      {
        // Indicate failure.
        success = false;
      } // else
    } // if
    else
    {
      // Indicate failure.
      success = false;
    } // else
  } // if

  return (success);
  
} // startReceiver

/**************************************************************************

  Name: stopReceiver

  Purpose: The purpose of this function is to disable the receiver.  The
  underlying system software will disable the receive path for the RFICs
  of interest, configure the hardware to disable DMA for the receive path,
  and disable receive event generation.

  Calling Sequence: stopReceiver()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::stopReceiver(void)
{

  if (receiveEnabled)
  {
    // Request to stop the receiver.
    requestReceiverToStop = true;

    // Acquire the radio lock.
    pthread_mutex_lock(&radioLock);

    // Ensure that the system will discard any arriving data.
    receiveEnabled = false;

    // Ensure that the data consumer will not accept data.
    dataConsumerPtr->stop();

    // Stop the radio.
    rtlsdr_close((rtlsdr_dev_t *)devicePtr);

    // We don't like dangling pointers.
    devicePtr = 0;

    // The radio is stopped, so we're done..
    requestReceiverToStop = false;

    // Release the radio lock.
    pthread_mutex_unlock(&radioLock);
  } // if

  return;
  
} // stopReceiver

/**************************************************************************

  Name: startTransmitter

  Purpose: The purpose of this function is to enable the transmitter.  The
  underlying system software will enable the transmit path for the RFICs
  of interest, configure the hardware to enable DMA for the transmit path,
  and enable transmit event generation.

  Calling Sequence: startTransmitter()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::startTransmitter(void)
{

  if (!transmitEnabled)
  {
    // Ensure that the system will allow transmission of data.
    transmitEnabled = true;

    // Do whatever is necessary to start the transmit process.
  } // if

  return;
  
} // startTransmitter

/**************************************************************************

  Name: stopTransmitter

  Purpose: The purpose of this function is to disable the transmitter.  The
  underlying system software will disable the transmit path for the RFICs
  of interest, configure the hardware to disable DMA for the transmit path,
  and disable transmit event generation.

  Calling Sequence: stopTransmitter()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::stopTransmitter(void)
{

  if (transmitEnabled)
  {
    // Ensure that the system will disallow transmission of data.
    transmitEnabled = false;

    // Ensure that we indicate that we are not modulating the carrier.
    transmittingData = false;

    // Do whatever is necessary to stop the transmit process.
  } // if

  return;
  
} // stopTransmitter

/**************************************************************************

  Name: startTransmitData

  Purpose: The purpose of this function is to disable the transmitter.  The
  underlying system software will disable the transmit path for the RFICs
  of interest, configure the hardware to disable DMA for the transmit path,
  and disable transmit event generation.

  Calling Sequence: startTransmitData()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::startTransmitData(void)
{
  int i;
  uint32_t timeStamp;

  if (transmitEnabled)
  {
    if (!transmittingData)
    {
      timeStamp = 0;

      // Get the current timestamp from the hardware

      // Let the first block be transmitted 0.1 seconds into the future.
      timeStamp += (transmitSampleRate / 10);

      // Inform the data provider about the initial timestamp value.
      dataProviderPtr->setInitialTimeStamp(timeStamp);

      // Buffer the data to be transmitted.

      // Indicate that system is transmitting data.
      transmittingData = true;
    } // if
  } // if

  return;
  
} // startTransmitData

/**************************************************************************

  Name: setReceiveFrequency

  Purpose: The purpose of this function is to set the operating frequency
  of the receiver.  If the receiver is not enabled, the appropriate
  attribute is updated.  The attribute will be used at a later time to
  set up the receiver.
  Note that the definition of receiver enabled is that devicePtr is not
  equal to zero.

  Calling Sequence: success = setReceiveFrequency(frequency)

  Inputs:

    frequency - The receiver frequency in Hertz.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setReceiveFrequency(uint64_t frequency)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (devicePtr != 0)
  {
    // Notify the driver of the new frequency.
    error = rtlsdr_set_center_freq((rtlsdr_dev_t *)devicePtr,
                                   (uint32_t)frequency);

    if (error == 0)
    {
      // Update attribute.
      receiveFrequency = frequency;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    receiveFrequency = frequency;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setReceiveFrequency

/**************************************************************************

  Name: setReceiveBandwidth

  Purpose: The purpose of this function is to set the baseband filter
  bandwidth of the receiver.  If the receiver is not enabled, the
  appropriate attribute is updated.  The attribute will be used at a
  later time to set up the receiver.
  Note that the definition of receiver enabled is that devicePtr is not
  equal to zero.

  Calling Sequence: success = setReceiveBandwidth(bandwidth)

  Inputs:

    bandwidth - The receive baseband filter bandwidth in Hertz.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setReceiveBandwidth(uint32_t bandwidth)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (devicePtr != 0)
  {
    // Notify the driver of the new bandwidth.
    error = rtlsdr_set_tuner_bandwidth((rtlsdr_dev_t *)devicePtr,bandwidth);

    if (error == 0)
    {
      // Update attribute.
      receiveBandwidth = bandwidth;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    receiveBandwidth = bandwidth;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setReceiveBandwidth

/**************************************************************************

  Name: setReceiveGainInDb

  Purpose: The purpose of this function is to set the gain of the
  receiver.  If the receiver is not enabled, the appropriate attribute is
  updated.  The attribute will be used at a later time to set up the
  receiver.
  Note that the definition of receiver enabled is that devicePtr is not
  equal to zero.

  Calling Sequence: success = setReceiveGainInDb(gain)

  Inputs:

    gain - The gain in decibels.  A value of 0xffffffff indicates that
    the system should be set to automatic gain mode.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setReceiveGainInDb(uint32_t gain)

{
  bool success;
  int nearestGain;
  int error;

  // Default to failure.
  success = false;

  if (devicePtr != 0)
  {
    // Notify the driver of the new gain.
    if (gain == RECEIVE_AUTO_GAIN)
    {
      // Set to automatic gain mode.
      error = verbose_auto_gain((rtlsdr_dev_t *)devicePtr);
    } // if
    else
    {
      // Retrieve the nearest gain value in 0.1 decibel.
      nearestGain = nearest_gain((rtlsdr_dev_t *)devicePtr,(gain * 10));

      // Set the gain.
      error = verbose_gain_set((rtlsdr_dev_t *)devicePtr,nearestGain);
    } // else

    if (error == 0)
    {
      // Update attribute.
      receiveGainInDb = gain;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    receiveGainInDb = gain;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setReceiveGainInDb

/**************************************************************************

  Name: setReceiveSampleRate

  Purpose: The purpose of this function is to set the sample rate
  of the receiver.  If the receiver is not enabled, the appropriate
  attribute is updated.  The attribute will be used at a later time to set
  up the receiver.
  Note that the definition of receiver enabled is that devicePtr is not
  equal to zero.

  Calling Sequence: success = setReceiveSampleRate(sampleRate)

  Inputs:

    sampleRate - The receiver sample rate in samples per second.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setReceiveSampleRate(uint32_t sampleRate)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (devicePtr != 0)
  {
    // Notify the driver of the new sample rate.
    error = verbose_set_sample_rate((rtlsdr_dev_t *)devicePtr,sampleRate);

    if (error == 0)
    {
      // Update attribute.
      receiveSampleRate = sampleRate;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    receiveSampleRate = sampleRate;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setReceiveSampleRate

/**************************************************************************

  Name: setReceiveWarpInPartsPerMillion

  Purpose: The purpose of this function is to set the frequency warp
  of the receiver.  If the receiver is not enabled, the appropriate
  attribute is updated.  The attribute will be used at a later time to set
  up the receiver.
  Note that the definition of receiver enabled is that devicePtr is not
  equal to zero.

  Calling Sequence: success = setReceiveWarpInPartsPerMillion(warp)

  Inputs:

    The warp value in parts per million.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setReceiveWarpInPartsPerMillion(int warp)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (devicePtr != 0)
  {
    // Notify the driver of the new warp value.
    error = verbose_ppm_set((rtlsdr_dev_t *)devicePtr,warp);

    if (error == 0)
    {
      // Update attribute.
      receiveWarpInPartsPerMillion = warp;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    receiveWarpInPartsPerMillion = warp;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setReceiveWarpInPartsPerMillion

/**************************************************************************

  Name: setTransmitFrequency

  Purpose: The purpose of this function is to set the operating frequency
  of the transmitter.  If the transmitter is not enabled, the appropriate
  attribute is updated.  The attribute will be used at a later time to
  set up the transmitter.

  Calling Sequence: success = setTransmitFrequency(frequency)

  Inputs:

    frequency - The transmitter frequency in Hertz.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setTransmitFrequency(uint64_t frequency)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (transmitEnabled)
  {
    // Notify the driver of the new frequency.
    error = 0;

    if (error == 0)
    {
      // Update attribute.
      transmitFrequency = frequency;

      // indicate sucess.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    transmitFrequency = frequency;

    // indicate sucess.
    success = true;
  } // else

  return (success);
  
} // setTransmitFrequency

/**************************************************************************

  Name: setTransmitBandwidth

  Purpose: The purpose of this function is to set the baseband filter
  bandwidth of the transmitterr.  If the transmitter is not enabled, the
  appropriate attribute is updated.  The attribute will be used at a later
  time to set up the transmitter.

  Calling Sequence: success = setTransmitBandwidth(bandwidth)

  Inputs:

    bandwidth - The bandwidth in Hertz.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setTransmitBandwidth(uint32_t bandwidth)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (transmitEnabled)
  {
    // Notify the driver of the new bandwidth.
    error = 0;

    if (error == 0)
    {
      // Update attribute.
      transmitBandwidth = bandwidth;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    transmitBandwidth = bandwidth;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setTransmitBandwidth

/**************************************************************************

  Name: setTransmitGainInDb

  Purpose: The purpose of this function is to set the gain of the
  transmitter.  If the transmitter is not enabled, the appropriate
  attribute is updated.  The attribute will be used at a later time to
  set up the transmitter.

  Calling Sequence: success = setTransmitGainInDb(gain)

  Inputs:

    gain - The gain in units of decibels.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setTransmitGainInDb(uint32_t gain)

{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (transmitEnabled)
  {
    // Notify the driver of the new gain.  We don't know the units yet.
    error = 0;

    if (error == 0)
    {
      // Update attribute.
      transmitGainInDb = gain;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    transmitGainInDb = gain;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setTransmitGain

/**************************************************************************

  Name: setTransmitSampleRate

  Purpose: The purpose of this function is to set the sample rate
  of the transmitter.  If the transmitter is not enabled, the appropriate
  attribute is updated.  The attribute will be used at a later time to
  set up the transmitter.

  Calling Sequence: success = setTransmitSampleRate(sampleRate)

  Inputs:

    sampleRate - The transmit sample rate in samples per second.

  Outputs:

    success - A boolean that indicates the outcome of the operation.  A
    value of true indicates success, and a value of false indicates
    failure.

**************************************************************************/
bool Radio::setTransmitSampleRate(uint32_t sampleRate)
{
  bool success;
  int error;

  // Default to failure.
  success = false;

  if (transmitEnabled)
  {
    // Notify the driver of the new sample rate.
    error = 0;

    if (error == 0)
    {
      // Update attribute.
      transmitSampleRate = sampleRate;

      // indicate success.
      success = true;
    } // if
  } // if
  else
  {
    // Update attribute.
    transmitSampleRate = sampleRate;

    // indicate success.
    success = true;
  } // else

  return (success);
  
} // setTransmitSampleRate

/**************************************************************************

  Name: getReceiveFrequency

  Purpose: The purpose of this function is to get the operating frequency
  of the receiver.

  Calling Sequence: frequency = getReceiveFrequency()

  Inputs:

    None.

  Outputs:

    frequency - The receiver frequency in Hertz.

**************************************************************************/
uint64_t Radio::getReceiveFrequency(void)
{

  return (receiveFrequency);
  
} // getReceiveFrequency

/**************************************************************************

  Name: getReceiveBandwidth

  Purpose: The purpose of this function is to get the baseband filter
  bandwidth of the receiver.

  Calling Sequence: bandwidth = setReceiveBandwidth()

  Inputs:

    None.

  Outputs:

    bandwidth - The receive baseband filter bandwidth in Hertz.

**************************************************************************/
uint32_t Radio::getReceiveBandwidth(void)
{
  return (receiveBandwidth);
  
} // getReceiveBandwidth

/**************************************************************************

  Name: getReceiveGainInDb

  Purpose: The purpose of this function is to get the gain of the
  receiver.

  Calling Sequence: gain = getReceiveGainInDb()

  Inputs:

    None.

  Outputs:

    gain - The gain in decibels.

**************************************************************************/
uint32_t Radio::getReceiveGainInDb(void)

{

  return (receiveGainInDb);
  
} // getReceiveGainInDb

/**************************************************************************

  Name: getReceiveSampleRate

  Purpose: The purpose of this function is to get the sample rate
  of the receiver.

  Calling Sequence: sampleRate = getReceiveSampleRate()

  Inputs:

    None.

  Outputs:

    sampleRate - The receiver sample rate in samples per second.

**************************************************************************/
uint32_t Radio::getReceiveSampleRate(void)
{

  return (receiveSampleRate);
  
} // getReceiveSampleRate

/**************************************************************************

  Name: getReceiveWarpInPartsPerMillion

  Purpose: The purpose of this function is to get the frequency warp value
  of the receiver.

  Calling Sequence: warp = getReceiveWarpInPartsPerMillion()

  Inputs:

    None.

  Outputs:

    warp - The frequency warp value in parts per million.

**************************************************************************/
int Radio::getReceiveWarpInPartsPerMillion(void)
{

  return (receiveWarpInPartsPerMillion);
  
} // getReceiveWarpInPartsPerMillion

/**************************************************************************

  Name: getTransmitFrequency

  Purpose: The purpose of this function is to get the operating frequency
  of the transmitter.

  Calling Sequence: frequency = getTransmitFrequency()

  Inputs:

    None.

  Outputs:

    frequency - The transmitter frequency in Hertz.

**************************************************************************/
uint64_t Radio::getTransmitFrequency(void)
{

  return (transmitFrequency);
  
} // getTransmitFrequency

/**************************************************************************

  Name: getTransmitBandwidth

  Purpose: The purpose of this function is to get the baseband filter
  bandwidth of the receiver.

  Calling Sequence: bandwidth = getTransmitBandwidth()

  Inputs:

    None.

  Outputs:

    bandwidth - The bandwidth in Hertz.

**************************************************************************/
uint32_t Radio::getTransmitBandwidth(void)
{

  return (transmitBandwidth);
  
} // getTransmitBandwidth

/**************************************************************************

  Name: getTransmitGainInDb

  Purpose: The purpose of this function is to get the gain of the
  transmitter.

  Calling Sequence: gain = getTransmitGainInDb()

  Inputs:

    None.

  Outputs:

    gain - The gain in units of decibels.

**************************************************************************/
uint32_t Radio::getTransmitGainInDb(void)

{

  return (transmitGainInDb);
  
} // getTransmitGainInDb

/**************************************************************************

  Name: getTransmitSampleRate

  Purpose: The purpose of this function is to get the sample rate
  of the transmitter.

  Calling Sequence: sampleRate = getTransmitSampleRate()

  Inputs:

    None.

  Outputs:

    sampleRate - The transmit sample rate in samples per second.

**************************************************************************/
uint32_t Radio::getTransmitSampleRate(void)
{

  return (transmitSampleRate);
  
} // getTransmitSampleRate

/**************************************************************************

  Name: getDataProvider

  Purpose: The purpose of this function is to get the pointer to the
  transmit data provider.

  Calling Sequence: providerPtr = getDataProvider()

  Inputs:

    None.

  Outputs:

    providerPtr - A pointer to the transmit data provider object.

**************************************************************************/
DataProvider * Radio::getDataProvider(void)
{

  return (dataProviderPtr);

} // getDataProvider

/**************************************************************************

  Name: isReceiving

  Purpose: The purpose of this function is to indicate whether or not the
  receiver is enabled.

  Calling Sequence: enabled = isReceiving()

  Inputs:

    None.

  Outputs:

    Enabled - A boolean that indicates whether or not the receiver is
    enabled.  A value of true indicates that the receiver is enabled,
    and a value of false indicates that the receiveer is disabled.

**************************************************************************/
bool  Radio::isReceiving(void)
{

  return (receiveEnabled);
  
} // isReceiving

/**************************************************************************

  Name: isTransmitting

  Purpose: The purpose of this function is to indicate whether or not the
  transmitter is enabled for the RF path of interest.

  Calling Sequence: enabled = isTransmitting()

  Inputs:

    None.

  Outputs:

    Enabled - A boolean that indicates whether or not the transmitter is
    enabled.  A value of true indicates that the transmitter is enabled,
    and a value of false indicates that the transmitter is disabled.

**************************************************************************/
bool  Radio::isTransmitting(void)
{

  return (transmitEnabled);
  
} // isTransmitting

/**************************************************************************

  Name: setDemodulatorMode

  Purpose: The purpose of this function is to set the demodulator mode.
  This mode dictates which demodulator should be used when processing IQ
  data samples.

  Calling Sequence: setDemodulatorMode(mode)

  Inputs:

    mode - The demodulator mode.  Valid values are None, Am, and Fm.

  Outputs:

    None.

**************************************************************************/
void Radio::setDemodulatorMode(uint8_t mode)
{
  IqDataProcessor::demodulatorType kludge;

  // Let's make things happy.
  kludge = static_cast<IqDataProcessor::demodulatorType>(mode);

  // Notify the IQ data processor to use this mode.
  receiveDataProcessorPtr->setDemodulatorMode(kludge);

  return;

} // setDemodulaterMode

/*****************************************************************************

  Name: setAmDemodulatorGain

  Purpose: The purpose of this function is to set the gain of the
  AM demodulator.

  Calling Sequence: setAmDemodulatorGain(gain)

  Inputs:

    gain - The demodulator gain.

  Outputs:

    None.

*****************************************************************************/
void Radio::setAmDemodulatorGain(float gain)
{

  // Notifier the AM demodulator to set its gain.
  amDemodulatorPtr->setDemodulatorGain(gain);

  return;

} // setAmDemodulatorGain

/*****************************************************************************

  Name: setFmDemodulatorGain

  Purpose: The purpose of this function is to set the gain of the
  FM demodulator.

  Calling Sequence: setFmDemodulatorGain(gain)

  Inputs:

    gain - The demodulator gain.

  Outputs:

    None.

*****************************************************************************/
void Radio::setFmDemodulatorGain(float gain)
{

  // Notifier the FM demodulator to set its gain.
  fmDemodulatorPtr->setDemodulatorGain(gain);

  return;

} // setFmDemodulatorGain

/*****************************************************************************

  Name: setWbFmDemodulatorGain

  Purpose: The purpose of this function is to set the gain of the wideband
  FM demodulator.

  Calling Sequence: setWbFmDemodulatorGain(gain)

  Inputs:

    gain - The demodulator gain.

  Outputs:

    None.

*****************************************************************************/
void Radio::setWbFmDemodulatorGain(float gain)
{

  // Notifier the FM demodulator to set its gain.
  wbFmDemodulatorPtr->setDemodulatorGain(gain);

  return;

} // setWbFmDemodulatorGain

/*****************************************************************************

  Name: setSsbDemodulatorGain

  Purpose: The purpose of this function is to set the gain of the SSB
  demodulator.

  Calling Sequence: setSsbDemodulatorGain(gain)

  Inputs:

    gain - The demodulator gain.

  Outputs:

    None.

*****************************************************************************/
void Radio::setSsbDemodulatorGain(float gain)
{

  // Notifier the SSB demodulator to set its gain.
  ssbDemodulatorPtr->setDemodulatorGain(gain);

  return;

} // setSsbDemodulatorGain

/**************************************************************************

  Name: displayInternalInformation

  Purpose: The purpose of this function is to display internal
  in the radio.

  Calling Sequence: displayInternalInformation()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
void Radio::displayInternalInformation(void)
{

  // Display top level radio information.
  nprintf(stderr,"\n------------------------------------------------------\n");
  nprintf(stderr,"Radio Internal Information\n");
  nprintf(stderr,"------------------------------------------------------\n");

  nprintf(stderr,"Receive Enabled                     : ");
  if (receiveEnabled)
  {
    nprintf(stderr,"Yes\n");
  } // if
  else
  {
     nprintf(stderr,"No\n");
   } // else

  if (receiveGainInDb == RECEIVE_AUTO_GAIN)
  {
    nprintf(stderr,"Receive Gain:                       : Auto\n");
  } // if
  else
  {
    nprintf(stderr,"Receive Gain:                       : %u dB\n",
          receiveGainInDb);
  } // else

  nprintf(stderr,"Receive Frequency                   : %llu Hz\n",
          receiveFrequency);
  nprintf(stderr,"Receive Bandwidth                   : %u Hz\n",
          receiveBandwidth);
  nprintf(stderr,"Receive Sample Rate:                : %u S/s\n",
          receiveSampleRate);
  nprintf(stderr,"Receive Frequency Warp:             : %d ppm\n",
          receiveWarpInPartsPerMillion);
  nprintf(stderr,"Receive Timestamp                   : %u\n",
          receiveTimeStamp);
  nprintf(stderr,"Receive Block Count                 : %u\n",
          receiveBlockCount);

  nprintf(stderr,"Transmit Enabled                    : ");
  if (transmitEnabled)
  {
    nprintf(stderr,"Yes\n");
  } // if
  else
  {
     nprintf(stderr,"No\n");
   } // else

  nprintf(stderr,"Transmitting Data                   : ");
  if (transmittingData)
  {
    nprintf(stderr,"Yes\n");
  } // if
  else
  {
     nprintf(stderr,"No\n");
   } // else

  nprintf(stderr,"Transmit Gain:                      : %d\n",
          (unsigned int)transmitGainInDb);
  nprintf(stderr,"Transmit Frequency                  : %llu Hz\n",
          transmitFrequency);
  nprintf(stderr,"Transmit Bandwidth                  : %u Hz\n",
          transmitBandwidth);
  nprintf(stderr,"Transmit Sample Rate                : %u S/s\n",
          transmitSampleRate);
 
  nprintf(stderr,"Transmit Block Count                : %u\n",
          transmitBlockCount);

  // Display data consumer information.
  dataConsumerPtr->displayInternalInformation();

  // Display data provider information.
  dataProviderPtr->displayInternalInformation();

  // Display IQ data processor information.
  receiveDataProcessorPtr->displayInternalInformation();

  // Display AM demodulator information to the user.
  amDemodulatorPtr->displayInternalInformation();

  // Display FM demodulator information to the user.
  fmDemodulatorPtr->displayInternalInformation();

  // Display wideband FM demodulator information to the user.
  wbFmDemodulatorPtr->displayInternalInformation();

  // Display wideband SSB demodulator information to the user.
  ssbDemodulatorPtr->displayInternalInformation();

  return;

} // displayInternalInformation

/*****************************************************************************

  Name: eventConsumerProcedure

  Purpose: The purpose of this function is to handle event notification
  functionality of the system.

  Calling Sequence: eventConsumerProcedure(arg)

  Inputs:

    arg - A pointer to an argument.

  Outputs:

    None.

*****************************************************************************/
void Radio::eventConsumerProcedure(void *arg)
{
  Radio *mePtr;
  int numberOfBytesRead;
  int error;

  fprintf(stderr,"Entering Radio Event Consumer.\n");

  // Save the pointer to the object.
  mePtr = (Radio *)arg;

  // Allow this thread to run.
  mePtr->timeToExit = false;

  while (!mePtr->timeToExit)
  {
    if (mePtr->receiveEnabled)
    {
      if (!mePtr->requestReceiverToStop)
      {
        // Acquire the radio lock.
        pthread_mutex_lock(&mePtr->radioLock);

        // Perform what is needed to read IQ samples.
        error = rtlsdr_read_sync((rtlsdr_dev_t *)mePtr->devicePtr,
                                 mePtr->receiveBufferPtr, 
                                 RECEIVE_BUFFER_SIZE,
                                 &numberOfBytesRead);

        // Indicate that more samples have been received.
        mePtr->receiveTimeStamp += (uint32_t )(numberOfBytesRead >> 1);

        // Send the IQ data to the data consumer system.
        mePtr->dataConsumerPtr->acceptData(mePtr->receiveTimeStamp,
                                           mePtr->receiveBufferPtr,
                                           (uint32_t)numberOfBytesRead);

        // Indicate that one more block of data has been received.
        mePtr->receiveBlockCount++;

        // Release the radio lock.
        pthread_mutex_unlock(&mePtr->radioLock);
      } // if
    } // if
    else
    {
      // Be nice to the system.
      usleep(5000);
    } // else
  } // while */

  fprintf(stderr,"Exiting Radio Event Consumer.\n");

  pthread_exit(0);

} // eventConsumerProcedure

