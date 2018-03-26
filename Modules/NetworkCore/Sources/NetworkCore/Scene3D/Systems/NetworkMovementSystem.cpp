#include "NetworkCore/Scene3D/Systems/NetworkMovementSystem.h"

#include "NetworkCore/Scene3D/Components/NetworkMovementComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

#include <Debug/DVAssert.h>
#include <Entity/ComponentUtils.h>
#include <Engine/EngineSettings.h>
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
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkMovementSystem)
{
    ReflectionRegistrator<NetworkMovementSystem>::Begin()[M::Tags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkMovementSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::FIXED, 5.0f)]
    .Method("Process", &NetworkMovementSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 1.9f)]
    .End();
}

class NetworkMovementSettings
{
public:
    enum class SmoothingRule
    {
        Linear,
        Sin,
        Log,
        Cubic,
        Hermite
    };

    struct HistoryPosEntry
    {
        Vector3 pos;
        float32 speed;
    };

    uint32 interpolationFrameOffset = 1;
    uint32 interpolationDebugDrawCount = 200;
    bool interpolationDebugDrawEnabled = false;

    SmoothingRule smoothigRulePosition = SmoothingRule::Hermite;
    SmoothingRule smoothigRuleRotation = SmoothingRule::Sin;
    float32 smoothingTime = 0.5f;

    mutable std::deque<HistoryPosEntry> debugDrawHistory;

    NetworkMovementSettings()
    {
        EngineSettings* es = GetEngineContext()->settings;

        varInterpolationFrameOffset = es->RegisterVar(
        FastName("cl_interpolation_frame_offset"), interpolationFrameOffset,
        "Offset back to the previous frame to interpolate between it and next one",
        M::Range(1, 7, 1));

        varInterpolationDebugDraw = es->RegisterVar(
        FastName("cl_interpolation_draw"), interpolationDebugDrawEnabled,
        "Draw interpolated positions");

        varInterpolationDebugDrawCount = es->RegisterVar(
        FastName("cl_interpolation_draw_count"), interpolationDebugDrawCount,
        "Count of last interpolated positions to draw");

        varSmoothingRulePosition = es->RegisterVar(
        FastName("cl_smoothing_rule_position"), smoothigRulePosition,
        "Rule, used for smoothly compensate position");

        varSmoothingRuleRotation = es->RegisterVar(
        FastName("cl_smoothing_rule_rotation"), smoothigRuleRotation,
        "Rule, used for smoothly compensate position");

        varSmoothingTime = es->RegisterVar(
        FastName("cl_smoothing_time"), smoothingTime,
        "Time for mis-prediction smoothly compensation");

        es->varChanged.Connect(this, [this](EngineSettingsVar* var) {

            if (var == varInterpolationFrameOffset)
                interpolationFrameOffset = var->GetValue().Cast<uint32>();
            else if (var == varInterpolationDebugDrawCount)
                interpolationDebugDrawCount = var->GetValue().Cast<uint32>();
            else if (var == varInterpolationDebugDraw)
                interpolationDebugDrawEnabled = var->GetValue().Cast<bool>();
            else if (var == varSmoothingRulePosition)
                smoothigRulePosition = var->GetValue().Cast<SmoothingRule>();
            else if (var == varSmoothingRuleRotation)
                smoothigRuleRotation = var->GetValue().Cast<SmoothingRule>();
            else if (var == varSmoothingTime)
                smoothingTime = var->GetValue().Cast<float32>();
        });
    }

    static float32 ApplySmoothRule(float32 v, NetworkMovementSettings::SmoothingRule rule)
    {
        static const float32 pi = static_cast<float32>(std::acos(-1));
        static const float32 pi05 = pi * 0.5f;

        float32 ret = 0;
        switch (rule)
        {
        case NetworkMovementSettings::SmoothingRule::Sin:
            ret = std::sin(pi05 * v);
            break;
        case NetworkMovementSettings::SmoothingRule::Log:
            ret = std::max(0.f, 1.f + 0.3f * std::log(v));
            break;
        case NetworkMovementSettings::SmoothingRule::Cubic:
            ret = v * v * v;
            break;
        case NetworkMovementSettings::SmoothingRule::Hermite:
            ret = -2 * v * v * v + 3 * v * v;
            break;

        case NetworkMovementSettings::SmoothingRule::Linear:
        default:
            ret = v;
            break;
        }

        return ret;
    }

