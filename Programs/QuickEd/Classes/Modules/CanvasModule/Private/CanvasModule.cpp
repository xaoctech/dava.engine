#include "Modules/CanvasModule/CanvasModule.h"
#include "Modules/CanvasModule/CanvasModuleData.h"
#include "Modules/CanvasModule/EditorCanvas.h"
#include "Modules/CanvasModule/EditorControlsView.h"
#include "Modules/CanvasModule/CanvasData.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Utils/ModuleCollection.h>

#include <UI/Input/UIInputSystem.h>
#include <UI/UIControlSystem.h>

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
    std::unique_ptr<CanvasModuleData> data = std::make_unique<CanvasModuleData>();
    data->editorCanvas = std::make_unique<EditorCanvas>(GetAccessor());
    data->controlsView = std::make_unique<EditorControlsView>(data->canvas.Get(), GetAccessor());

    data->controlsView->workAreaSizeChanged.Connect(this, &CanvasModule::OnWorkAreaSizeChanged);
    data->controlsView->rootControlSizeChanged.Connect(this, &CanvasModule::OnRootControlSizeChanged);
    data->controlsView->rootControlPositionChanged.Connect(this, &CanvasModule::OnRootControlPositionChanged);
    data->controlsView->needCentralizeChanged.Connect(this, &CanvasModule::OnNeedCentralizeChanged);

    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
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

void CanvasModule::OnNeedCentralizeChanged(bool needCentralize)
{
    DAVA::TArc::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->displacement.SetZero();
}

void CanvasModule::OnRootControlPositionChanged(const DAVA::Vector2& rootControlPos)
{
    DAVA::TArc::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->rootPosition = rootControlPos;
}

void CanvasModule::OnRootControlSizeChanged(const DAVA::Vector2& rootControlSize)
{
    DAVA::TArc::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->rootControlSize = rootControlSize;
}

void CanvasModule::OnWorkAreaSizeChanged(const DAVA::Vector2& workAreaSize)
{
    DAVA::TArc::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->workAreaSize = workAreaSize;
}

DECL_GUI_MODULE(CanvasModule);
