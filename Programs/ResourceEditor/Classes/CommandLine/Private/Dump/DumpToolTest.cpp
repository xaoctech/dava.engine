#include "CommandLine/DumpTool.h"
#include "CommandLine/Private/CommandLineModuleTestUtils.h"
#include "Utils/FileSystemUtils/FileSystemTagGuard.h"

#include <TArc/Testing/TArcUnitTests.h>
#include <TArc/Testing/ConsoleModuleTestExecution.h>

#include <Base/ScopedPtr.h>
#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <Render/TextureDescriptor.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Highlevel/Vegetation/VegetationRenderObject.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/SlotComponent.h>

#include <memory>

namespace DTestDetail
{
const DAVA::String projectStr = "~doc:/Test/DumpTool/";
const DAVA::String scenePathnameStr = projectStr + "DataSource/3d/Scene/testScene.sc2";
const DAVA::String linksStr = "~doc:/Test/DumpTool/links.txt";

DAVA::Set<DAVA::String> ReadLinks()
{
    using namespace DAVA;

    Set<String> dumpedLinks;
    ScopedPtr<File> linksFile(File::Create(linksStr, File::READ | File::OPEN));
    if (linksFile)
    {
        do
        {
            String link = linksFile->ReadLine();
            if (!link.empty())
            {
                dumpedLinks.insert(link);
            }

        } while (!linksFile->IsEof());
    }
    return dumpedLinks;
}
}

