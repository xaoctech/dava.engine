#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class Scene;
class Entity;
class RenderHelper;
}

class ShooterStateComponent;

class DrawShootSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(DrawShootSystem, DAVA::SceneSystem);

    DrawShootSystem(DAVA::Scene* scene);

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    DAVA::ComponentGroup<ShooterStateComponent>* stateGroup = nullptr;
    DAVA::RenderHelper* renderHelper = nullptr;
};