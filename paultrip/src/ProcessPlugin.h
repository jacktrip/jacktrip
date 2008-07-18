//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.
  
  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:
  
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file ProcessPlugin.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __PROCESSPLUGIN_H__
#define __PROCESSPLUGIN_H__

#include <jack/jack.h>


/** \brief Interface for the process plugins.
 *
 * This class contains the same methods of the FAUST dsp class. A mydsp class can inherit from
 * this class the same wway it inherits from dsp. The class contains also the interface with
 * Jack and the other classes of PaulTrip for network streaming.
 */
class ProcessPlugin 
{
public:

  ProcessPlugin() {};
  virtual ~ProcessPlugin() {};
  

  /** \brief Faust Pure Virtual Methods
   */
  virtual int getNumInputs() = 0;
  virtual int getNumOutputs() = 0;
  //virtual void buildUserInterface(UI* interface) = 0;
  virtual void init(int samplingRate) = 0;
  
  /** \brief
   *
   */
  virtual void compute(jack_nframes_t nframes, float** inputs, float** outputs) = 0;
  //virtual void compute(jack_nframes_t nframes,
  //		       QVector<sample_t*>& inputs,
  //	       QVector<sample_t*>& outputs) = 0;
  
protected:
  int fSamplingFreq; ///< Faust Data member, Sampling Rate
};

#endif
