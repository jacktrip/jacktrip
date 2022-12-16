//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2022 Juan-Pablo Caceres, Chris Chafe.
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
 * \file AudioInterfaceMode.h
 * \author Matt Horton
 * \date December 2022
 */

enum class AudioInterfaceMode {
    JACK,     ///< Jack Mode
    RTAUDIO,  ///< RtAudio Mode
    ALL,
    NONE
};

#ifdef RT_AUDIO
#ifndef NO_JACK
constexpr AudioInterfaceMode mode = AudioInterfaceMode::ALL;
#else
constexpr AudioInterfaceMode mode = AudioInterfaceMode::RTAUDIO;
#endif
#else
#ifndef NO_JACK
constexpr AudioInterfaceMode mode = AudioInterfaceMode::JACK;
#else
constexpr AudioInterfaceMode mode = AudioInterfaceMode::NONE;
#endif
#endif

template<AudioInterfaceMode backend>
constexpr auto isBackendAvailable()
{
    if constexpr (backend == AudioInterfaceMode::RTAUDIO) {
        if (mode == AudioInterfaceMode::RTAUDIO || mode == AudioInterfaceMode::ALL) {
            return true;
        } else {
            return false;
        }
    } else if constexpr (backend == AudioInterfaceMode::JACK) {
        if (mode == AudioInterfaceMode::JACK || mode == AudioInterfaceMode::ALL) {
            return true;
        } else {
            return false;
        }
    } else if constexpr (backend == AudioInterfaceMode::ALL) {
        if (mode == AudioInterfaceMode::ALL) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}