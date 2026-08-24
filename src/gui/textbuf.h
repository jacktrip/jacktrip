//*****************************************************************
/*
  QJackTrip: Bringing a graphical user interface to JackTrip, a
  system for high quality audio network performance over the
  internet.

  Copyright (c) 2021 Aaron Wyatt.

  This file is part of QJackTrip.

  QJackTrip is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  QJackTrip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with QJackTrip.  If not, see <https://www.gnu.org/licenses/>.
*/
//*****************************************************************

#ifndef TEXTBUF_H
#define TEXTBUF_H

#include <QMutex>
#include <QPlainTextEdit>
#include <iostream>
#include <streambuf>
#include <string>

// Extension of a stream buffer to output to a QTextEdit via a signal
//
// std::cout and std::cerr are redirected here (see QJackTrip's constructor) and
// are written to from every thread in the application, so this buffer has to be
// thread safe. It deliberately runs unbuffered: with a null put area, the base
// class never touches a shared character array of its own, and every write is
// funnelled through overflow() and xsputn(), which serialise on m_mutex.
class textbuf
    : public QObject
    , public std::basic_streambuf<char, std::char_traits<char>>
{
    Q_OBJECT

   public:
    textbuf(QObject* parent = nullptr) : QObject(parent)
    {
        // run unbuffered; see the note above
        setp(nullptr, nullptr);
    }

    void setOutStream(std::ostream* output);

   signals:
    void outputString(const QString& output);

   protected:
    virtual int overflow(int c = traits_t::eof());
    virtual std::streamsize xsputn(const char* s, std::streamsize n);
    virtual int sync();

   private:
    typedef std::char_traits<char> traits_t;

    // flush unconditionally once this much output is pending, so that a caller
    // that never flushes and never emits a newline can't grow m_pending forever
    static const size_t MAX_PENDING = 8192;

    QMutex m_mutex;
    std::string m_pending;

    std::ostream* m_outStream = nullptr;

    void appendChars(const char* s, size_t n);
    // both require m_mutex to be held by the caller
    void flushLocked();
    void putChars(const char* begin, const char* end);
};

#endif  // TEXTBUF_H
