#ifndef __DAVAENGINE_SCENE3D_SPEEDTREEUPDATESYSTEM_H__
#define __DAVAENGINE_SCENE3D_SPEEDTREEUPDATESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Observer.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Entity;
class SpeedTreeComponent;

class SpeedTreeUpdateSystem : public SceneSystem, public Observer
{
public:
    DAVA_VIRTUAL_REFLECTION(SpeedTreeUpdateSystem, SceneSystem);

    SpeedTreeUpdateSystem(Scene* scene);
    ~SpeedTreeUpdateSystem() override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

    void HandleEvent(Observable* observable) override;

    void SceneDidLoaded() override;

protected:
    void UpdateAnimationFlag(Entity* entity);

private:
    Vector<Entity*> allTrees;

    bool isAnimationEnabled = true;
    bool isVegetationAnimationEnabled = true;
};

} // ns

#endif /* __DAVAENGINE_SCENE3D_SPEEDTREEUPDATESYSTEM_H__ */
