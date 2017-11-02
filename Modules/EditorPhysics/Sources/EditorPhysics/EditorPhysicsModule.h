#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>

class EditorPhysicsModule : public DAVA::ClientModule
{
public:
    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextDeleted(DAVA::DataContext* context) override;

    void PostInit() override;

private:
    DAVA_VIRTUAL_REFLECTION(EditorPhysicsModule, DAVA::ClientModule);
};