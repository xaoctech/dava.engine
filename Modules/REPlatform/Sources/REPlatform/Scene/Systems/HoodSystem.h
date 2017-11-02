#pragma once

#include "REPlatform/DataNodes/Selectable.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/Scene/Private/Systems/HoodSystem/MoveHood.h"
#include "REPlatform/Scene/Private/Systems/HoodSystem/NormalHood.h"
#include "REPlatform/Scene/Private/Systems/HoodSystem/RotateHood.h"
#include "REPlatform/Scene/Private/Systems/HoodSystem/ScaleHood.h"
#include "REPlatform/Scene/SceneTypes.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"

// bullet
#include <bullet/btBulletCollisionCommon.h>

// framework
#include <Entity/SceneSystem.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class SceneCameraSystem;
struct HoodCollObject;
struct HoodObject;

class HoodSystem : public SceneSystem, public SelectionSystemDelegate, public EditorSceneSystem
{
public:
    HoodSystem(Scene* scene);
    ~HoodSystem();

    void SetTransformType(Selectable::TransformType mode);
    Selectable::TransformType GetTransformType() const;

    Vector3 GetPosition() const;
    void SetPosition(const Vector3& pos);

    void SetModifOffset(const Vector3& offset);
    void SetModifRotate(const float32& angle);
    void SetModifScale(const float32& scale);

    void SetModifAxis(ST_Axis axis);
    ST_Axis GetModifAxis() const;
    ST_Axis GetPassingAxis() const;

    void SetScale(float32 scale);
    float32 GetScale() const;

    void LockScale(bool lock);
    void LockModif(bool lock);
    void LockAxis(bool lock);

    void SetVisible(bool visible);
    bool IsVisible() const;

    void Process(float32 timeElapsed) override;
    void PrepareForRemove() override
    {
    }
    bool Input(UIEvent* event) override;

protected:
    void Draw() override;

private:
    void AddCollObjects(const Vector<HoodCollObject*>* objects);
    void RemCollObjects(const Vector<HoodCollObject*>* objects);
    void ResetModifValues();

    bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection) override;
    bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection) override;

    btCollisionWorld* collWorld = nullptr;
    btAxisSweep3* collBroadphase = nullptr;
    btDefaultCollisionConfiguration* collConfiguration = nullptr;
    btCollisionDispatcher* collDispatcher = nullptr;
    btIDebugDraw* collDebugDraw = nullptr;

    HoodObject* curHood = nullptr;
    SceneCameraSystem* cameraSystem = nullptr;
    NormalHood normalHood;
    MoveHood moveHood;
    RotateHood rotateHood;
    ScaleHood scaleHood;

    Vector3 curPos;
    float32 curScale = 1.0f;
    Vector3 modifOffset;
    Selectable::TransformType curMode = Selectable::TransformType::Disabled;
    ST_Axis curAxis = ST_AXIS_NONE;
    ST_Axis moseOverAxis = ST_AXIS_NONE;
    bool lockedScale = false;
    bool lockedModif = false;
    bool lockedAxis = false;
    bool isVisible = true;
};
} // namespace DAVA