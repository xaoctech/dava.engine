#pragma once

#include <TArc/Controls/PropertyPanel/PropertiesView.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/QtConnections.h>

namespace DAVA
{
class NMaterial;
}

class MaterialEditorModule final : public DAVA::ClientModule
{
public:
    void PostInit() override;

private:
    void OnSelectionChanged(const DAVA::Any& selection);
    std::unique_ptr<DAVA::FieldBinder> librarySelectionBinder;
    std::shared_ptr<DAVA::PropertiesView::Updater> updater;

    DAVA_VIRTUAL_REFLECTION(PropertyPanelModule, DAVA::ClientModule);
};
