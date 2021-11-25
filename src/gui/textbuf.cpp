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

#include "textbuf.h"

void textbuf::setOutStream(std::ostream* output)
{
    m_outStream = output;
}

int textbuf::overflow(int c)
{
    //Output our buffer.
    putChars(pbase(), pptr());
    
    if (c != traits_t::eof()) {
        char out = c;
        putChars(&out, &out + 1);
    }
    
    //Set buffer to empty again
    setp(m_buf, m_buf + BUF_SIZE);
    
    return c;
}

int textbuf::sync()
{
    //Flush our buffer.
    putChars(pbase(), pptr());
    setp(m_buf, m_buf + BUF_SIZE);
    return 0;
}

void textbuf::putChars(const char* begin, const char* end)
{
    if (m_outStream) {
        for (const char* c = begin; c < end; c++) {
            *m_outStream << *c;
        }
        m_outStream->flush();
    }
    
    emit outputString(QString(QByteArray(begin, end - begin)));
}

