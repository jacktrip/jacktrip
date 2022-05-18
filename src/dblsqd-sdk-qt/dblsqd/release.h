#ifndef DBLSQD_RELEASE_H
#define DBLSQD_RELEASE_H

#include <QtCore>
#include <QUrl>
#include "dblsqd/semver.h"

namespace  dblsqd {

class Release
{
public:
    Release(QJsonObject releaseInfo);
    Release(QString version = QString(), QDateTime date = QDateTime());

    friend bool operator<(const Release &one, const Release &other);
    friend bool operator==(const Release &one, const Release &other);
    friend bool operator<=(const Release &one, const Release &other);

    QString getVersion() const;
    QString getChangelog() const;
    QDateTime getDate() const;
    QUrl getDownloadUrl() const;
    QString getDownloadSHA1() const;
    QString getDownloadSHA256() const;
    QString getDownloadDSA() const;
    qint64 getDownloadSize() const;

private:
    QString version;
    QDateTime date;
    QString changelog;
    QUrl downloadUrl;
    long downloadSize;
    QString downloadSHA1;
    QString downloadSHA256;
    QString downloadDSA;
};

} // namespace dblsqd

#endif // DBLSQD_RELEASE_H
