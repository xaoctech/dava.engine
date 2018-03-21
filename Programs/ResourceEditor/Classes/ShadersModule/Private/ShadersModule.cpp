#include "Classes/ShadersModule/ShadersModule.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"

#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/Systems/EditorMaterialSystem.h>
#include <REPlatform/Scene/Systems/VisibilityCheckSystem.h>

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FilePath.h>
#include <Functional/Function.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialManager.h>
#include <Scene3D/Systems/FoliageSystem.h>
#include <Scene3D/Systems/ParticleEffectDebugDrawSystem.h>

namespace ShadersModuleDetail
{
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
DAVA::FilePath GetDevMaterialsPath()
{
    DAVA::FilePath devShadersPath(LOCAL_FRAMEWORK_SOURCE_PATH);
    devShadersPath += "/Programs/Data/";
    return devShadersPath;
}
#endif
}

void ShadersModule::PostInit()
{
    using namespace DAVA;
    FieldDescriptor fieldDescriptor(DAVA::ReflectedTypeDB::Get<ProjectManagerData>(), DAVA::FastName(ProjectManagerData::ProjectPathProperty));
    fieldBinder.reset(new DAVA::FieldBinder(GetAccessor()));
    fieldBinder->BindField(fieldDescriptor, DAVA::MakeFunction(this, &ShadersModule::OnProjectChanged));
}

void ShadersModule::InitDevShaders()
{
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
    DAVA::GetEngineContext()->assetManager->AddResourceFolder(ShadersModuleDetail::GetDevMaterialsPath().GetAbsolutePathname());
    DAVA::FilePath::AddResourcesFolder(ShadersModuleDetail::GetDevMaterialsPath());
#endif
}

void ShadersModule::OnProjectChanged(const DAVA::Any& projectFieldValue)
{
    DAVA::FilePath newProjectPathname;
    if (projectFieldValue.CanGet<DAVA::FilePath>())
    {
        newProjectPathname = projectFieldValue.Get<DAVA::FilePath>();
    }

    if (newProjectPathname.IsEmpty())
    {
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
        DAVA::FilePath::RemoveResourcesFolder(ShadersModuleDetail::GetDevMaterialsPath());
#endif
    }
    else
    {
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
        DAVA::FilePath::AddResourcesFolder(ShadersModuleDetail::GetDevMaterialsPath());
#endif
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ShadersModule)
{
    DAVA::ReflectionRegistrator<ShadersModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(ShadersModule);
