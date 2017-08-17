#pragma once

#include <Base/Singleton.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class Scene;
class FilePath;
}

class BeastManager;
class BeastProxyImpl : public DAVA::Singleton<BeastProxy>
{
public:
    virtual BeastManager* CreateManager();
    virtual void SafeDeleteManager(BeastManager** manager);
    virtual void Update(BeastManager* manager);
    virtual bool IsJobDone(BeastManager* manager);

    virtual int GetCurTaskProcess(BeastManager* manager) const;
    virtual DAVA::String GetCurTaskName(BeastManager* manager) const;

    virtual void Run(BeastManager* manager, DAVA::Scene* scene);
    virtual void SetLightmapsDirectory(BeastManager* manager, const DAVA::FilePath& path);
    virtual void SetMode(BeastManager* manager, eBeastMode mode);
    virtual void UpdateAtlas(BeastManager* manager, DAVA::Vector<LightmapAtlasingData>* atlasData);

    virtual void Cancel(BeastManager* manager);
    virtual bool WasCancelled(BeastManager* manager) const;
};
