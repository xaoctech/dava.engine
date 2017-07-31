#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>

class EditorPhysicsModule : public DAVA::TArc::ClientModule
{
public:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void PostInit() override;

private:
    DAVA_VIRTUAL_REFLECTION(EditorPhysicsModule, DAVA::TArc::ClientModule);
};