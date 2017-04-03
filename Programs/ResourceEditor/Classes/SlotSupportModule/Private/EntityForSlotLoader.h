#pragma once

#include <Scene3D/Systems/SlotSystem.h>

namespace DAVA
{
class Entity;
class FilePath;
}

class EntityForSlotLoader : public DAVA::SlotSystem::ExternalEntityLoader
{
public:
    DAVA::Entity* Load(const DAVA::FilePath& path) override;
    void AddEntity(DAVA::Entity* parent, DAVA::Entity* child) override;
};