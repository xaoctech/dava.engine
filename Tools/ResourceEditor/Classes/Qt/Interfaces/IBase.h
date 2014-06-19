#ifndef __INTERFACE_IBASE_H__
#define __INTERFACE_IBASE_H__

#include <QObject>
#include <QMetaType>

#include "Scene/SceneEditor2.h"


class IBase
{
public:
    virtual ~IBase(){}
};


Q_DECLARE_METATYPE( SceneEditor2 * )


#endif //__INTERFACE_IBASE_H__
