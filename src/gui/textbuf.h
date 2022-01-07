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

#include <QPlainTextEdit>
#include <iostream>
#include <streambuf>

// Extension of a stream buffer to output to a QTextEdit via a signal
class textbuf
    : public QObject
    , public std::basic_streambuf<char, std::char_traits<char>>
{
    Q_OBJECT

   public:
    textbuf(QObject* parent = nullptr) : QObject(parent)
    {
        setp(m_buf, m_buf + BUF_SIZE);
    }

    void setOutStream(std::ostream* output);

   signals:
    void outputString(const QString& output);

   protected:
    virtual int overflow(int c = traits_t::eof());
    virtual int sync();

   private:
    typedef std::char_traits<char> traits_t;

    static const size_t BUF_SIZE = 64;
    char m_buf[BUF_SIZE];

    std::ostream* m_outStream = nullptr;

    void putChars(const char* begin, const char* end);
};

#endif  // TEXTBUF_H
