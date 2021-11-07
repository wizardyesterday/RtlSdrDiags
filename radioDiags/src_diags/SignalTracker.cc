#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "SignalTracker.h"

/*****************************************************************************

  Name: SignalTracker

  Purpose: The purpose of this function is to serve as the contructor for
  an instance of a SignalTracker.

  Calling Sequence: SignalTracker(threshold)

  Inputs:

    threshold - The detection threshold that is used to determine signal
    presence or absense..

 Outputs:

    None.

*****************************************************************************/
SignalTracker::SignalTracker(uint32_t threshold)
{

  // Associate the signal tracker with this object.
  detectorPtr = new SignalDetector(threshold);

  // Keep this around for queries.
  this->threshold = threshold;

  // Initially, there is no signal.
  state = NoSignal;

} // SignalTracker

/*****************************************************************************

  Name: ~SignalTracker

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of a SignalTracker.

  Calling Sequence: ~SignalTracker()

  Inputs:

    detectorPtr - A pointer to an instance of a signal detector.

 Outputs:

    None.

*****************************************************************************/
SignalTracker::~SignalTracker(void)
{

  // Release resources.
  if (detectorPtr != NULL)
  {
    delete detectorPtr;
  } // if

} // ~SignalTracker

/*****************************************************************************

  Name: reset

  Purpose: The purpose of this function is to reset the system so that it
  is ready to track the next signal.
 
  Calling Sequence: reset()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void SignalTracker::reset(void)
{

  // Get ready for the next signal.
  state = NoSignal;

  return;

} // reset

/*****************************************************************************

  Name: setThreshold

  Purpose: The purpose of this function is to set the detection threshold
  of the signal detector.

 
  Calling Sequence: setThreshold(threshold)

  Inputs:

    threshold - The threshold, in linear units, that determines whether
    or not a signal is present.

  Outputs:

    None.

*****************************************************************************/
void SignalTracker::setThreshold(uint32_t threshold)
{

  // This is probably not done too often.
  this->threshold = threshold;

  // Inform th detector of the new threshold.
  detectorPtr->setThreshold(threshold);

  return;

} // setThreshold

/*****************************************************************************

  Name: getThreshold

  Purpose: The purpose of this function is to retrieve detection threshold
  of the signal detector.

 
  Calling Sequence: threshold = getThreshold()

  Inputs:

    None.

  Outputs:

    threshold - The threshold, in linear units, that determines whether
    or not a signal is present.

*****************************************************************************/
uint32_t SignalTracker::getThreshold(void)
{

  return (threshold);

} // getThreshold

/*****************************************************************************

  Name: run

  Purpose: The purpose of this function is to set the detection threshold
  of the signal detector. The format of the signal, within the input buffer,
  appears below.

  phase0,magnitude0,phase1,magnitude1,

  where,

  magnitude = max(I,Q) + 0.5*min(I,Q),

  phase is represented as a signed fractional quantity with a sign bit,
  2 mantissa bits, and 13 fractional bits formated as SMMFFFFFFFFFFFFF.
  This represents a value bounded by -PI < phase < PI.  The only reason
  that this format is being presented is due to the fact that the FPGA
  presents phase in this format.

  Calling Sequence: signalPresenceIndicator = run(bufferPtr,bufferLength)

  Inputs:

    bufferPtr - A pointer to IQ data.  The representation is described
    above.

    bufferLength - The number of samples contained in the buffer pointed
    to by bufferPtr.

  Outputs:

    signalPresenceIndicator - The event associated with a signal.

*****************************************************************************/
uint16_t SignalTracker::run(int8_t *bufferPtr,uint32_t bufferLength)
{
  uint16_t signalPresenceIndicator;
  bool signalIsPresent;

  // Determine signal presence.
  signalIsPresent = detectorPtr->detectSignal(bufferPtr,bufferLength);

  switch (state)
  {
    case NoSignal:
    {
      if (signalIsPresent)
      {
        state = Tracking;
        signalPresenceIndicator = SIGNALTRACKER_STARTOFSIGNAL;
      } // if
      else
      {
        state = NoSignal;
        signalPresenceIndicator = SIGNALTRACKER_NOISE;
      } // else

      break;
    } // case

    case Tracking:
    {
      if (signalIsPresent)
      {
        state = Tracking;
        signalPresenceIndicator = SIGNALTRACKER_SIGNALPRESENT;
      } // if
      else
      {
        state = NoSignal;
        signalPresenceIndicator = SIGNALTRACKER_ENDOFSIGNAL;
      } // if

      break;
    } // case
  } // switch

  return (signalPresenceIndicator);

} // run

