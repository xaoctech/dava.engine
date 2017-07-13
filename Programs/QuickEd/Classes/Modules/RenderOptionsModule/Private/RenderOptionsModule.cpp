#include "Modules/RenderOptionsModule/RenderOptionsModule.h"
#include "Modules/RenderOptionsModule/Private/OptionWrapper.h"

#include <Engine/EngineContext.h>
#include <Render/Renderer.h>
#include <Render/RenderOptions.h>

#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <QAction>
#include <QDialog>
#include <QScrollArea>
#include <QWidget>

void RenderOptionsModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
}

void RenderOptionsModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void RenderOptionsModule::PostInit()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    // Toolbar

    {
        QWidget* w = new QWidget();
        QtHBoxLayout* layout = new QtHBoxLayout(w);
        layout->setMargin(0);
        layout->setSpacing(4);

        {
            ReflectedButton::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[ReflectedButton::Fields::Enabled] = "isEnabled";
            params.fields[ReflectedButton::Fields::Clicked] = "showRenderOptionsDialog";
            params.fields[ReflectedButton::Fields::Tooltip] = "toolbarButtonHint";
            params.fields[ReflectedButton::Fields::Icon] = "toolbarButtonIcon";
            layout->AddControl(new ReflectedButton(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(this)), w));
        }

        QString toolbarName = "Render Options Toolbar";
        ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                    << "Toolbars"));
        ui->DeclareToolbar(DAVA::TArc::mainWindowKey, toolbarTogglePlacement, toolbarName);

        QAction* action = new QAction(nullptr);
        AttachWidgetToAction(action, w);

        ActionPlacementInfo placementInfo(CreateToolbarPoint(toolbarName));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }

    // Dialog

    {
        for (uint32 option = 0; option < RenderOptions::OPTIONS_COUNT; ++option)
        {
            optionsRefs.emplace_back(std::make_shared<OptionWrapper>(static_cast<RenderOptions::RenderOption>(option)));
        }

        optionsDialog = std::make_unique<QDialog>();
        optionsDialog->setWindowTitle("Render Options");
        optionsDialog->resize(300, 400);

        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(new QWidget());

        QVBoxLayout* layout = new QtVBoxLayout(optionsDialog.get());
        layout->setMargin(5);
        layout->addWidget(scrollArea);

        QtVBoxLayout* scrollLayout = new QtVBoxLayout(scrollArea->widget());
        scrollLayout->setSpacing(5);

        for (std::shared_ptr<OptionWrapper>& option : optionsRefs)
        {
            CheckBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "enabled";
            params.fields[CheckBox::Fields::TextHint] = "title";
            scrollLayout->AddControl(new CheckBox(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(option.get())), optionsDialog.get()));
        }
    }
}

const QIcon& RenderOptionsModule::GetToolbarButtonIcon() const
{
    return SharedIcon(":/Icons/configure.png");
}

const DAVA::String& RenderOptionsModule::GetToolbarButtonHint() const
{
    static const DAVA::String hint = "Show Render Options dialog";
    return hint;
}

void RenderOptionsModule::ShowRenderOptionsDialog()
{
    if (optionsDialog)
    {
        optionsDialog->exec();
    }
}

bool RenderOptionsModule::IsEnabled() const
{
    return DAVA::Renderer::IsInitialized();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RenderOptionsModule)
{
    DAVA::ReflectionRegistrator<RenderOptionsModule>::Begin()
    .ConstructorByPointer()
    .Field("isEnabled", &RenderOptionsModule::IsEnabled, nullptr)
    .Field("toolbarButtonHint", &RenderOptionsModule::GetToolbarButtonHint, nullptr)
    .Field("toolbarButtonIcon", &RenderOptionsModule::GetToolbarButtonIcon, nullptr)
    .Method("showRenderOptionsDialog", &RenderOptionsModule::ShowRenderOptionsDialog)
    .End();
}

DECL_GUI_MODULE(RenderOptionsModule);
