#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <Base/AlignedAllocator.h>
#include <Scene3D/Scene.h>
#include <TArc/Core/Deprecated.h>

namespace DAVA
{
HoodSystem::HoodSystem(Scene* scene)
    : SceneSystem(scene)
{
    btVector3 worldMin(-1000, -1000, -1000);
    btVector3 worldMax(1000, 1000, 1000);

    collConfiguration = new btDefaultCollisionConfiguration();
    collDispatcher = CreateObjectAligned<btCollisionDispatcher, 16>(collConfiguration);
    collBroadphase = new btAxisSweep3(worldMin, worldMax);
    collDebugDraw = new SceneCollisionDebugDrawer(scene->GetRenderSystem()->GetDebugDrawer());
    collDebugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    collWorld = new btCollisionWorld(collDispatcher, collBroadphase, collConfiguration);
    collWorld->setDebugDrawer(collDebugDraw);

    SetModifAxis(ST_AXIS_X);
    SetTransformType(Selectable::TransformType::Translation);

    moveHood.colorX = Color(1, 0, 0, 1);
    moveHood.colorY = Color(0, 1, 0, 1);
    moveHood.colorZ = Color(0, 0, 1, 1);
    moveHood.colorS = Color(1, 1, 0, 1);

    rotateHood.colorX = Color(1, 0, 0, 1);
    rotateHood.colorY = Color(0, 1, 0, 1);
    rotateHood.colorZ = Color(0, 0, 1, 1);
    rotateHood.colorS = Color(1, 1, 0, 1);

    scaleHood.colorX = Color(1, 0, 0, 1);
    scaleHood.colorY = Color(0, 1, 0, 1);
    scaleHood.colorZ = Color(0, 0, 1, 1);
    scaleHood.colorS = Color(1, 1, 0, 1);

    normalHood.colorX = Color(0.7f, 0.3f, 0.3f, 1);
    normalHood.colorY = Color(0.3f, 0.7f, 0.3f, 1);
    normalHood.colorZ = Color(0.3f, 0.3f, 0.7f, 1);
    normalHood.colorS = Color(0, 0, 0, 1);

    cameraSystem = scene->GetSystem<DAVA::SceneCameraSystem>();
}

HoodSystem::~HoodSystem()
{
    delete collWorld;
    delete collDebugDraw;
    delete collBroadphase;
    DestroyObjectAligned(collDispatcher);
    delete collConfiguration;
}

Vector3 HoodSystem::GetPosition() const
{
    return (curPos + modifOffset);
}

void HoodSystem::SetPosition(const Vector3& pos)
{
    if (!IsLocked() && !lockedScale)
    {
        if (curPos != pos || !modifOffset.IsZero())
        {
            curPos = pos;
            ResetModifValues();

            if (NULL != curHood)
            {
                curHood->UpdatePos(curPos);
                normalHood.UpdatePos(curPos);

                collWorld->updateAabbs();
            }
        }
    }
}

void HoodSystem::SetModifOffset(const Vector3& offset)
{
    if (!IsLocked())
    {
        moveHood.modifOffset = offset;

        if (modifOffset != offset)
        {
            modifOffset = offset;

            if (NULL != curHood)
            {
                curHood->UpdatePos(curPos + modifOffset);
                normalHood.UpdatePos(curPos + modifOffset);

                collWorld->updateAabbs();
            }
        }
    }
}

void HoodSystem::SetModifRotate(const float32& angle)
{
    if (!IsLocked())
    {
        rotateHood.modifRotate = angle;
    }
}

void HoodSystem::SetModifScale(const float32& scale)
{
    if (!IsLocked())
    {
        scaleHood.modifScale = scale;
    }
}

void HoodSystem::SetScale(float32 scale)
{
    if (!IsLocked())
    {
        scale = scale * Deprecated::GetDataNode<GlobalSceneSettings>()->gizmoScale;

        if (curScale != scale && 0 != scale)
        {
            curScale = scale;

            if (NULL != curHood)
            {
                curHood->UpdateScale(curScale);
                normalHood.UpdateScale(curScale);

                collWorld->updateAabbs();
            }
        }
    }
}

float32 HoodSystem::GetScale() const
{
    return curScale;
}

void HoodSystem::SetTransformType(Selectable::TransformType mode)
{
    if (!IsLocked())
    {
        if (curMode != mode)
        {
            if (NULL != curHood)
            {
                RemCollObjects(&curHood->collObjects);
            }

            curMode = mode;
            switch (mode)
            {
            case Selectable::TransformType::Translation:
                curHood = &moveHood;
                break;
            case Selectable::TransformType::Scale:
                curHood = &scaleHood;
                break;
            case Selectable::TransformType::Rotation:
                curHood = &rotateHood;
                break;
            default:
                curHood = &normalHood;
                break;
            }

            if (NULL != curHood)
            {
                AddCollObjects(&curHood->collObjects);

                curHood->UpdatePos(curPos + modifOffset);
                curHood->UpdateScale(curScale);
            }

            collWorld->updateAabbs();
        }
    }
}

Selectable::TransformType HoodSystem::GetTransformType() const
{
    if (lockedModif)
    {
        return Selectable::TransformType::Disabled;
    }

    return curMode;
}

void HoodSystem::SetVisible(bool visible)
{
    if (!IsLocked())
    {
        isVisible = visible;
    }
}

bool HoodSystem::IsVisible() const
{
    return isVisible;
}

void HoodSystem::AddCollObjects(const Vector<HoodCollObject*>* objects)
{
    if (NULL != objects)
    {
        for (size_t i = 0; i < objects->size(); ++i)
        {
            collWorld->addCollisionObject(objects->operator[](i)->btObject);
        }
    }
}

void HoodSystem::RemCollObjects(const Vector<HoodCollObject*>* objects)
{
    if (NULL != objects)
    {
        for (size_t i = 0; i < objects->size(); ++i)
        {
            collWorld->removeCollisionObject(objects->operator[](i)->btObject);
        }
    }
}

void HoodSystem::ResetModifValues()
{
    modifOffset = Vector3(0, 0, 0);

    rotateHood.modifRotate = 0;
    scaleHood.modifScale = 0;
}

void HoodSystem::Process(float32 timeElapsed)
{
    if (!IsLocked() && !lockedScale)
    {
        // scale hood depending on current camera position
        Camera* curCamera = cameraSystem->GetCurCamera();
        if (NULL != curCamera)
        {
            float32 camToHoodDist = (GetPosition() - curCamera->GetPosition()).Length();
            if (curCamera->GetIsOrtho())
            {
                SetScale(30.0f);
            }
            else
            {
                SetScale(camToHoodDist / 20.f);
            }
        }
    }
}

bool HoodSystem::Input(UIEvent* event)
{
    if (!event->point.IsZero())
    {
        // before checking result mark that there is no hood axis under mouse
        if (!lockedScale && !lockedAxis)
        {
            moseOverAxis = ST_AXIS_NONE;

            // if is visible and not locked check mouse over status
            if (!lockedModif && NULL != curHood)
            {
                // get intersected items in the line from camera to current mouse position
                Vector3 traceFrom;
                Vector3 traceTo;

                cameraSystem->GetRayTo2dPoint(event->point, 99999.0f, traceFrom, traceTo);

                btVector3 btFrom(traceFrom.x, traceFrom.y, traceFrom.z);
                btVector3 btTo(traceTo.x, traceTo.y, traceTo.z);

                btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
                collWorld->rayTest(btFrom, btTo, btCallback);

                if (btCallback.hasHit())
                {
                    const Vector<HoodCollObject*>* curHoodObjects = &curHood->collObjects;
                    for (size_t i = 0; i < curHoodObjects->size(); ++i)
                    {
                        HoodCollObject* hObj = curHoodObjects->operator[](i);

                        if (hObj->btObject == btCallback.m_collisionObjects[0])
                        {
                            // mark that mouse is over one of hood axis
                            moseOverAxis = hObj->axis;
                            break;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void HoodSystem::Draw()
{
    if ((curHood == nullptr) || !IsVisible())
        return;

    TextDrawSystem* textDrawSys = GetScene()->GetSystem<TextDrawSystem>();

    // modification isn't locked and whole system isn't locked
    if (!IsLocked() && !lockedModif)
    {
        ST_Axis showAsSelected = curAxis;
        if ((GetTransformType() != Selectable::TransformType::Disabled) && (ST_AXIS_NONE != moseOverAxis))
        {
            showAsSelected = moseOverAxis;
        }

        curHood->Draw(showAsSelected, moseOverAxis, GetScene()->GetRenderSystem()->GetDebugDrawer(), textDrawSys);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(AABBox3(GetPosition(), curHood->objScale * .04f), Color::White, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        normalHood.Draw(curAxis, ST_AXIS_NONE, GetScene()->GetRenderSystem()->GetDebugDrawer(), textDrawSys);
    }
}

void HoodSystem::SetModifAxis(ST_Axis axis)
{
    if (ST_AXIS_NONE != axis)
    {
        curAxis = axis;
    }
}

ST_Axis HoodSystem::GetModifAxis() const
{
    return curAxis;
}

ST_Axis HoodSystem::GetPassingAxis() const
{
    return moseOverAxis;
}

void HoodSystem::LockScale(bool lock)
{
    lockedScale = lock;
}

void HoodSystem::LockModif(bool lock)
{
    lockedModif = lock;
}

void HoodSystem::LockAxis(bool lock)
{
    lockedAxis = lock;
}

bool HoodSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    return !IsVisible() || (GetTransformType() == Selectable::TransformType::Disabled) || (ST_AXIS_NONE == GetPassingAxis());
}

bool HoodSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    return true;
}
} // namespace DAVA
