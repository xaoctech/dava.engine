#pragma once

#include <TArc/Core/ContextAccessor.h>
#include <Scene3D/Systems/SlotSystem.h>

namespace DAVA
{
class Entity;
class FilePath;
}

class EntityForSlotLoader : public DAVA::SlotSystem::ExternalEntityLoader
{
public:
    EntityForSlotLoader(DAVA::TArc::ContextAccessor* accessor);

    DAVA::Entity* Load(const DAVA::FilePath& path) override;
    void AddEntity(DAVA::Entity* parent, DAVA::Entity* child) override;
    void Process(DAVA::float32 delta) override
    {
    }

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
