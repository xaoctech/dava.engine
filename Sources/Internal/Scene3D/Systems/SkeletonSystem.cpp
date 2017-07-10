#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/TransformComponent.h"
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
}

SkeletonSystem::~SkeletonSystem()
{
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

void SkeletonSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SKELETON_SYSTEM);

    for (int32 i = 0, sz = static_cast<int32>(entities.size()); i < sz; ++i)
    {
        SkeletonComponent* component = GetSkeletonComponent(entities[i]);

        //For debug. Manipulate test skinned mesh in 'Debug Functions' in RE
        //{
        //    static float32 _time = 0.f;
        //    _time += timeElapsed;

        //    uint16 jointCount = component->GetJointsCount();
        //    for (uint16 j = 1; j < jointCount; ++j)
        //    {
        //        component->GetJoint(j).bindTransform.GetTranslationVector();

        //        SkeletonComponent::JointTransform transform;
        //        transform.position = component->GetJoint(j).bindTransform.GetTranslationVector();
        //        transform.position.z += 5.f * sinf(float32(j + _time));
        //        transform.scale = 1.f;

        //        component->SetJointTransform(j, transform);
        //    }
        //}

        if (component != nullptr)
        {
            if (component->configUpdated)
            {
                RebuildSkeleton(entities[i]);
            }

            if (component->startJoint != SkeletonComponent::INVALID_JOINT_INDEX)
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

            const Vector<SkeletonComponent::Joint>& joints = component->jointsArray;
            for (uint16 i = 0; i < component->GetJointsCount(); ++i)
            {
                const SkeletonComponent::Joint& cfg = joints[i];
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
    skinnedMeshObject->SetBoundingBox(resBox); //TODO: *Skinning* decide on bbox calculation
    GetScene()->renderSystem->MarkForUpdate(skinnedMeshObject);
}

void SkeletonSystem::RebuildSkeleton(Entity* entity)
{
    SkeletonComponent* component = GetSkeletonComponent(entity);
    DVASSERT(component);

    component->configUpdated = false;

    /*convert joint configs to joints*/
    component->jointsCount = uint16(component->jointsArray.size());

    component->jointInfo.resize(component->jointsCount);
    component->localSpaceTransforms.resize(component->jointsCount);
    component->objectSpaceTransforms.resize(component->jointsCount);
    component->inverseBindTransforms.resize(component->jointsCount);
    component->jointSpaceBoxes.resize(component->jointsCount);
    component->objectSpaceBoxes.resize(component->jointsCount);
    component->jointMap.clear();

    component->targetJointsCount = 0;
    int32 maxTargetJoint = 0;
    DVASSERT(component->jointsArray.size() < SkeletonComponent::INFO_PARENT_MASK);
    for (int32 i = 0, sz = static_cast<int32>(component->jointsArray.size()); i < sz; ++i)
    {
        DVASSERT((component->jointsArray[i].parentIndex == SkeletonComponent::INVALID_JOINT_INDEX) || (component->jointsArray[i].parentIndex < i)); //order
        DVASSERT((component->jointsArray[i].parentIndex == SkeletonComponent::INVALID_JOINT_INDEX) || ((component->jointsArray[i].parentIndex & SkeletonComponent::INFO_PARENT_MASK) == component->jointsArray[i].parentIndex)); //parent fits mask
        DVASSERT((component->jointsArray[i].targetIndex == SkeletonComponent::INVALID_JOINT_INDEX) || ((component->jointsArray[i].targetIndex & SkeletonComponent::INFO_PARENT_MASK) == component->jointsArray[i].targetIndex)); //target fits mask
        DVASSERT((component->jointsArray[i].targetIndex == SkeletonComponent::INVALID_JOINT_INDEX) || (component->jointsArray[i].targetIndex < SkeletonComponent::MAX_TARGET_JOINTS));
        DVASSERT(component->jointMap.find(component->jointsArray[i].uid) == component->jointMap.end()); //duplicate bone name

        component->jointInfo[i] = component->jointsArray[i].parentIndex | (component->jointsArray[i].targetIndex << SkeletonComponent::INFO_TARGET_SHIFT) | SkeletonComponent::FLAG_MARKED_FOR_UPDATED;
        if ((component->jointsArray[i].targetIndex != SkeletonComponent::INVALID_JOINT_INDEX) && component->jointsArray[i].targetIndex > maxTargetJoint)
            maxTargetJoint = component->jointsArray[i].targetIndex;

        component->jointMap[component->jointsArray[i].uid] = i;

        SkeletonComponent::JointTransform localTransform;
        localTransform.Construct(component->jointsArray[i].bindTransform);

        component->localSpaceTransforms[i] = localTransform;
        if (component->jointsArray[i].parentIndex == SkeletonComponent::INVALID_JOINT_INDEX)
            component->objectSpaceTransforms[i] = localTransform;
        else
            component->objectSpaceTransforms[i] = component->objectSpaceTransforms[component->jointsArray[i].parentIndex].AppendTransform(localTransform);

        component->jointSpaceBoxes[i] = component->jointsArray[i].bbox;

        component->inverseBindTransforms[i].Construct(component->jointsArray[i].bindTransformInv);
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