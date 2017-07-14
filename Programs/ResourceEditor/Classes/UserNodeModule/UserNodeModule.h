#pragma once

#include <TArc/Core/ClientModule.h>
#include <Reflection/Reflection.h>

#include <Base/ScopedPtr.h>

namespace DAVA
{
class RenderObject;
}

class UserNodeModule : public DAVA::TArc::ClientModule
{
protected:
    void PostInit() override;
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

private:
    DAVA::ScopedPtr<DAVA::RenderObject> spawnObject;

    DAVA_VIRTUAL_REFLECTION(UserNodeModule, DAVA::TArc::ClientModule);
};
