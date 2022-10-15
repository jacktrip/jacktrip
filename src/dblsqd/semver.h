#ifndef DBLSQD_SEMVER_H
#define DBLSQD_SEMVER_H

#include <QObject>
#include <QRegularExpression>

namespace dblsqd
{

class SemVer
{
   public:
    SemVer(QString version);

    bool operator<(const SemVer& other);

    bool isValid() const;
    QString toString();

   private:
    QString original;
    int major;
    int minor;
    int patch;
    QString prerelease;
    QString build;
    bool valid;

    static QString getRegExp();
};

}  // namespace dblsqd

#endif  // DBLSQD_SEMVER_H
