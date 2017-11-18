#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

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
    void CreateCarEntity();
    void CreateTankEntity();

private:
    DAVA::TArc::QtConnections connections;
    DAVA_VIRTUAL_REFLECTION(EditorPhysicsModule, DAVA::TArc::ClientModule);
};