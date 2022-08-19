//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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

#include <QObject>
#include <QThread>

/** \brief Interface for the process plugins to add to the JACK callback process in
 * JackAudioInterface
 *
 * This class contains the same methods of the FAUST dsp class. A mydsp class can inherit
 * from this class the same way it inherits from dsp. Subclass should implement all
 * methods except init, which is optional for processing that are sampling rate dependent
 * or that need specific initialization.
 */
class ProcessPlugin : public QObject
{
    Q_OBJECT;

   public:
    /// \brief The Class Constructor
    ProcessPlugin(){};
    /// \brief The Class Destructor
    virtual ~ProcessPlugin(){};

    /// \brief Return Number of Input Channels
    virtual int getNumInputs() = 0;
    /// \brief Return Number of Output Channels
    virtual int getNumOutputs() = 0;

    // virtual void buildUserInterface(UI* interface) = 0;

    virtual const char* getName() const = 0;  // get name of DERIVED class

    /** \brief Do proper Initialization of members and class instances. By default this
     * initializes the Sampling Frequency. If a class instance depends on the
     * sampling frequency, it should be initialize here.
     */
    virtual void init(int samplingRate)
    {
        fSamplingFreq = samplingRate;
        if (verbose) {
            printf("%s: init(%d)\n", getName(), samplingRate);
        }
    }
    virtual bool getInited() { return inited; }
    virtual void setVerbose(bool v) { verbose = v; }

    virtual void setOutgoingToNetwork(bool toNetwork)
    {
        outgoingPluginToNetwork = toNetwork;
    }

    /// \brief Compute process
    virtual void compute(int nframes, float** inputs, float** outputs) = 0;

    /**
     * @brief This function may optionally be used by plugins. This is useful
     * if the number of audio channels in the parent audio interface has changed
     * after the plugin instance was instantiated, to tell the plugin to modify its
     * functionality.
     */
    virtual void updateNumChannels(int /*nChansIn*/, int /*nChansOut*/) { return; };

   protected:
    int fSamplingFreq;  //< Faust Data member, Sampling Rate
    bool inited                  = false;
    bool verbose                 = false;
    bool outgoingPluginToNetwork = false;  //< Tells the plugin if it processes audio
                                           // going into or out of the network
};

#endif
