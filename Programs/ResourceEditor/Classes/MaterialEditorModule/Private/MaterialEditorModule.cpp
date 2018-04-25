#include "Classes/MaterialEditorModule/MaterialEditorModule.h"
#include "Classes/Library/Private/LibraryData.h"

#include <TArc/Controls/PropertyPanel/SimpleModifyExtension.h>
#include <TArc/Controls/PropertyPanel/TimerUpdater.h>
#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <Reflection/Reflection.h>
#include <Render/Material/Material.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialReflection.h>

namespace MaterialEditorModuleDetail
{
using namespace DAVA;

class MaterialEditorData : public TArcDataNode
{
public:
    Reflection materialRefl;
    Asset<Material> currentMaterial;

    UnorderedSet<Asset<Material>> materials;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MaterialEditorData, TArcDataNode)
    {
        ReflectionRegistrator<MaterialEditorData>::Begin()
        .Field("material", &MaterialEditorData::materialRefl)
        .End();
    }
};

class MaterialNodeModifyExtension : public SimpleModifyExtension
{
public:
    MaterialNodeModifyExtension(std::weak_ptr<PropertiesView::Updater> updater, ContextAccessor* accessor)
        : SimpleModifyExtension(updater)
        , accessor(accessor)
    {
    }

    void ProduceCommand(const Reflection::Field& object, const Any& newValue) override
    {
        SimpleModifyExtension::ProduceCommand(object, newValue);

        DataContext* ctx = accessor->GetGlobalContext();
        MaterialEditorModuleDetail::MaterialEditorData* data = ctx->GetData<MaterialEditorData>();
        Asset<Material> asset = data->currentMaterial;
        if (GetEngineContext()->assetManager->SaveAsset(asset) == false)
        {
            Logger::Error("Failed to save material asset");
        }
    }

private:
    ContextAccessor* accessor;
};
} // namespace MaterialEditorModuleDetail

MaterialEditorModule::~MaterialEditorModule()
{
    GetAccessor()->GetEngineContext()->assetManager->UnregisterListener(&listener);
}

void MaterialEditorModule::PostInit()
{
    using namespace DAVA;
    using namespace MaterialEditorModuleDetail;

    updater.reset(new TimerUpdater(2500, TimerUpdater::DisableFastUpdate));

    UI* ui = GetUI();
    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    ctx->CreateData(std::make_unique<MaterialEditorData>());

    PropertiesView::Params params(DAVA::mainWindowKey);
    params.accessor = accessor;
    params.invoker = GetInvoker();
    params.wndKey = DAVA::mainWindowKey;
    params.isInDevMode = false;
    params.showToolBar = false;
    params.ui = ui;
    params.settingsNodeName = "MaterialEditor";
    params.updater = updater;
    params.objectsField.type = ReflectedTypeDB::Get<MaterialEditorModuleDetail::MaterialEditorData>();
    params.objectsField.fieldName = FastName("material");
    PropertiesView* view = new PropertiesView(params);
    using ModifyExt = MaterialEditorModuleDetail::MaterialNodeModifyExtension;
    view->RegisterExtension(std::make_shared<ModifyExt>(std::weak_ptr<DAVA::PropertiesView::Updater>(updater), accessor));

    FieldDescriptor librarySelection;
    librarySelection.fieldName = FastName(LibraryData::selectedPathProperty);
    librarySelection.type = ReflectedTypeDB::Get<LibraryData>();
    librarySelectionBinder.reset(new FieldBinder(accessor));
    librarySelectionBinder->BindField(librarySelection, MakeFunction(this, &MaterialEditorModule::OnSelectionChanged));

    DockPanelInfo info;
    info.title = QStringLiteral("Material editor");
    info.area = Qt::LeftDockWidgetArea;
    PanelKey panelKey(QStringLiteral("MaterialEditorDock"), info);

    ui->AddView(DAVA::mainWindowKey, panelKey, view);

    listener.onReloaded = MakeFunction(this, &MaterialEditorModule::OnAssetReloaded);
    listener.onLoaded = MakeFunction(this, &MaterialEditorModule::OnAssetLoaded);
    GetAccessor()->GetEngineContext()->assetManager->RegisterListener(&listener, DAVA::Type::Instance<DAVA::Material>());
}

void MaterialEditorModule::OnSelectionChanged(const DAVA::Any& selection)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    MaterialEditorModuleDetail::MaterialEditorData* data = ctx->GetData<MaterialEditorModuleDetail::MaterialEditorData>();
    data->currentMaterial.reset();
    data->materialRefl = DAVA::Reflection();

    FilePath path = selection.Cast<FilePath>(FilePath());
    if (path.IsEqualToExtension(".mat") == false)
    {
        return;
    }

    Material::PathKey key(path);
    Asset<Material> materialAsset = GetEngineContext()->assetManager->GetAsset<Material>(key, AssetManager::SYNC);
    DVASSERT(materialAsset != nullptr);

    data->currentMaterial = materialAsset;
    data->materialRefl = CreateNMaterialReflection(data->currentMaterial->GetMaterial());
}

void MaterialEditorModule::OnAssetLoaded(const DAVA::Asset<DAVA::AssetBase>& asset)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    MaterialEditorModuleDetail::MaterialEditorData* data = ctx->GetData<MaterialEditorModuleDetail::MaterialEditorData>();

    Asset<Material> loadedMaterial = std::static_pointer_cast<Material>(asset);
    data->materials.insert(loadedMaterial);
}

void MaterialEditorModule::OnAssetReloaded(const DAVA::Asset<DAVA::AssetBase>& original, const DAVA::Asset<DAVA::AssetBase>& reloaded)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* ctx = accessor->GetGlobalContext();
    MaterialEditorModuleDetail::MaterialEditorData* data = ctx->GetData<MaterialEditorModuleDetail::MaterialEditorData>();

    Asset<Material> reloadedMaterial = std::static_pointer_cast<Material>(reloaded);
    data->materials.erase(std::static_pointer_cast<Material>(original));
    data->materials.insert(reloadedMaterial);

    if (data->currentMaterial == original)
    {
        data->currentMaterial = reloadedMaterial;
        data->materialRefl = CreateNMaterialReflection(data->currentMaterial->GetMaterial());
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialEditorModule)
{
    DAVA::ReflectionRegistrator<MaterialEditorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(MaterialEditorModule);
