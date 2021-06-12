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
 * \file Auth.cpp
 * \author Aaron Wyatt
 * \date September 2020
 */

#include "Auth.h"

#include <QCryptographicHash>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <iostream>

Auth::Auth(QString fileName)
    : m_days({"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"}), m_authFileName(fileName)
{
    // Load our credentials file.
    loadAuthFile(m_authFileName);

    // Monitor the file for any changes. (Reload when it does.)
    m_authFileWatcher.addPath(m_authFileName);
    QObject::connect(&m_authFileWatcher, &QFileSystemWatcher::fileChanged, this,
                     &Auth::reloadAuthFile, Qt::QueuedConnection);
}

Auth::AuthResponseT Auth::checkCredentials(QString username, QString password)
{
    if (username.isEmpty() || password.isEmpty()) { return WRONGCREDS; }

    if (m_passwordTable.contains(username)) {
        // Check our generated hash against our stored hash.
        QString salt = m_passwordTable[username].section("$", 2, 2);
        QString hash(generateSha512Hash(password, salt));

        if (hash == m_passwordTable[username]) {
            if (checkTime(username)) {
                return OK;
            } else {
                return WRONGTIME;
            }
        }
    }

    return WRONGCREDS;
}

void Auth::reloadAuthFile()
{
    // Some text editors will replace the original file instead of modifying the existing
    // one. Re-add our file to the watcher. (This has no effect if it's still there.)
    QThread::msleep(200);
    std::cout << "Auth file changed. Reloading..." << std::endl;
    m_authFileWatcher.addPath(m_authFileName);
    loadAuthFile(m_authFileName);
}

void Auth::loadAuthFile(QString filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        m_passwordTable.clear();
        m_timesTable.clear();

        // Read our file into our password table
        QTextStream input(&file);
        int lineNumber = 0;
        while (!input.atEnd()) {
            lineNumber++;
            QStringList lineParts = input.readLine().split(":");
            if (lineParts.count() < 3) {
                // We don't have a correctly formatted line. Ignore it.
                std::cout
                    << "WARNING: Incorrectly formatted line in auth file ignored. (Line "
                    << lineNumber << ")" << std::endl;
                continue;
            }

            // Check that our password hash is useable.
            bool invalid = false;
            if (lineParts.at(1).startsWith("$6$")) {
                QStringList hashParts = lineParts.at(1).split("$");
                if (hashParts.count() < 4) {
                    invalid = true;
                } else if (hashParts.at(2).isEmpty() || hashParts.at(3).isEmpty()) {
                    invalid = true;
                }
            } else {
                invalid = true;
            }
            if (invalid) {
                std::cout << "WARNING: Invalid password hash in auth file. (Line "
                          << lineNumber << ")" << std::endl;
                continue;
            }

            m_passwordTable[lineParts.at(0)] = lineParts.at(1);
            m_timesTable[lineParts.at(0)]    = lineParts.at(2);
        }
        file.close();
    }
}

bool Auth::checkTime(QString username)
{
    QStringList times = m_timesTable[username].split(",");
    // First check for the all or none cases.
    if (times.count() == 1 && times.at(0).isEmpty()) {
        return false;
    } else if (times.contains("*")) {
        return true;
    }

    // Now check for weekly schedule information.
    QString dayOfWeek = m_days.at(QDate::currentDate().dayOfWeek() - 1);
    for (int i = 0; i < times.count(); i++) {
        if (times.at(i).startsWith(dayOfWeek)) {
            QString accessTime = QString(times.at(i)).remove(0, 2);
            // Check for the all day option first.
            if (accessTime == "*") { return true; }

            // See if we can interpret it as a time range.
            bool valid        = false;
            QStringList range = accessTime.split("-");
            if (range.count() == 2) {
                QTime start = QTime::fromString(range.at(0), "hhmm");
                QTime end   = QTime::fromString(range.at(1), "hhmm");

                if (start.isValid() && end.isValid()) {
                    valid = true;
                    if (QTime::currentTime() >= start && QTime::currentTime() <= end) {
                        return true;
                    }
                }
            }
            if (!valid) {
                std::cout << "WARNING: The access time \"" << times.at(i).toStdString()
                          << "\" in the auth file for user \"" << username.toStdString()
                          << "\" is not valid." << std::endl;
            }
        }
    }

    // We didn't find a match.
    return false;
}

char Auth::char64(int value)
{
    // Returns a base 64 enconding using the following characters:
    // ./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
    if (value < 0 || value >= 64) { return 0; }

    if (value < 12) {
        return (value + 46);
    } else if (value < 38) {
        return (value + 53);
    } else {
        return (value + 59);
    }
}

