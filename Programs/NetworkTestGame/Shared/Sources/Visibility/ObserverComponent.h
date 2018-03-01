#pragma once

#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Scene3D/Entity.h"
#include "ObservableIdComponent.h"
#include "ObservableComponent.h"

class ObserverComponent : public DAVA::Component
{
public:
    DAVA_VIRTUAL_REFLECTION(ObserverComponent, DAVA::Component);

    ObserverComponent();
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    void SetVisible(ObservableId observableId, bool isVisible);
    bool IsVisible(ObservableId observableId) const;
    const DAVA::Vector<ObservableId>& GetUpdatedObservablesIds() const;
    void ClearCache();
    void Reset();

    DAVA::float32 maxVisibilityRadius = 1e6f;
    DAVA::float32 unconditionalVisibilityRadius = 0.f;

private:
    DAVA::Vector<DAVA::uint8> visibilityByObservableId;
    DAVA::Vector<ObservableId> updatedObservablesIds;
};

inline void ObserverComponent::SetVisible(ObservableId observableId, bool isVisible)
{
    DAVA::uint8 isVisbleU8 = static_cast<DAVA::uint8>(isVisible);
    if (visibilityByObservableId[observableId] != isVisbleU8)
    {
        visibilityByObservableId[observableId] = isVisbleU8;
        updatedObservablesIds.push_back(observableId);
    }
}

inline bool ObserverComponent::IsVisible(ObservableId observableId) const
{
    return static_cast<bool>(visibilityByObservableId[observableId]);
}

inline const DAVA::Vector<ObservableId>& ObserverComponent::GetUpdatedObservablesIds() const
{
    return updatedObservablesIds;
}
