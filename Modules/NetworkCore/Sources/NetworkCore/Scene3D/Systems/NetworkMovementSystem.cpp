#include "NetworkCore/Scene3D/Systems/NetworkMovementSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkMovementComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

#include <Debug/DVAssert.h>
#include <Entity/ComponentUtils.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/TransformInterpolatedComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderSystem.h>

namespace DAVA
{
//#define MOVEMENT_DEBUG_DRAW

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkMovementSystem)
{
    ReflectionRegistrator<NetworkMovementSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkMovementSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 5.0f)]
    .Method("Process", &NetworkMovementSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 1.9f)]
    .End();
}

NetworkMovementSystem::NetworkMovementSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkMovementComponent, TransformComponent>())
{
    transformAndMovementGroup = GetScene()->AquireEntityGroup<NetworkMovementComponent, TransformComponent>();

    DVASSERT(frameOffset > 0);
    DVASSERT(frameOffset < historySize);
}

void NetworkMovementSystem::ReSimulationStart()
{
    isResimulation = true;
}

void NetworkMovementSystem::ReSimulationEnd()
{
    isResimulation = false;
}

float32 NetworkMovementSystem::ApplySmoothFn(float32 v, uint32 type)
{
    static const float32 pi = static_cast<float32>(std::acos(-1));
    static const float32 pi05 = pi * 0.5f;

    float32 ret = 0;
    switch (type)
    {
    case 1:
        ret = std::sin(pi05 * v);
        break;
    case 2:
        ret = std::max(0.f, 1.f + 0.3f * std::log(v));
        break;
    case 3:
        ret = v * v * v;
        break;
    case 4:
        ret = -2 * v * v * v + 3 * v * v;
        break;
    default:
        ret = v;
        break;
    }

    return ret;
}

void NetworkMovementSystem::ProcessFixed(float32 timeElapsed)
{
    for (Entity* entity : transformAndMovementGroup->GetEntities())
    {
        NetworkMovementComponent* nmc = entity->GetComponent<NetworkMovementComponent>();
        if (nmc->HistoryGetSize() != historySize)
        {
            nmc->HistoryResize(historySize);
        }
    }

    const NetworkTimeSingleComponent* timesc = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);
    const TransformSingleComponent* tsc = GetScene()->GetSingleComponentForRead<TransformSingleComponent>(this);
    for (Entity* e : tsc->localTransformChanged)
    {
        NetworkMovementComponent* nmc = e->GetComponent<NetworkMovementComponent>();
        if (nullptr != nmc)
        {
            TransformComponent* tc = e->GetComponent<TransformComponent>();

            if (!isResimulation)
            {
                NetworkMovementComponent::MoveState moveState;
                moveState.translation = tc->GetPosition();
                moveState.rotation = tc->GetRotation();
                moveState.frameId = timesc->GetFrameId();

                //Logger::Info("%u |u (%f %f)", moveState.frameId, moveState.translation.x, moveState.translation.y);

                nmc->HistoryPushBack(std::move(moveState));
            }
            else
            {
                NetworkMovementComponent::MoveState& lastMoveState = nmc->HistoryBack();

                if (lastMoveState.frameId == timesc->GetFrameId())
                {
                    NetworkMovementComponent::MoveState correctionState;
                    correctionState.translation = lastMoveState.translation - tc->GetPosition();
                    correctionState.rotation = lastMoveState.rotation * tc->GetRotation().GetInverse();
                    correctionState.frameId = lastMoveState.frameId;

                    lastMoveState.translation = tc->GetPosition();
                    lastMoveState.rotation = tc->GetRotation();

                    //Logger::Info("%u |c (%f %f)", correctionState.frameId, correctionState.translation.x, correctionState.translation.y);

                    nmc->CorrectionApply(std::move(correctionState));
                }
            }
        }
    }
}

