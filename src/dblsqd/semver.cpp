#include "semver.h"

namespace dblsqd
{

/*!
 * \class SemVer
 * \brief SemVer encapsulates a version according to
 * Semantic Versioning 2.0.
 */

/*!
 * \brief Constructs a new SemVer object from a string.
 */
SemVer::SemVer(QString version) : original(version), valid(false)
{
    QRegularExpression rx(getRegExp());
    QRegularExpressionMatch match = rx.match(version);
    if (match.hasMatch()) {
        this->major      = match.captured(1).toInt();
        this->minor      = match.captured(2).toInt();
        this->patch      = match.captured(3).toInt();
        this->prerelease = match.captured(4);
        this->build      = match.captured(5);
        this->valid      = true;
    } else {
        this->major      = 0;
        this->minor      = 0;
        this->patch      = 0;
        this->prerelease = "";
        this->build      = "";
        this->valid      = false;
    }
}

/*!
 * \brief Returns true if this version is valid according to the SemVer
 * specification. Otherwise returns false.
 */
bool SemVer::isValid() const
{
    return this->valid;
}

/*!
 * \brief Compares two SemVer objects.
 *
 * Returns true if the left-hand SemVer object represents a lower version
 * according to the SemVer 2.0 specification.
 * Otherweise returns false.
 * Returns false if one of the SemVer objects does not represent a valid
 * SemVer.
 * \sa isValid()
 */
bool SemVer::operator<(const SemVer& other)
{
    if (!this->isValid() || !other.isValid()) {
        return false;
    }

    if (this->major != other.major) {
        return this->major < other.major;
    } else if (this->minor != other.minor) {
        return this->minor < other.minor;
    } else if (this->patch != other.patch) {
        return this->patch < other.patch;
    } else if (this->prerelease != other.prerelease) {
        if (this->prerelease == "") {
            return false;
        } else if (other.prerelease == "") {
            return true;
        }
        return (QString::localeAwareCompare(this->prerelease, other.prerelease) < 0);
    } else {
        return (QString::localeAwareCompare(this->build, other.build) < 0);
    }
}

QString SemVer::getRegExp()
{
    QString v = "(0|[1-9]\\d*)";
    QString p =
        "(?:-((?:0|[1-9A-Za-z][0-9A-Za-z]*)(?:\\.(?:0|[1-9A-Za-z][0-9A-Za-z]*))*))?";
    QString b = "(?:\\+((?:[0-9A-Za-z]*)(?:\\.(?:[0-9A-Za-z][0-9A-Za-z]*))*))?";
    return "^" + v + "." + v + "." + v + p + b + "$";
}

}  // namespace dblsqd
