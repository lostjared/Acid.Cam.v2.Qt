/****************************************************************************
** Meta object code from reading C++ file 'main_window.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.1.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "main_window.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslError>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.1.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AC_MainWindow_t {
    const uint offsetsAndSize[140];
    char stringdata0[875];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_AC_MainWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_AC_MainWindow_t qt_meta_stringdata_AC_MainWindow = {
    {
QT_MOC_LITERAL(0, 13), // "AC_MainWindow"
QT_MOC_LITERAL(14, 9), // "showRange"
QT_MOC_LITERAL(24, 0), // ""
QT_MOC_LITERAL(25, 12), // "chk_Joystick"
QT_MOC_LITERAL(38, 10), // "addClicked"
QT_MOC_LITERAL(49, 10), // "rmvClicked"
QT_MOC_LITERAL(60, 9), // "upClicked"
QT_MOC_LITERAL(70, 6), // "setSub"
QT_MOC_LITERAL(77, 11), // "downClicked"
QT_MOC_LITERAL(89, 9), // "file_Exit"
QT_MOC_LITERAL(99, 13), // "file_NewVideo"
QT_MOC_LITERAL(113, 14), // "file_NewCamera"
QT_MOC_LITERAL(128, 13), // "controls_Stop"
QT_MOC_LITERAL(142, 13), // "controls_Snap"
QT_MOC_LITERAL(156, 14), // "controls_Pause"
QT_MOC_LITERAL(171, 13), // "controls_Step"
QT_MOC_LITERAL(185, 17), // "controls_SetImage"
QT_MOC_LITERAL(203, 18), // "controls_ShowVideo"
QT_MOC_LITERAL(222, 15), // "controls_SetKey"
QT_MOC_LITERAL(238, 14), // "controls_Reset"
QT_MOC_LITERAL(253, 18), // "controls_ShowDisp2"
QT_MOC_LITERAL(272, 10), // "help_About"
QT_MOC_LITERAL(283, 11), // "updateFrame"
QT_MOC_LITERAL(295, 3), // "img"
QT_MOC_LITERAL(299, 13), // "stopRecording"
QT_MOC_LITERAL(313, 10), // "resetIndex"
QT_MOC_LITERAL(324, 11), // "chk_Clicked"
QT_MOC_LITERAL(336, 11), // "cb_SetIndex"
QT_MOC_LITERAL(348, 5), // "index"
QT_MOC_LITERAL(354, 8), // "frameInc"
QT_MOC_LITERAL(363, 12), // "slideChanged"
QT_MOC_LITERAL(376, 3), // "pos"
QT_MOC_LITERAL(380, 12), // "colorChanged"
QT_MOC_LITERAL(393, 15), // "colorMapChanged"
QT_MOC_LITERAL(409, 18), // "comboFilterChanged"
QT_MOC_LITERAL(428, 15), // "setFilterSingle"
QT_MOC_LITERAL(444, 15), // "setFilterCustom"
QT_MOC_LITERAL(460, 10), // "openSearch"
QT_MOC_LITERAL(471, 15), // "movementOption1"
QT_MOC_LITERAL(487, 15), // "movementOption2"
QT_MOC_LITERAL(503, 15), // "movementOption3"
QT_MOC_LITERAL(519, 6), // "speed1"
QT_MOC_LITERAL(526, 6), // "speed2"
QT_MOC_LITERAL(533, 6), // "speed3"
QT_MOC_LITERAL(540, 6), // "speed4"
QT_MOC_LITERAL(547, 6), // "speed5"
QT_MOC_LITERAL(554, 6), // "speed6"
QT_MOC_LITERAL(561, 6), // "speed7"
QT_MOC_LITERAL(568, 13), // "showGLDisplay"
QT_MOC_LITERAL(582, 12), // "flip1_action"
QT_MOC_LITERAL(595, 12), // "flip2_action"
QT_MOC_LITERAL(608, 12), // "flip3_action"
QT_MOC_LITERAL(621, 13), // "noflip_action"
QT_MOC_LITERAL(635, 15), // "clear_subfilter"
QT_MOC_LITERAL(651, 9), // "clear_img"
QT_MOC_LITERAL(661, 10), // "repeat_vid"
QT_MOC_LITERAL(672, 7), // "setFade"
QT_MOC_LITERAL(680, 15), // "openColorWindow"
QT_MOC_LITERAL(696, 17), // "menuFilterChanged"
QT_MOC_LITERAL(714, 14), // "show_Favorites"
QT_MOC_LITERAL(729, 15), // "load_CustomFile"
QT_MOC_LITERAL(745, 15), // "save_CustomFile"
QT_MOC_LITERAL(761, 8), // "showFull"
QT_MOC_LITERAL(770, 15), // "showImageWindow"
QT_MOC_LITERAL(786, 14), // "showPrefWindow"
QT_MOC_LITERAL(801, 20), // "setRandomFilterValue"
QT_MOC_LITERAL(822, 19), // "setCustomCycle_Menu"
QT_MOC_LITERAL(842, 11), // "next_filter"
QT_MOC_LITERAL(854, 11), // "prev_filter"
QT_MOC_LITERAL(866, 8) // "showSlit"

    },
    "AC_MainWindow\0showRange\0\0chk_Joystick\0"
    "addClicked\0rmvClicked\0upClicked\0setSub\0"
    "downClicked\0file_Exit\0file_NewVideo\0"
    "file_NewCamera\0controls_Stop\0controls_Snap\0"
    "controls_Pause\0controls_Step\0"
    "controls_SetImage\0controls_ShowVideo\0"
    "controls_SetKey\0controls_Reset\0"
    "controls_ShowDisp2\0help_About\0updateFrame\0"
    "img\0stopRecording\0resetIndex\0chk_Clicked\0"
    "cb_SetIndex\0index\0frameInc\0slideChanged\0"
    "pos\0colorChanged\0colorMapChanged\0"
    "comboFilterChanged\0setFilterSingle\0"
    "setFilterCustom\0openSearch\0movementOption1\0"
    "movementOption2\0movementOption3\0speed1\0"
    "speed2\0speed3\0speed4\0speed5\0speed6\0"
    "speed7\0showGLDisplay\0flip1_action\0"
    "flip2_action\0flip3_action\0noflip_action\0"
    "clear_subfilter\0clear_img\0repeat_vid\0"
    "setFade\0openColorWindow\0menuFilterChanged\0"
    "show_Favorites\0load_CustomFile\0"
    "save_CustomFile\0showFull\0showImageWindow\0"
    "showPrefWindow\0setRandomFilterValue\0"
    "setCustomCycle_Menu\0next_filter\0"
    "prev_filter\0showSlit"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AC_MainWindow[] = {

 // content:
       9,       // revision
       0,       // classname
       0,    0, // classinfo
      65,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  404,    2, 0x0a,    0 /* Public */,
       3,    0,  405,    2, 0x0a,    1 /* Public */,
       4,    0,  406,    2, 0x0a,    2 /* Public */,
       5,    0,  407,    2, 0x0a,    3 /* Public */,
       6,    0,  408,    2, 0x0a,    4 /* Public */,
       7,    0,  409,    2, 0x0a,    5 /* Public */,
       8,    0,  410,    2, 0x0a,    6 /* Public */,
       9,    0,  411,    2, 0x0a,    7 /* Public */,
      10,    0,  412,    2, 0x0a,    8 /* Public */,
      11,    0,  413,    2, 0x0a,    9 /* Public */,
      12,    0,  414,    2, 0x0a,   10 /* Public */,
      13,    0,  415,    2, 0x0a,   11 /* Public */,
      14,    0,  416,    2, 0x0a,   12 /* Public */,
      15,    0,  417,    2, 0x0a,   13 /* Public */,
      16,    0,  418,    2, 0x0a,   14 /* Public */,
      17,    0,  419,    2, 0x0a,   15 /* Public */,
      18,    0,  420,    2, 0x0a,   16 /* Public */,
      19,    0,  421,    2, 0x0a,   17 /* Public */,
      20,    0,  422,    2, 0x0a,   18 /* Public */,
      21,    0,  423,    2, 0x0a,   19 /* Public */,
      22,    1,  424,    2, 0x0a,   20 /* Public */,
      24,    0,  427,    2, 0x0a,   22 /* Public */,
      25,    0,  428,    2, 0x0a,   23 /* Public */,
      26,    0,  429,    2, 0x0a,   24 /* Public */,
      27,    1,  430,    2, 0x0a,   25 /* Public */,
      29,    0,  433,    2, 0x0a,   27 /* Public */,
      30,    1,  434,    2, 0x0a,   28 /* Public */,
      32,    1,  437,    2, 0x0a,   30 /* Public */,
      33,    1,  440,    2, 0x0a,   32 /* Public */,
      34,    1,  443,    2, 0x0a,   34 /* Public */,
      35,    0,  446,    2, 0x0a,   36 /* Public */,
      36,    0,  447,    2, 0x0a,   37 /* Public */,
      37,    0,  448,    2, 0x0a,   38 /* Public */,
      38,    0,  449,    2, 0x0a,   39 /* Public */,
      39,    0,  450,    2, 0x0a,   40 /* Public */,
      40,    0,  451,    2, 0x0a,   41 /* Public */,
      41,    0,  452,    2, 0x0a,   42 /* Public */,
      42,    0,  453,    2, 0x0a,   43 /* Public */,
      43,    0,  454,    2, 0x0a,   44 /* Public */,
      44,    0,  455,    2, 0x0a,   45 /* Public */,
      45,    0,  456,    2, 0x0a,   46 /* Public */,
      46,    0,  457,    2, 0x0a,   47 /* Public */,
      47,    0,  458,    2, 0x0a,   48 /* Public */,
      48,    0,  459,    2, 0x0a,   49 /* Public */,
      49,    0,  460,    2, 0x0a,   50 /* Public */,
      50,    0,  461,    2, 0x0a,   51 /* Public */,
      51,    0,  462,    2, 0x0a,   52 /* Public */,
      52,    0,  463,    2, 0x0a,   53 /* Public */,
      53,    0,  464,    2, 0x0a,   54 /* Public */,
      54,    0,  465,    2, 0x0a,   55 /* Public */,
      55,    0,  466,    2, 0x0a,   56 /* Public */,
      56,    0,  467,    2, 0x0a,   57 /* Public */,
      57,    0,  468,    2, 0x0a,   58 /* Public */,
      58,    1,  469,    2, 0x0a,   59 /* Public */,
      59,    0,  472,    2, 0x0a,   61 /* Public */,
      60,    0,  473,    2, 0x0a,   62 /* Public */,
      61,    0,  474,    2, 0x0a,   63 /* Public */,
      62,    0,  475,    2, 0x0a,   64 /* Public */,
      63,    0,  476,    2, 0x0a,   65 /* Public */,
      64,    0,  477,    2, 0x0a,   66 /* Public */,
      65,    0,  478,    2, 0x0a,   67 /* Public */,
      66,    0,  479,    2, 0x0a,   68 /* Public */,
      67,    0,  480,    2, 0x0a,   69 /* Public */,
      68,    0,  481,    2, 0x0a,   70 /* Public */,
      69,    0,  482,    2, 0x0a,   71 /* Public */,

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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QImage,   23,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   28,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   31,
    QMetaType::Void, QMetaType::Int,   31,
    QMetaType::Void, QMetaType::Int,   31,
    QMetaType::Void, QMetaType::Int,   31,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   28,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void AC_MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AC_MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->showRange(); break;
        case 1: _t->chk_Joystick(); break;
        case 2: _t->addClicked(); break;
        case 3: _t->rmvClicked(); break;
        case 4: _t->upClicked(); break;
        case 5: _t->setSub(); break;
        case 6: _t->downClicked(); break;
        case 7: _t->file_Exit(); break;
        case 8: _t->file_NewVideo(); break;
        case 9: _t->file_NewCamera(); break;
        case 10: _t->controls_Stop(); break;
        case 11: _t->controls_Snap(); break;
        case 12: _t->controls_Pause(); break;
        case 13: _t->controls_Step(); break;
        case 14: _t->controls_SetImage(); break;
        case 15: _t->controls_ShowVideo(); break;
        case 16: _t->controls_SetKey(); break;
        case 17: _t->controls_Reset(); break;
        case 18: _t->controls_ShowDisp2(); break;
        case 19: _t->help_About(); break;
        case 20: _t->updateFrame((*reinterpret_cast< QImage(*)>(_a[1]))); break;
        case 21: _t->stopRecording(); break;
        case 22: _t->resetIndex(); break;
        case 23: _t->chk_Clicked(); break;
        case 24: _t->cb_SetIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 25: _t->frameInc(); break;
        case 26: _t->slideChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 27: _t->colorChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: _t->colorMapChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: _t->comboFilterChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: _t->setFilterSingle(); break;
        case 31: _t->setFilterCustom(); break;
        case 32: _t->openSearch(); break;
        case 33: _t->movementOption1(); break;
        case 34: _t->movementOption2(); break;
        case 35: _t->movementOption3(); break;
        case 36: _t->speed1(); break;
        case 37: _t->speed2(); break;
        case 38: _t->speed3(); break;
        case 39: _t->speed4(); break;
        case 40: _t->speed5(); break;
        case 41: _t->speed6(); break;
        case 42: _t->speed7(); break;
        case 43: _t->showGLDisplay(); break;
        case 44: _t->flip1_action(); break;
        case 45: _t->flip2_action(); break;
        case 46: _t->flip3_action(); break;
        case 47: _t->noflip_action(); break;
        case 48: _t->clear_subfilter(); break;
        case 49: _t->clear_img(); break;
        case 50: _t->repeat_vid(); break;
        case 51: _t->setFade(); break;
        case 52: _t->openColorWindow(); break;
        case 53: _t->menuFilterChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 54: _t->show_Favorites(); break;
        case 55: _t->load_CustomFile(); break;
        case 56: _t->save_CustomFile(); break;
        case 57: _t->showFull(); break;
        case 58: _t->showImageWindow(); break;
        case 59: _t->showPrefWindow(); break;
        case 60: _t->setRandomFilterValue(); break;
        case 61: _t->setCustomCycle_Menu(); break;
        case 62: _t->next_filter(); break;
        case 63: _t->prev_filter(); break;
        case 64: _t->showSlit(); break;
        default: ;
        }
    }
}

const QMetaObject AC_MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_AC_MainWindow.offsetsAndSize,
    qt_meta_data_AC_MainWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_AC_MainWindow_t

, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<QImage, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *AC_MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AC_MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AC_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int AC_MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 65)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 65;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 65)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 65;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
