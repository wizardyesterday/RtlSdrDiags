//**********************************************************************
// file name: AutomaticGainControl.h
//**********************************************************************

#ifndef _AUTOMATICGAINCONTROL_H_
#define _AUTOMATICGAINCONTROL_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "DbfsCalculator.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// New AGC algorithms will be added as time progresses.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
#define AGC_TYPE_LOWPASS (0)
#define AGC_TYPE_HARRIS (1)
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

class AutomaticGainControl
{
  public:

  AutomaticGainControl(void *radioPtr,int32_t operatingPointInDbFs);

  ~AutomaticGainControl(void);

  void setOperatingPoint(int32_t operatingPointInDbFs);
  bool setAgcFilterCoefficient(float coefficient);
  bool setType(uint32_t type);
  bool setDeadband(uint32_t deadbandInDb);
  bool setBlankingLimit(uint32_t blankingLimit);
  bool enable(void);
  bool disable(void);
  bool isEnabled(void);
  void run(uint32_t signalMagnitude);
  void runLowpass(uint32_t signalMagnitude);
  void runHarris(uint32_t signalMagnitude);
  void displayInternalInformation(void);

  // This is needed by a static callback.
  uint32_t getSignalMagnitude(void);

  private:

  //*****************************************
  // Utility functions.
  //*****************************************
  void resetBlankingSystem(void);

  //*****************************************
  // Attributes.
  //*****************************************
  // The AGC algorithm to be used.
  uint32_t agcType;

  // These parameters are sometimes needed to avoid transients.
  uint32_t blankingCounter;
  uint32_t blankingLimit;
  bool receiveGainWasAdjusted;

  // Yes, we need some deadband.
  int32_t deadbandInDb;

  // If true, the AGC is running.
  bool enabled;

  // The goal.
  int32_t operatingPointInDbFs;

  // AGC lowpass filter coefficient for baseband gain filtering.
  float alpha;

  // System gains.
  uint32_t ifGainInDb;
 
  // Filtered gain.
  float filteredIfGainInDb;

  // Magnitude of the latest signal.
  uint32_t signalMagnitude;

  // Signal level before amplification.
  int32_t normalizedSignalLevelInDbFs;

  void *radioPtr;
  void *dataProcessorPtr;
  DbfsCalculator *calculatorPtr;
};

#endif // _AUTOMATICGAINCONTROL_H_
