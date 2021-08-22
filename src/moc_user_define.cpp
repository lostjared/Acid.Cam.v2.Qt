/****************************************************************************
** Meta object code from reading C++ file 'user_define.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "user_define.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'user_define.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DefineWindow_t {
    const uint offsetsAndSize[12];
    char stringdata0[65];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_DefineWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_DefineWindow_t qt_meta_stringdata_DefineWindow = {
    {
QT_MOC_LITERAL(0, 12), // "DefineWindow"
QT_MOC_LITERAL(13, 13), // "setFilterName"
QT_MOC_LITERAL(27, 0), // ""
QT_MOC_LITERAL(28, 16), // "clearFilterNames"
QT_MOC_LITERAL(45, 9), // "saveNames"
QT_MOC_LITERAL(55, 9) // "loadNames"

    },
    "DefineWindow\0setFilterName\0\0"
    "clearFilterNames\0saveNames\0loadNames"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DefineWindow[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   38,    2, 0x0a,    0 /* Public */,
       3,    0,   39,    2, 0x0a,    1 /* Public */,
       4,    0,   40,    2, 0x0a,    2 /* Public */,
       5,    0,   41,    2, 0x0a,    3 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DefineWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DefineWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->setFilterName(); break;
        case 1: _t->clearFilterNames(); break;
        case 2: _t->saveNames(); break;
        case 3: _t->loadNames(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject DefineWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_DefineWindow.offsetsAndSize,
    qt_meta_data_DefineWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_DefineWindow_t

, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *DefineWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DefineWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DefineWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int DefineWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
