#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>
#include <Base/ScopedPtr.h>

namespace DAVA
{
class RenderObject;
class FilePath;
}

class UserNodeModule : public DAVA::TArc::ClientModule
{
public:
    static DAVA::FilePath GetBotSpawnPath();

protected:
    void PostInit() override;
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

private:
    void ChangeDrawingState();

    DAVA::ScopedPtr<DAVA::RenderObject> spawnObject;
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(UserNodeModule, DAVA::TArc::ClientModule);
};
