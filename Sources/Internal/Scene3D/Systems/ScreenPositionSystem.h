#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Entity/SceneSystem.h"
#include "Math/Rect.h"

namespace DAVA
{
class ScreenPositionComponent;

/** System collects information about screen position of entities from current camera. */
class ScreenPositionSystem : public SceneSystem
{
public:
    ScreenPositionSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

    /** Return stored scene viewport. */
    const Rect& GetViewport() const;
    /** Setup scene viewport. */
    void SetViewport(const Rect& r);

private:
    Vector<ScreenPositionComponent*> components;
    Rect viewport;
};
}
