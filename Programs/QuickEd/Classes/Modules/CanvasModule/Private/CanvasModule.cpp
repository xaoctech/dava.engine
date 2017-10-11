#include "Modules/CanvasModule/CanvasModule.h"
#include "Modules/CanvasModule/CanvasModuleData.h"
#include "Modules/CanvasModule/EditorCanvas.h"
#include "Modules/CanvasModule/EditorControlsView.h"
#include "Modules/CanvasModule/CanvasData.h"
#include "UI/Preview/PreviewWidgetSettings.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/DataProcessing/Common.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <UI/Input/UIInputSystem.h>
#include <UI/UIControlSystem.h>

#include <QList>
#include <QString>

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
    CreateActions();
}

void CanvasModule::CreateData()
{
    std::unique_ptr<CanvasModuleData> data = std::make_unique<CanvasModuleData>();
    data->editorCanvas = std::make_unique<EditorCanvas>(GetAccessor());
    data->controlsView = std::make_unique<EditorControlsView>(data->canvas.Get(), GetAccessor());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void CanvasModule::CreateActions()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    FieldDescriptor indexFieldDescr;
    indexFieldDescr.type = ReflectedTypeDB::Get<PreviewWidgetSettings>();
    indexFieldDescr.fieldName = FastName("backgroundColorIndex");

    FieldDescriptor colorsFieldDescr;
    colorsFieldDescr.type = ReflectedTypeDB::Get<PreviewWidgetSettings>();
    colorsFieldDescr.fieldName = DAVA::FastName("backgroundColors");

    ActionPlacementInfo info(CreateMenuPoint(QList<QString>() << "View"
                                                              << "menuGridColor"));

    PreviewWidgetSettings* settings = GetAccessor()->GetGlobalContext()->GetData<PreviewWidgetSettings>();
    const Vector<Color>& colors = settings->backgroundColors;

    for (DAVA::uint32 currentIndex = 0; currentIndex < colors.size(); ++currentIndex)
    {
        QtAction* action = new QtAction(GetAccessor(), QString("Background color %1").arg(currentIndex));
        action->SetStateUpdationFunction(QtAction::Icon, colorsFieldDescr, [currentIndex](const Any& v)
                                         {
                                             const Vector<Color>& colors = v.Cast<Vector<Color>>();
                                             Any color = colors[currentIndex];
                                             return color.Cast<QIcon>(QIcon());
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

        GetUI()->AddAction(DAVA::TArc::mainWindowKey, info, action);
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

void CanvasModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    std::unique_ptr<CanvasData> canvasData = std::make_unique<CanvasData>();
    context->CreateData(std::move(canvasData));
}

DECL_GUI_MODULE(CanvasModule);
