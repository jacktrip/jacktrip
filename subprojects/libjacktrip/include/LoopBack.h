//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
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
 * \file LoopBack.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */


/** \brief Connect Inputs to Outputs
 *
 */
#ifndef __LOOPBACK_H__
#define __LOOPBACK_H__

#include "ProcessPlugin.h"


/** \brief This Class just copy audio from its inputs to its outputs.
 *
 * It can be use to do loopback without the need to externally connect channels
 * in JACK. Note that if you <EM>do</EM> connect the channels in jack, you'll
 * be effectively multiplying the signal by 2.
 */
class LoopBack : public ProcessPlugin
{
public:
    /// \brief The class constructor sets the number of channels to connect as loopback
    LoopBack(int numchans) { mNumChannels = numchans; };
    /// \brief The class destructor
    virtual ~LoopBack() {};

    virtual int getNumInputs() { return(mNumChannels); };
    virtual int getNumOutputs() { return(mNumChannels); };
    virtual void compute(int nframes, float** inputs, float** outputs);

private:
    int mNumChannels;
};

#endif
