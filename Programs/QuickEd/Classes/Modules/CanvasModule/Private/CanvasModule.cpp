#include "Modules/CanvasModule/CanvasModule.h"
#include "Modules/CanvasModule/CanvasModuleData.h"
#include "Modules/CanvasModule/EditorCanvas.h"
#include "Modules/CanvasModule/EditorControlsView.h"
#include "Modules/CanvasModule/CanvasData.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Utils/ModuleCollection.h>

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
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void CanvasModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
        DVASSERT(data != nullptr);

        EditorCanvas* editorCanvas = data->editorCanvas.get();
        EditorControlsView* controlsView = data->controlsView.get();

        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->RegisterEditorSystem(editorCanvas);
        systemsManager->RegisterEditorSystem(controlsView);
    }
}

void CanvasModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
        DVASSERT(data != nullptr);

        EditorCanvas* editorCanvas = data->editorCanvas.get();
        EditorControlsView* controlsView = data->controlsView.get();

        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->UnregisterEditorSystem(editorCanvas);
        systemsManager->UnregisterEditorSystem(controlsView);
    }
}

void CanvasModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    std::unique_ptr<CanvasData> canvasData = std::make_unique<CanvasData>();
    context->CreateData(std::move(canvasData));
}

DECL_GUI_MODULE(CanvasModule);
