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
#include <unordered_map>
#include <vector>

#if defined(RT_AUDIO)
#include <RtAudio.h>
#endif

#if defined(USE_WEAK_JACK)
#include "weak_libjack.h"
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

class Audio
{
   public:
    enum class Backend : unsigned int {
        NONE        = 0,
        JACK        = 1 << 0,  ///< Jack Mode
        ALSA        = 1 << 1,  ///< RtAudio Mode
        PulseAudio  = 1 << 2,
        OSS         = 1 << 3,
        CoreAudio   = 1 << 4,
        WASAPI      = 1 << 5,
        ASIO        = 1 << 6,
        DirectSound = 1 << 7,
        Pipewire    = 1 << 8,  ///< Pipewire Mode
    };

    Audio() { fillAvailableBackends(); }
    //    availableBackends(std::vector<AudioInterfaceMode>&)->void;

#if !defined(NO_JACK)
#if !defined(USE_WEAK_JACK)
    static constexpr bool isJackAvailable() { return true; }
#else
    static constexpr bool isJackAvailable() { return have_libjack() == 0; }
#endif
#else
    static constexpr bool isJackAvailable() { return false; }
#endif

#if defined(RT_AUDIO)
    std::vector<RtAudio::Api> m_RtAudioApis;
#endif

    void fillAvailableBackends() noexcept
    {
#if defined(RT_AUDIO)
        RtAudio::getCompiledApi(m_RtAudioApis);
        for (RtAudio::Api& rtapi : m_RtAudioApis) {
            switch (rtapi) {
            case RtAudio::LINUX_ALSA:
                m_availableBackends.try_emplace(Backend::ALSA, "ALSA");
                break;
            case RtAudio::LINUX_PULSE:
                m_availableBackends.try_emplace(Backend::PulseAudio, "PulseAudio");
                break;
            case RtAudio::LINUX_OSS:
                m_availableBackends.try_emplace(Backend::OSS, "OSS");
                break;
            case RtAudio::UNIX_JACK:
                break;
            case RtAudio::MACOSX_CORE:
                m_availableBackends.try_emplace(Backend::CoreAudio, "CoreAudio");
                break;
            case RtAudio::WINDOWS_WASAPI:
                m_availableBackends.try_emplace(Backend::WASAPI, "WASAPI");
                break;
            case RtAudio::WINDOWS_ASIO:
                m_availableBackends.try_emplace(Backend::ASIO, "ASIO");
                break;
            case RtAudio::WINDOWS_DS:
                m_availableBackends.try_emplace(Backend::DirectSound, "DirectSound");
                break;
            default:
                break;
            }
        }
#endif
        if (isJackAvailable()) {
            m_availableBackends.try_emplace(Backend::JACK, "JACK");
        }
    }

    void getAvailableBackendNames(std::vector<std::string>& backendNames)
    {
        for (const auto& backend : m_availableBackends) {
            backendNames.push_back(backend.second);
        }
    }

    auto getAudioBackend() -> Backend { return mBackend; }

    bool setAudioBackendByString(std::string backend)
    {
        for (const auto& [available_enum, available_string] : m_availableBackends) {
            if (backend == available_string) {
                mBackend = available_enum;
                return true;
            }
        }
        return false;
    }

   private:
    std::unordered_map<Backend, std::string> m_availableBackends;
    Backend mBackend = Backend::JACK;
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
