/****************************************************************************
** Meta object code from reading C++ file 'UdpMasterListener.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../UdpMasterListener.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UdpMasterListener.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_UdpMasterListener_t {
    QByteArrayData data[7];
    char stringdata0[80];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UdpMasterListener_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UdpMasterListener_t qt_meta_stringdata_UdpMasterListener = {
    {
QT_MOC_LITERAL(0, 0, 17), // "UdpMasterListener"
QT_MOC_LITERAL(1, 18, 9), // "Listening"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 16), // "ClientAddressSet"
QT_MOC_LITERAL(4, 46, 18), // "signalRemoveThread"
QT_MOC_LITERAL(5, 65, 2), // "id"
QT_MOC_LITERAL(6, 68, 11) // "testReceive"

    },
    "UdpMasterListener\0Listening\0\0"
    "ClientAddressSet\0signalRemoveThread\0"
    "id\0testReceive"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UdpMasterListener[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    0,   35,    2, 0x06 /* Public */,
       4,    1,   36,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   39,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void UdpMasterListener::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UdpMasterListener *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->Listening(); break;
        case 1: _t->ClientAddressSet(); break;
        case 2: _t->signalRemoveThread((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->testReceive(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UdpMasterListener::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpMasterListener::Listening)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UdpMasterListener::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpMasterListener::ClientAddressSet)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UdpMasterListener::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UdpMasterListener::signalRemoveThread)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject UdpMasterListener::staticMetaObject = { {
    &QThread::staticMetaObject,
    qt_meta_stringdata_UdpMasterListener.data,
    qt_meta_data_UdpMasterListener,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *UdpMasterListener::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UdpMasterListener::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_UdpMasterListener.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int UdpMasterListener::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void UdpMasterListener::Listening()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void UdpMasterListener::ClientAddressSet()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void UdpMasterListener::signalRemoveThread(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
