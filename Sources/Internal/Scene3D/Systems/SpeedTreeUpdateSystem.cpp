#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Math/Math2D.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Render/3D/MeshUtils.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Renderer.h"
#include "Utils/Random.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SpeedTreeUpdateSystem)
{
    ReflectionRegistrator<SpeedTreeUpdateSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &SpeedTreeUpdateSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 13.0f)]
    .End();
}

const float minTrunkFlexibility = 0.001f;
const float maxTrunkFlexibility = 0.1f;

SpeedTreeUpdateSystem::SpeedTreeUpdateSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<SpeedTreeComponent>() | ComponentUtils::MakeMask<RenderComponent>())
{
    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);

    isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);
}

SpeedTreeUpdateSystem::~SpeedTreeUpdateSystem()
{
    DVASSERT(allTrees.empty());
    Renderer::GetOptions()->RemoveObserver(this);
}

void SpeedTreeUpdateSystem::AddEntity(Entity* entity)
{
    SpeedTreeComponent* component = GetSpeedTreeComponent(entity);
    DVASSERT(component != nullptr);
    allTrees.emplace_back(entity);

    //GFX_COMPLETE - set inv matrix in advance
    const Matrix4* wtMxPrt = GetTransformComponent(component->GetEntity())->GetWorldTransformPtr();
    wtMxPrt->GetInverse(component->worldTransfromInv);
    DVASSERT(GetSpeedTreeObject(entity) != nullptr);
    GetSpeedTreeObject(entity)->SetInvWorldTransformPtr(&component->worldTransfromInv);
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity* entity)
{
    uint32 treeCount = static_cast<uint32>(allTrees.size());
    for (uint32 i = 0; i < treeCount; ++i)
    {
        if (allTrees[i] == entity)
        {
            RemoveExchangingWithLast(allTrees, i);
            break;
        }
    }
}

void SpeedTreeUpdateSystem::PrepareForRemove()
{
    allTrees.clear();
}

void SpeedTreeUpdateSystem::UpdateAnimationFlag(Entity* entity)
{
    DVASSERT(GetRenderObject(entity)->GetType() == RenderObject::TYPE_SPEED_TREE);

    SpeedTreeObject* treeObject = static_cast<SpeedTreeObject*>(GetRenderObject(entity));
    SpeedTreeComponent* component = GetSpeedTreeComponent(entity);
    treeObject->UpdateAnimationFlag(0);
}

