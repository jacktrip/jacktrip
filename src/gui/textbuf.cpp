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

#include <cstring>

void textbuf::setOutStream(std::ostream* output)
{
    m_outStream = output;
}

int textbuf::overflow(int c)
{
    // We run unbuffered, so every single character written to the stream
    // arrives here rather than in a put area shared between threads.
    if (c != traits_t::eof()) {
        const char out = traits_t::to_char_type(c);
        appendChars(&out, 1);
    }

    return c;
}

std::streamsize textbuf::xsputn(const char* s, std::streamsize n)
{
    // The base class implementation of this copies into the put area, which
    // isn't safe to share between threads, so accumulate the characters
    // ourselves instead.
    if (n <= 0) {
        return 0;
    }

    appendChars(s, static_cast<size_t>(n));
    return n;
}

int textbuf::sync()
{
    QMutexLocker lock(&m_mutex);
    flushLocked();
    return 0;
}

void textbuf::appendChars(const char* s, size_t n)
{
    QMutexLocker lock(&m_mutex);
    m_pending.append(s, n);

    // Flush complete lines as they arrive so that output still shows up
    // promptly for callers that end a line with "\n" rather than std::endl.
    if (m_pending.size() >= MAX_PENDING || memchr(s, '\n', n) != nullptr) {
        flushLocked();
    }
}

void textbuf::flushLocked()
{
    if (m_pending.empty()) {
        return;
    }

    putChars(m_pending.data(), m_pending.data() + m_pending.size());
    m_pending.clear();
}

void textbuf::putChars(const char* begin, const char* end)
{
    const qsizetype length = end - begin;

    if (m_outStream) {
        // Write the whole block in one call. Relaying character by character
        // lets output from different threads interleave mid-line, which makes
        // the console logs users send us hard to read.
        m_outStream->write(begin, length);
        m_outStream->flush();
    }

    // Send a signal here rather than writing directly to our
    // QTextEdit to avoid any issues with threading.
    emit outputString(QString(QByteArray(begin, length)));
}
