#include "Scene3D/SkeletonAnimation/MotionUtils.h"
#include "Scene3D/SkeletonAnimation/MotionLayer.h"
#include "Scene3D/SkeletonAnimation/SimpleMotion.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
void MotionUtils::UpdateMotionLayers(MotionComponent* motionComponent, float32 dTime)
{
    DVASSERT(motionComponent);

    Entity* entity = motionComponent->GetEntity();
    DVASSERT(entity != nullptr);

    Scene* scene = entity->GetScene();
    DVASSERT(scene != nullptr);

    MotionSingleComponent* motionSingleComponent = scene->GetSingleComponent<MotionSingleComponent>();
    DVASSERT(motionSingleComponent != nullptr);

    SkeletonComponent* skeleton = GetSkeletonComponent(motionComponent->GetEntity());
    if (skeleton != nullptr && (motionComponent->GetMotionLayersCount() != 0 || (motionComponent->simpleMotion != nullptr && motionComponent->simpleMotion->IsPlaying())))
    {
        dTime *= motionComponent->playbackRate;
        SkeletonPose resultPose = skeleton->GetDefaultPose();

        uint32 motionLayersCount = motionComponent->GetMotionLayersCount();
        for (uint32 l = 0; l < motionLayersCount; ++l)
        {
            MotionLayer* motionLayer = motionComponent->GetMotionLayer(l);

            motionLayer->Update(dTime);

            for (const auto& motionEnd : motionLayer->GetEndedMotions())
                motionSingleComponent->animationEnd.insert(MotionSingleComponent::AnimationInfo(motionComponent, motionLayer->GetName(), motionEnd));

            for (const auto& motionMarker : motionLayer->GetReachedMarkers())
                motionSingleComponent->animationMarkerReached.insert(MotionSingleComponent::AnimationInfo(motionComponent, motionLayer->GetName(), motionMarker.first, motionMarker.second));

            const SkeletonPose& pose = motionLayer->GetCurrentSkeletonPose();
            MotionLayer::eMotionBlend blendMode = motionLayer->GetBlendMode();
            switch (blendMode)
            {
            case MotionLayer::BLEND_OVERRIDE:
                resultPose.Override(pose);
                motionComponent->rootOffsetDelta = motionLayer->GetCurrentRootOffsetDelta();
                break;
            case MotionLayer::BLEND_ADD:
                resultPose.Add(pose);
                break;
            case MotionLayer::BLEND_DIFF:
                resultPose.Diff(pose);
                break;
            default:
                break;
            }
        }

        SimpleMotion* simpleMotion = motionComponent->simpleMotion;
        if (simpleMotion != nullptr && simpleMotion->IsPlaying())
        {
            simpleMotion->Update(dTime);
            if (!simpleMotion->IsPlaying())
                motionSingleComponent->simpleMotionFinished.emplace_back(motionComponent);

            simpleMotion->EvaluatePose(&resultPose);
        }

        skeleton->ApplyPose(resultPose);
    }
}
}