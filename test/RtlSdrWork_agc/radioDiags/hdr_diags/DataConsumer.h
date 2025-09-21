//**********************************************************************
// file name: DataConsumer.h
//**********************************************************************

#ifndef _DATACONSUMER_H_
#define _DATACONSUMER_H_

#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "MessageQueue.h"

// Make sure this agrees with the Radio RECEIVE_BUFFER_SIZE.
#define DATA_CONSUMER_BUFFER_SIZE (32768)

// This should be enough messages to queue up.
#define DATA_CONSUMER_NUMBER_OF_MESSAGES (64)

class IqDataProcessor;

struct iqMessage
{
  uint32_t timeStamp;
  uint32_t byteCount;
  uint8_t buffer[DATA_CONSUMER_BUFFER_SIZE];
};

class DataConsumer
{
  public:

  DataConsumer(IqDataProcessor *dataProcessorPtr);
  ~DataConsumer(void);

  void start(void);
  void stop(void);
  void reset(void);

  void acceptData(uint32_t timeStamp,
                  void *bufferPtr,
                  uint32_t bufferLength);

  void displayInternalInformation(void);

  bool done(void);

  private:

  // Utility functions.
  static void dataConsumerProcedure(void *arg);

  // Attributes.

  // Coherent IQ detection support.
  uint32_t lastTimeStamp;
  uint32_t shortBlockCount;

  // Data processor callback object.
  IqDataProcessor *dataProcessorPtr;

  // Control information.
  bool enabled;
  bool timeToExit;

  // Message support.
  unsigned long messageIndex;
  iqMessage message[DATA_CONSUMER_NUMBER_OF_MESSAGES];
  MessageQueue *iqMessageQueuePtr;

  // Data consumer thread support.
  pthread_t dataConsumerThread;

};

#endif // _DATACONSUMER_H_
