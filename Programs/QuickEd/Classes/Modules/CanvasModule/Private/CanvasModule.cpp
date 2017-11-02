#include "Modules/CanvasModule/CanvasModule.h"
#include "Modules/CanvasModule/CanvasModuleData.h"
#include "Modules/CanvasModule/EditorCanvas.h"
#include "Modules/CanvasModule/EditorControlsView.h"
#include "Modules/CanvasModule/CanvasData.h"
#include "UI/Preview/PreviewWidgetSettings.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/DataProcessing/Common.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <UI/Input/UIInputSystem.h>
#include <UI/UIControlSystem.h>

#include <QList>
#include <QString>
#include <QMenu>

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasModule)
{
    DAVA::ReflectionRegistrator<CanvasModule>::Begin()
    .ConstructorByPointer()
    .End();
}

CanvasModule::CanvasModule()
{
}

void CanvasModule::PostInit()
{
    CreateData();
    CreateMenuSeparator();
    RecreateBgrColorActions();

    wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<PreviewWidgetSettings>());
    wrapper.SetListener(this);
}

void CanvasModule::CreateData()
{
    std::unique_ptr<CanvasModuleData> data = std::make_unique<CanvasModuleData>();
    data->editorCanvas = std::make_unique<EditorCanvas>(GetAccessor());
    data->controlsView = std::make_unique<EditorControlsView>(data->canvas.Get(), GetAccessor());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void CanvasModule::CreateMenuSeparator()
{
    using namespace DAVA;
    QAction* separator = new QAction(nullptr);
    separator->setObjectName("bgrMenuSeparator");
    separator->setSeparator(true);
    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, "Dock" }));
    GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, separator);
}

void CanvasModule::RecreateBgrColorActions()
{
    using namespace DAVA;

    CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
    std::for_each(data->bgrColorActions.begin(), data->bgrColorActions.end(), [this](const CanvasModuleData::ActionInfo& actionInfo)
                  {
                      GetUI()->RemoveAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.name);
                  });

    QString menuBgrColor = "Background Color";
    {
        QMenu* backgroundMenu = new QMenu(menuBgrColor, nullptr);
        ActionPlacementInfo placement;
        placement.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, "bgrMenuSeparator" }));
        GetUI()->AddAction(mainWindowKey, placement, backgroundMenu->menuAction());
    }

    FieldDescriptor indexFieldDescr;
    indexFieldDescr.type = ReflectedTypeDB::Get<PreviewWidgetSettings>();
    indexFieldDescr.fieldName = FastName("backgroundColorIndex");

    FieldDescriptor colorsFieldDescr;
    colorsFieldDescr.type = ReflectedTypeDB::Get<PreviewWidgetSettings>();
    colorsFieldDescr.fieldName = DAVA::FastName("backgroundColors");

    ActionPlacementInfo placement(CreateMenuPoint(QList<QString>() << MenuItems::menuView << menuBgrColor));

    PreviewWidgetSettings* settings = GetAccessor()->GetGlobalContext()->GetData<PreviewWidgetSettings>();
    const Vector<Color>& colors = settings->backgroundColors;

    data->bgrColorActions.resize(colors.size());
    for (DAVA::uint32 currentIndex = 0; currentIndex < colors.size(); ++currentIndex)
    {
        QtAction* action = new QtAction(GetAccessor(), QString("Background color %1").arg(currentIndex));
        action->SetStateUpdationFunction(QtAction::Icon, colorsFieldDescr, [currentIndex](const Any& v)
                                         {
                                             const Vector<Color>& colors = v.Cast<Vector<Color>>();
                                             if (currentIndex < colors.size())
                                             {
                                                 Any color = colors[currentIndex];
                                                 return color.Cast<QIcon>(QIcon());
                                             }
                                             else
                                             {
                                                 return QIcon();
                                             }
                                         });

        action->SetStateUpdationFunction(QtAction::Checked, indexFieldDescr, [currentIndex](const Any& v)
                                         {
                                             return v.Cast<DAVA::uint32>(-1) == currentIndex;
                                         });
        connections.AddConnection(action, &QAction::triggered, [this, currentIndex]()
                                  {
                                      PreviewWidgetSettings* settings = GetAccessor()->GetGlobalContext()->GetData<PreviewWidgetSettings>();
                                      settings->backgroundColorIndex = currentIndex;
                                  });

        GetUI()->AddAction(DAVA::mainWindowKey, placement, action);
        CanvasModuleData::ActionInfo& actionInfo = data->bgrColorActions[currentIndex];
        actionInfo.name = action->text();
        actionInfo.placement = placement;
    }
}

void CanvasModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
    DVASSERT(data != nullptr);

    EditorCanvas* editorCanvas = data->editorCanvas.get();
    EditorControlsView* controlsView = data->controlsView.get();

    systemsManager->RegisterEditorSystem(editorCanvas);
    systemsManager->RegisterEditorSystem(controlsView);
}

void CanvasModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
    DVASSERT(data != nullptr);

    EditorCanvas* editorCanvas = data->editorCanvas.get();
    EditorControlsView* controlsView = data->controlsView.get();

    systemsManager->UnregisterEditorSystem(editorCanvas);
    systemsManager->UnregisterEditorSystem(controlsView);
}

void CanvasModule::OnContextCreated(DAVA::DataContext* context)
{
    std::unique_ptr<CanvasData> canvasData = std::make_unique<CanvasData>();
    context->CreateData(std::move(canvasData));
}

void CanvasModule::OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    RecreateBgrColorActions();
}

DECL_GUI_MODULE(CanvasModule);
