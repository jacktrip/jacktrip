/**
 * \file RtAudioInterface.h
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#ifndef __RTAUDIOINTERFACE_H__
#define __RTAUDIOINTERFACE_H__

#include "JackAudioInterface.h"
#include "RtAudio.h"

class RtAudioInterface : public JackAudioInterface
{
public:

  /** \brief The class constructor
   * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
   * \param NumInChans Number of Input Channels
   * \param NumOutChans Number of Output Channels
   * \param AudioBitResolution Audio Sample Resolutions in bits
   * \param ClientName Client name in Jack
   */
  RtAudioInterface(JackTrip* jacktrip,
                   int NumInChans = 2, int NumOutChans = 2,
                   audioBitResolutionT AudioBitResolution = JackAudioInterface::BIT16,
                   const char* ClientName = "JamTest");

  virtual ~RtAudioInterface();

  /** \brief Setup the client
   */
  virtual void setup();


private:

  RtAudio* mRtAudio; ///< RtAudio class

};

#endif // __RTAUDIOINTERFACE_H__
