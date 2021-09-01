/****************************************************************************
** Meta object code from reading C++ file 'image_window.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "image_window.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'image_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ImageWindow_t {
    const uint offsetsAndSize[20];
    char stringdata0[114];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_ImageWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_ImageWindow_t qt_meta_stringdata_ImageWindow = {
    {
QT_MOC_LITERAL(0, 11), // "ImageWindow"
QT_MOC_LITERAL(12, 14), // "image_AddFiles"
QT_MOC_LITERAL(27, 0), // ""
QT_MOC_LITERAL(28, 13), // "image_RmvFile"
QT_MOC_LITERAL(42, 13), // "image_SetFile"
QT_MOC_LITERAL(56, 16), // "image_RowChanged"
QT_MOC_LITERAL(73, 5), // "index"
QT_MOC_LITERAL(79, 14), // "image_SetCycle"
QT_MOC_LITERAL(94, 9), // "video_Set"
QT_MOC_LITERAL(104, 9) // "video_Clr"

    },
    "ImageWindow\0image_AddFiles\0\0image_RmvFile\0"
    "image_SetFile\0image_RowChanged\0index\0"
    "image_SetCycle\0video_Set\0video_Clr"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ImageWindow[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x0a,    0 /* Public */,
       3,    0,   57,    2, 0x0a,    1 /* Public */,
       4,    0,   58,    2, 0x0a,    2 /* Public */,
       5,    1,   59,    2, 0x0a,    3 /* Public */,
       7,    0,   62,    2, 0x0a,    5 /* Public */,
       8,    0,   63,    2, 0x0a,    6 /* Public */,
       9,    0,   64,    2, 0x0a,    7 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ImageWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ImageWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->image_AddFiles(); break;
        case 1: _t->image_RmvFile(); break;
        case 2: _t->image_SetFile(); break;
        case 3: _t->image_RowChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->image_SetCycle(); break;
        case 5: _t->video_Set(); break;
        case 6: _t->video_Clr(); break;
        default: ;
        }
    }
}

const QMetaObject ImageWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ImageWindow.offsetsAndSize,
    qt_meta_data_ImageWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_ImageWindow_t

, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *ImageWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImageWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ImageWindow.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ImageWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
