/****************************************************************************
** Meta object code from reading C++ file 'Ortho_RADARSAT.h'
**
** Created: Thu 9. Aug 12:13:33 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../application/PlugIns/src/SAR_PlugIn/Code/Ortho_RADARSAT.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Ortho_RADARSAT.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Ortho_RADARSAT[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Ortho_RADARSAT[] = {
    "Ortho_RADARSAT\0\0dialogClosed()\0"
};

const QMetaObject Ortho_RADARSAT::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Ortho_RADARSAT,
      qt_meta_data_Ortho_RADARSAT, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Ortho_RADARSAT::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Ortho_RADARSAT::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Ortho_RADARSAT::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Ortho_RADARSAT))
        return static_cast<void*>(const_cast< Ortho_RADARSAT*>(this));
    if (!strcmp(_clname, "ViewerShell"))
        return static_cast< ViewerShell*>(const_cast< Ortho_RADARSAT*>(this));
    return QObject::qt_metacast(_clname);
}

int Ortho_RADARSAT::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: dialogClosed(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
