/****************************************************************************
** Meta object code from reading C++ file 'csvworker.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.6.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../HT_view/csvworker.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'csvworker.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSCSVWorkerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSCSVWorkerENDCLASS = QtMocHelpers::stringData(
    "CSVWorker",
    "progressChanged",
    "",
    "percentage",
    "statusChanged",
    "status",
    "finished",
    "QList<DataPoint>",
    "data",
    "error",
    "message",
    "parseFile"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSCSVWorkerENDCLASS_t {
    uint offsetsAndSizes[24];
    char stringdata0[10];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[14];
    char stringdata5[7];
    char stringdata6[9];
    char stringdata7[17];
    char stringdata8[5];
    char stringdata9[6];
    char stringdata10[8];
    char stringdata11[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSCSVWorkerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSCSVWorkerENDCLASS_t qt_meta_stringdata_CLASSCSVWorkerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 9),  // "CSVWorker"
        QT_MOC_LITERAL(10, 15),  // "progressChanged"
        QT_MOC_LITERAL(26, 0),  // ""
        QT_MOC_LITERAL(27, 10),  // "percentage"
        QT_MOC_LITERAL(38, 13),  // "statusChanged"
        QT_MOC_LITERAL(52, 6),  // "status"
        QT_MOC_LITERAL(59, 8),  // "finished"
        QT_MOC_LITERAL(68, 16),  // "QList<DataPoint>"
        QT_MOC_LITERAL(85, 4),  // "data"
        QT_MOC_LITERAL(90, 5),  // "error"
        QT_MOC_LITERAL(96, 7),  // "message"
        QT_MOC_LITERAL(104, 9)   // "parseFile"
    },
    "CSVWorker",
    "progressChanged",
    "",
    "percentage",
    "statusChanged",
    "status",
    "finished",
    "QList<DataPoint>",
    "data",
    "error",
    "message",
    "parseFile"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCSVWorkerENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   44,    2, 0x06,    1 /* Public */,
       4,    1,   47,    2, 0x06,    3 /* Public */,
       6,    1,   50,    2, 0x06,    5 /* Public */,
       9,    1,   53,    2, 0x06,    7 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,   56,    2, 0x0a,    9 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, QMetaType::QString,   10,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject CSVWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSCSVWorkerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCSVWorkerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCSVWorkerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CSVWorker, std::true_type>,
        // method 'progressChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'statusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'finished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<DataPoint> &, std::false_type>,
        // method 'error'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'parseFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CSVWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CSVWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->progressChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->statusChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->finished((*reinterpret_cast< std::add_pointer_t<QList<DataPoint>>>(_a[1]))); break;
        case 3: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->parseFile(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CSVWorker::*)(int );
            if (_t _q_method = &CSVWorker::progressChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CSVWorker::*)(const QString & );
            if (_t _q_method = &CSVWorker::statusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CSVWorker::*)(const QVector<DataPoint> & );
            if (_t _q_method = &CSVWorker::finished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CSVWorker::*)(const QString & );
            if (_t _q_method = &CSVWorker::error; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *CSVWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CSVWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCSVWorkerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CSVWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void CSVWorker::progressChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CSVWorker::statusChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CSVWorker::finished(const QVector<DataPoint> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CSVWorker::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
