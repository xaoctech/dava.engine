#pragma once

#include "Beast/BeastConstants.h"

#include <Base/Singleton.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class Scene;
class FilePath;
}

class BeastManager;
struct LightmapAtlasingData;
class BeastProxy
{
public:
    BeastManager* CreateManager();
    void SafeDeleteManager(BeastManager** manager);
    void Update(BeastManager* manager);
    bool IsJobDone(BeastManager* manager);

    int GetCurTaskProcess(BeastManager* manager) const;
    DAVA::String GetCurTaskName(BeastManager* manager) const;

    void Run(BeastManager* manager, DAVA::Scene* scene);
    void SetLightmapsDirectory(BeastManager* manager, const DAVA::FilePath& path);
    void SetMode(BeastManager* manager, eBeastMode mode);
    void UpdateAtlas(BeastManager* manager, DAVA::Vector<LightmapAtlasingData>* atlasData);

    void Cancel(BeastManager* manager);
    bool WasCancelled(BeastManager* manager) const;
};
