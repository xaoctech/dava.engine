#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>

class EditorSlotSystem : public DAVA::SceneSystem,
                         public EditorSceneSystem
{
public:
    EditorSlotSystem(DAVA::Scene* scene);

    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;

    void RegisterEntity(DAVA::Entity* entity) override;
    void UnregisterEntity(DAVA::Entity* entity) override;

    void RegisterComponent(DAVA::Entity* entity, DAVA::Component* component);
    void UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component);

    void Process(DAVA::float32 timeElapsed) override;

protected:
    friend class DetachEntityFromSlot;
    friend class AttachEntityToSlot;

    void DetachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity);
    void AttachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity);
    DAVA::Entity* AttachEntity(DAVA::SlotComponent* component, const DAVA::FastName& itemName);

protected:
    std::unique_ptr<DAVA::Command> PrepareForSave(bool saveForGame) override;

private:
    DAVA::Vector<DAVA::Entity*> entities;
    DAVA::Set<DAVA::Entity*> pendingOnInitialize;
};