DAVA_TARC_TESTCLASS(DumpToolTest)
{
    void TestTags(const DAVA::String& tag)
    {
        using namespace DAVA;

        FileSystem* fs = GetEngineContext()->fileSystem;
        TEST_VERIFY(fs->GetFilenamesTag() == tag);

        Set<String> dumpedLinks = DTestDetail::ReadLinks();
        TEST_VERIFY(dumpedLinks.empty() == false);

        auto it = std::find_if(std::begin(dumpedLinks), std::end(dumpedLinks), [](const String& link) {
            return link.find(CommandLineModuleTestUtils::SceneBuilder::tagJapan) != String::npos;
        });
        TEST_VERIFY(it == std::end(dumpedLinks));

        it = std::find_if(std::begin(dumpedLinks), std::end(dumpedLinks), [](const String& link) {
            return link.find("China.slot/box_slot.sc2") != String::npos;
        });
        TEST_VERIFY(it != std::end(dumpedLinks));

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(DTestDetail::scenePathnameStr) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR);

        uint32 entityCount = scene->GetChildrenCount();
        for (uint32 e = 0; e < entityCount; ++e)
        {
            Entity* child = scene->GetChild(e);

            RenderObject* ro = GetRenderObject(child);
            if (ro == nullptr)
                continue;

            if (ro->GetType() != RenderObject::TYPE_MESH)
                continue;

            SlotComponent* comp = static_cast<SlotComponent*>(child->GetComponent(DAVA::Component::SLOT_COMPONENT));
            if (comp == nullptr)
                continue;

            auto pathInLinks = [&dumpedLinks, &tag](const FilePath& path) -> bool
            {
                FilePath taggedPath = path;
                taggedPath.ReplaceBasename(taggedPath.GetBasename() + tag);
                auto it = std::find(std::begin(dumpedLinks), std::end(dumpedLinks), taggedPath.GetAbsolutePathname());
                return it != std::end(dumpedLinks);
            };
            TEST_VERIFY(pathInLinks(comp->GetConfigFilePath()) == true);

            uint32 rbCount = ro->GetRenderBatchCount();
            for (uint32 r = 0; r < rbCount; ++r)
            {
                RenderBatch* rb = ro->GetRenderBatch(r);
                TEST_VERIFY(rb != nullptr);

                NMaterial* mat = rb->GetMaterial();
                if (mat == nullptr)
                    continue;

                const UnorderedMap<FastName, MaterialTextureInfo*>& textures = mat->GetLocalTextures();
                for (auto& tx : textures)
                {
                    if (tx.first == FastName("heightmap"))
                        continue;

                    TEST_VERIFY(dumpedLinks.count(tx.second->path.GetAbsolutePathname()) == 1);

                    std::unique_ptr<TextureDescriptor> texDescriptor(TextureDescriptor::CreateFromFile(tx.second->path));
                    if (texDescriptor == nullptr)
                        continue;

                    Vector<FilePath> pathes;
                    texDescriptor->CreateLoadPathnamesForGPU(eGPUFamily::GPU_ORIGIN, pathes);
                    for (const FilePath& p : pathes)
                    {
                        TEST_VERIFY(dumpedLinks.count(p.GetAbsolutePathname()) == 1);
                        TEST_VERIFY(pathInLinks(p) == true);
                    }
                }
            }
        }
    }

    void TestLinks(SceneDumper::eMode mode, const DAVA::Vector<DAVA::eGPUFamily>& compressedGPUs)
    {
        using namespace DAVA;

        Set<String> dumpedLinks = DTestDetail::ReadLinks();
        TEST_VERIFY(dumpedLinks.empty() == false);

        ScopedPtr<Scene> scene(new Scene());
        TEST_VERIFY(scene->LoadScene(DTestDetail::scenePathnameStr) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR);

        uint32 entityCount = scene->GetChildrenCount();
        for (uint32 e = 0; e < entityCount; ++e)
        {
            Entity* child = scene->GetChild(e);

            //test reference to owner
            String ownerName = child->GetName().c_str() + String(".sc2");

            auto it = std::find_if(dumpedLinks.begin(), dumpedLinks.end(), [&ownerName](const String& str) {
                return str.find(ownerName) != String::npos;
            });

            TEST_VERIFY((it == dumpedLinks.end()) == (mode == SceneDumper::eMode::REQUIRED));

            RenderObject* ro = GetRenderObject(child);
            if (ro == nullptr)
                continue;

            auto testMaterial = [&dumpedLinks, &compressedGPUs](NMaterial* mat)
            {
                if (mat == nullptr)
                    return;

                const UnorderedMap<FastName, MaterialTextureInfo*>& textures = mat->GetLocalTextures();
                for (auto& tx : textures)
                {
                    if (tx.first == FastName("heightmap"))
                        continue;

                    TEST_VERIFY(dumpedLinks.count(tx.second->path.GetAbsolutePathname()) == 1);

                    std::unique_ptr<TextureDescriptor> texDescriptor(TextureDescriptor::CreateFromFile(tx.second->path));
                    if (texDescriptor == nullptr)
                        continue;

                    for (eGPUFamily gpu : compressedGPUs)
                    {
                        Vector<FilePath> pathes;
                        texDescriptor->CreateLoadPathnamesForGPU(gpu, pathes);
                        for (const FilePath& p : pathes)
                        {
                            TEST_VERIFY(dumpedLinks.count(p.GetAbsolutePathname()) == 1);
                        }
                    }
                }
            };

            if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
            {
                Landscape* landscape = static_cast<Landscape*>(ro);
                testMaterial(landscape->GetMaterial());

                TEST_VERIFY(dumpedLinks.count(landscape->GetHeightmapPathname().GetAbsolutePathname()) == 1);
            }
            else if (ro->GetType() == RenderObject::TYPE_VEGETATION)
            {
                VegetationRenderObject* vegetation = static_cast<VegetationRenderObject*>(ro);

                const size_t expectedCount = (mode == SceneDumper::eMode::REQUIRED) ? 0 : 1;
                TEST_VERIFY(dumpedLinks.count(vegetation->GetCustomGeometryPath().GetAbsolutePathname()) == expectedCount);
                TEST_VERIFY(dumpedLinks.count(vegetation->GetLightmapPath().GetAbsolutePathname()) == 1);
            }
            else
            {
                uint32 rbCount = ro->GetRenderBatchCount();
                for (uint32 r = 0; r < rbCount; ++r)
                {
                    RenderBatch* rb = ro->GetRenderBatch(r);
                    TEST_VERIFY(rb != nullptr);
                    testMaterial(rb->GetMaterial());
                }
            }
        }
    }

    DAVA_TEST (DumpFileExtended)
    {
        using namespace DAVA;

        std::unique_ptr<CommandLineModuleTestUtils::TextureLoadingGuard> guard = CommandLineModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        CommandLineModuleTestUtils::CreateProjectInfrastructure(DTestDetail::projectStr);
        CommandLineModuleTestUtils::SceneBuilder::CreateFullScene(DTestDetail::scenePathnameStr, DTestDetail::projectStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-dump",
          "-links",
          "-indir",
          FilePath(DTestDetail::scenePathnameStr).GetDirectory().GetAbsolutePathname(),
          "-processfile",
          FilePath(DTestDetail::scenePathnameStr).GetFilename(),
          "-outfile",
          FilePath(DTestDetail::linksStr).GetAbsolutePathname(),
          "-mode",
          "e",
          "-gpu",
          "all"
        };

        std::unique_ptr<CommandLineModule> tool = std::make_unique<DumpTool>(cmdLine);
        DAVA::TArc::ConsoleModuleTestExecution::ExecuteModule(tool.get());

        TestLinks(SceneDumper::eMode::EXTENDED, { GPU_POWERVR_IOS, GPU_POWERVR_ANDROID, GPU_TEGRA, GPU_MALI, GPU_ADRENO, GPU_DX11 });

        CommandLineModuleTestUtils::ClearTestFolder(DTestDetail::projectStr);
    }

    DAVA_TEST (DumpFileRequired)
    {
        using namespace DAVA;

        std::unique_ptr<CommandLineModuleTestUtils::TextureLoadingGuard> guard = CommandLineModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        CommandLineModuleTestUtils::CreateProjectInfrastructure(DTestDetail::projectStr);
        CommandLineModuleTestUtils::SceneBuilder::CreateFullScene(DTestDetail::scenePathnameStr, DTestDetail::projectStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-dump",
          "-links",
          "-indir",
          FilePath(DTestDetail::scenePathnameStr).GetDirectory().GetAbsolutePathname(),
          "-processfile",
          FilePath(DTestDetail::scenePathnameStr).GetFilename(),
          "-outfile",
          FilePath(DTestDetail::linksStr).GetAbsolutePathname(),
          "-mode",
          "r",
          "-gpu",
          "mali,tegra"
        };

        std::unique_ptr<CommandLineModule> tool = std::make_unique<DumpTool>(cmdLine);
        DAVA::TArc::ConsoleModuleTestExecution::ExecuteModule(tool.get());

        TestLinks(SceneDumper::eMode::REQUIRED, { eGPUFamily::GPU_MALI, eGPUFamily::GPU_TEGRA });

        CommandLineModuleTestUtils::ClearTestFolder(DTestDetail::projectStr);
    }

    DAVA_TEST (FileSystemTagsTest)
    {
        using namespace DAVA;

        FileSystemTagGuard tagGuard(CommandLineModuleTestUtils::SceneBuilder::tagChina);
        std::unique_ptr<CommandLineModuleTestUtils::TextureLoadingGuard> guard = CommandLineModuleTestUtils::CreateTextureGuard({ eGPUFamily::GPU_ORIGIN });
        CommandLineModuleTestUtils::CreateProjectInfrastructure(DTestDetail::projectStr);
        CommandLineModuleTestUtils::SceneBuilder::CreateFullScene(DTestDetail::scenePathnameStr, DTestDetail::projectStr);

        Vector<String> cmdLine =
        {
          "ResourceEditor",
          "-dump",
          "-links",
          "-indir",
          FilePath(DTestDetail::scenePathnameStr).GetDirectory().GetAbsolutePathname(),
          "-processfile",
          FilePath(DTestDetail::scenePathnameStr).GetFilename(),
          "-outfile",
          FilePath(DTestDetail::linksStr).GetAbsolutePathname(),
          "-gpu",
          "origin",
          "-taglist",
          CommandLineModuleTestUtils::SceneBuilder::tagChina
        };

        std::unique_ptr<CommandLineModule> tool = std::make_unique<DumpTool>(cmdLine);
        DAVA::TArc::ConsoleModuleTestExecution::ExecuteModule(tool.get());

        TestTags(CommandLineModuleTestUtils::SceneBuilder::tagChina);

        CommandLineModuleTestUtils::ClearTestFolder(DTestDetail::projectStr);
    }

    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("DumpTool.cpp")
    END_FILES_COVERED_BY_TESTS();
};
