/****************************************************************************
** Meta object code from reading C++ file 'chroma_window.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "chroma_window.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chroma_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ChromaWindow_t {
    const uint offsetsAndSize[36];
    char stringdata0[214];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_ChromaWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_ChromaWindow_t qt_meta_stringdata_ChromaWindow = {
    {
QT_MOC_LITERAL(0, 12), // "ChromaWindow"
QT_MOC_LITERAL(13, 20), // "openColorSelectRange"
QT_MOC_LITERAL(34, 0), // ""
QT_MOC_LITERAL(35, 24), // "openColorSelectTolerance"
QT_MOC_LITERAL(60, 8), // "colorAdd"
QT_MOC_LITERAL(69, 11), // "colorRemove"
QT_MOC_LITERAL(81, 8), // "colorSet"
QT_MOC_LITERAL(90, 11), // "setColorLow"
QT_MOC_LITERAL(102, 12), // "setColorHigh"
QT_MOC_LITERAL(115, 8), // "setImage"
QT_MOC_LITERAL(124, 9), // "toggleKey"
QT_MOC_LITERAL(134, 11), // "editSetLowB"
QT_MOC_LITERAL(146, 4), // "text"
QT_MOC_LITERAL(151, 11), // "editSetLowG"
QT_MOC_LITERAL(163, 11), // "editSetLowR"
QT_MOC_LITERAL(175, 12), // "editSetHighB"
QT_MOC_LITERAL(188, 12), // "editSetHighG"
QT_MOC_LITERAL(201, 12) // "editSetHighR"

    },
    "ChromaWindow\0openColorSelectRange\0\0"
    "openColorSelectTolerance\0colorAdd\0"
    "colorRemove\0colorSet\0setColorLow\0"
    "setColorHigh\0setImage\0toggleKey\0"
    "editSetLowB\0text\0editSetLowG\0editSetLowR\0"
    "editSetHighB\0editSetHighG\0editSetHighR"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ChromaWindow[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  104,    2, 0x0a,    0 /* Public */,
       3,    0,  105,    2, 0x0a,    1 /* Public */,
       4,    0,  106,    2, 0x0a,    2 /* Public */,
       5,    0,  107,    2, 0x0a,    3 /* Public */,
       6,    0,  108,    2, 0x0a,    4 /* Public */,
       7,    0,  109,    2, 0x0a,    5 /* Public */,
       8,    0,  110,    2, 0x0a,    6 /* Public */,
       9,    0,  111,    2, 0x0a,    7 /* Public */,
      10,    0,  112,    2, 0x0a,    8 /* Public */,
      11,    1,  113,    2, 0x0a,    9 /* Public */,
      13,    1,  116,    2, 0x0a,   11 /* Public */,
      14,    1,  119,    2, 0x0a,   13 /* Public */,
      15,    1,  122,    2, 0x0a,   15 /* Public */,
      16,    1,  125,    2, 0x0a,   17 /* Public */,
      17,    1,  128,    2, 0x0a,   19 /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   12,

       0        // eod
};

void ChromaWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ChromaWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->openColorSelectRange(); break;
        case 1: _t->openColorSelectTolerance(); break;
        case 2: _t->colorAdd(); break;
        case 3: _t->colorRemove(); break;
        case 4: _t->colorSet(); break;
        case 5: _t->setColorLow(); break;
        case 6: _t->setColorHigh(); break;
        case 7: _t->setImage(); break;
        case 8: _t->toggleKey(); break;
        case 9: _t->editSetLowB((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->editSetLowG((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->editSetLowR((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->editSetHighB((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->editSetHighG((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: _t->editSetHighR((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ChromaWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ChromaWindow.offsetsAndSize,
    qt_meta_data_ChromaWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_ChromaWindow_t

, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>


>,
    nullptr
} };


const QMetaObject *ChromaWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ChromaWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ChromaWindow.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ChromaWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
