/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../HT_view/mainwindow.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.6.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMainWindowENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMainWindowENDCLASS = QtMocHelpers::stringData(
    "MainWindow",
    "openFile",
    "",
    "openRecentFile",
    "onParsingProgress",
    "percentage",
    "onParsingStatus",
    "status",
    "onParsingFinished",
    "QList<DataPoint>",
    "data",
    "onParsingError",
    "message",
    "onMouseMove",
    "QMouseEvent*",
    "event",
    "onXAxisRangeChanged",
    "QCPRange",
    "newRange",
    "printChart",
    "printPreview",
    "copyChartToClipboard",
    "exportChart",
    "zoomVertical",
    "factor",
    "panVertical",
    "direction",
    "panHorizontal",
    "resetAllZoom",
    "goToStart",
    "goToEnd",
    "jumpToPeriodBoundary",
    "toggleTemperatureGraph",
    "toggleHumidityGraph",
    "updateTemperatureGrid"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMainWindowENDCLASS_t {
    uint offsetsAndSizes[70];
    char stringdata0[11];
    char stringdata1[9];
    char stringdata2[1];
    char stringdata3[15];
    char stringdata4[18];
    char stringdata5[11];
    char stringdata6[16];
    char stringdata7[7];
    char stringdata8[18];
    char stringdata9[17];
    char stringdata10[5];
    char stringdata11[15];
    char stringdata12[8];
    char stringdata13[12];
    char stringdata14[13];
    char stringdata15[6];
    char stringdata16[20];
    char stringdata17[9];
    char stringdata18[9];
    char stringdata19[11];
    char stringdata20[13];
    char stringdata21[21];
    char stringdata22[12];
    char stringdata23[13];
    char stringdata24[7];
    char stringdata25[12];
    char stringdata26[10];
    char stringdata27[14];
    char stringdata28[13];
    char stringdata29[10];
    char stringdata30[8];
    char stringdata31[21];
    char stringdata32[23];
    char stringdata33[20];
    char stringdata34[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMainWindowENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMainWindowENDCLASS_t qt_meta_stringdata_CLASSMainWindowENDCLASS = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 8),  // "openFile"
        QT_MOC_LITERAL(20, 0),  // ""
        QT_MOC_LITERAL(21, 14),  // "openRecentFile"
        QT_MOC_LITERAL(36, 17),  // "onParsingProgress"
        QT_MOC_LITERAL(54, 10),  // "percentage"
        QT_MOC_LITERAL(65, 15),  // "onParsingStatus"
        QT_MOC_LITERAL(81, 6),  // "status"
        QT_MOC_LITERAL(88, 17),  // "onParsingFinished"
        QT_MOC_LITERAL(106, 16),  // "QList<DataPoint>"
        QT_MOC_LITERAL(123, 4),  // "data"
        QT_MOC_LITERAL(128, 14),  // "onParsingError"
        QT_MOC_LITERAL(143, 7),  // "message"
        QT_MOC_LITERAL(151, 11),  // "onMouseMove"
        QT_MOC_LITERAL(163, 12),  // "QMouseEvent*"
        QT_MOC_LITERAL(176, 5),  // "event"
        QT_MOC_LITERAL(182, 19),  // "onXAxisRangeChanged"
        QT_MOC_LITERAL(202, 8),  // "QCPRange"
        QT_MOC_LITERAL(211, 8),  // "newRange"
        QT_MOC_LITERAL(220, 10),  // "printChart"
        QT_MOC_LITERAL(231, 12),  // "printPreview"
        QT_MOC_LITERAL(244, 20),  // "copyChartToClipboard"
        QT_MOC_LITERAL(265, 11),  // "exportChart"
        QT_MOC_LITERAL(277, 12),  // "zoomVertical"
        QT_MOC_LITERAL(290, 6),  // "factor"
        QT_MOC_LITERAL(297, 11),  // "panVertical"
        QT_MOC_LITERAL(309, 9),  // "direction"
        QT_MOC_LITERAL(319, 13),  // "panHorizontal"
        QT_MOC_LITERAL(333, 12),  // "resetAllZoom"
        QT_MOC_LITERAL(346, 9),  // "goToStart"
        QT_MOC_LITERAL(356, 7),  // "goToEnd"
        QT_MOC_LITERAL(364, 20),  // "jumpToPeriodBoundary"
        QT_MOC_LITERAL(385, 22),  // "toggleTemperatureGraph"
        QT_MOC_LITERAL(408, 19),  // "toggleHumidityGraph"
        QT_MOC_LITERAL(428, 21)   // "updateTemperatureGrid"
    },
    "MainWindow",
    "openFile",
    "",
    "openRecentFile",
    "onParsingProgress",
    "percentage",
    "onParsingStatus",
    "status",
    "onParsingFinished",
    "QList<DataPoint>",
    "data",
    "onParsingError",
    "message",
    "onMouseMove",
    "QMouseEvent*",
    "event",
    "onXAxisRangeChanged",
    "QCPRange",
    "newRange",
    "printChart",
    "printPreview",
    "copyChartToClipboard",
    "exportChart",
    "zoomVertical",
    "factor",
    "panVertical",
    "direction",
    "panHorizontal",
    "resetAllZoom",
    "goToStart",
    "goToEnd",
    "jumpToPeriodBoundary",
    "toggleTemperatureGraph",
    "toggleHumidityGraph",
    "updateTemperatureGrid"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMainWindowENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  146,    2, 0x08,    1 /* Private */,
       3,    0,  147,    2, 0x08,    2 /* Private */,
       4,    1,  148,    2, 0x08,    3 /* Private */,
       6,    1,  151,    2, 0x08,    5 /* Private */,
       8,    1,  154,    2, 0x08,    7 /* Private */,
      11,    1,  157,    2, 0x08,    9 /* Private */,
      13,    1,  160,    2, 0x08,   11 /* Private */,
      16,    1,  163,    2, 0x08,   13 /* Private */,
      19,    0,  166,    2, 0x08,   15 /* Private */,
      20,    0,  167,    2, 0x08,   16 /* Private */,
      21,    0,  168,    2, 0x08,   17 /* Private */,
      22,    0,  169,    2, 0x08,   18 /* Private */,
      23,    1,  170,    2, 0x08,   19 /* Private */,
      25,    1,  173,    2, 0x08,   21 /* Private */,
      27,    1,  176,    2, 0x08,   23 /* Private */,
      28,    0,  179,    2, 0x08,   25 /* Private */,
      29,    0,  180,    2, 0x08,   26 /* Private */,
      30,    0,  181,    2, 0x08,   27 /* Private */,
      31,    1,  182,    2, 0x08,   28 /* Private */,
      32,    0,  185,    2, 0x08,   30 /* Private */,
      33,    0,  186,    2, 0x08,   31 /* Private */,
      34,    0,  187,    2, 0x08,   32 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   24,
    QMetaType::Void, QMetaType::Int,   26,
    QMetaType::Void, QMetaType::Int,   26,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   26,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSMainWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMainWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMainWindowENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'openFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openRecentFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onParsingProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onParsingStatus'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onParsingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<DataPoint> &, std::false_type>,
        // method 'onParsingError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onMouseMove'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QMouseEvent *, std::false_type>,
        // method 'onXAxisRangeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QCPRange &, std::false_type>,
        // method 'printChart'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'printPreview'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'copyChartToClipboard'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'exportChart'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomVertical'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'panVertical'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'panHorizontal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'resetAllZoom'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goToStart'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'goToEnd'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'jumpToPeriodBoundary'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'toggleTemperatureGraph'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleHumidityGraph'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateTemperatureGrid'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->openFile(); break;
        case 1: _t->openRecentFile(); break;
        case 2: _t->onParsingProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->onParsingStatus((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->onParsingFinished((*reinterpret_cast< std::add_pointer_t<QList<DataPoint>>>(_a[1]))); break;
        case 5: _t->onParsingError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->onMouseMove((*reinterpret_cast< std::add_pointer_t<QMouseEvent*>>(_a[1]))); break;
        case 7: _t->onXAxisRangeChanged((*reinterpret_cast< std::add_pointer_t<QCPRange>>(_a[1]))); break;
        case 8: _t->printChart(); break;
        case 9: _t->printPreview(); break;
        case 10: _t->copyChartToClipboard(); break;
        case 11: _t->exportChart(); break;
        case 12: _t->zoomVertical((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 13: _t->panVertical((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->panHorizontal((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->resetAllZoom(); break;
        case 16: _t->goToStart(); break;
        case 17: _t->goToEnd(); break;
        case 18: _t->jumpToPeriodBoundary((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->toggleTemperatureGraph(); break;
        case 20: _t->toggleHumidityGraph(); break;
        case 21: _t->updateTemperatureGrid(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMainWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    return _id;
}
QT_WARNING_POP
