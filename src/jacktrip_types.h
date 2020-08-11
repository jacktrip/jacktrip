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
 * \file jacktrip_types_jacktrip.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */


#ifndef __JACKTRIP_TYPES_H__
#define __JACKTRIP_TYPES_H__

//#include <jack/types.h>
#include <QtGlobal> //For QT4 types

//namespace JackTripNamespace
//{

  //-------------------------------------------------------------------------------
  /** \name Audio typedefs
 *
 */
  //-------------------------------------------------------------------------------
  //@{
  /// Audio sample type
  //typedef jack_default_audio_sample_t sample_t;
  typedef float sample_t;
  //@}


  //-------------------------------------------------------------------------------
  /** \name Typedefs that guaranty some specific bit length
 *
 * It uses the QT4 types. This can be changed in the future, keeping
 * compatibility for the rest of the code.
 */
  //-------------------------------------------------------------------------------
  //@{
  /// Typedef for <tt>unsigned char</tt>. This type is guaranteed to be 8-bit.
  typedef quint8 uint8_t;
  /// Typedef for <tt>unsigned short</tt>. This type is guaranteed to be 16-bit.
  typedef quint16 uint16_t;
  /// Typedef for <tt>unsigned int</tt>. This type is guaranteed to be 32-bit.
  typedef quint32 uint32_t;
  /// \brief Typedef for <tt>unsigned long long int</tt>. This type is guaranteed to
  /// be 64-bit.
  //typedef quint64 uint64_t;
  /// Typedef for <tt>signed char</tt>. This type is guaranteed to be 8-bit.
  typedef qint8 int8_t;
  /// Typedef for <tt>signed short</tt>. This type is guaranteed to be 16-bit.
  typedef qint16 int16_t;
  /// Typedef for <tt>signed int</tt>. This type is guaranteed to be 32-bit.
  typedef qint32 int32_t;
  /// \brief Typedef for <tt>long long int</tt>. This type is guaranteed to
  /// be 64-bit.
  //typedef qint64 int64_t;
  //@}
//} // end of namespace JackTripNamespace

#endif
