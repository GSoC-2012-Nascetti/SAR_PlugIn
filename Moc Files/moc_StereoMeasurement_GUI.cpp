/****************************************************************************
** Meta object code from reading C++ file 'StereoMeasurement_GUI.h'
**
** Created: Thu 9. Aug 12:04:42 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../application/PlugIns/src/SAR_PlugIn/Code/StereoMeasurement_GUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StereoMeasurement_GUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_StereoMeasurement_GUI[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      23,   22,   22,   22, 0x0a,
      40,   22,   22,   22, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_StereoMeasurement_GUI[] = {
    "StereoMeasurement_GUI\0\0GetMapLocation()\0"
    "GetPixelLocation(bool)\0"
};

const QMetaObject StereoMeasurement_GUI::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_StereoMeasurement_GUI,
      qt_meta_data_StereoMeasurement_GUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &StereoMeasurement_GUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *StereoMeasurement_GUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *StereoMeasurement_GUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_StereoMeasurement_GUI))
        return static_cast<void*>(const_cast< StereoMeasurement_GUI*>(this));
    return QDialog::qt_metacast(_clname);
}

int StereoMeasurement_GUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: GetMapLocation(); break;
        case 1: GetPixelLocation((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
