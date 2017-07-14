#include "Classes/UserNodeModule/UserNodeModule.h"
#include "Classes/UserNodeModule/Private/UserNodeSystem.h"
#include "Classes/UserNodeModule/Private/UserNodeData.h"

#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/Utils/ModuleCollection.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderObject.h>

#include <Scene3D/Scene.h>
#include <Scene3D/SceneFileV2.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <FileSystem/FilePath.h>
#include <Logger/Logger.h>

namespace UserNodeModuleDetails
{
DAVA::RenderObject* CreateRenderObject()
{
    using namespace DAVA;

    FilePath scenePath = "~res:/ResourceEditor/3DObjects/Botspawn/tanksbox.sc2";

    ScopedPtr<Scene> scene(new Scene());
    SceneFileV2::eError result = scene->LoadScene(scenePath);
    if (result == SceneFileV2::ERROR_NO_ERROR)
    {
        Vector<Entity*> entities;
        scene->GetChildEntitiesWithComponent(entities, Component::RENDER_COMPONENT);
        if (entities.size() == 1)
        {
            return SafeRetain(GetRenderObject(entities[0]));
        }
    }

    Logger::Error("[%s] Can't open scene %s properly", __FUNCTION__, scenePath.GetStringValue().c_str());
    return nullptr;
}
}

void UserNodeModule::PostInit()
{
}

void UserNodeModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    if (spawnObject.get() == nullptr)
    { //we cannot create scene at post init, so I call it here
        spawnObject = UserNodeModuleDetails::CreateRenderObject();
    }

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();
    DVASSERT(scene != nullptr);

    std::unique_ptr<UserNodeData> userData = std::make_unique<UserNodeData>();
    userData->system.reset(new UserNodeSystem(scene, spawnObject.get()));
    scene->AddSystem(userData->system.get(), MAKE_COMPONENT_MASK(DAVA::Component::USER_COMPONENT), DAVA::Scene::SCENE_SYSTEM_REQUIRE_PROCESS);

    context->CreateData(std::move(userData));
}

void UserNodeModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA::TArc;

    SceneData* sceneData = context->GetData<SceneData>();
    SceneEditor2* scene = sceneData->GetScene().Get();

    UserNodeData* userData = context->GetData<UserNodeData>();
    scene->RemoveSystem(userData->system.get());
}

DAVA_VIRTUAL_REFLECTION_IMPL(UserNodeModule)
{
    DAVA::ReflectionRegistrator<UserNodeModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(UserNodeModule);
