#include "feed.h"

namespace dblsqd
{

/*!
  \class Feed
 * \brief The Feed class provides methods for accessing DBLSQD Feeds and downloading
 Releases.
 *
 * A Feed is a representation of an Application’s Releases.
 * This class can retrieve Feeds via HTTP(S) and offers convenience methods for
 *
 * \section3 Loading Feeds
 *
 * Before a Feed can be loaded with load(), it needs to be initialized with setUrl().
 *
 * \section3 Downloading Updates
 * This class also allows downloading updates through the downloadRelease() method.
 */

/*!
 * \brief Constructs a new Feed object.
 *
 * \sa setUrl()
 */
Feed::Feed(QString baseUrl, QString channel, QString os, QString arch, QString type)
    : feedReply(NULL)
    , downloadReply(NULL)
    , downloadFile(NULL)
    , redirects(0)
    , _ready(false)
{
    if (!baseUrl.isEmpty()) {
        this->setUrl(baseUrl, channel, os, arch, type);
    }
}

/*!
 * \brief Sets the Feed URL.
 *
 * This method can be used to manually set the Feed URL.
 */
void Feed::setUrl(QUrl url)
{
    this->url = url;
}

/*!
 * \brief Sets the Feed URL by specifying its components.
 *
 * The only required component is baseUrl which must be the base URL for an Application
 * provided by the DBSLQD CLI Tool. It should include the full schema and does not require
 * a trailing "/".
 */
void Feed::setUrl(QString baseUrl, QString channel, QString os, QString arch,
                  QString type)
{
    QStringList urlParts;
    urlParts << baseUrl;
    urlParts << channel;

    if (!os.isEmpty()) {
        urlParts << os;
    } else {
        QString autoOs = QSysInfo::productType().toLower();
        if (autoOs == "windows") {
            autoOs = "win";
        } else if (autoOs == "osx" || autoOs == "macos") {
            autoOs = "mac";
        } else {
            autoOs = QSysInfo::kernelType();
        }
        urlParts << autoOs;
    }

    if (!arch.isEmpty()) {
        urlParts << arch;
    } else {
        QString autoArch = QSysInfo::buildCpuArchitecture();
        if (autoArch == "i386" || autoArch == "i586" || autoArch == "i586") {
            autoArch = "x86";
        }
        urlParts << autoArch;
    }

    if (!type.isEmpty()) {
        urlParts << "?t=" + type;
    }

    this->url = QUrl(urlParts.join("/"));
}

/*!
 * \brief Returns the Feed URL.
 */
QUrl Feed::getUrl()
{
    return QUrl(url);
}

/*!
 * \brief Returns a list of all Releases in the Feed.
 *
 * The list is sorted in descending order by version number/release date.
 * If called before ready() was emitted, an empty list is returned.
 * \sa getReleases()
 */
QList<Release> Feed::getReleases()
{
    return releases;
}

/*!
 * \brief Returns a list of all Releases in the Feed that are newer than the given
 * Release.
 *
 * The list is sorted in descending order by version number/release date.
 * If called before ready() was emitted, an empty list is returned.
 * \sa getReleases()
 */
QList<Release> Feed::getUpdates(Release currentRelease)
{
    QList<Release> updates;
    for (int i = 0; i < releases.size(); i++) {
        if (currentRelease < releases.at(i))
            updates << releases.at(i);
    }
    return updates;
}

/*!
 * \brief Returns the pointer to a QTemporaryFile for a downloaded file.
 *
 * If called before downloadFinished() was emitted, this might return a NULL
 * pointer.
 */
QTemporaryFile* Feed::getDownloadFile()
{
    return downloadFile;
}

/*!
 * \brief Returns true if Feed information has been retrieved successfully.
 *
 * A ready Feed might not contain any release information.
 * If downloading the Feed failed, false is returned.
 */
bool Feed::isReady()
{
    return _ready;
}

/*
 * Async API functions
 */
/*!
 * \brief Retrieves and parses data from the Feed.
 *
 * A Feed URL must have been set before with setUrl(). Emits ready() or loadError() on
 * completion.
 */
void Feed::load()
{
    if (feedReply != NULL && !feedReply->isFinished()) {
        return;
    }

    QNetworkRequest request(getUrl());
    feedReply = nam.get(request);
    connect(feedReply, SIGNAL(finished()), this, SLOT(handleFeedFinished()));
}

/*!
 * \brief Starts the download of a given Release.
 * \sa downloadFinished() downloadError() downloadProgress()
 */
void Feed::downloadRelease(Release release)
{
    redirects = 0;
    makeDownloadRequest(release.getDownloadUrl());
    this->release = release;
}

/*
 * Private methods
 */
void Feed::makeDownloadRequest(QUrl url)
{
    if (downloadReply != NULL && !downloadReply->isFinished()) {
        disconnect(downloadReply);
        downloadReply->abort();
        downloadReply->deleteLater();
    }
    if (downloadFile != NULL) {
        disconnect(downloadFile);
        downloadFile->close();
        downloadFile->deleteLater();
        downloadFile = NULL;
    }

    QNetworkRequest request(url);
    downloadReply = nam.get(request);
    connect(downloadReply, SIGNAL(downloadProgress(qint64, qint64)), this,
            SLOT(handleDownloadProgress(qint64, qint64)));
    connect(downloadReply, SIGNAL(readyRead()), this, SLOT(handleDownloadReadyRead()));
    connect(downloadReply, SIGNAL(finished()), this, SLOT(handleDownloadFinished()));
}

/*
 * Signals
 */
/*! \fn void Feed::ready()
 * This signal is emitted when a Feed has been successfully downloaded and parsed.
 * \sa loadError() load()
 */

/*! \fn void Feed::loadError(QString message)
 * This signal is emitted when a Feed could not be downloaded.
 * When loadError() is emitted, ready() is not emitted.
 * \sa ready() load()
 */

/*! \fn void Feed::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
 * This signal is emitted during the download of a Release through downloadRelease().
 * \sa downloadRelease()
 */

/*! \fn void Feed::downloadFinished()
 * This signal is emitted when the download of a Release was successful.
 * A QTemporaryFile* of the downloaded file can then be retrieved with getDownloadFile().
 * \sa downloadRelease()
 */

/*! \fn void Feed::downloadError()
 * This signal is emitted when there was an error downloading or verifying a Release.
 * When downloadError() is emitted, downloadFinished() is not emitted.
 * \sa downloadFinished() downloadRelease()
 */

/*
 * Private Slots
 */
void Feed::handleFeedFinished()
{
    if (feedReply->error() != QNetworkReply::NoError) {
        emit loadError(feedReply->errorString());
        return;
    }

    releases.clear();
    QByteArray json         = feedReply->readAll();
    QJsonDocument doc       = QJsonDocument::fromJson(json);
    QJsonArray releasesInfo = doc.object().value("releases").toArray();
    for (int i = 0; i < releasesInfo.size(); i++) {
        releases << Release(releasesInfo.at(i).toObject());
    }
    std::sort(releases.begin(), releases.end());
    std::reverse(releases.begin(), releases.end());

    _ready = true;
    emit ready();
}

void Feed::handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void Feed::handleDownloadReadyRead()
{
    if (downloadFile == NULL) {
        QString fileName = downloadReply->url().fileName();
        // Workaround for dblsqd to extract filename via query params from a
        // Github-formatted redirect URL
        QUrl url     = downloadReply->url();
        QString host = url.host();
        if (host.contains("github", Qt::CaseInsensitive) && url.hasQuery()) {
            QString query = url.query();
            QRegularExpression rx("filename%3D(.*?)(&|$)");
            QRegularExpressionMatch match = rx.match(query);
            if (match.hasMatch()) {
                fileName = match.captured(1);
            }
        }
        // End workaround
        int extensionPos =
            fileName.indexOf(QRegularExpression("(?:\\.tar)?\\.[a-zA-Z0-9]+$"));
        if (extensionPos > -1) {
            fileName.insert(extensionPos, "-XXXXXX");
        }
        downloadFile = new QTemporaryFile(QDir::tempPath() + "/" + fileName);
        downloadFile->open();
    }
    downloadFile->write(downloadReply->readAll());
}

void Feed::handleDownloadFinished()
{
    if (downloadReply->error() != QNetworkReply::NoError) {
        emit downloadError(downloadReply->errorString());
        return;
    } else if (!downloadReply->attribute(QNetworkRequest::RedirectionTargetAttribute)
                    .isNull()) {
        if (redirects >= 8) {
            emit downloadError(tr("Too many redirects."));
            return;
        }
        QUrl redirectionTarget =
            downloadReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        QUrl redirectedUrl = downloadReply->url().resolved(redirectionTarget);
        redirects++;
        makeDownloadRequest(redirectedUrl);
        return;
    } else if (downloadFile == NULL) {
        emit downloadError(tr("No data received from server"));
        return;
    }

    downloadFile->flush();
    downloadFile->seek(0);
    QCryptographicHash fileHash(QCryptographicHash::Sha256);
    fileHash.addData(downloadFile->readAll());
    QString hashResult = fileHash.result().toHex();
    if (hashResult.toLower() != release.getDownloadSHA256().toLower()) {
        emit downloadError(tr("Could not verify download integrity."));
        return;
    }

    emit downloadFinished();
}

}  // namespace dblsqd
