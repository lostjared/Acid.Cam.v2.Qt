/****************************************************************************
** Meta object code from reading C++ file 'playback_thread.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "playback_thread.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'playback_thread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Playback_t {
    const uint offsetsAndSize[14];
    char stringdata0[66];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Playback_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Playback_t qt_meta_stringdata_Playback = {
    {
QT_MOC_LITERAL(0, 8), // "Playback"
QT_MOC_LITERAL(9, 9), // "procImage"
QT_MOC_LITERAL(19, 0), // ""
QT_MOC_LITERAL(20, 5), // "image"
QT_MOC_LITERAL(26, 13), // "stopRecording"
QT_MOC_LITERAL(40, 14), // "frameIncrement"
QT_MOC_LITERAL(55, 10) // "resetIndex"

    },
    "Playback\0procImage\0\0image\0stopRecording\0"
    "frameIncrement\0resetIndex"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Playback[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    0 /* Public */,
       4,    0,   41,    2, 0x06,    2 /* Public */,
       5,    0,   42,    2, 0x06,    3 /* Public */,
       6,    0,   43,    2, 0x06,    4 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QImage,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Playback::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Playback *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->procImage((*reinterpret_cast< const QImage(*)>(_a[1]))); break;
        case 1: _t->stopRecording(); break;
        case 2: _t->frameIncrement(); break;
        case 3: _t->resetIndex(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Playback::*)(const QImage );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Playback::procImage)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Playback::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Playback::stopRecording)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Playback::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Playback::frameIncrement)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Playback::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Playback::resetIndex)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject Playback::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_Playback.offsetsAndSize,
    qt_meta_data_Playback,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Playback_t
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QImage, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>



>,
    nullptr
} };


const QMetaObject *Playback::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Playback::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Playback.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int Playback::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Playback::procImage(const QImage _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Playback::stopRecording()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Playback::frameIncrement()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Playback::resetIndex()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
