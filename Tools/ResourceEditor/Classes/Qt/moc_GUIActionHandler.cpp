/****************************************************************************
** Meta object code from reading C++ file 'GUIActionHandler.h'
**
** Created: Mon Jul 23 12:27:01 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "GUIActionHandler.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GUIActionHandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GUIActionHandler[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x0a,
      29,   17,   17,   17, 0x0a,
      41,   17,   17,   17, 0x0a,
      61,   55,   17,   17, 0x0a,
      90,   17,   17,   17, 0x0a,
     102,   17,   17,   17, 0x0a,
     116,   17,   17,   17, 0x0a,
     130,   17,   17,   17, 0x0a,
     144,   17,   17,   17, 0x0a,
     162,   17,   17,   17, 0x0a,
     176,   17,   17,   17, 0x0a,
     196,   17,   17,   17, 0x0a,
     208,   17,   17,   17, 0x0a,
     223,   17,   17,   17, 0x0a,
     238,   17,   17,   17, 0x0a,
     255,   17,   17,   17, 0x0a,
     272,   17,   17,   17, 0x0a,
     284,   17,   17,   17, 0x0a,
     302,   17,   17,   17, 0x0a,
     320,   17,   17,   17, 0x0a,
     336,   17,   17,   17, 0x0a,
     353,   17,   17,   17, 0x0a,
     370,   17,   17,   17, 0x0a,
     385,   17,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GUIActionHandler[] = {
    "GUIActionHandler\0\0NewScene()\0OpenScene()\0"
    "OpenProject()\0index\0OpenResentScene(DAVA::int32)\0"
    "SaveScene()\0ExportAsPNG()\0ExportAsPVR()\0"
    "ExportAsDXT()\0CreateLandscape()\0"
    "CreateLight()\0CreateServiceNode()\0"
    "CreateBox()\0CreateSphere()\0CreateCamera()\0"
    "CreateImposter()\0CreateUserNode()\0"
    "Materials()\0ConvertTextures()\0"
    "HeightmapEditor()\0TilemapEditor()\0"
    "ViewportiPhone()\0VeiwportRetina()\0"
    "ViewportiPad()\0ViewportDefault()\0"
};

void GUIActionHandler::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GUIActionHandler *_t = static_cast<GUIActionHandler *>(_o);
        switch (_id) {
        case 0: _t->NewScene(); break;
        case 1: _t->OpenScene(); break;
        case 2: _t->OpenProject(); break;
        case 3: _t->OpenResentScene((*reinterpret_cast< DAVA::int32(*)>(_a[1]))); break;
        case 4: _t->SaveScene(); break;
        case 5: _t->ExportAsPNG(); break;
        case 6: _t->ExportAsPVR(); break;
        case 7: _t->ExportAsDXT(); break;
        case 8: _t->CreateLandscape(); break;
        case 9: _t->CreateLight(); break;
        case 10: _t->CreateServiceNode(); break;
        case 11: _t->CreateBox(); break;
        case 12: _t->CreateSphere(); break;
        case 13: _t->CreateCamera(); break;
        case 14: _t->CreateImposter(); break;
        case 15: _t->CreateUserNode(); break;
        case 16: _t->Materials(); break;
        case 17: _t->ConvertTextures(); break;
        case 18: _t->HeightmapEditor(); break;
        case 19: _t->TilemapEditor(); break;
        case 20: _t->ViewportiPhone(); break;
        case 21: _t->VeiwportRetina(); break;
        case 22: _t->ViewportiPad(); break;
        case 23: _t->ViewportDefault(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GUIActionHandler::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GUIActionHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_GUIActionHandler,
      qt_meta_data_GUIActionHandler, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GUIActionHandler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GUIActionHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GUIActionHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GUIActionHandler))
        return static_cast<void*>(const_cast< GUIActionHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int GUIActionHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
