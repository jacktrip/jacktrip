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


#include "globals.h"

#include <sched.h>


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


//*******************************************************************************
int set_fifo_priority (bool half)
{
  struct sched_param p;
  int priority;
  //  scheduling priority


  if (true) // (!getuid () || !geteuid ())
    {
      priority = get_fifo_priority (half);
      p.sched_priority = priority;

      if (sched_setscheduler (0, SCHED_FIFO, &p) == -1)
	{
	  fprintf (stderr,
		   "\ncould not activate scheduling with priority %d\n",
		   priority);
	  return -1;
	}
      seteuid (getuid ());
      fprintf (stderr,
	       "\nset scheduling priority to %d (SCHED_FIFO)\n",
	       priority);
    }
  else
    {
      fprintf (stderr,
	       "\ninsufficient privileges to set scheduling priority\n");
      priority = 0;
    }
  return priority;
}


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
#endif
