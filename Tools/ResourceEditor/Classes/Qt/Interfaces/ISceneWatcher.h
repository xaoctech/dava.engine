#ifndef __INTERFACE_ISCENEWATCHER_H__
#define __INTERFACE_ISCENEWATCHER_H__


#include <QObject>

#include "IBase.h"


class ISceneWatcher
    : public IBase
{
public:
    virtual void OnSceneActivated(SceneEditor2 *scene) = 0;
    virtual void OnSceneDeactivated(SceneEditor2 *scene) = 0;
};


Q_DECLARE_INTERFACE( ISceneWatcher, "dava.scenewatcher" )


#endif //__INTERFACE_ISCENEWATCHER_H__
