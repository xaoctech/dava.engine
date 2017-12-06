#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class ScreenPositionComponent;
class UI3DView;

class ScreenPositionSystem : public SceneSystem
{
public:
    ScreenPositionSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

    void SetUI3DView(UI3DView* view);
    UI3DView* GetUI3DView() const;

private:
    Vector<ScreenPositionComponent*> components;
    UI3DView* ui3dView = nullptr;
};
}
