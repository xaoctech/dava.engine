#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class ScreenPositionComponent;
class UI3DView;

/** System collects information about screen position of entities from current camera. */
class ScreenPositionSystem : public SceneSystem
{
public:
    ScreenPositionSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

    /** Return pointer to UI3DView with current scene. */
    void SetUI3DView(UI3DView* view);
    /** Setup pointer to UI3DView with current scene. */
    UI3DView* GetUI3DView() const;

private:
    Vector<ScreenPositionComponent*> components;
    UI3DView* ui3dView = nullptr;
};
}