void NetworkMovementSystem::Process(float32 timeElapsed)
{
#ifdef MOVEMENT_DEBUG_DRAW
    struct Stat
    {
        Vector3 pos;
        float32 speed;
    };

    static std::deque<Stat> stat;
#endif

    const NetworkTimeSingleComponent* timesc = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);

    static bool process = true;
    if (!process)
    {
        return;
    }

    uint32 currentFrame = timesc->GetFrameId();
    if (currentFrame > frameOffset)
    {
        float32 overlap = GetScene()->GetFixedUpdateOverlap();
        for (Entity* entity : transformAndMovementGroup->GetEntities())
        {
            uint32 viewFrame = currentFrame;

            NetworkPredictComponent* npc = entity->GetComponent<NetworkPredictComponent>();
            if (nullptr == npc || !npc->GetPredictionMask().IsSet<NetworkTransformComponent>())
            {
                viewFrame -= frameOffset;
            }

            NetworkMovementComponent* nmc = entity->GetComponent<NetworkMovementComponent>();
            NetworkMovementComponent::MoveState* begin = nullptr;
            NetworkMovementComponent::MoveState* end = nullptr;

            TransformInterpolatedComponent* tic = entity->GetComponent<TransformInterpolatedComponent>();
            if (nullptr == tic)
            {
                tic = new TransformInterpolatedComponent();
                entity->AddComponent(tic);
            }

            uint32 lastDistance = currentFrame;
            size_t sz = nmc->HistoryGetSize();
            for (size_t i = 0; i < (sz - 1); ++i)
            {
                NetworkMovementComponent::MoveState& b = nmc->HistoryAt(i);
                NetworkMovementComponent::MoveState& e = nmc->HistoryAt(i + 1);

                if (e.frameId > 0)
                {
                    uint32 distance = lastDistance;
                    if (viewFrame >= b.frameId && viewFrame < e.frameId)
                    {
                        distance = 0;
                    }
                    else if (viewFrame < b.frameId)
                    {
                        distance = b.frameId - viewFrame;
                    }
                    else if (viewFrame >= e.frameId)
                    {
                        distance = viewFrame - e.frameId + 1;
                    }

                    if (distance < lastDistance)
                    {
                        lastDistance = distance;
                        begin = &b;
                        end = &e;
                    }
                }
            }

            if (nullptr != begin && nullptr != end && (begin->translation != end->translation || begin->rotation != end->rotation) && end->frameId > viewFrame)
            {
                float32 intervalLength = static_cast<float32>(end->frameId - begin->frameId);
                float32 intervalIndex = static_cast<float32>(viewFrame - begin->frameId);
                float32 interpolation = (intervalIndex + overlap) / intervalLength;

                tic->isActual = true;
                tic->translation = Lerp(begin->translation, end->translation, interpolation);
                tic->rotation.Slerp(begin->rotation, end->rotation, interpolation);
            }

            static bool correctionEnabled = true;
            if (correctionEnabled && nmc->correctionTimeLeft > 0)
            {
                nmc->correctionTimeLeft -= timeElapsed;
                if (nmc->correctionTimeLeft < 0)
                {
                    nmc->correctionTimeLeft = 0;
                }

                Quaternion smoothRotation;
                float32 smoothK = (nmc->correctionTimeoutSec - nmc->correctionTimeLeft) / nmc->correctionTimeoutSec;
                float32 smoothTranslationK = ApplySmoothFn(smoothK, 1);
                float32 smoothRotationK = ApplySmoothFn(smoothK, 2);

                Vector3 smoothTranslation = Lerp(nmc->correction.translation, Vector3(), smoothTranslationK);
                smoothRotation.Slerp(nmc->correction.rotation, Quaternion(), smoothRotationK);

                if (tic->isActual)
                {
                    //Logger::Info("%u |+ tic (%f + %f, %f + %f)", viewFrame, tic->translation.x, smoothTranslation.x, tic->translation.y, smoothTranslation.y);

                    tic->translation += smoothTranslation;
                    tic->rotation *= smoothRotation;
                }
                else
                {
                    TransformComponent* tc = entity->GetComponent<TransformComponent>();

                    //Logger::Info("%u |+ tc (%f + %f, %f + %f)", viewFrame, tc->GetPosition().x, smoothTranslation.x, tc->GetPosition().y, smoothTranslation.y);

                    tic->isActual = true;
                    tic->translation = tc->GetPosition() + smoothTranslation;
                    tic->rotation = tc->GetRotation() * smoothRotation;
                }
            }

#ifdef MOVEMENT_DEBUG_DRAW
            if (tic->isActual)
            {
                stat.push_back({ tic->translation, (tic->translation - begin->translation).Length() / timeElapsed });
                if (stat.size() > 200)
                {
                    stat.pop_front();
                }
            }
#endif
        }
    }

#ifdef MOVEMENT_DEBUG_DRAW
    RenderHelper* rh = GetScene()->GetRenderSystem()->GetDebugDrawer();
    if (nullptr != rh)
    {
        size_t i = 0;
        for (auto s : stat)
        {
            float32 sz = 0.002f * s.speed;
            if (sz < 2.0f)
            {
                rh->DrawIcosahedron(s.pos, sz, Color::Red, RenderHelper::DRAW_SOLID_NO_DEPTH);
            }
        }
    }
#endif
}

} // namespace DAVA
