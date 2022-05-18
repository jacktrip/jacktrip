#include "dblsqd/release.h"

namespace dblsqd
{

/*!
 * \class Release
 * \brief This class is used to represent information about a single Release
 * from a Feed.
 */

/*!
 * \brief Constructs a new Release from q QJsonObject.
 */
Release::Release(QJsonObject releaseInfo)
{
    this->version   = releaseInfo.value("version").toString();
    this->changelog = releaseInfo.value("changelog").toString();

    QJsonObject downloadInfo = releaseInfo.value("download").toObject();
    this->date =
        QDateTime::fromString(downloadInfo.value("date").toString(), Qt::ISODate);
    this->downloadUrl    = QUrl(downloadInfo.value("url").toString());
    this->downloadSize   = downloadInfo.value("size").toDouble();
    this->downloadSHA1   = downloadInfo.value("sha1").toString();
    this->downloadSHA256 = downloadInfo.value("sha256").toString();
    this->downloadDSA    = downloadInfo.value("dsa").toString();
}

/*!
 * \brief Constructs a new Release from a version string and a date.
 *
 * This method is useful when constructing a "virtual" Release for comparing
 * it with Releases retrieved from a Feed.
 */
Release::Release(QString version, QDateTime date)
    : version(version)
    , date(date)
    , changelog("")
    , downloadUrl("")
    , downloadSize(0)
    , downloadSHA1("")
    , downloadSHA256("")
    , downloadDSA("")
{
}

/*!
 * \brief Compares two Releases.
 *
 * If the Release version is compatible with SemVer, the version determines
 * Release order. If versions are identical or not compatible with SemVer,
 * Release date is used for determining order instead.
 */
bool operator<(const Release& one, const Release& other)
{
    SemVer v1(one.version);
    SemVer v2(other.version);
    if (v1.isValid() && v2.isValid()) {
        return (v1 < v2);
    } else {
        return (one.date < other.date);
    }
}

bool operator==(const Release& one, const Release& other)
{
    return one.version == other.version;
}

bool operator<=(const Release& one, const Release& other)
{
    return one == other || one < other;
}

/*
 * Getters
 */
/*!
 * \brief Returns the Release version.
 */
QString Release::getVersion() const
{
    return this->version;
}

/*!
 * \brief Returns the Release changelog.
 */
QString Release::getChangelog() const
{
    return this->changelog;
}

/*!
 * \brief Returns the Release date.
 */
QDateTime Release::getDate() const
{
    return this->date;
}

/*!
 * \brief Returns the Release download URL.
 */
QUrl Release::getDownloadUrl() const
{
    return this->downloadUrl;
}

/*!
 * \brief Returns the SHA1 hash of the Release download.
 */
QString Release::getDownloadSHA1() const
{
    return this->downloadSHA1;
}

/*!
 * \brief Returns the SHA256 hash of the Release download.
 */
QString Release::getDownloadSHA256() const
{
    return this->downloadSHA256;
}

/*!
 * \brief Returns the DSA signature of the Release download.
 */
QString Release::getDownloadDSA() const
{
    return this->downloadDSA;
}

/*!
 * \brief Returns the size of the Release download in bytes.
 */
qint64 Release::getDownloadSize() const
{
    return (qint64)this->downloadSize;
}

}  // namespace dblsqd
