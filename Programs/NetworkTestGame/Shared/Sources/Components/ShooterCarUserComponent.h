#pragma once

#include <Base/BaseTypes.h>
#include <Entity/Component.h>
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class Entity;
}

// Represents information about car driver/passenger
// (i.e. is he inside a car and is he a driver or a passenger)
class ShooterCarUserComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCarUserComponent, DAVA::Component);
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::NetworkID GetCarNetworkId() const;
    DAVA::uint32 GetPassengerIndex() const;

private:
    friend class ShooterCarSystem; // For setting carNetworkId and passengerIndex

    // Id of the car player is inside of (NId2Invalid = outside)
    DAVA::NetworkID carNetworkId;

    // Passenger index, 0 = driver
    DAVA::uint32 passengerIndex = 0;
};
