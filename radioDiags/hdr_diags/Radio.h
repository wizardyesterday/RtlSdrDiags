//**********************************************************************
// file name: Radio.h
//**********************************************************************

#ifndef _RADIO_H_
#define _RADIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include "IqDataProcessor.h"
#include "DataConsumer.h"
#include "DataProvider.h"
#include "AmDemodulator.h"
#include "FmDemodulator.h"
#include "WbFmDemodulator.h"
#include "SsbDemodulator.h"

class Radio
{
  public:

  Radio(int deviceNumber,uint32_t txSampleRate,uint32_t rxSampleRate);
  ~Radio(void);

  // Setters.
  bool setReceiveFrequency(uint64_t frequency);
  bool setReceiveBandwidth(uint32_t bandwidth);
  bool setReceiveGainInDb(uint32_t gain);
  bool setReceiveSampleRate(uint32_t sampleRate);
  bool setReceiveWarpInPartsPerMillion(int warp);
  bool setTransmitFrequency(uint64_t frequency);
  bool setTransmitBandwidth(uint32_t bandwidth);
  bool setTransmitGainInDb(uint32_t gain);
  bool setTransmitSampleRate(uint32_t sampleRate);

  // Getters.
  uint64_t getReceiveFrequency(void);
  uint32_t getReceiveBandwidth(void);
  uint32_t getReceiveGainInDb(void);
  uint32_t getReceiveSampleRate(void);
  int getReceiveWarpInPartsPerMillion(void);
  uint64_t getTransmitFrequency(void);
  uint32_t getTransmitBandwidth(void);
  uint32_t getTransmitGainInDb(void);
  uint32_t getTransmitSampleRate(void);
  DataProvider *getDataProvider();

  bool isReceiving(void);
  bool isTransmitting(void);
  bool startReceiver(void);
  void stopReceiver(void);
  void startTransmitter(void);
  void stopTransmitter(void);
  void startTransmitData(void);

  // Demodulator support.
  void setDemodulatorMode(uint8_t mode);
  void setAmDemodulatorGain(float gain);
  void setFmDemodulatorGain(float gain);
  void setWbFmDemodulatorGain(float gain);
  void setSsbDemodulatorGain(float gain);

  void displayInternalInformation(void);

  private:

  // Utility functions.
  bool setupReceiver(void);
  bool setupTransmitter(void);
  void tearDownReceiver(void);
  void tearDownTransmitter(void);

  static void eventConsumerProcedure(void *arg);

  // Attributes.

  //------------------------------------------------------------
  // Receiver configuration.
  //------------------------------------------------------------
  uint64_t receiveFrequency;
  uint32_t receiveSampleRate;
  uint32_t receiveBandwidth;
  uint32_t receiveGainInDb;
  int receiveWarpInPartsPerMillion;

  // RtlSdr device support.
  int deviceNumber;
  void *devicePtr;

  // Transmitter configuration.
  uint32_t transmitGainInDb;
  uint32_t transmitBandwidth;
  uint64_t transmitFrequency;
  uint32_t transmitSampleRate;

  // Buffer support.
  uint8_t *receiveBufferPtr;
  uint8_t *transmitBufferPtr;

  // Data handler support.
  IqDataProcessor *receiveDataProcessorPtr;
  DataConsumer *dataConsumerPtr;
  DataProvider *dataProviderPtr;

  // Demodulator support.
  AmDemodulator *amDemodulatorPtr;
  FmDemodulator *fmDemodulatorPtr;
  WbFmDemodulator *wbFmDemodulatorPtr;
  SsbDemodulator *ssbDemodulatorPtr;

  // Control information.
  bool receiveEnabled;
  bool transmitEnabled;
  bool transmittingData;
  bool timeToExit;

  // Event consumer thread support.
  bool requestReceiverToStop;
  pthread_mutex_t radioLock;
  pthread_t eventConsumerThread;

  // Statistics.
  uint32_t receiveTimeStamp;
  uint32_t receiveBlockCount;
  uint32_t transmitBlockCount;
};

#endif // _RADIO_H_