void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_SPEEDTREE_SYSTEM);

    TransformSingleComponent* tsc = GetScene()->GetSingletonComponent<TransformSingleComponent>();

    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<SpeedTreeComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
                SpeedTreeComponent* component = entity->GetComponent<SpeedTreeComponent>();
                const Matrix4* wtMxPrt = GetTransformComponent(component->GetEntity())->GetWorldTransformPtr();
                wtMxPrt->GetInverse(component->worldTransfromInv);

                DVASSERT(GetSpeedTreeObject(entity) != nullptr);
                GetSpeedTreeObject(entity)->SetInvWorldTransformPtr(&component->worldTransfromInv);
            }
        }
    }

    if (!isAnimationEnabled || !isVegetationAnimationEnabled)
        return;

    WindSystem* windSystem = GetScene()->windSystem;
    WaveSystem* waveSystem = GetScene()->waveSystem;

    for (Entity* tree : allTrees)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(tree);
        if (skeleton == nullptr)
            continue;

        TransformComponent* transform = GetTransformComponent(tree);
        if (transform == nullptr)
            continue;

        SpeedTreeComponent* treeComponent = GetSpeedTreeComponent(tree);
        DVASSERT(treeComponent != nullptr);

        RenderObject* renderObject = GetRenderObject(tree);
        DVASSERT(renderObject->GetType() == RenderObject::TYPE_SPEED_TREE);

        // Get wind direction and force in object's local space
        Vector3 treePosition = transform->GetWorldTransform().GetTranslationVector();
        Vector3 wind = windSystem->GetWind(treePosition) + waveSystem->GetWaveDisturbance(treePosition);
        float windForce = wind.Length();
        if (windForce > 0.0f)
        {
            wind = MultiplyVectorMat3x3(wind, treeComponent->worldTransfromInv);
            wind.Normalize();
        }
        else
        {
            wind = Vector3(1.0f, 0.0f, 0.0f);
        }

        static_cast<SpeedTreeObject*>(renderObject)->wind = Vector4(wind, windForce);
        static_cast<SpeedTreeObject*>(renderObject)->flexibility = Vector4(treeComponent->GetLeavesFlexibility() / (0.5f * PI), 0.0f, 0.0f, 0.0f);

        const Vector<SpeedTreeComponent::Bone>& treeBones = treeComponent->GetBones();
        if (treeBones.empty() == false)
        {
            // get max weight for trunk and branches bones
            float maxTrunkWeight = 1.0f;
            float maxBranchWeight = 0.001f;
            for (const SpeedTreeComponent::Bone& bone : treeBones)
            {
                if (bone.element == SpeedTreeComponent::Element::Trunk)
                    maxTrunkWeight = std::max(maxTrunkWeight, bone.massWithChildren);
                else
                    maxBranchWeight = std::max(maxBranchWeight, bone.massWithChildren);
            }

            // rotate trunk and branch bones
            Vector<SpeedTreeComponent::RuntimeBone>& runtimeBones = treeComponent->GetRuntimeBones();
            for (const SpeedTreeComponent::Bone& bone : treeBones)
            {
                const JointTransform& boneTransform = skeleton->GetJointObjectSpaceTransform(bone.id);
                const Vector3& jointPos = boneTransform.GetPosition();
                Vector3 parentPos = (bone.parentId == -1) ? Vector3(0.0f, 0.0f, 0.0f) : boneTransform.GetPosition();

                Vector3 boneAxis = jointPos - parentPos;
                float boneAxisLength = boneAxis.Length();
                if (boneAxisLength > 0.0f)
                {
                    boneAxis /= boneAxisLength;
                }
                else
                {
                    boneAxis = Vector3(0.0f, 0.0f, 1.0f);
                }

                if (bone.element == SpeedTreeComponent::Element::Trunk)
                {
                    Vector3 torque(0.0f, 0.0f, 0.0f);
                    Vector3 rotationAxis = boneAxis.CrossProduct(wind);
                    rotationAxis.Normalize();

                    if (treeComponent->GetTrunkFlexibility() > 0.0f)
                    {
                        float flex = Clamp(treeComponent->GetTrunkFlexibility(), minTrunkFlexibility, maxTrunkFlexibility);
                        Vector3 v = runtimeBones[bone.id].velocities;
                        torque = rotationAxis * (1.0f - bone.massWithChildren / maxTrunkWeight) * windForce;
                        torque -= runtimeBones[bone.id].angles / flex;
                    }

                    runtimeBones[bone.id].velocities += torque * timeElapsed;
                    runtimeBones[bone.id].velocities *= Clamp(1.0f - treeComponent->GetTrunkDamping() * timeElapsed, 0.0f, 1.0f);
                    runtimeBones[bone.id].angles += runtimeBones[bone.id].velocities * timeElapsed;

                    Quaternion orientation;
                    orientation.Construct(runtimeBones[bone.id].angles);

                    JointTransform transform = skeleton->GetJointTransform(bone.id);
                    transform.SetOrientation(orientation);
                    skeleton->SetJointTransform(bone.id, transform);
                }
                else if (bone.element == SpeedTreeComponent::Element::Branch)
                {
                    Vector3 rotationAxis = boneAxis.CrossProduct(wind).z < 0.0f ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);

                    float rotationAngle = (1.0f - bone.massWithChildren / maxBranchWeight);
                    float angleVariation = (bone.start.x + bone.start.y + bone.start.z) * bone.massWithChildren / maxBranchWeight;
                    rotationAngle += 0.25f * std::cos(GetScene()->GetGlobalTime() * angleVariation);

                    Quaternion orientation;
                    orientation.Construct(rotationAxis, rotationAngle * treeComponent->GetBranchesFlexibility() * windForce / (2.0f * PI));

                    JointTransform transform = skeleton->GetJointTransform(bone.id);
                    transform.SetOrientation(orientation);
                    skeleton->SetJointTransform(bone.id, transform);
                }
            }
        }
    }
}

void SpeedTreeUpdateSystem::HandleEvent(Observable* observable)
{
    RenderOptions* options = static_cast<RenderOptions*>(observable);

    if (isAnimationEnabled != options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS))
    {
        isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);

        uint32 treeCount = static_cast<uint32>(allTrees.size());
        for (uint32 i = 0; i < treeCount; ++i)
        {
            UpdateAnimationFlag(allTrees[i]);
        }
    }
}

void SpeedTreeUpdateSystem::SceneDidLoaded()
{
    for (Entity* tree : allTrees)
    {
        RenderComponent* renderComponent = GetRenderComponent(tree);
        if (renderComponent != nullptr)
        {
            RenderObject* ro = renderComponent->GetRenderObject();
            if (ro != nullptr)
            {
                ro->RecalcBoundingBox();
                ro->RecalculateWorldBoundingBox();
            }
        }
    }
}
} // namespace DAVA
