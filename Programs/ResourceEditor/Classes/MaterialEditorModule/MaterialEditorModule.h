#pragma once

#include <TArc/Controls/PropertyPanel/PropertiesView.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/QtConnections.h>
#include <Asset/AssetListener.h>

namespace DAVA
{
class NMaterial;
}

class MaterialEditorModule final : public DAVA::ClientModule
{
public:
    ~MaterialEditorModule();

protected:
    void PostInit() override;

private:
    void OnAssetLoaded(const DAVA::Asset<DAVA::AssetBase>& asset);
    void OnAssetReloaded(const DAVA::Asset<DAVA::AssetBase>& original, const DAVA::Asset<DAVA::AssetBase>& reloaded);
    void OnSelectionChanged(const DAVA::Any& selection);
    std::unique_ptr<DAVA::FieldBinder> librarySelectionBinder;
    std::shared_ptr<DAVA::PropertiesView::Updater> updater;

    DAVA::SimpleAssetListener listener;

    DAVA_VIRTUAL_REFLECTION(PropertyPanelModule, DAVA::ClientModule);
};
