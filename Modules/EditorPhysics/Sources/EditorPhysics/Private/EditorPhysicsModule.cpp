#include "EditorPhysics/EditorPhysicsModule.h"
#include "EditorPhysics/Private/EditorPhysicsSystem.h"
#include "EditorPhysics/Private/EditorIntegrationHelper.h"
#include "EditorPhysics/Private/EditorPhysicsData.h"
#include "EditorPhysics/Private/PhysicsWidget.h"

#include <Scene3D/Scene.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

void EditorPhysicsModule::OnContextCreated(DAVA::DataContext* context)
{
    using namespace EditorPhysicsDetail;
    DAVA::Scene* scene = ExtractScene(context);
    DVASSERT(scene != nullptr);

    std::unique_ptr<EditorPhysicsData> data(new EditorPhysicsData());
    data->system = new EditorPhysicsSystem(scene);
    scene->AddSystem(data->system, 0, DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    context->CreateData(std::move(data));
}

void EditorPhysicsModule::OnContextDeleted(DAVA::DataContext* context)
{
    using namespace EditorPhysicsDetail;
    DAVA::Scene* scene = ExtractScene(context);
    DVASSERT(scene != nullptr);

    EditorPhysicsData* data = context->GetData<EditorPhysicsData>();
    DVASSERT(data != nullptr);
    DVASSERT(data->system != nullptr);
    scene->RemoveSystem(data->system);
    DAVA::SafeDelete(data->system);
    context->DeleteData<EditorPhysicsData>();
}

void EditorPhysicsModule::PostInit()
{
    using namespace DAVA;

    QWidget* physicsPanel = new PhysicsWidget(GetAccessor(), GetUI());

    UI* ui = GetUI();

    DockPanelInfo info;
    info.title = QString("Physics");
    info.area = Qt::LeftDockWidgetArea;

    PanelKey key("Physics", info);
    ui->AddView(DAVA::mainWindowKey, key, physicsPanel);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EditorPhysicsModule)
{
    DAVA::ReflectionRegistrator<EditorPhysicsModule>::Begin()
    .ConstructorByPointer()
    .End();
}
