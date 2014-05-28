/*
   Copyright (C) 2004 Paul Davis

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#ifndef __jack_thread_h__
#define __jack_thread_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include <jack/systemdeps.h>

/** @file thread.h
 *
 * Library functions to standardize thread creation for JACK and its
 * clients.  These interfaces hide some system variations in the
 * handling of realtime scheduling and associated privileges.
 */

/**
 * @defgroup ClientThreads Creating and managing client threads
 * @{
 */

 /**
 * @returns if JACK is running with realtime scheduling, this returns
 * the priority that any JACK-created client threads will run at.
 * Otherwise returns -1.
 */

int jack_client_real_time_priority (jack_client_t*);

/**
 * @returns if JACK is running with realtime scheduling, this returns
 * the maximum priority that a JACK client thread should use if the thread
 * is subject to realtime scheduling. Otherwise returns -1.
 */

int jack_client_max_real_time_priority (jack_client_t*);

/**
 * Attempt to enable realtime scheduling for a thread.  On some
 * systems that may require special privileges.
 *
 * @param thread POSIX thread ID.
 * @param priority requested thread priority.
 *
 * @returns 0, if successful; EPERM, if the calling process lacks
 * required realtime privileges; otherwise some other error number.
 */
int jack_acquire_real_time_scheduling (pthread_t thread, int priority);

/**
 * Create a thread for JACK or one of its clients.  The thread is
 * created executing @a start_routine with @a arg as its sole
 * argument.
 *
 * @param client the JACK client for whom the thread is being created. May be
 * NULL if the client is being created within the JACK server.
 * @param thread place to return POSIX thread ID.
 * @param priority thread priority, if realtime.
 * @param realtime true for the thread to use realtime scheduling.  On
 * some systems that may require special privileges.
 * @param start_routine function the thread calls when it starts.
 * @param arg parameter passed to the @a start_routine.
 *
 * @returns 0, if successful; otherwise some error number.
 */
int jack_client_create_thread (jack_client_t* client,
                               pthread_t *thread,
                               int priority,
                               int realtime, 	/* boolean */
                               void *(*start_routine)(void*),
                               void *arg);

/**
 * Drop realtime scheduling for a thread.
 *
 * @param thread POSIX thread ID.
 *
 * @returns 0, if successful; otherwise an error number.
 */
int jack_drop_real_time_scheduling (pthread_t thread);

/**
 * Stop the thread, waiting for the thread handler to terminate.
 *
 * @param thread POSIX thread ID.
 *
 * @returns 0, if successful; otherwise an error number.
 */
int jack_client_stop_thread(jack_client_t* client, pthread_t thread);

/**
 * Cancel the thread then waits for the thread handler to terminate.
 *
 * @param thread POSIX thread ID.
 *
 * @returns 0, if successful; otherwise an error number.
 */
 int jack_client_kill_thread(jack_client_t* client, pthread_t thread);

#ifndef WIN32

 typedef int (*jack_thread_creator_t)(pthread_t*,
				     const pthread_attr_t*,
				     void* (*function)(void*),
				     void* arg);
/**
 * This function can be used in very very specialized cases
 * where it is necessary that client threads created by JACK
 * are created by something other than pthread_create(). After
 * it is used, any threads that JACK needs for the client will
 * will be created by calling the function passed to this
 * function.
 *
 * No normal application/client should consider calling this.
 * The specific case for which it was created involves running
 * win32/x86 plugins under Wine on Linux, where it is necessary
 * that all threads that might call win32 functions are known
 * to Wine.
 *
 * @param creator a function that creates a new thread
 *
 */
void jack_set_thread_creator (jack_thread_creator_t creator);

#endif

/* @} */

#ifdef __cplusplus
}
#endif

#endif /* __jack_thread_h__ */
