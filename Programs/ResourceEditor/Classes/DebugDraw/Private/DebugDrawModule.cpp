#include "Classes/DebugDraw/DebugDrawModule.h"
#include "Classes/DebugDraw/DebugDrawData.h"

#include "Classes/Application/REGlobal.h"
#include "Classes/SceneManager/SceneData.h"

#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"
#include "Classes/Qt/Scene/System/HoodSystem.h"
#include "Classes/Qt/Scene/System/WayEditSystem.h"

#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/Utils/ModuleCollection.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Logger/Logger.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"

void DebugDrawModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<DebugDrawData> debugDrawData = std::make_unique<DebugDrawData>();
    debugDrawData->debugDrawSystem.reset(new DebugDrawSystem(scene));
    scene->AddSystem(debugDrawData->debugDrawSystem.get(), 0 );

    context->CreateData(std::move(debugDrawData));
}

void DebugDrawModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    DebugDrawData* debugDrawData = context->GetData<DebugDrawData>();
    scene->RemoveSystem(debugDrawData->debugDrawSystem.get());
}

void DebugDrawModule::PostInit()
{
    DAVA::TArc::UI* ui = GetUI();
    CreateModuleActions(ui);
}
void DebugDrawModule::CreateModuleActions(DAVA::TArc::UI* ui)
{
    using namespace DAVA::TArc;

    ContextAccessor* accessor = GetAccessor();
}

DAVA_VIRTUAL_REFLECTION_IMPL(DebugDrawModule)
{
    DAVA::ReflectionRegistrator<DebugDrawModule>::Begin()
        .ConstructorByPointer()
        .End();
}

DECL_GUI_MODULE(DebugDrawModule);



