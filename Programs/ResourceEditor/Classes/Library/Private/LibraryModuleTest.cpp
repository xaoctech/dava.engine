#include "Classes/Library/LibraryModule.h"
#include "Classes/Library/Private/DAEConverter.h"
#include "Classes/CommandLine/Private/CommandLineModuleTestUtils.h"

#include <REPlatform/DataNodes/ProjectResources.h>

#include <TArc/Testing/TArcUnitTests.h>
#include <TArc/Testing/MockDefine.h>
#include <TArc/Testing/MockListener.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/Core/ContextAccessor.h>

#include <Base/Any.h>
#include <Entity/Component.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Scene3D/Components/CustomPropertiesComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Lod/LodComponent.h>
#include <Scene3D/Scene.h>
#include <Scene3D/SceneFileV2.h>

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QTest>
#include <QWidget>

#include <gmock/gmock.h>

#include <memory>

namespace LibraryTest
{
const DAVA::String testFolder = DAVA::String("~doc:/Test/");
const DAVA::String testProjectPath = testFolder + DAVA::String("LibraryTest/");
const DAVA::String testDataSource3dPath = testProjectPath + DAVA::String("DataSource/3d/");
const DAVA::String testDAEPathname = testDataSource3dPath + "test.dae";

class ProjectManagerDummyModule : public DAVA::ClientModule
{
protected:
    void PostInit() override
    {
        using namespace DAVA;
        using namespace DAVA;

        // prepare test environment
        {
            CommandLineModuleTestUtils::CreateTestFolder(testFolder);
            CommandLineModuleTestUtils::CreateProjectInfrastructure(testProjectPath);

            FileSystem* fs = GetEngineContext()->fileSystem;
            TEST_VERIFY(fs->CopyFile("~res:/ResourceEditor/Selftest/LibraryModuleTest/test.dae", testDAEPathname));
        }

        projectResources.reset(new ProjectResources(GetAccessor()));
        projectResources->LoadProject(testProjectPath);
    }

    ~ProjectManagerDummyModule() override
    {
        CommandLineModuleTestUtils::ClearTestFolder(testFolder);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ProjectManagerDummyModule, DAVA::ClientModule)
    {
        DAVA::ReflectionRegistrator<ProjectManagerDummyModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

    std::unique_ptr<DAVA::ProjectResources> projectResources;
};
}

// clang-format off
DAVA_TARC_TESTCLASS(LibraryModuleTests)
{
    void ModifyScene()
    {
        using namespace DAVA;
        FilePath scenePathname = FilePath::CreateWithNewExtension(LibraryTest::testDAEPathname, ".sc2");

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(scenePathname) == SceneFileV2::eError::ERROR_NO_ERROR);

        Vector<Entity*> lodEntities;
        scene->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
        TEST_VERIFY(lodEntities.empty() == false);

        for (Entity* e : lodEntities)
        {
            uint32 count = e->GetComponentCount(Component::LOD_COMPONENT);
            TEST_VERIFY(count > 0);
            for (uint32 ic = 0; ic < count; ++ic)
            {
                LodComponent* lod = static_cast<LodComponent*>(e->GetComponent(Component::LOD_COMPONENT, ic));
                for (int32 layer = 0; layer < LodComponent::MAX_LOD_LAYERS; ++layer)
                {
                    lod->SetLodLayerDistance(layer, 10.f + 10.f * layer);
                }
            }

            //add CP
            CustomPropertiesComponent* cp = new CustomPropertiesComponent();
            cp->GetArchive()->SetString("editor.referenceToOwner", scenePathname.GetAbsolutePathname());
            cp->GetArchive()->SetFastName("name", e->GetName());
            e->AddComponent(cp);
        }

        Vector<Entity*> renderEntities;
        scene->GetChildEntitiesWithComponent(renderEntities, Component::RENDER_COMPONENT);
        for (Entity* e : renderEntities)
        {
            uint32 count = e->GetComponentCount(Component::RENDER_COMPONENT);
            TEST_VERIFY(count > 0);
            for (uint32 ic = 0; ic < count; ++ic)
            {
                RenderComponent* rc = static_cast<RenderComponent*>(e->GetComponent(Component::RENDER_COMPONENT, ic));
                RenderObject* ro = rc->GetRenderObject();
                TEST_VERIFY(ro->GetRenderBatchCount() > 0);
                for (uint32 ib = 0; ib < ro->GetRenderBatchCount(); ++ib)
                {
                    RenderBatch* batch = ro->GetRenderBatch(ib);
                    NMaterial* material = batch->GetMaterial();

                    TEST_VERIFY(material->HasLocalFlag(NMaterialFlagName::FLAG_FLATCOLOR) == false);
                    material->AddFlag(NMaterialFlagName::FLAG_FLATCOLOR, ib % 2);
                }
            }
        }

        TEST_VERIFY(scene->SaveScene(scenePathname, false) == SceneFileV2::eError::ERROR_NO_ERROR);
    }

