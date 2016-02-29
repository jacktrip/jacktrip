/*
Copyright (C) 2004-2009 Grame

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef __jack_systemdeps_h__
#define __jack_systemdeps_h__

#ifdef WIN32

#include <windows.h>

#ifdef _MSC_VER     /* Microsoft compiler */
    #define __inline__ inline
    #ifndef int8_t
        typedef char int8_t;
        typedef unsigned char uint8_t;
        typedef short int16_t;
        typedef unsigned short uint16_t;
        typedef long int32_t;
        typedef unsigned long uint32_t;
        typedef LONGLONG int64_t;
        typedef ULONGLONG uint64_t;
    #endif
    #ifndef pthread_t
        typedef HANDLE pthread_t;
    #endif
#elif __MINGW32__   /* MINGW */
    #include <stdint.h>
    #include <sys/types.h>
    #ifndef pthread_t
        typedef HANDLE pthread_t;
    #endif
#else               /* other compilers ...*/
    #include <inttypes.h>
    #include <pthread.h>
    #include <sys/types.h>
#endif

#endif /* WIN32 */

#if defined(__APPLE__) || defined(__linux__) || defined(__sun__) || defined(sun)
    #include <inttypes.h>
    #include <pthread.h>
    #include <sys/types.h>
#endif /* __APPLE__ || __linux__ || __sun__ || sun */

#endif
