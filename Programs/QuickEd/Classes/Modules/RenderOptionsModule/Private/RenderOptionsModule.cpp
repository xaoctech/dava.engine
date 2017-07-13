#include "Modules/RenderOptionsModule/RenderOptionsModule.h"
#include "Modules/RenderOptionsModule/Private/OptionWrapper.h"
#include "Modules/RenderOptionsModule/Private/RenderOptionsData.h"

#include <Engine/EngineContext.h>
#include <Render/RenderOptions.h>
#include <Render/Renderer.h>

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

const QString RenderOptionsModule::renderOptionsMenuItemName = QString("Render Options");

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

    // Data node

    {
        std::unique_ptr<RenderOptionsData> data(new RenderOptionsData());
        DataContext* globalContext = accessor->GetGlobalContext();
        globalContext->CreateData(std::move(data));
    }

    // Menu item

    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/render_options.png"), renderOptionsMenuItemName);

        FieldDescriptor fieldIsEnabled;
        fieldIsEnabled.type = ReflectedTypeDB::Get<RenderOptionsData>();
        fieldIsEnabled.fieldName = FastName("isEnabled");
        action->SetStateUpdationFunction(QtAction::Enabled, fieldIsEnabled, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &RenderOptionsModule::ShowRenderOptionsDialog));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(QString("Tools"), { InsertionParams::eInsertionMethod::AfterItem }));

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

void RenderOptionsModule::ShowRenderOptionsDialog()
{
    DVASSERT(optionsDialog);
    optionsDialog->exec();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RenderOptionsModule)
{
    DAVA::ReflectionRegistrator<RenderOptionsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(RenderOptionsModule);
