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
#include "AmDemodulator.h"
#include "FmDemodulator.h"
#include "WbFmDemodulator.h"
#include "SsbDemodulator.h"

class Radio
{
  public:

  Radio(int deviceNumber,uint32_t rxSampleRate);
  ~Radio(void);

  // Setters.
  bool setReceiveFrequency(uint64_t frequency);
  bool setReceiveBandwidth(uint32_t bandwidth);
  bool setReceiveGainInDb(uint32_t gain);
  bool setReceiveSampleRate(uint32_t sampleRate);
  bool setReceiveWarpInPartsPerMillion(int warp);

  // Getters.
  uint64_t getReceiveFrequency(void);
  uint32_t getReceiveBandwidth(void);
  uint32_t getReceiveGainInDb(void);
  uint32_t getReceiveSampleRate(void);
  int getReceiveWarpInPartsPerMillion(void);

  bool isReceiving(void);
  bool startReceiver(void);
  void stopReceiver(void);

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
  void tearDownReceiver(void);

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

  // Buffer support.
  uint8_t *receiveBufferPtr;

  // Data handler support.
  IqDataProcessor *receiveDataProcessorPtr;
  DataConsumer *dataConsumerPtr;

  // Demodulator support.
  AmDemodulator *amDemodulatorPtr;
  FmDemodulator *fmDemodulatorPtr;
  WbFmDemodulator *wbFmDemodulatorPtr;
  SsbDemodulator *ssbDemodulatorPtr;

  // Control information.
  bool receiveEnabled;
  bool timeToExit;

  // Event consumer thread support.
  bool requestReceiverToStop;
  pthread_mutex_t radioLock;
  pthread_t eventConsumerThread;

  // Statistics.
  uint32_t receiveTimeStamp;
  uint32_t receiveBlockCount;
};

#endif // _RADIO_H_
