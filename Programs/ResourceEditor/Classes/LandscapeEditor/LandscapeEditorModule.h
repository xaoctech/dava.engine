#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>

#include <Reflection/Reflection.h>

class LandscapeEditorModule : public DAVA::ClientModule
{
public:
    LandscapeEditorModule();

protected:
    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextDeleted(DAVA::DataContext* context) override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void PostInit() override;

private:
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;
    DAVA_VIRTUAL_REFLECTION(LandscapeEditorModule, DAVA::ClientModule);
};