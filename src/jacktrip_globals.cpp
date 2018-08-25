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
#include <cstring>
#include <cstdio>

#include "jacktrip_globals.h"
#include "jacktrip_types.h"

#if defined ( __LINUX__ )
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#endif //__LINUX__

#if defined ( __MAC_OSX__ )
#include <mach/mach.h>
#include <mach/thread_policy.h>

//#include <mach/processor.h>

//#include <mach/clock.h>
//#include <sys/kernel.h>
//#include <mach/kern/clock.h>

//#include <Kernel/kern/clock.h>
//#include <kern/kern_types.h>
//m#include <kern/kern_types.h>
//#include <Kernel/kern/clock.h>
//#include <kern/clock.h>


//#include <assert.h>
//#include <CoreServices/CoreServices.h>
//#include <mach/mach.h>
//#include <mach/mach_time.h>
//#include <unistd.h>




//#include <mach/machine.h>
//#include <mach/mach_time.h>
//#include <mach/thread_call.h>
//#include <mach/processor.h>
//#include <mach/macro_help.h>

#endif //__MAC_OSX__


#if defined ( __MAC_OSX__ )
//*******************************************************************************
//http://developer.apple.com/DOCUMENTATION/Darwin/Conceptual/KernelProgramming/scheduler/chapter_8_section_4.html
//http://lists.apple.com/archives/darwin-dev/2007/Sep/msg00035.html
int set_realtime(int period, int computation, int constraint)
{
  //AbsoluteTime time;
  //clock_get_uptime((uint64_t *)&time);

  //uint64_t result;
  //clock_get_uptime(&result);
  //clock_get_system_microtime(&result,&result);

  struct thread_time_constraint_policy ttcpolicy;
  int ret;
  
  ttcpolicy.period=period; // HZ/160
  ttcpolicy.computation=computation; // HZ/3300;
  ttcpolicy.constraint=constraint; // HZ/2200;
  ttcpolicy.preemptible=1;
  
  if ((ret=thread_policy_set(mach_thread_self(),
			     THREAD_TIME_CONSTRAINT_POLICY, (thread_policy_t)&ttcpolicy,
			     THREAD_TIME_CONSTRAINT_POLICY_COUNT)) != KERN_SUCCESS) {
    fprintf(stderr, "set_realtime() failed.\n");
    return 0;
  }
  return 1;
}
#endif //__MAC_OSX__ 


#if defined ( __LINUX__ )
//*******************************************************************************
int get_fifo_priority (bool half)
{
  int min, max, priority;
  min = sched_get_priority_min (SCHED_FIFO);
  max = sched_get_priority_max (SCHED_FIFO);
  if (half) {
    priority = (max  - (max - min) / 2); }
  else {
    priority = max; }

  //priority=min;
  return priority;
}
#endif //__LINUX__


#if defined ( __LINUX__ )
//*******************************************************************************
int set_fifo_priority (bool half)
{
  struct sched_param p;
  int priority;
  //  scheduling priority


  if (true) // (!getuid () || !geteuid ())
    {
      priority = get_fifo_priority (half);
#ifdef __UBUNTU__
      priority = 95; // anything higher is ignored silently by Ubuntu 18.04
#endif
      fprintf (stderr,
           "\n running elevated priority for network threads, but Ubuntu 18.04 maxed out at priority %d rather than 99 in Fedora 28\n",
           priority);
      p.sched_priority = priority;

      if (sched_setscheduler (0, SCHED_FIFO, &p) == -1)
	{
	  fprintf (stderr,
           "\ncould not activate scheduling with priority %d\n",
		   priority);
	  return -1;
	}
      seteuid (getuid ());
      //fprintf (stderr,
      //       "\nset scheduling priority to %d (SCHED_FIFO)\n",
      //       priority);
    }
  else
    {
      fprintf (stderr,
	       "\ninsufficient privileges to set scheduling priority\n");
      priority = 0;
    }
  return priority;
}
#endif //__LINUX__


#if defined ( __LINUX__ )
//*******************************************************************************
int set_realtime_priority (void)
{
  struct sched_param schp;

  memset (&schp, 0, sizeof (schp));
  schp.sched_priority = sched_get_priority_max (SCHED_FIFO);
  if (sched_setscheduler (0, SCHED_FIFO, &schp) != 0)
    {
      perror ("set_scheduler");
      return -1;
    }
  return 0;
}
#endif //__LINUX__


#if defined ( __WIN_32__ )
int win_priority()
{
  if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == 0)
  {
    printf("set Priority Class failed \n");
    return -1;
  }
  if(SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) == 0)
  {
    printf("set Thread Priority failed \n");
    return -1;
  }
  return GetThreadPriority(GetCurrentThread());
}
#endif //__WIN_32__


void set_crossplatform_realtime_priority()
{
#if defined ( __LINUX__ )
  set_fifo_priority (false);
#endif //__LINUX__
#if defined ( __MAC_OSX__ )
  set_realtime(1250000,60000,90000);
#endif //__MAC_OSX__
#if defined __WIN_32__
  win_priority();
#endif

}