    static const NetworkMovementSettings* Instance()
    {
        static NetworkMovementSettings settings;
        return &settings;
    }

private:
    EngineSettingsVar* varInterpolationFrameOffset;
    EngineSettingsVar* varInterpolationDebugDrawCount;
    EngineSettingsVar* varInterpolationDebugDraw;
    EngineSettingsVar* varSmoothingRulePosition;
    EngineSettingsVar* varSmoothingRuleRotation;
    EngineSettingsVar* varSmoothingTime;
};

NetworkMovementSystem::NetworkMovementSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkMovementComponent, TransformComponent>())
{
    transformAndMovementGroup = GetScene()->AquireEntityGroup<NetworkMovementComponent, TransformComponent>();
    timeSingleComponent = GetScene()->GetSingleComponentForRead<NetworkTimeSingleComponent>(this);

    transformAndMovementGroup->onEntityAdded->Connect(this, &NetworkMovementSystem::OnMovementEntityAdded);
    settings = NetworkMovementSettings::Instance();
}

NetworkMovementSystem::~NetworkMovementSystem()
{
    transformAndMovementGroup->onEntityAdded->Disconnect(this);
}

void NetworkMovementSystem::ReSimulationStart()
{
    isResimulation = true;
}

void NetworkMovementSystem::ReSimulationEnd()
{
    isResimulation = false;
}

void NetworkMovementSystem::OnMovementEntityAdded(Entity* entity)
{
    NetworkMovementComponent* nmc = entity->GetComponent<NetworkMovementComponent>();
    TransformComponent* tc = entity->GetComponent<TransformComponent>();

    NetworkMovementComponent::MoveState moveState;
    moveState.translation = tc->GetPosition();
    moveState.rotation = tc->GetRotation();
    moveState.frameId = timeSingleComponent->GetFrameId();

    size_t historySize = settings->interpolationFrameOffset + 1;
    nmc->HistoryResize(historySize);
    nmc->HistoryPushBack(std::move(moveState));
}

void NetworkMovementSystem::ProcessFixed(float32 timeElapsed)
{
    uint32 frameId = timeSingleComponent->GetFrameId();

    size_t historySize = settings->interpolationFrameOffset + 1;
    for (Entity* entity : transformAndMovementGroup->GetEntities())
    {
        NetworkMovementComponent* nmc = entity->GetComponent<NetworkMovementComponent>();
        if (historySize != nmc->HistoryGetSize())
        {
            nmc->HistoryResize(historySize);
        }
    }

    const TransformSingleComponent* tsc = GetScene()->GetSingleComponentForRead<TransformSingleComponent>(this);
    for (Entity* e : tsc->localTransformChanged)
    {
        NetworkMovementComponent* nmc = e->GetComponent<NetworkMovementComponent>();
        if (nullptr != nmc)
        {
            TransformComponent* tc = e->GetComponent<TransformComponent>();
            NetworkMovementComponent::MoveState& lastMoveState = nmc->HistoryBack();

            if (!isResimulation)
            {
                DVASSERT(frameId >= lastMoveState.frameId);
                if (frameId > lastMoveState.frameId)
                {
                    // add new changed transform to movement history
                    // it will be used to interpolate between
                    NetworkMovementComponent::MoveState moveState;
                    moveState.translation = tc->GetPosition();
                    moveState.rotation = tc->GetRotation();
                    moveState.frameId = frameId;
                    nmc->HistoryPushBack(std::move(moveState));

                    //Logger::Info("%u | history add %f", frameId, tc->GetPosition().y);
                }
                else
                {
                    // position was changed more than once per current frame,
                    // so update it to the last one
                    lastMoveState.translation = tc->GetPosition();
                    lastMoveState.rotation = tc->GetRotation();

                    //Logger::Info("%u | history up %f", frameId, lastMoveState.translation.y);
                }
            }
            else
            {
                if (lastMoveState.frameId == frameId)
                {
                    Vector3 curSmoothPos = lastMoveState.translation;
                    Quaternion curSmoothRotation = lastMoveState.rotation;

                    // check if we have active smooth correction
                    if (nmc->smoothCorrectionTimeLeft != 0)
                    {
                        // we should calculate new correction
                        // that will also have part of currently
                        // unfinished correction
                        float32 interpolationPos = GetInteplocationPos(nmc);
                        curSmoothPos = GetSmoothCorrectedPosition(curSmoothPos, interpolationPos, nmc);
                        curSmoothRotation = GetSmoothCorrectedRotation(curSmoothRotation, interpolationPos, nmc);
                    }

                    // Now set new correction: Vector and rotation that
                    // need to be applied to move object from its right
                    // position to the last wrongly predicted position.
                    // That vector and rotation will be decreased in time
                    // by NetworkMovementSystem.
                    nmc->smoothCorrection.frameId = lastMoveState.frameId;
                    nmc->smoothCorrection.translation = curSmoothPos - tc->GetPosition();
                    nmc->smoothCorrection.rotation = curSmoothRotation * tc->GetRotation().GetInverse();
                    nmc->smoothCorrectionTimeLeft = settings->smoothingTime;

                    // if re-simulation happens, interpolation history
                    // has wrong state and shouldn't forget to change it
                    // to the right state
                    lastMoveState.translation = tc->GetPosition();
                    lastMoveState.rotation = tc->GetRotation();
                }
            }
        }
    }
}

