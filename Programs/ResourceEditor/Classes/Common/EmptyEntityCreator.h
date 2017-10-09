#pragma once

#include "EntityTypeCreator.h"

class EmptyEntityCreator : public EntityTypeCreator
{
public:
    DAVA::Entity* CreateEntity() override;
    bool IsComponentsMatched(DAVA::Entity* entity) const override;
    QString GetEntityTypeName() const override;
    QIcon GetIcon() const override;

private:
    DAVA_VIRTUAL_REFLECTION(EmptyEntityCreator, EntityTypeCreator);
};