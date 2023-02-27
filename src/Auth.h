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
 * \file Auth.h
 * \author Aaron Wyatt
 * \date September 2020
 */

#ifndef __AUTH_H__
#define __AUTH_H__

#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>

class Auth : public QObject
{
    Q_OBJECT;

   public:
    enum AuthResponseT {
        OK          = 1 << 16,
        REQUIRED    = 2 << 16,
        NOTREQUIRED = 3 << 16,
        WRONGCREDS  = 4 << 16,
        WRONGTIME   = 5 << 16
    };

    Auth(const QString& fileName = "", bool monitorChanges = false,
         QObject* parent = nullptr);
    ~Auth();

    AuthResponseT checkCredentials(const QString& username, const QString& password);
    bool setFileName(const QString& fileName, bool readFile = true);
    QStringList getUsers();
    QString getTimes(const QString& username);
    void deleteUser(const QString& username);
    void editUser(const QString& username, const QString& times,
                  const QString& password = "");
    bool writeFile();

    QString generateSalt();

   private slots:
    void reloadAuthFile();

   private:
    bool loadAuthFile(const QString& filename);
    bool checkTime(const QString& username);
    bool verifyTimeFormat(const QString& times);

    char char64(int value);
    QByteArray charGroup(unsigned char byte3, unsigned char byte2, unsigned char byte1,
                         unsigned int n);
    QByteArray generateSha512Hash(const QString& passwordString,
                                  const QString& saltString);

    QStringList m_days;
    QHash<QString, QString> m_passwordTable;
    QHash<QString, QString> m_timesTable;

    QString m_authFileName;
    bool m_monitorChanges;
    QFileSystemWatcher m_authFileWatcher;
};

#endif  // __AUTH_H__
