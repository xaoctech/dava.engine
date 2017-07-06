#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
SkeletonSystem::SkeletonSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

SkeletonSystem::~SkeletonSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
}

void SkeletonSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);

    SkeletonComponent* component = GetSkeletonComponent(entity);
    DVASSERT(component);

    if (component->configUpdated)
        RebuildSkeleton(entity);
}

void SkeletonSystem::RemoveEntity(Entity* entity)
{
    uint32 size = static_cast<uint32>(entities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (entities[i] == entity)
        {
            entities[i] = entities[size - 1];
            entities.pop_back();
            return;
        }
    }
    DVASSERT(0);
}

void SkeletonSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::SKELETON_CONFIG_CHANGED)
        RebuildSkeleton(component->GetEntity());
}

void SkeletonSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SKELETON_SYSTEM);

    for (int32 i = 0, sz = static_cast<int32>(entities.size()); i < sz; ++i)
    {
        SkeletonComponent* component = GetSkeletonComponent(entities[i]);
        /*test/
        {
            static float32 t=0;
            t += timeElapsed;
            for (int32 i=0, sz = component->GetJointsCount(); i<sz; ++i)
            {
                float32 fi = t;
                Quaternion q;
                q.Construct(Vector3(0,1,0), fi);
                component->SetJointOrientation(i, q);
        }
        }*/
        if (component && (component->startJoint != SkeletonComponent::INVALID_JOINT_INDEX))
        {
            UpdatePose(component);
            RenderObject* ro = GetRenderObject(entities[i]);
            if (ro && (RenderObject::TYPE_SKINNED_MESH == ro->GetType()))
            {
                SkinnedMesh* skinnedMeshObject = static_cast<SkinnedMesh*>(ro);
                DVASSERT(skinnedMeshObject);
                UpdateSkinnedMesh(component, skinnedMeshObject);
            }
        }
    }

    DrawSkeletons(GetScene()->renderSystem->GetDebugDrawer());
}

