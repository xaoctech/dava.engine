#include "NetworkInputSystem.h"
#include "NetworkTimeSystem.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkInputSystem)
{
    ReflectionRegistrator<NetworkInputSystem>::Begin()[M::Tags("network", "input")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkInputSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 5.0f)]
    .End();
}

namespace NetworkInputSystemDetail
{
//bits
static const int32 DIGITAL_INPUT_SIZE = 6;
static const int32 DIGITAL_INPUT_MASK = 0x3F;
static const int32 DIGITAL_COUNT_MASK = 0xF;
static const int32 DIGITAL_COUNT_OFFSET = 59;
static const int32 ANALOG_INPUT_OFFSET = 54;
static const uint64 DUPLICATE_MASK = ~(1ull << 63);
}

void NetworkInputSystem::PackDigitalActions(Scene* scene, uint64& packedActions, Entity* entity, const UnorderedSet<FastName>* filter)
{
    using namespace NetworkInputSystemDetail;
    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    const auto& allActions = GetCollectedActionsForClient(scene, entity);
    int32 count = 0;
    if (allActions.size() > 0)
    {
        const auto& digitalActions = allActions.back().digitalActions;
        ;
        for (const FastName& action : digitalActions)
        {
            if (filter == nullptr || filter->find(action) != filter->end())
            {
                int32 offset = count * DIGITAL_INPUT_SIZE;
                int32 numId = actionsSingleComponent->GetDigitalActionNumericId(action);
                packedActions |= static_cast<uint64>(numId & DIGITAL_INPUT_MASK) << offset;
                ++count;
            }
        }
    }
    packedActions |= static_cast<uint64>(count & DIGITAL_COUNT_MASK) << DIGITAL_COUNT_OFFSET;
    packedActions &= DUPLICATE_MASK;
}

Vector<FastName> NetworkInputSystem::UnpackDigitalActions(uint64 packedActions, Scene* scene)
{
    using namespace NetworkInputSystemDetail;
    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    Vector<FastName> result;
    int32 offset = DIGITAL_COUNT_OFFSET;
    int32 count = static_cast<int32>((packedActions >> offset) & DIGITAL_COUNT_MASK);
    for (int32 i = 0; i < count; ++i)
    {
        offset = i * DIGITAL_INPUT_SIZE;
        int32 numId = static_cast<int32>((packedActions >> offset) & DIGITAL_INPUT_MASK);
        result.push_back(actionsSingleComponent->GetDigitalActionId(numId));
    }

    return result;
}

uint64 NetworkInputSystem::PackAnalogActions(Scene* scene, uint64& packedActions, Entity* entity, const UnorderedSet<FastName>* filter)
{
    using namespace NetworkInputSystemDetail;
    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    const auto& availableAnalogActions = actionsSingleComponent->GetAvailableAnalogActions();
    const auto& allActions = GetCollectedActionsForClient(scene, entity);
    uint64 packedAnalogStates = 0;
    if (allActions.size() > 0)
    {
        const AnalogActionsMap& analogActions = allActions.back().analogActions;

        const int32 bufferSize = 64;
        int32 offset = 0;
        for (size_t i = 0; i < availableAnalogActions.size(); ++i)
        {
            if (filter == nullptr || filter->find(availableAnalogActions[i].actionId) != filter->end())
            {
                auto findIt = analogActions.find(availableAnalogActions[i]);
                if (findIt != analogActions.end())
                {
                    auto precision = availableAnalogActions[i].precision;
                    int32 precisionInt = precision == AnalogPrecision::ANALOG_FLOAT32 ? 32 : static_cast<int32>(precision);
                    if ((offset + 2 * precisionInt) > bufferSize)
                    {
                        DVASSERT(false, "Too many analog actions per frame");
                        break;
                    }
                    packedActions |= (1ull << (i + ANALOG_INPUT_OFFSET));

                    if (precision == AnalogPrecision::ANALOG_FLOAT32)
                    {
                        FloatCast cast;
                        cast.f = findIt->second.x;
                        packedAnalogStates |= (static_cast<uint64>(cast.u) << 32);
                        cast.f = findIt->second.y;
                        packedAnalogStates |= cast.u;
                    }
                    else
                    {
                        uint64 x = static_cast<uint64>(findIt->second.x);
                        uint64 y = static_cast<uint64>(findIt->second.y);
                        packedAnalogStates |= (x << (precisionInt + offset));
                        packedAnalogStates |= (y << (offset));
                    }
                    offset += (2 * precisionInt);
                }
            }
        }
    }
    packedActions &= DUPLICATE_MASK;
    return packedAnalogStates;
}

AnalogActionsMap NetworkInputSystem::UnpackAnalogActions(uint64 packedActions,
                                                         uint64 packedAnalogStates,
                                                         Scene* scene)
{
    using namespace NetworkInputSystemDetail;
    AnalogActionsMap result;
    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    const auto& availableAnalogActions = actionsSingleComponent->GetAvailableAnalogActions();
    const int32 bufferSize = 64;
    int32 offset = 0;
    for (size_t i = 0; i < availableAnalogActions.size() && offset < bufferSize; ++i)
    {
        uint64 mask = 1ull << (i + ANALOG_INPUT_OFFSET);
        if (mask & packedActions)
        {
            Vector2 position;
            auto precision = availableAnalogActions[i].precision;
            if (precision == AnalogPrecision::ANALOG_FLOAT32)
            {
                FloatCast cast;
                cast.u = static_cast<uint32>(packedAnalogStates >> 32);
                position.x = cast.f;
                cast.u = static_cast<uint32>(packedAnalogStates & GetAnalogPrecisionMask(AnalogPrecision::ANALOG_FLOAT32));
                position.y = cast.f;
                offset += 64;
            }
            else
            {
                uint64 precisionMask = GetAnalogPrecisionMask(precision);
                int32 precisionInt = static_cast<int32>(precision);
                position.x = static_cast<float32>((packedAnalogStates >> (offset + precisionInt)) & precisionMask);
                position.y = static_cast<float32>((packedAnalogStates >> offset) & precisionMask);
                offset += (2 * precisionInt);
            }
            result.emplace(availableAnalogActions[i], position);
        }
    }
    return result;
}
}
