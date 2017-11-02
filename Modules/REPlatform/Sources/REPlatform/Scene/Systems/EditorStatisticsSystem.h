#pragma once

#include "REPlatform/Scene/SceneTypes.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
class RenderObject;
class RenderComponent;
class EditorStatisticsSystemUIDelegate;
class Camera;

struct TrianglesData;
class EditorStatisticsSystem : public SceneSystem
{
    enum eStatisticsSystemFlag : uint32
    {
        FLAG_TRIANGLES = 1 << 0,
        FLAG_NONE = 0
    };

public:
    static const int32 INDEX_OF_ALL_LODS_TRIANGLES = 0;
    static const int32 INDEX_OF_FIRST_LOD_TRIANGLES = 1;

    EditorStatisticsSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    const Vector<uint32>& GetTriangles(eEditorMode mode, bool allTriangles) const;

    void AddDelegate(EditorStatisticsSystemUIDelegate* uiDelegate);
    void RemoveDelegate(EditorStatisticsSystemUIDelegate* uiDelegate);

private:
    void CalculateTriangles();
    void ClipSelection(Camera* camera, Vector<RenderObject*>& selection,
                       Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria);

    //signals
    void EmitInvalidateUI(uint32 flags);
    void DispatchSignals();
    //signals

private:
    Vector<TrianglesData> triangles;

    Vector<EditorStatisticsSystemUIDelegate*> uiDelegates;
    uint32 invalidateUIflag = FLAG_NONE;
};

class EditorStatisticsSystemUIDelegate
{
public:
    virtual ~EditorStatisticsSystemUIDelegate() = default;

    virtual void UpdateTrianglesUI(EditorStatisticsSystem* forSystem){};
};
} //namespace DAVA
