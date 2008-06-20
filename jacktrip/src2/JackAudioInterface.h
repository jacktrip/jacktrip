/**
 * \file JackAudioInterface.h
 * author: Juan-Pablo Caceres
 * date: June 2008
 */


#ifndef __JACKAUDIOINTERFACE_H__
#define __JACKAUDIOINTERFACE_H__

#include <iostream>
#include <jack/jack.h>

#include <QtGlobal> //For QT4 types
#include <QVector>


/** \brief 
 *
 */
class JackAudioInterface
{
public:  
  JackAudioInterface(int NumInChans, int NumOutChans);
  //JackAudioInterface(int NumChans);
  virtual ~JackAudioInterface();

  quint32 getSampleRate() const;
  quint32 getBufferSize() const;

private:
  void openClientConnection();
  void createChannels();
  //void jackShutdown (void* arg);
  
  int mNumInChans;///< Number of Input Channels
  int mNumOutChans; ///<  Number of Output Channels
  int mNumFrames; ///< Buffer block size, in samples
  jack_client_t* mClient; ///< Jack Client
  QVector<jack_port_t*> mInPorts; ///< Vector of Input Ports (Channels)
  QVector<jack_port_t*> mOutPorts; ///< Vector of Output Ports (Channels)
};

#endif
