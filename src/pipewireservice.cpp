#include "pipewireservice.h"
#include <QDebug>
#include <QtDBus/QtDBus>

PipewireService::PipewireService()
{

}

void PipewireService::printStatus()
{
    qDebug() << "\n************ checking for pipewire\n";
}

void PipewireService::method1()
{
    qDebug() << "Method 1:";

    QDBusReply<QStringList> reply = QDBusConnection::systemBus().interface()->registeredServiceNames();
    if (!reply.isValid()) {
        qDebug() << "Error:" << reply.error().message();
        exit(1);
    }
    const QStringList values = reply.value();
    for (const QString &name : values)
        qDebug() << name;
}

void PipewireService::method2()
{
    qDebug() << "Method 2:";

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface dbus_iface("org.freedesktop.DBus", "/org/freedesktop/DBus",
                              "org.freedesktop.DBus", bus);
    qDebug() << dbus_iface.call("ListNames").arguments().at(0);
}

void PipewireService::method3()
{
    qDebug() << "Method 3:";
    qDebug() << QDBusConnection::systemBus().interface()->registeredServiceNames().value();
}

void PipewireService::method4()
{
//    qDebug() << "Method 4:";
    //    qDebug() << QDBusConnection::systemBus().interface()->isServiceRegistered(QString(":1.61"));
    //    qDebug() << QDBusConnection::systemBus().interface()->servicePid(QString(":1.61"));
    //    int pid = QDBusConnection::systemBus().interface()->servicePid(QString(":1.61"));
    QProcess process;
    QString pgm("pgrep");
    QString pws("pipewire");
    QStringList args = QStringList() << pws << "-x";
    process.start(pgm, args);
    process.waitForReadyRead();
    QByteArray qba = process.readAllStandardOutput();
    if(!qba.isEmpty()) {
        QString qs(qba);
        QStringList qsl = qs.split("\n");
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().interface()->registeredServiceNames();
        if (!reply.isValid()) {
            qDebug() << "Error:" << reply.error().message();
        }
        const QStringList values = reply.value();
        for (const QString &name : values) {
//            qDebug() << name;
            int pid = QDBusConnection::systemBus().interface()->servicePid(QString(name));
            if (qsl.contains(QString::number(pid))) {
                qDebug().noquote() << QString(pws + " service running with PID") << pid ;
                break;
            }
        }
    }
    process.waitForFinished();
}
