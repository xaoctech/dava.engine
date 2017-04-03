#pragma once

#include <Entity/SceneSystem.h>

class EditorSlotSystem : public DAVA::SceneSystem
{
public:
    EditorSlotSystem(DAVA::Scene* scene);

    void AddComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void RemoveComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void Process(float32 timeElapsed) override;

private:
    Vector<DAVA::Entity*> entities;
};
