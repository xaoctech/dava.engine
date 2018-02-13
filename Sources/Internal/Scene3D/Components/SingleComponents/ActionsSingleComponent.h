#pragma once

#include "Base/FastName.h"
#include "Base/Vector.h"
#include "Base/UnordererSet.h"
#include "Base/UnordererMap.h"
#include "Input/ActionSystem.h"
#include "Math/Vector.h"
#include "Math/Quaternion.h"
#include "Reflection/Reflection.h"
#include "Entity/SingletonComponent.h"

namespace DAVA
{
enum class AnalogPrecision
{
    ANALOG_UINT8 = 8,
    ANALOG_UINT16 = 16,
    ANALOG_UINT32 = 32,
    ANALOG_FLOAT32 = 0
};

class ActionsSingleComponent : public SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(ActionsSingleComponent, SingletonComponent);

public:
    static const int32 MAX_DIGITAL_SIZE = 64;
    static const int32 SIMULTANEOUS_DIGITAL_SIZE = 9;
    static const int32 MAX_ANALOG_SIZE = 64; // bits

    struct AnalogActionInfo
    {
        FastName actionId;
        AnalogPrecision precision;

        bool operator==(const AnalogActionInfo& info) const;
    };

    struct AnalogActionInfoHasher
    {
        size_t operator()(const ActionsSingleComponent::AnalogActionInfo& info) const
        {
            return std::hash<FastName>()(info.actionId);
        }
    };

    struct Actions
    {
        Vector<FastName> digitalActions;
        UnorderedMap<AnalogActionInfo, Vector2, AnalogActionInfoHasher> analogActions;
        Quaternion cameraDelta;
        uint32 clientFrameId = 0;
    };

    ActionsSingleComponent();

    void AddDigitalAction(const FastName& actionId, uint8 playerId = 0);
    void AddAnalogAction(const FastName& actionId, const Vector2& state, uint8 playerId = 0);

    void AddAvailableDigitalAction(const FastName& actionId);
    const Vector<FastName>& GetAvailableDigitalActions() const;
    bool IsAvaildableDigitalAction(const FastName& actionId) const;

    void AddAvailableAnalogAction(const FastName& actionId, AnalogPrecision precision);
    const Vector<AnalogActionInfo>& GetAvailableAnalogActions() const;
    bool IsAvaildableAnalogAction(const FastName& actionId) const;

    void SetCameraDelta(const Quaternion& cameraDelta_, uint8 playerId = 0);

    void AddActions(uint8 playerId, Actions&& actions);
    Vector<Actions>& GetActions(uint8 playerId = 0);

    int32 GetDigitalActionNumericId(const FastName& actionId) const;
    const FastName& GetDigitalActionId(int32 actionNumericId) const;

    void Clear() override;

    void SetLocalPlayerId(int32 playerId);
    int32 GetLocalPlayerId() const;

    /** Add new digital action. */
    void CollectDigitalAction(const FastName& actionId, eInputElements element, uint32 deviceId, DigitalElementState state = DigitalElementState::Pressed());

    /** Add new analog action. */
    void CollectAnalogAction(const FastName& actionId, AnalogBinding::eAnalogStateType type, AnalogPrecision precision,
                             eInputElements analogElement, eInputElements digitalModifier, uint32 deviceId);

    struct DigitalAction
    {
        DigitalBinding digitalBinding;
        uint32 deviceId;
    };

    struct AnalogAction
    {
        AnalogBinding analogBinding;
        AnalogPrecision analogPrecision;
        uint32 deviceId;
    };

    Vector<DigitalAction> collectedDigitalActions;
    Vector<AnalogAction> collectedAnalogActions;

private:
    UnorderedMap<uint8, Vector<Actions>> playerIdsToActions;

    Vector<FastName> availableDigitalActions;
    UnorderedSet<FastName> digitalIndex;

    Vector<AnalogActionInfo> availableAnalogActions;
    UnorderedSet<FastName> analogIndex;

    int32 localPlayerId = -1;
};
using AnalogActionsMap = UnorderedMap<ActionsSingleComponent::AnalogActionInfo, Vector2,
                                      ActionsSingleComponent::AnalogActionInfoHasher>;

template <typename T>
Vector2 ConvertAnalogToFixedPrecision(AnalogPrecision precision, Vector2 value);
Vector2 ConvertFixedPrecisionToAnalog(AnalogPrecision precision, Vector2 value);

uint64 GetAnalogPrecisionMask(AnalogPrecision precision);
}
