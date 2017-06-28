#include "Modules/SpineModule/SpineControlModule.h"

#include <Base/BaseTypes.h>
#include <Engine/EngineContext.h>
#include <UI/UIControlSystem.h>

#include <UI/Spine/UISpineSystem.h>

#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/DataNode.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QAction>
#include <QWidget>

class SpineControlData : public DAVA::TArc::DataNode
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SpineControlData, DAVA::TArc::DataNode)
    {
    }

public:
    bool isPauseOn = false;
};

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
    accessor->GetGlobalContext()->CreateData(std::make_unique<SpineControlData>());

    QWidget* w = new QWidget();
    QtHBoxLayout* layout = new QtHBoxLayout(w);
    layout->setMargin(0);
    layout->setSpacing(4);

    UI* ui = GetUI();

    {
        CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[CheckBox::Fields::IsEnabled] = "isEnabled";
        params.fields[CheckBox::Fields::Checked] = "isPauseProperty";
        params.fields[CheckBox::Fields::TextHint] = "pausePropertyTitle";
        layout->AddControl(new CheckBox(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(this)), w));
    }

    {
        ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[ReflectedButton::Fields::Enabled] = "isEnabled";
        params.fields[ReflectedButton::Fields::Clicked] = "rebuildAllBoneLinks";
        params.fields[ReflectedButton::Fields::Text] = "rebuildButtonTitle";
        layout->AddControl(new ReflectedButton(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(this)), w));
    }

    QString toolbarName = "SpineControlToolbar";
    ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                << "Toolbars"));
    ui->DeclareToolbar(DAVA::TArc::mainWindowKey, toolbarTogglePlacement, toolbarName);

    QAction* action = new QAction(nullptr);
    AttachWidgetToAction(action, w);

    ActionPlacementInfo placementInfo(CreateToolbarPoint(toolbarName));
    ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
}

bool SpineControlModule::GetSystemPauseState() const
{
    return GetAccessor()->GetGlobalContext()->GetData<SpineControlData>()->isPauseOn;
}

void SpineControlModule::SetSystemPauseState(bool pause)
{
    GetAccessor()->GetGlobalContext()->GetData<SpineControlData>()->isPauseOn = pause;
    UpdateSceneSystem();
}

void SpineControlModule::RebuildAllBoneLinks()
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    SpineControlData* data = accessor->GetGlobalContext()->GetData<SpineControlData>();
    UISpineSystem* spineSystem = accessor->GetEngineContext()->uiControlSystem->GetSystem<UISpineSystem>();
    if (spineSystem != nullptr)
    {
        spineSystem->RebuildAllBoneLinks();
    }
}

bool SpineControlModule::IsEnabled() const
{
    return GetAccessor()->GetContextCount() != 0;
}

void SpineControlModule::UpdateSceneSystem()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    SpineControlData* data = accessor->GetGlobalContext()->GetData<SpineControlData>();
    DAVA::UISpineSystem* spineSystem = accessor->GetEngineContext()->uiControlSystem->GetSystem<DAVA::UISpineSystem>();
    if (spineSystem != nullptr)
    {
        spineSystem->SetPause(data->isPauseOn);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(SpineControlModule)
{
    DAVA::ReflectionRegistrator<SpineControlModule>::Begin()
    .ConstructorByPointer()
    .Field("isPauseProperty", &SpineControlModule::GetSystemPauseState, &SpineControlModule::SetSystemPauseState)
    .Field("isEnabled", &SpineControlModule::IsEnabled, nullptr)
    .Field("pausePropertyTitle", &SpineControlModule::pausePropertyTitle)
    .Field("rebuildButtonTitle", &SpineControlModule::rebuildButtonTitle)
    .Method("rebuildAllBoneLinks", &SpineControlModule::RebuildAllBoneLinks)
    .End();
}

DECL_GUI_MODULE(SpineControlModule);
