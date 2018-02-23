#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ActionsSingleComponent)
{
    ReflectionRegistrator<ActionsSingleComponent>::Begin()
    .ConstructorByPointer()
    .Method("AddDigitalAction", &ActionsSingleComponent::AddDigitalAction)
    .Method("AddAnalogAction", &ActionsSingleComponent::AddAnalogAction)
    .End();
}

template <typename C, typename I>
void AddAvailableAction(const FastName& actionId, const I& item,
                        C& container, UnorderedSet<FastName>& index)
{
    auto findIt = index.find(actionId);
    if (findIt == index.end())
    {
        container.push_back(item);
        index.insert(actionId);
    }
}

bool IsAvaildableAction(const FastName& actionId, const UnorderedSet<FastName>& index)
{
    return index.find(actionId) != index.end();
}

bool ActionsSingleComponent::AnalogActionInfo::operator==(const AnalogActionInfo& info) const
{
    return actionId == info.actionId;
}

ActionsSingleComponent::ActionsSingleComponent()
{
    Clear();
}

void ActionsSingleComponent::AddDigitalAction(const FastName& actionId, uint8 playerId /* = 0 */)
{
    Vector<FastName>& digitalActions = GetActions(playerId).back().digitalActions;
    DVASSERT(digitalActions.size() < SIMULTANEOUS_DIGITAL_SIZE);
    if (digitalActions.size() < SIMULTANEOUS_DIGITAL_SIZE)
    {
        auto findIt = std::find(digitalActions.begin(), digitalActions.end(), actionId);
        if (findIt == digitalActions.end())
        {
            digitalActions.push_back(actionId);
        }
    }
}

void ActionsSingleComponent::AddAvailableDigitalAction(const FastName& actionId)
{
    DVASSERT(availableDigitalActions.size() < MAX_DIGITAL_SIZE);
    if (availableDigitalActions.size() < MAX_DIGITAL_SIZE)
    {
        AddAvailableAction(actionId, actionId, availableDigitalActions, digitalIndex);
    }
}

const Vector<FastName>& ActionsSingleComponent::GetAvailableDigitalActions() const
{
    return availableDigitalActions;
}

bool ActionsSingleComponent::IsAvaildableDigitalAction(const FastName& actionId) const
{
    return IsAvaildableAction(actionId, digitalIndex);
}

void ActionsSingleComponent::AddAnalogAction(const FastName& actionId, const Vector2& state, uint8 playerId /* = 0 */)
{
    auto infoIt = std::find_if(availableAnalogActions.begin(), availableAnalogActions.end(),
                               [&actionId](const AnalogActionInfo& info) {
                                   return info.actionId == actionId;
                               });
    if (infoIt != availableAnalogActions.end())
    {
        AnalogActionsMap& analogActions = GetActions(playerId).back().analogActions;
        Vector2 position;
        switch (infoIt->precision)
        {
        case AnalogPrecision::ANALOG_FLOAT32:
            position = state;
            break;
        case AnalogPrecision::ANALOG_UINT8:
            position = ConvertAnalogToFixedPrecision<uint8>(AnalogPrecision::ANALOG_UINT8, state);
            break;
        case AnalogPrecision::ANALOG_UINT16:
            position = ConvertAnalogToFixedPrecision<uint16>(AnalogPrecision::ANALOG_UINT16, state);
            break;
        case AnalogPrecision::ANALOG_UINT32:
            position = ConvertAnalogToFixedPrecision<uint32>(AnalogPrecision::ANALOG_UINT32, state);
            break;
        }

        analogActions.emplace(*infoIt, position);
    }
}

void ActionsSingleComponent::AddAvailableAnalogAction(const FastName& actionId,
                                                      AnalogPrecision precision)
{
    int32 size = precision == AnalogPrecision::ANALOG_FLOAT32 ? 64 :
                                                                static_cast<int32>(precision) * 2;
    AddAvailableAction(actionId, AnalogActionInfo{ actionId, precision },
                       availableAnalogActions, analogIndex);
}

const Vector<ActionsSingleComponent::AnalogActionInfo>& ActionsSingleComponent::GetAvailableAnalogActions() const
{
    return availableAnalogActions;
}

bool ActionsSingleComponent::IsAvaildableAnalogAction(const FastName& actionId) const
{
    return IsAvaildableAction(actionId, analogIndex);
}

