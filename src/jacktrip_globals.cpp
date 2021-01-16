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
 * \file globals.cpp
 * \author Juan-Pablo Caceres
 * \date August 2008
 */

#include <iostream>

#if defined ( __LINUX__ )
    #include <sched.h>
    #include <unistd.h>
    #include <sys/types.h>
#endif //__LINUX__

#if defined ( __MAC_OSX__ )
    #include <mach/mach.h>
    #include <mach/mach_time.h>
    #include <mach/thread_policy.h>
#endif //__MAC_OSX__

#include "jacktrip_globals.h"


#if defined ( __MAC_OSX__ )

// The following function is taken from the chromium source code
// https://github.com/chromium/chromium/blob/master/base/threading/platform_thread_mac.mm
// For the following macOS implementation of the function setRealtimeProcessPriority() only: Copyright (c) 2012 The Chromium Authors. All rights reserved.

// Enables time-contraint policy and priority suitable for low-latency,
// glitch-resistant audio.
void setRealtimeProcessPriority() {
    // Increase thread priority to real-time.

    // Please note that the thread_policy_set() calls may fail in
    // rare cases if the kernel decides the system is under heavy load
    // and is unable to handle boosting the thread priority.
    // In these cases we just return early and go on with life.

    mach_port_t mach_thread_id = mach_thread_self();

    // Make thread fixed priority.
    thread_extended_policy_data_t policy;
    policy.timeshare = 0;  // Set to 1 for a non-fixed thread.
    kern_return_t result =
            thread_policy_set(mach_thread_id,
                              THREAD_EXTENDED_POLICY,
                              reinterpret_cast<thread_policy_t>(&policy),
                              THREAD_EXTENDED_POLICY_COUNT);
    if (result != KERN_SUCCESS) {
        std::cerr << "Failed to make thread fixed priority. " << result << std::endl;
        return;
    }

    // Set to relatively high priority.
    thread_precedence_policy_data_t precedence;
    precedence.importance = 63;
    result = thread_policy_set(mach_thread_id,
                               THREAD_PRECEDENCE_POLICY,
                               reinterpret_cast<thread_policy_t>(&precedence),
                               THREAD_PRECEDENCE_POLICY_COUNT);
    if (result != KERN_SUCCESS) {
        std::cerr << "Failed to set thread priority. " << result << std::endl;
        return;
    }

    // Most important, set real-time constraints.

    // Define the guaranteed and max fraction of time for the audio thread.
    // These "duty cycle" values can range from 0 to 1.  A value of 0.5
    // means the scheduler would give half the time to the thread.
    // These values have empirically been found to yield good behavior.
    // Good means that audio performance is high and other threads won't starve.
    const double kGuaranteedAudioDutyCycle = 0.75;
    const double kMaxAudioDutyCycle = 0.85;

    // Define constants determining how much time the audio thread can
    // use in a given time quantum.  All times are in milliseconds.

    // About 128 frames @44.1KHz
    const double kTimeQuantum = 2.9;

    // Time guaranteed each quantum.
    const double kAudioTimeNeeded = kGuaranteedAudioDutyCycle * kTimeQuantum;

    // Maximum time each quantum.
    const double kMaxTimeAllowed = kMaxAudioDutyCycle * kTimeQuantum;

    // Get the conversion factor from milliseconds to absolute time
    // which is what the time-constraints call needs.
    mach_timebase_info_data_t tb_info;
    mach_timebase_info(&tb_info);
    double ms_to_abs_time =
            (static_cast<double>(tb_info.denom) / tb_info.numer) * 1000000;

    thread_time_constraint_policy_data_t time_constraints;
    time_constraints.period = kTimeQuantum * ms_to_abs_time;
    time_constraints.computation = kAudioTimeNeeded * ms_to_abs_time;
    time_constraints.constraint = kMaxTimeAllowed * ms_to_abs_time;
    time_constraints.preemptible = 0;

    result = thread_policy_set(mach_thread_id,
                               THREAD_TIME_CONSTRAINT_POLICY,
                               reinterpret_cast<thread_policy_t>(&time_constraints),
                               THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (result != KERN_SUCCESS)
        std::cerr << "Failed to set thread realtime constraints. " << result << std::endl;

    return;
}

#endif //__MAC_OSX__


#if defined ( __LINUX__ )
//*******************************************************************************
void setRealtimeProcessPriority()
{
    int priority = sched_get_priority_max(SCHED_FIFO); // 99 is the highest possible
#ifdef __UBUNTU__
    priority = 95; // anything higher is silently ignored by Ubuntu 18.04
#endif
    priority = 3;

    struct sched_param sp = { .sched_priority = priority };

    if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1) {
        std::cerr << "Failed to set the scheduler policy and priority." << std::endl;;
    }
}
#endif //__LINUX__


#if defined ( __WIN_32__ )
void setRealtimeProcessPriority()
{
    if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == 0)
    {
        std::cerr << "Failed to set process priority class." << std::endl;
    }
    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) == 0)
    {
        std::cerr << "Failed to set thread priority." << std::endl;
    }
}
#endif //__WIN_32__
