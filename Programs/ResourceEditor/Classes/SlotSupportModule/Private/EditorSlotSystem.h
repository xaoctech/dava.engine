#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

class EditorSlotSystem : public DAVA::SceneSystem
{
public:
    EditorSlotSystem(DAVA::Scene* scene);

    void AddComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void RemoveComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void Process(DAVA::float32 timeElapsed) override;

private:
    DAVA::Vector<DAVA::Entity*> entities;
};
