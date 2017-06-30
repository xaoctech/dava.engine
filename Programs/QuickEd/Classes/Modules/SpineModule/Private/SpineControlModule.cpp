#include "Modules/SpineModule/SpineControlModule.h"

#include <Engine/EngineContext.h>
#include <UI/UIControlSystem.h>

#include <UI/Spine/UISpineSystem.h>

#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <QAction>
#include <QWidget>

void SpineControlModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
}

void SpineControlModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void SpineControlModule::PostInit()
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    QWidget* w = new QWidget();
    QtHBoxLayout* layout = new QtHBoxLayout(w);
    layout->setMargin(0);
    layout->setSpacing(4);

    {
        ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[ReflectedButton::Fields::Enabled] = "isEnabled";
        params.fields[ReflectedButton::Fields::Clicked] = "playPause";
        params.fields[ReflectedButton::Fields::Tooltip] = "pauseButtonHint";
        params.fields[ReflectedButton::Fields::Icon] = "pauseButtonIcon";
        layout->AddControl(new ReflectedButton(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(this)), w));
    }

    {
        ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[ReflectedButton::Fields::Enabled] = "isEnabled";
        params.fields[ReflectedButton::Fields::Clicked] = "rebuildAllBoneLinks";
        params.fields[ReflectedButton::Fields::Tooltip] = "rebuildButtonHint";
        params.fields[ReflectedButton::Fields::Icon] = "rebuildButtonIcon";
        layout->AddControl(new ReflectedButton(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(this)), w));
    }

    QString toolbarName = "Spine Toolbar";
    ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                << "Toolbars"));
    ui->DeclareToolbar(DAVA::TArc::mainWindowKey, toolbarTogglePlacement, toolbarName);

    QAction* action = new QAction(nullptr);
    AttachWidgetToAction(action, w);

    ActionPlacementInfo placementInfo(CreateToolbarPoint(toolbarName));
    ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

const QIcon& SpineControlModule::GetRebuildButtonIcon()
{
    return SharedIcon(":/Icons/reload.png");
}

const QIcon& SpineControlModule::GetPauseButtonIcon()
{
    DAVA::UISpineSystem* spineSystem = GetSpineSystem();
    if (spineSystem != nullptr && !spineSystem->IsPause())
    {
        return SharedIcon(":/Icons/pause.png");
    }
    else
    {
        return SharedIcon(":/Icons/play.png");
    }
}

void SpineControlModule::RebuildAllBoneLinks()
{
    DAVA::UISpineSystem* spineSystem = GetSpineSystem();
    if (spineSystem != nullptr)
    {
        spineSystem->RebuildAllBoneLinks();
    }
}

void SpineControlModule::PlayPause()
{
    DAVA::UISpineSystem* spineSystem = GetSpineSystem();
    if (spineSystem != nullptr)
    {
        spineSystem->SetPause(!spineSystem->IsPause());
    }
}

bool SpineControlModule::IsEnabled() const
{
    return GetSpineSystem() != nullptr;
}

DAVA::UISpineSystem* SpineControlModule::GetSpineSystem() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    const ContextAccessor* accessor = GetAccessor();
    return accessor != nullptr ? accessor->GetEngineContext()->uiControlSystem->GetSystem<UISpineSystem>() : nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(SpineControlModule)
{
    DAVA::ReflectionRegistrator<SpineControlModule>::Begin()
    .ConstructorByPointer()
    .Field("isEnabled", &SpineControlModule::IsEnabled, nullptr)
    .Field("pauseButtonHint", &SpineControlModule::pauseButtonHint)
    .Field("rebuildButtonHint", &SpineControlModule::rebuildButtonHint)
    .Field("pauseButtonIcon", &SpineControlModule::GetPauseButtonIcon, nullptr)
    .Field("rebuildButtonIcon", &SpineControlModule::GetRebuildButtonIcon, nullptr)
    .Method("rebuildAllBoneLinks", &SpineControlModule::RebuildAllBoneLinks)
    .Method("playPause", &SpineControlModule::PlayPause)
    .End();
}

DECL_GUI_MODULE(SpineControlModule);
