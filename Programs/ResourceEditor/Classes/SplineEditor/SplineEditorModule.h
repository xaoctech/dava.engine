#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class SplineEditorModule : public DAVA::ClientModule
{
private:
    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextDeleted(DAVA::DataContext* context) override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void PostInit() override;

    void OnSelectionChanged(const DAVA::Any& selection);

private:
    DAVA::QtConnections connections;
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;

    DAVA_VIRTUAL_REFLECTION(SplineEditorModule, DAVA::ClientModule);
};