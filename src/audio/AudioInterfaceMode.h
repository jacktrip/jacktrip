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
 * \author Matt Horton, Nils Tonnätt
 * \date December 2022
 */

#pragma once

#include <type_traits>
#include <vector>

#if defined(RT_AUDIO)
#include <RtAudio.h>
#endif

#if defined(__cpp_lib_to_underlying)
#include <utility>
using std::to_underlying(auto value);

#else
template<class T>
constexpr std::underlying_type_t<T> to_underlying(T value) noexcept
{
    return static_cast<std::underlying_type_t<T> >(value);
}
#endif

enum class AudioInterfaceMode : unsigned int {
    NONE     = 0,
    JACK     = 1 << 0,  ///< Jack Mode
    RTAUDIO  = 1 << 1,  ///< RtAudio Mode
    PIPEWIRE = 1 << 2,  ///< Pipewire Mode
    ALL      = JACK | RTAUDIO
};

#ifdef RT_AUDIO
#define RT_AUDIO_AVAIL AudioInterfaceMode::RTAUDIO
#else
#define RT_AUDIO_AVAIL AudioInterfaceMode::NONE
#endif

#ifndef NO_JACK
#define JACK_AVAIL AudioInterfaceMode::JACK
#else
#define JACK_AVAIL AudioInterfaceMode::NONE
#endif

#ifdef PIPEWIRE
#define PW_AVAIL AudioInterfaceMode::PIPEWIRE
#else
#define PW_AVAIL AudioInterfaceMode::NONE
#endif

constexpr std::underlying_type_t<AudioInterfaceMode> mode =
    to_underlying(RT_AUDIO_AVAIL) | to_underlying(JACK_AVAIL) | to_underlying(PW_AVAIL);

template<AudioInterfaceMode backend>
constexpr bool isBackendAvailable()
{
    return (mode & to_underlying(backend)) != 0;
}
