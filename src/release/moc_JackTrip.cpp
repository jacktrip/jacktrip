/****************************************************************************
** Meta object code from reading C++ file 'JackTrip.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../JackTrip.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'JackTrip.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_JackTrip_t {
    QByteArrayData data[11];
    char stringdata0[217];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_JackTrip_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_JackTrip_t qt_meta_stringdata_JackTrip = {
    {
QT_MOC_LITERAL(0, 0, 8), // "JackTrip"
QT_MOC_LITERAL(1, 9, 16), // "signalUdpTimeOut"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 22), // "signalProcessesStopped"
QT_MOC_LITERAL(4, 50, 28), // "signalNoUdpPacketsForSeconds"
QT_MOC_LITERAL(5, 79, 24), // "signalTcpClientConnected"
QT_MOC_LITERAL(6, 104, 17), // "slotStopProcesses"
QT_MOC_LITERAL(7, 122, 39), // "slotUdpWaitingTooLongClientGo..."
QT_MOC_LITERAL(8, 162, 9), // "wait_msec"
QT_MOC_LITERAL(9, 172, 13), // "slotPrintTest"
QT_MOC_LITERAL(10, 186, 30) // "slotReceivedConnectionFromPeer"

    },
    "JackTrip\0signalUdpTimeOut\0\0"
    "signalProcessesStopped\0"
    "signalNoUdpPacketsForSeconds\0"
    "signalTcpClientConnected\0slotStopProcesses\0"
    "slotUdpWaitingTooLongClientGoneProbably\0"
    "wait_msec\0slotPrintTest\0"
    "slotReceivedConnectionFromPeer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_JackTrip[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    0,   55,    2, 0x06 /* Public */,
       4,    0,   56,    2, 0x06 /* Public */,
       5,    0,   57,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   58,    2, 0x0a /* Public */,
       7,    1,   59,    2, 0x0a /* Public */,
       9,    0,   62,    2, 0x0a /* Public */,
      10,    0,   63,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void JackTrip::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<JackTrip *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalUdpTimeOut(); break;
        case 1: _t->signalProcessesStopped(); break;
        case 2: _t->signalNoUdpPacketsForSeconds(); break;
        case 3: _t->signalTcpClientConnected(); break;
        case 4: _t->slotStopProcesses(); break;
        case 5: _t->slotUdpWaitingTooLongClientGoneProbably((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->slotPrintTest(); break;
        case 7: _t->slotReceivedConnectionFromPeer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (JackTrip::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&JackTrip::signalUdpTimeOut)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (JackTrip::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&JackTrip::signalProcessesStopped)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (JackTrip::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&JackTrip::signalNoUdpPacketsForSeconds)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (JackTrip::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&JackTrip::signalTcpClientConnected)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject JackTrip::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_JackTrip.data,
    qt_meta_data_JackTrip,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *JackTrip::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *JackTrip::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_JackTrip.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int JackTrip::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void JackTrip::signalUdpTimeOut()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void JackTrip::signalProcessesStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void JackTrip::signalNoUdpPacketsForSeconds()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void JackTrip::signalTcpClientConnected()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