void ActionsSingleComponent::SetCameraDelta(const Quaternion& cameraDelta_, uint8 playerId /* = 0 */)
{
    Actions& lastActions = GetActions(playerId).back();
    lastActions.cameraDelta = cameraDelta_;
}

void ActionsSingleComponent::AddActions(uint8 playerId, ActionsSingleComponent::Actions&& actions)
{
    playerIdsToActions[playerId].push_back(actions);
}

Vector<ActionsSingleComponent::Actions>& ActionsSingleComponent::GetActions(uint8 playerId /* = 0 */)
{
    Vector<ActionsSingleComponent::Actions>& allActions = playerIdsToActions[playerId];
    if (allActions.size() == 0)
    {
        allActions.emplace_back();
    }

    return allActions;
}

int32 ActionsSingleComponent::GetDigitalActionNumericId(const FastName& actionId) const
{
    auto findIt = std::find(availableDigitalActions.begin(), availableDigitalActions.end(), actionId);
    if (findIt == availableDigitalActions.end())
    {
        DVASSERT(false && "Action is not available");
        return -1;
    }
    return static_cast<int32>(std::distance(availableDigitalActions.begin(), findIt));
}

const FastName& ActionsSingleComponent::GetDigitalActionId(int32 actionNumericId) const
{
    DVASSERT(actionNumericId >= 0 && actionNumericId < static_cast<int32>(availableDigitalActions.size()));
    return availableDigitalActions[actionNumericId];
}

void ActionsSingleComponent::Clear()
{
    playerIdsToActions.clear();
}

void ActionsSingleComponent::SetLocalPlayerId(int32 playerId)
{
    DVASSERT(playerId >= 0);
    localPlayerId = playerId;
}

int32 ActionsSingleComponent::GetLocalPlayerId() const
{
    return localPlayerId;
}

void ActionsSingleComponent::CollectDigitalAction(const FastName& actionId, eInputElements element, uint32 deviceId, DigitalElementState state)
{
    collectedDigitalActions.emplace_back();

    DigitalAction& action = collectedDigitalActions.back();
    action.digitalBinding.actionId = actionId;
    action.digitalBinding.digitalElements[0] = element;
    action.digitalBinding.digitalStates[0] = state;

    action.deviceId = deviceId;
}

void ActionsSingleComponent::CollectAnalogAction(const FastName& actionId, AnalogBinding::eAnalogStateType type, AnalogPrecision precision, eInputElements analogElement, eInputElements digitalModifier, uint32 deviceId)
{
    collectedAnalogActions.emplace_back();

    AnalogAction& action = collectedAnalogActions.back();
    action.analogBinding.actionId = actionId;
    action.analogBinding.analogElementId = analogElement;
    action.analogBinding.analogStateType = type;
    if (digitalModifier != eInputElements::NONE)
    {
        action.analogBinding.digitalElements[0] = digitalModifier;
        action.analogBinding.digitalStates[0] = DigitalElementState::Pressed();
    }

    action.analogPrecision = precision;
    action.deviceId = deviceId;
}

template <typename T>
Vector2 ConvertAnalogToFixedPrecision(AnalogPrecision precision, Vector2 value)
{
    int32 precisionInt = static_cast<int32>(precision);
    float32 halfRange = static_cast<float32>((1 << precisionInt) - 1) / 2.f;
    return Vector2(static_cast<float32>(static_cast<T>((value.x + 1.f) * halfRange)),
                   static_cast<float32>(static_cast<T>((value.y + 1.f) * halfRange)));
}

Vector2 ConvertFixedPrecisionToAnalog(AnalogPrecision precision, Vector2 value)
{
    int32 precisionInt = static_cast<int32>(precision);
    float32 halfRange = static_cast<float32>((1 << precisionInt) - 1) / 2.f;
    float32 x = Clamp((value.x - halfRange) / halfRange, -1.f, 1.f);
    float32 y = Clamp((value.y - halfRange) / halfRange, -1.f, 1.f);
    return Vector2(x, y);
}

uint64 GetAnalogPrecisionMask(AnalogPrecision precision)
{
    switch (precision)
    {
    case AnalogPrecision::ANALOG_UINT8:
        return 0xFF;
    case AnalogPrecision::ANALOG_UINT16:
        return 0xFFFF;
    case AnalogPrecision::ANALOG_UINT32:
        return 0xFFFFFFFF;
    case AnalogPrecision::ANALOG_FLOAT32:
        return 0xFFFFFFFF;
    }
    DVASSERT(false && "Unknown precision");
    return 0;
}
}