void SkeletonSystem::DrawSkeletons(RenderHelper* drawer)
{
    for (Entity* entity : entities)
    {
        SkeletonComponent* component = GetSkeletonComponent(entity);
        if (component->drawSkeleton)
        {
            const Matrix4& worldTransform = GetTransformComponent(entity)->GetWorldTransform();

            Vector<Vector3> positions(component->GetJointsCount());
            for (uint16 i = 0; i < component->GetJointsCount(); ++i)
            {
                positions[i] = component->objectSpaceTransforms[i].position * worldTransform;
            }

            const Vector<SkeletonComponent::JointConfig>& joints = component->configJoints;
            for (uint16 i = 0; i < component->GetJointsCount(); ++i)
            {
                const SkeletonComponent::JointConfig& cfg = joints[i];
                if (cfg.parentIndex != SkeletonComponent::INVALID_JOINT_INDEX)
                {
                    float32 dl = (positions[cfg.parentIndex] - positions[i]).Length();
                    drawer->DrawArrow(positions[cfg.parentIndex], positions[i], 0.25f * dl, Color(1.0f, 0.5f, 0.0f, 1.0), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
                }

                Vector3 xAxis = component->objectSpaceTransforms[i].TransformPoint(Vector3(1.f, 0.f, 0.f)) * worldTransform;
                Vector3 yAxis = component->objectSpaceTransforms[i].TransformPoint(Vector3(0.f, 1.f, 0.f)) * worldTransform;
                Vector3 zAxis = component->objectSpaceTransforms[i].TransformPoint(Vector3(0.f, 0.f, 1.f)) * worldTransform;

                drawer->DrawLine(positions[i], xAxis, Color::Red, RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
                drawer->DrawLine(positions[i], yAxis, Color::Green, RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
                drawer->DrawLine(positions[i], zAxis, Color::Blue, RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);

                //drawer->DrawAABoxTransformed(component->objectSpaceBoxes[i], worldTransform, DAVA::Color::Red, RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            }
        }
    }
}

void SkeletonSystem::UpdatePose(SkeletonComponent* component)
{
    DVASSERT(!component->configUpdated);

    uint16 count = component->GetJointsCount();
    for (uint16 currJoint = component->startJoint; currJoint < count; ++currJoint)
    {
        uint16 parentJoint = component->jointInfo[currJoint] & SkeletonComponent::INFO_PARENT_MASK;
        if ((component->jointInfo[currJoint] & SkeletonComponent::FLAG_MARKED_FOR_UPDATED) || ((parentJoint != SkeletonComponent::INVALID_JOINT_INDEX) && (component->jointInfo[parentJoint] & SkeletonComponent::FLAG_UPDATED_THIS_FRAME)))
        {
            //calculate local pose
            if (parentJoint == SkeletonComponent::INVALID_JOINT_INDEX) //root
            {
                component->objectSpaceTransforms[currJoint] = component->localSpaceTransforms[currJoint]; //just copy
            }
            else
            {
                component->objectSpaceTransforms[currJoint] = component->objectSpaceTransforms[parentJoint].AppendTransform(component->localSpaceTransforms[currJoint]);
            }
            //calculate final transform including bindTransform

            uint16 targetId = (component->jointInfo[currJoint] >> SkeletonComponent::INFO_TARGET_SHIFT) & SkeletonComponent::INFO_PARENT_MASK;
            if (targetId != SkeletonComponent::INVALID_JOINT_INDEX)
            {
                component->objectSpaceBoxes[currJoint] = component->objectSpaceTransforms[currJoint].TransformAABBox(component->jointSpaceBoxes[currJoint]);

                SkeletonComponent::JointTransform finalTransform = component->objectSpaceTransforms[currJoint].AppendTransform(component->inverseBindTransforms[currJoint]);
                component->resultPositions[targetId].Set(finalTransform.position.x, finalTransform.position.y, finalTransform.position.z, finalTransform.scale);
                component->resultQuaternions[targetId].Set(finalTransform.orientation.x, finalTransform.orientation.y, finalTransform.orientation.z, finalTransform.orientation.w);
            }

            //  add [was updated]  remove [marked for update]
            component->jointInfo[currJoint] &= ~SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
            component->jointInfo[currJoint] |= SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
        else
        {
            /*  remove was updated  - note that as bones come in descending order we do not care that was updated flag would be cared to next frame*/
            component->jointInfo[currJoint] &= ~SkeletonComponent::FLAG_UPDATED_THIS_FRAME;
        }
    }
    component->startJoint = SkeletonComponent::INVALID_JOINT_INDEX;
}

void SkeletonSystem::UpdateSkinnedMesh(SkeletonComponent* component, SkinnedMesh* skinnedMeshObject)
{
    DVASSERT(!component->configUpdated);

    //recalculate object box
    uint16 count = component->GetJointsCount();
    AABBox3 resBox;
    for (uint16 currJoint = 0; currJoint < count; ++currJoint)
    {
        uint16 targetId = (component->jointInfo[currJoint] >> SkeletonComponent::INFO_TARGET_SHIFT) & SkeletonComponent::INFO_PARENT_MASK;
        if (targetId != SkeletonComponent::INVALID_JOINT_INDEX)
            resBox.AddAABBox(component->objectSpaceBoxes[currJoint]);
    }

    //set data to SkinnedMesh
    skinnedMeshObject->SetJointsPtr(&component->resultPositions[0], &component->resultQuaternions[0], component->targetJointsCount);
    skinnedMeshObject->SetObjectSpaceBoundingBox(resBox);
    GetScene()->renderSystem->MarkForUpdate(skinnedMeshObject);
}

void SkeletonSystem::RebuildSkeleton(Entity* entity)
{
    SkeletonComponent* component = GetSkeletonComponent(entity);
    DVASSERT(component);

    component->configUpdated = false;

    /*convert joint configs to joints*/
    component->jointsCount = uint16(component->GetConfigJointsCount());

    component->jointInfo.resize(component->jointsCount);
    component->localSpaceTransforms.resize(component->jointsCount);
    component->objectSpaceTransforms.resize(component->jointsCount);
    component->inverseBindTransforms.resize(component->jointsCount);
    component->jointSpaceBoxes.resize(component->jointsCount);
    component->objectSpaceBoxes.resize(component->jointsCount);
    component->jointMap.clear();

    component->targetJointsCount = 0;
    int32 maxTargetJoint = 0;
    DVASSERT(component->configJoints.size() < SkeletonComponent::INFO_PARENT_MASK);
    for (int32 i = 0, sz = static_cast<int32>(component->configJoints.size()); i < sz; ++i)
    {
        DVASSERT((component->configJoints[i].parentIndex == SkeletonComponent::INVALID_JOINT_INDEX) || (component->configJoints[i].parentIndex < i)); //order
        DVASSERT((component->configJoints[i].parentIndex == SkeletonComponent::INVALID_JOINT_INDEX) || ((component->configJoints[i].parentIndex & SkeletonComponent::INFO_PARENT_MASK) == component->configJoints[i].parentIndex)); //parent fits mask
        DVASSERT((component->configJoints[i].targetId == SkeletonComponent::INVALID_JOINT_INDEX) || ((component->configJoints[i].targetId & SkeletonComponent::INFO_PARENT_MASK) == component->configJoints[i].targetId)); //target fits mask
        DVASSERT((component->configJoints[i].targetId == SkeletonComponent::INVALID_JOINT_INDEX) || (component->configJoints[i].targetId < SkeletonComponent::MAX_TARGET_JOINTS));
        DVASSERT(component->jointMap.find(component->configJoints[i].uid) == component->jointMap.end()); //duplicate bone name

        component->jointInfo[i] = component->configJoints[i].parentIndex | (component->configJoints[i].targetId << SkeletonComponent::INFO_TARGET_SHIFT) | SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
        if ((component->configJoints[i].targetId != SkeletonComponent::INVALID_JOINT_INDEX) && component->configJoints[i].targetId > maxTargetJoint)
            maxTargetJoint = component->configJoints[i].targetId;

        component->jointMap[component->configJoints[i].uid] = i;

        SkeletonComponent::JointTransform localTransform;
        localTransform.position = component->configJoints[i].position;
        localTransform.orientation = component->configJoints[i].orientation;
        localTransform.scale = component->configJoints[i].scale;

        component->localSpaceTransforms[i] = localTransform;
        if (component->configJoints[i].parentIndex == SkeletonComponent::INVALID_JOINT_INDEX)
            component->objectSpaceTransforms[i] = localTransform;
        else
            component->objectSpaceTransforms[i] = component->objectSpaceTransforms[component->configJoints[i].parentIndex].AppendTransform(localTransform);

        component->jointSpaceBoxes[i] = component->configJoints[i].bbox;

        component->inverseBindTransforms[i].Construct(component->configJoints[i].bindTransformInv);
    }
    component->targetJointsCount = maxTargetJoint + 1;
    component->resultPositions.resize(component->targetJointsCount);
    component->resultQuaternions.resize(component->targetJointsCount);

    component->startJoint = 0;
    UpdatePose(component);

    RenderObject* ro = GetRenderObject(entity);
    if (ro && (RenderObject::TYPE_SKINNED_MESH == ro->GetType()))
    {
        SkinnedMesh* skinnedMeshObject = static_cast<SkinnedMesh*>(ro);
        DVASSERT(skinnedMeshObject);
        UpdateSkinnedMesh(component, skinnedMeshObject);
    }
}
}