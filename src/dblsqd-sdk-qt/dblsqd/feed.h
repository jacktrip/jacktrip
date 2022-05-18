#ifndef DBLSQD_FEED_H
#define DBLSQD_FEED_H

#include <QtCore>
#include <QtNetwork>
#include "dblsqd/release.h"

namespace dblsqd {

class Feed : public QObject
{
    Q_OBJECT

public:
    Feed(QString baseUrl = "", QString channel = "release", QString os = QString(), QString arch = QString(), QString type = QString());

    void setUrl(QUrl url);
    void setUrl(QString baseUrl, QString channel = "release", QString os = QString(), QString arch = QString(), QString type = QString());
    QUrl getUrl();

    //Async API
    void load();
    void downloadRelease(Release release);

    //Sync API
    QList<Release> getUpdates(Release currentRelease = Release(QCoreApplication::applicationVersion()));
    QList<Release> getReleases();
    QTemporaryFile* getDownloadFile();
    bool isReady();

signals:
    void ready();
    void loadError(QString message);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadError(QString message);

private:
    QUrl url;

    QList<Release> releases;

    void makeDownloadRequest(QUrl url);

    QNetworkAccessManager nam;
    QNetworkReply* feedReply;
    Release release;
    QNetworkReply* downloadReply;
    QTemporaryFile* downloadFile;
    uint redirects;
    bool _ready;

private slots:
    void handleFeedFinished();
    void handleDownloadProgress(qint64, qint64);
    void handleDownloadReadyRead();
    void handleDownloadFinished();
};

} // namespace dblsqd

#endif // DBLSQD_FEED_H
