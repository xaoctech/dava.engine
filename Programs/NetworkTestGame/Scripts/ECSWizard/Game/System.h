#pragma once

#include "Scene3D/Systems/BaseSimulationSystem.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Scene;
class Entity;
class Component;
}

class TEMPLATESystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(TEMPLATESystem, DAVA::SceneSystem);

    TEMPLATESystem(DAVA::Scene* scene);
    void PrepareForRemove() override{};

    void ProcessFixed(DAVA::float32 timeElapsed) override;
};
