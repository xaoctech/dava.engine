#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>

class EditorPhysicsModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void PostInit() override;

    void OnInterfaceRegistered(const DAVA::Type* interfaceType);
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType);

private:
    DAVA_VIRTUAL_REFLECTION(EditorPhysicsModule, DAVA::TArc::ClientModule);
};