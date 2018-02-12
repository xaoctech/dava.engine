#include "Classes/LightingPresetsModule/LightingPresetsModule.h"

#include "REPlatform/DataNodes/SceneData.h"
#include "REPlatform/Scene/Systems/LightingPresetsSystem.h"

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <TArc/Controls/Label.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/ModuleCollection.h>

#include <FileSystem/FilePath.h>

#include <QAction>

void LightingPresetsModule::PostInit()
{
    DAVA::ContextAccessor* accessor = GetAccessor();
    DAVA::UI* ui = GetUI();

    QWidget* widget = new QWidget();
    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    DAVA::QtHBoxLayout* layout = new DAVA::QtHBoxLayout(widget);
    layout->setMargin(0);
    layout->setMargin(4);
    {
        DAVA::Label::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[DAVA::Label::Fields::Text] = "descrText";
        layout->AddControl(new DAVA::Label(params, accessor, model, widget));
    }
    {
        DAVA::ReflectedButton::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[DAVA::ReflectedButton::Fields::Icon] = "importIcon";
        params.fields[DAVA::ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[DAVA::ReflectedButton::Fields::Tooltip] = "importToolTip";
        params.fields[DAVA::ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[DAVA::ReflectedButton::Fields::Clicked] = "ImportLightingPreset";
        layout->AddControl(new DAVA::ReflectedButton(params, accessor, model, widget));
    }
    {
        DAVA::ReflectedButton::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[DAVA::ReflectedButton::Fields::Icon] = "exportIcon";
        params.fields[DAVA::ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[DAVA::ReflectedButton::Fields::Tooltip] = "exportToolTip";
        params.fields[DAVA::ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[DAVA::ReflectedButton::Fields::Clicked] = "ExportLightingPreset";
        layout->AddControl(new DAVA::ReflectedButton(params, accessor, model, widget));
    }

    QString toolbarName = "LightingPresetsToolbar";
    DAVA::ActionPlacementInfo toolbarTogglePlacement(DAVA::CreateMenuPoint(QList<QString>() << "View"
                                                                                            << "Toolbars"));
    ui->DeclareToolbar(DAVA::mainWindowKey, toolbarTogglePlacement, toolbarName);

    QAction* action = new QAction(nullptr);
    DAVA::AttachWidgetToAction(action, widget);

    DAVA::ActionPlacementInfo placementInfo(DAVA::CreateToolbarPoint(toolbarName));
    ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
}

void LightingPresetsModule::ImportLightingPreset()
{
    DAVA::FileDialogParams params;
    params.title = "Import lighting preset";
    params.filters = "Lighting preset (*.iblp)";
    QString fileName = GetUI()->GetOpenFileName(DAVA::mainWindowKey, params);
    if (fileName.isEmpty())
        return;
    DAVA::ContextAccessor* acessor = GetAccessor();
    DAVA::DataContext* ctx = acessor->GetActiveContext();

    if (ctx == nullptr)
        return;

    DAVA::SceneData::TSceneType scene = ctx->GetData<DAVA::SceneData>()->GetScene();
    DAVA::LightingPresetsSystem* lightPresets = scene->GetSystem<DAVA::LightingPresetsSystem>();
    if (lightPresets != nullptr)
    {
        lightPresets->LoadPreset(DAVA::FilePath(fileName.toStdWString()));
    }
}

void LightingPresetsModule::ExportLightingPreset()
{
    DAVA::FileDialogParams params;
    params.title = "Export lighting preset";
    params.filters = "Lighting preset (*.iblp)";
    QString fileName = GetUI()->GetSaveFileName(DAVA::mainWindowKey, params);
    if (fileName.isEmpty())
        return;
    DAVA::ContextAccessor* acessor = GetAccessor();
    DAVA::DataContext* ctx = acessor->GetActiveContext();

    if (ctx == nullptr)
        return;

    DAVA::SceneData::TSceneType scene = ctx->GetData<DAVA::SceneData>()->GetScene();
    DAVA::LightingPresetsSystem* lightPresets = scene->GetSystem<DAVA::LightingPresetsSystem>();
    if (lightPresets != nullptr)
        lightPresets->SavePreset(DAVA::FilePath(fileName.toStdWString()));
}

DAVA_VIRTUAL_REFLECTION_IMPL(LightingPresetsModule)
{
    DAVA::ReflectionRegistrator<LightingPresetsModule>::Begin()
    .ConstructorByPointer()
    .Field("iconSize", [](LightingPresetsModule*) { return QSize(16, 16); }, nullptr)
    .Field("autoRaise", [](LightingPresetsModule*) { return false; }, nullptr)
    .Field("importIcon", [](LightingPresetsModule*) { return QIcon(":/TArc/Resources/import.png"); }, nullptr)
    .Field("exportIcon", [](LightingPresetsModule*) { return QIcon(":/TArc/Resources/export.png"); }, nullptr)
    .Field("descrText", [](LightingPresetsModule*) { return "Lighting presets:"; }, nullptr)
    .Field("importToolTip", [](LightingPresetsModule*) { return "Import lighting settings"; }, nullptr)
    .Field("exportToolTip", [](LightingPresetsModule*) { return "Export lighting settings from current scene"; }, nullptr)
    .Method("ImportLightingPreset", &LightingPresetsModule::ImportLightingPreset)
    .Method("ExportLightingPreset", &LightingPresetsModule::ExportLightingPreset)
    .End();
}

DECL_TARC_MODULE(LightingPresetsModule);