    void TestScene()
    {
        using namespace DAVA;
        FilePath scenePathname = FilePath::CreateWithNewExtension(LibraryTest::testDAEPathname, ".sc2");

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(scenePathname) == SceneFileV2::eError::ERROR_NO_ERROR);

        Vector<Entity*> lodEntities;
        scene->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);
        TEST_VERIFY(lodEntities.empty() == false);

        for (Entity* e : lodEntities)
        {
            uint32 count = e->GetComponentCount(Component::LOD_COMPONENT);
            TEST_VERIFY(count > 0);
            for (uint32 ic = 0; ic < count; ++ic)
            {
                LodComponent* lod = static_cast<LodComponent*>(e->GetComponent(Component::LOD_COMPONENT, ic));
                for (int32 layer = 0; layer < LodComponent::MAX_LOD_LAYERS; ++layer)
                {
                    TEST_VERIFY(lod->GetLodLayerDistance(layer) - (10.f + 10.f * layer) < DAVA::EPSILON);
                }
            }

            CustomPropertiesComponent* cp = static_cast<CustomPropertiesComponent*>(e->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT, 0));
            TEST_VERIFY(cp != nullptr);
            if (cp != nullptr)
            {
                TEST_VERIFY(cp->GetArchive()->GetString("editor.referenceToOwner") == scenePathname.GetAbsolutePathname());
                TEST_VERIFY(cp->GetArchive()->GetFastName("name") == e->GetName());
            }
        }

        Vector<Entity*> renderEntities;
        scene->GetChildEntitiesWithComponent(renderEntities, Component::RENDER_COMPONENT);
        for (Entity* e : renderEntities)
        {
            uint32 count = e->GetComponentCount(Component::RENDER_COMPONENT);
            TEST_VERIFY(count > 0);
            for (uint32 ic = 0; ic < count; ++ic)
            {
                RenderComponent* rc = static_cast<RenderComponent*>(e->GetComponent(Component::RENDER_COMPONENT, ic));
                RenderObject* ro = rc->GetRenderObject();
                TEST_VERIFY(ro->GetRenderBatchCount() > 0);
                for (uint32 ib = 0; ib < ro->GetRenderBatchCount(); ++ib)
                {
                    RenderBatch* batch = ro->GetRenderBatch(ib);
                    NMaterial* material = batch->GetMaterial();

                    TEST_VERIFY(material->HasLocalFlag(NMaterialFlagName::FLAG_FLATCOLOR) == true);
                    TEST_VERIFY(material->GetLocalFlagValue(NMaterialFlagName::FLAG_FLATCOLOR) == (ib % 2));
                }
            }
        }
    }

    DAVA_TEST (ConvertDAETest)
    {
        TEST_VERIFY(DAEConverter::Convert(LibraryTest::testDAEPathname) == true);
        ModifyScene();

        TEST_VERIFY(DAEConverter::Convert(LibraryTest::testDAEPathname) == true);
        TestScene();
    }

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(LibraryTest::ProjectManagerDummyModule)
    DECLARE_TESTED_MODULE(LibraryModule)
    END_TESTED_MODULES()
};

// clang-format on