void NetworkMovementSystem::Process(float32 timeElapsed)
{
    if (timeSingleComponent->GetFrameId() > settings->interpolationFrameOffset)
    {
        float32 overlap = GetScene()->GetFixedUpdateOverlap();
        for (Entity* entity : transformAndMovementGroup->GetEntities())
        {
            ApplyInterpolation(entity, overlap, timeElapsed);
            ApplySmoothCorrection(entity, timeElapsed);
        }
    }

    if (settings->interpolationDebugDrawEnabled)
    {
        DrawInterpolatedPosHistory();
    }
}

void NetworkMovementSystem::ApplyInterpolation(Entity* entity, float32 overlap, float32 timeElapsed)
{
    NetworkPredictComponent* npc = entity->GetComponent<NetworkPredictComponent>();
    NetworkMovementComponent* nmc = entity->GetComponent<NetworkMovementComponent>();

    uint32 currentFrameOffset = settings->interpolationFrameOffset;
    if (nullptr != npc && npc->GetPredictionMask().IsSet<NetworkTransformComponent>())
    {
        // Set currentFrameOffset for entities that have predicted
        // NetworkTransformComponent as low as possible, so that user
        // will see its locally predicted transform as fast as possible.
        currentFrameOffset = 1;
    }

    uint32 viewFrame = timeSingleComponent->GetFrameId() - currentFrameOffset;

    NetworkMovementComponent::MoveState* begin = nullptr;
    NetworkMovementComponent::MoveState* end = nullptr;

    uint32 lastDistance = viewFrame;
    size_t sz = nmc->HistoryGetSize();
    for (size_t i = 0; i < (sz - 1); ++i)
    {
        NetworkMovementComponent::MoveState& b = nmc->HistoryAt(i);
        NetworkMovementComponent::MoveState& e = nmc->HistoryAt(i + 1);

        if (b.frameId > 0 && e.frameId > 0)
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

    if (nullptr != begin && nullptr != end)
    {
        bool hasInterpolation = false;

        TransformInterpolatedComponent* tic = entity->GetComponent<TransformInterpolatedComponent>();
        if (nullptr == tic)
        {
            tic = new TransformInterpolatedComponent();
            entity->AddComponent(tic);
        }

        // no extrapolation for now
        if (end->frameId > viewFrame)
        {
            if (begin->translation != end->translation || begin->rotation != end->rotation)
            {
                Vector3 prevPos = tic->translation;
                uint32 beginFrameId = begin->frameId;

                if (viewFrame - beginFrameId >= currentFrameOffset)
                {
                    beginFrameId = viewFrame;
                }

                DVASSERT(beginFrameId < end->frameId);
                DVASSERT(beginFrameId <= viewFrame);

                float32 intervalLength = static_cast<float32>(end->frameId - beginFrameId);
                float32 intervalIndex = static_cast<float32>(viewFrame - beginFrameId);
                float32 interpolation = (intervalIndex + overlap) / intervalLength;

                tic->translation = Lerp(begin->translation, end->translation, interpolation);
                tic->rotation.Slerp(begin->rotation, end->rotation, interpolation);
                hasInterpolation = true;

                if (settings->interpolationDebugDrawEnabled)
                {
                    AppendInterpolatedPosHistory(prevPos, tic->translation, timeElapsed);
                }

                //Logger::Info("%u | view %f", viewFrame, tic->translation.y);
            }
        }
        else
        {
            // TODO:
            // check if extrapolation is allowed and extrapolate
            // ...
        }

        if (!hasInterpolation)
        {
            tic->translation = end->translation;
            tic->rotation = end->rotation;
        }
    }
}

void NetworkMovementSystem::ApplySmoothCorrection(Entity* entity, float32 timeElapsed)
{
    TransformInterpolatedComponent* tic = entity->GetComponent<TransformInterpolatedComponent>();
    NetworkMovementComponent* nmc = entity->GetComponent<NetworkMovementComponent>();

    if (nullptr != tic && nmc->smoothCorrectionTimeLeft > 0)
    {
        // Update smoothCorrection timeout
        nmc->smoothCorrectionTimeLeft -= timeElapsed;
        if (nmc->smoothCorrectionTimeLeft <= 0)
        {
            nmc->smoothCorrectionTimeLeft = 0;
            nmc->smoothCorrection.translation = Vector3();
            nmc->smoothCorrection.rotation = Quaternion();
        }

        float32 interpolationPos = GetInteplocationPos(nmc);

        // after smoothCorrection timeout was updated
        // we should update transform with applied smooth correction
        tic->translation = GetSmoothCorrectedPosition(tic->translation, interpolationPos, nmc);
        tic->rotation = GetSmoothCorrectedRotation(tic->rotation, interpolationPos, nmc);
    }
}

void NetworkMovementSystem::AppendInterpolatedPosHistory(const Vector3& prevPos, const Vector3& pos, float32 timeElapsed)
{
    float32 speed = (pos - prevPos).Length() / timeElapsed;
    if (speed > 0)
    {
        settings->debugDrawHistory.push_back({ pos, speed });
        if (settings->debugDrawHistory.size() > settings->interpolationDebugDrawCount)
        {
            settings->debugDrawHistory.pop_front();
        }
    }
}

void NetworkMovementSystem::DrawInterpolatedPosHistory()
{
    RenderHelper* rh = GetScene()->GetRenderSystem()->GetDebugDrawer();
    if (nullptr != rh)
    {
        size_t i = 0;
        for (auto s : settings->debugDrawHistory)
        {
            float32 sz = 0.003f * s.speed;
            if (sz < 2.0f)
            {
                rh->DrawIcosahedron(s.pos, sz, Color::Red, RenderHelper::DRAW_SOLID_NO_DEPTH);
            }
        }
    }
}

float32 NetworkMovementSystem::GetInteplocationPos(NetworkMovementComponent* nmc)
{
    float32 time = settings->smoothingTime;
    float32 pos = (time - nmc->smoothCorrectionTimeLeft) / time;
    return pos;
}

Vector3 NetworkMovementSystem::GetSmoothCorrectedPosition(const Vector3& orig, float32 interpolationPos, NetworkMovementComponent* nmc)
{
    interpolationPos = NetworkMovementSettings::ApplySmoothRule(interpolationPos, settings->smoothigRulePosition);
    Vector3 v = Lerp(nmc->smoothCorrection.translation, Vector3(), interpolationPos);

    return (orig + v);
}

Quaternion NetworkMovementSystem::GetSmoothCorrectedRotation(const Quaternion& orig, float32 interpolationPos, NetworkMovementComponent* nmc)
{
    Quaternion smoothRotation;

    interpolationPos = NetworkMovementSettings::ApplySmoothRule(interpolationPos, settings->smoothigRuleRotation);
    smoothRotation.Slerp(nmc->smoothCorrection.rotation, Quaternion(), interpolationPos);

    return (orig * smoothRotation);
}

} // namespace DAVA