QByteArray Auth::charGroup(unsigned char byte2, unsigned char byte1, unsigned char byte0,
                           unsigned int n)
{
    // Returns n base64 encoded characters from 24bits of input.
    // Based on the SHA-crypt algorithm.

    unsigned int w = (byte2 << 16) | (byte1 << 8) | byte0;
    QByteArray output;
    while (n-- > 0) {
        output.append(char64(w & 0x3f));
        w >>= 6;
    }
    return output;
}

QByteArray Auth::generateSha512Hash(QString passwordString, QString saltString)
{
    // Qt implementation of the unix crypt using SHA-512
    // (Should give the same output as openssl passwd -6)

    // For details on the algorithm see https://www.akkadia.org/drepper/SHA-crypt.txt
    // (The steps referred to here follow the implementation instructions.)
    QByteArray passwd = passwordString.toUtf8();
    passwd.truncate(256);
    QByteArray salt = saltString.toUtf8();
    salt.truncate(16);

    int rounds = 5000;
    QCryptographicHash a(QCryptographicHash::Sha512);
    QCryptographicHash b(QCryptographicHash::Sha512);

    a.addData(passwd);
    a.addData(salt);

    b.addData(passwd);
    b.addData(salt);
    b.addData(passwd);
    QByteArray bResult = b.result();

    // Step 9 and 10
    int n;
    for (n = passwd.length(); n > 64; n -= 64) { a.addData(bResult); }
    a.addData(bResult.constData(), n);

    // Step 11
    n = passwd.length();
    while (n) {
        if (n & 1) {
            a.addData(bResult);
        } else {
            a.addData(passwd);
        }
        n >>= 1;
    }
    QByteArray aResult = a.result();

    // Step 13
    // Reuse a as dp.
    a.reset();
    for (n = 0; n < passwd.length(); n++) { a.addData(passwd); }
    QByteArray dp = a.result();

    // Step 16
    QByteArray p;
    for (n = passwd.length(); n > 64; n -= 64) { p.append(dp); }
    p.append(dp.constData(), n);

    // Step 17
    // Reuse b as ds
    b.reset();
    for (n = 16 + (unsigned char)aResult.at(0); n > 0; n--) { b.addData(salt); }
    QByteArray ds = b.result();

    // Step 20
    QByteArray s;
    for (n = salt.length(); n > 64; n -= 64) { s.append(ds); }
    s.append(ds.constData(), n);

    // Step 21
    for (n = 0; n < rounds; n++) {
        // Reuse a as c.
        a.reset();
        if (n & 1) {
            a.addData(p);
        } else {
            a.addData(aResult);
        }

        if (n % 3) { a.addData(s); }
        if (n % 7) { a.addData(p); }

        if (n & 1) {
            a.addData(aResult);
        } else {
            a.addData(p);
        }

        aResult = a.result();
    }

    // Step 22
    QByteArray output("$6$");
    output.append(salt);
    output.append("$");
    output.append(charGroup(aResult.at(0), aResult.at(21), aResult.at(42), 4));
    output.append(charGroup(aResult.at(22), aResult.at(43), aResult.at(1), 4));
    output.append(charGroup(aResult.at(44), aResult.at(2), aResult.at(23), 4));
    output.append(charGroup(aResult.at(3), aResult.at(24), aResult.at(45), 4));
    output.append(charGroup(aResult.at(25), aResult.at(46), aResult.at(4), 4));
    output.append(charGroup(aResult.at(47), aResult.at(5), aResult.at(26), 4));
    output.append(charGroup(aResult.at(6), aResult.at(27), aResult.at(48), 4));
    output.append(charGroup(aResult.at(28), aResult.at(49), aResult.at(7), 4));
    output.append(charGroup(aResult.at(50), aResult.at(8), aResult.at(29), 4));
    output.append(charGroup(aResult.at(9), aResult.at(30), aResult.at(51), 4));
    output.append(charGroup(aResult.at(31), aResult.at(52), aResult.at(10), 4));
    output.append(charGroup(aResult.at(53), aResult.at(11), aResult.at(32), 4));
    output.append(charGroup(aResult.at(12), aResult.at(33), aResult.at(54), 4));
    output.append(charGroup(aResult.at(34), aResult.at(55), aResult.at(13), 4));
    output.append(charGroup(aResult.at(56), aResult.at(14), aResult.at(35), 4));
    output.append(charGroup(aResult.at(15), aResult.at(36), aResult.at(57), 4));
    output.append(charGroup(aResult.at(37), aResult.at(58), aResult.at(16), 4));
    output.append(charGroup(aResult.at(59), aResult.at(17), aResult.at(38), 4));
    output.append(charGroup(aResult.at(18), aResult.at(39), aResult.at(60), 4));
    output.append(charGroup(aResult.at(40), aResult.at(61), aResult.at(19), 4));
    output.append(charGroup(aResult.at(62), aResult.at(20), aResult.at(41), 4));
    output.append(charGroup(0, 0, aResult.at(63), 2));

    return output;
}

Auth::Auth::~Auth() = default;
