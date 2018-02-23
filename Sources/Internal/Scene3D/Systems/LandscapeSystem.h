#ifndef __DAVAENGINE_SCENE3D_LANDSCAPESYSTEM_H__
#define __DAVAENGINE_SCENE3D_LANDSCAPESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"
#include "Render/Highlevel/LandscapeSubdivision.h"

namespace DAVA
{
class Entity;
class Landscape;

class LandscapeSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(LandscapeSystem, SceneSystem);

    LandscapeSystem(Scene* scene);
    virtual ~LandscapeSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

    Vector<Landscape*> GetLandscapeObjects();
    const Vector<Entity*>& GetLandscapeEntities();

    void InvalidateVTPages(AABBox3 worldBox);

protected:
    void DrawPatchMetrics(const LandscapeSubdivision* subdivision, const LandscapeSubdivision::SubdivisionPatch* patch);

    Vector<Entity*> landscapeEntities;
};

} // ns

#endif /* __DAVAENGINE_SCENE3D_LANDSCAPESYSTEM_H__ */
