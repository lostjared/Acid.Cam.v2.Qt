/****************************************************************************
** Meta object code from reading C++ file 'new_dialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "new_dialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'new_dialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CaptureCamera_t {
    const uint offsetsAndSize[8];
    char stringdata0[36];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_CaptureCamera_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_CaptureCamera_t qt_meta_stringdata_CaptureCamera = {
    {
QT_MOC_LITERAL(0, 13), // "CaptureCamera"
QT_MOC_LITERAL(14, 10), // "btn_Select"
QT_MOC_LITERAL(25, 0), // ""
QT_MOC_LITERAL(26, 9) // "btn_Start"

    },
    "CaptureCamera\0btn_Select\0\0btn_Start"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CaptureCamera[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   26,    2, 0x0a,    0 /* Public */,
       3,    0,   27,    2, 0x0a,    1 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CaptureCamera::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CaptureCamera *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->btn_Select(); break;
        case 1: _t->btn_Start(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject CaptureCamera::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CaptureCamera.offsetsAndSize,
    qt_meta_data_CaptureCamera,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_CaptureCamera_t

, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *CaptureCamera::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CaptureCamera::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CaptureCamera.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CaptureCamera::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_CaptureVideo_t {
    const uint offsetsAndSize[10];
    char stringdata0[59];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_CaptureVideo_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_CaptureVideo_t qt_meta_stringdata_CaptureVideo = {
    {
QT_MOC_LITERAL(0, 12), // "CaptureVideo"
QT_MOC_LITERAL(13, 17), // "btn_SetSourceFile"
QT_MOC_LITERAL(31, 0), // ""
QT_MOC_LITERAL(32, 16), // "btn_SetOutputDir"
QT_MOC_LITERAL(49, 9) // "btn_Start"

    },
    "CaptureVideo\0btn_SetSourceFile\0\0"
    "btn_SetOutputDir\0btn_Start"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CaptureVideo[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x0a,    0 /* Public */,
       3,    0,   33,    2, 0x0a,    1 /* Public */,
       4,    0,   34,    2, 0x0a,    2 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void CaptureVideo::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CaptureVideo *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->btn_SetSourceFile(); break;
        case 1: _t->btn_SetOutputDir(); break;
        case 2: _t->btn_Start(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject CaptureVideo::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CaptureVideo.offsetsAndSize,
    qt_meta_data_CaptureVideo,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_CaptureVideo_t

, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *CaptureVideo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CaptureVideo::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CaptureVideo.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int CaptureVideo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
