#pragma once

#include "Classes/Common/EntityTypeCreator.h"

#include <Reflection/Reflection.h>

class CameraEntityCreator : public EntityTypeCreator
{
public:
    DAVA::Entity* CreateEntity() override;
    bool IsComponentsMatched(DAVA::Entity* entity) const override;
    QString GetEntityTypeName() const override;
    QIcon GetIcon() const override;

private:
    DAVA_VIRTUAL_REFLECTION(CameraEntityCreator, EntityTypeCreator);
};