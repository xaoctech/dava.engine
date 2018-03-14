#include "Classes/ReflectionsModule/ReflectionsModule.h"

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

#include <FileSystem/FileSystem.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Scene3D/Systems/ReflectionSystem.h>
#include <Debug/MessageBox.h>
#include <FileSystem/FileSystem.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Render/Image/ImageSystem.h>
#include <Time/DateTime.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ReflectionsModule)
{
    DAVA::ReflectionRegistrator<ReflectionsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void ReflectionsModule::PostInit()
{
    using namespace DAVA;

    QtAction* bakeReflectionsAction = new QtAction(GetAccessor(), QIcon(":/QtIcons/sphere.png"), QString("Bake Reflections"));

    FieldDescriptor fieldDescriptor(DAVA::ReflectedTypeDB::Get<ProjectManagerData>(), DAVA::FastName(ProjectManagerData::ProjectPathProperty));
    bakeReflectionsAction->SetStateUpdationFunction(QtAction::Enabled, fieldDescriptor, [](const DAVA::Any& fieldValue) -> DAVA::Any {
        return fieldValue.CanCast<DAVA::FilePath>() && !fieldValue.Cast<DAVA::FilePath>().IsEmpty();
    });

    ActionPlacementInfo menuPlacement(CreateMenuPoint("Scene", InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "actionEnableCameraLight")));
    GetUI()->AddAction(DAVA::mainWindowKey, menuPlacement, bakeReflectionsAction);

    // ActionPlacementInfo toolbarPlacement(CreateToolbarPoint("sceneToolBar", InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "Reload Sprites")));
    // GetUI()->AddAction(DAVA::mainWindowKey, toolbarPlacement, bakeReflectionsAction);

    connections.AddConnection(bakeReflectionsAction, &QAction::triggered, DAVA::MakeFunction(this, &ReflectionsModule::BakeReflections));
}

void ReflectionsModule::BakeReflections()
{
    using namespace DAVA;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    DVASSERT(sceneData != nullptr);

    RefPtr<SceneEditor2> scene = sceneData->GetScene();
    DVASSERT(scene);

    if (scene->GetScenePath().IsEmpty())
    {
        Debug::MessageBox("Save Scene", "Please save scene before baking reflections", { "Ok, I'll do" });
        return;
    }

    String targetPath = scene->GetScenePath().GetDirectory().GetAbsolutePathname() + "reflections";
    if (FileSystem::Instance()->CreateDirectory(targetPath, true) == FileSystem::eCreateDirectoryResult::DIRECTORY_CANT_CREATE)
    {
        Debug::MessageBox("Save Scene", DAVA::Format("Failed to create directory\n%s", targetPath.c_str()), { "Close" });
        return;
    }

    ReflectionSystem* reflectionSystem = scene->reflectionSystem;

    const char* faceId[6] = { "px", "nx", "py", "ny", "pz", "nz" };

    Set<String> processedTextures;

    DateTime now = DateTime::Now();
    String timestamp = Format("%d-%d-%d_%d-%d-%d", now.GetYear(), now.GetMonth(), now.GetDay(), now.GetHour(), now.GetMinute(), now.GetSecond());

    for (ReflectionComponent* component : reflectionSystem->GetAllComponents())
    {
        ReflectionProbe* probe = component->GetReflectionProbe();
        if (probe->IsDynamicProbe())
        {
            uint32 probeIndex = 0;
            String imageId = DAVA::Format("%s_", component->GetEntity()->GetName().c_str());
            while (processedTextures.count(imageId) > 0)
            {
                imageId = DAVA::Format("%s%03u_", component->GetEntity()->GetName().c_str(), probeIndex);
                ++probeIndex;
            }
            processedTextures.emplace(imageId);
            imageId += timestamp;

            FilePath descriptorPath = DAVA::Format("%s/%s.tex", targetPath.c_str(), imageId.c_str());

            FilePath descriptorBaseName = descriptorPath;
            descriptorBaseName.ReplaceExtension(String());

            Vector<Vector<Image*>> images;
            Texture* tex = probe->GetCurrentTexture();
            tex->CreateCubemapMipmapImages(images, tex->GetMipLevelsCount());

            uint32 faceIndex = 0;
            for (Vector<Image*>& face : images)
            {
                FilePath targetFileName = DAVA::Format("%s_%s.dds", descriptorBaseName.GetAbsolutePathname().c_str(), faceId[faceIndex]);
                ImageSystem::Save(targetFileName, face, tex->GetDescriptor()->GetTextureFormat());
                ++faceIndex;
            }

            {
                TextureDescriptor desc;
                desc.pathname = descriptorPath;
                desc.dataSettings.sourceFileFormat = ImageFormat::IMAGE_FORMAT_DDS;
                desc.dataSettings.sourceFileExtension = ".dds";
                desc.dataSettings.cubefaceFlags = 0x3f /* all faces*/;
                desc.dataSettings.cubefaceExtensions[0] = ".dds";
                desc.dataSettings.cubefaceExtensions[1] = ".dds";
                desc.dataSettings.cubefaceExtensions[2] = ".dds";
                desc.dataSettings.cubefaceExtensions[3] = ".dds";
                desc.dataSettings.cubefaceExtensions[4] = ".dds";
                desc.dataSettings.cubefaceExtensions[5] = ".dds";
                desc.dataSettings.textureFlags = 0;
                desc.Save(descriptorPath);
            }

            component->SetReflectionsMap(descriptorPath);
            component->SetReflectionType(probe->IsGlobalProbe() ? ReflectionProbe::ProbeType::GLOBAL_STATIC : ReflectionProbe::ProbeType::LOCAL_STATIC);
            ++probeIndex;
        }
    }
}

DECL_TARC_MODULE(ReflectionsModule);
