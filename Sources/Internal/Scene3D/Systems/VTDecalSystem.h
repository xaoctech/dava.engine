#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class NMaterial;
class DecalRenderObject;
class VTDecalSystem : public SceneSystem
{
public:
    VTDecalSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    //GFX_COMPLETE - this workaround here is until material editor will be moved to reflection, as we cant get material owner from InspDynamicModifyCommand
    void NotifyMaterialChanged(NMaterial* material);

private:
    Vector<DecalRenderObject*> decalObjects;
};
}