#include <DAVAEngine.h>
#include <UnitTests/UnitTests.h>

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)

#include <DavaTools/TexturePacker/ResourcePacker2D.h>
#include <DavaTools/TextureCompression/PVRConverter.h>

#include <Render/PixelFormatDescriptor.h>

DAVA_TESTCLASS (ResourcePackerTest)
{
    const DAVA::FilePath resourcesDir = "~res:/TestData/ResourcePackerTest/SourceFiles/"; // this folder contains 5 files: air.png, air.psd, arrow_tut.psd, eye_tut.psd, target_tut.psd
    const DAVA::FilePath rootDir = "~doc:/TestData/ResourcePackerTest/";
    const DAVA::FilePath inputDir = rootDir + "Input/";
    const DAVA::FilePath outputDir = rootDir + "Output/";
    const DAVA::Vector<DAVA::String> srcBaseNames = { "air", "arrow_tut", "eye_tut", "target_tut" };

    ResourcePackerTest()
    {
#if defined(__DAVAENGINE_MACOS__)
        const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
        const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
        DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);
    }

    void ClearWorkingFolders()
    {
        DAVA::FileSystem::Instance()->DeleteDirectory(rootDir, true);
        DAVA::FileSystem::Instance()->CreateDirectory(inputDir, true);
    }

    void CopyPsdSources()
    {
        for (const DAVA::String& basename : srcBaseNames)
        {
            DAVA::String fullName = basename + ".psd";
            DAVA::FileSystem::Instance()->CopyFile(resourcesDir + fullName, inputDir + fullName);
        }
    }

    DAVA_TEST (GpuTest)
    {
        using namespace DAVA;

        ClearWorkingFolders();
        CopyPsdSources();

        struct GpuParams
        {
            eGPUFamily gpu = eGPUFamily::GPU_ORIGIN;
            PixelFormat pixelFormat = FORMAT_RGBA8888;
            ImageFormat imageFormat = IMAGE_FORMAT_PNG;
        };

        Vector<GpuParams> gpuParams;
        gpuParams.reserve(eGPUFamily::GPU_FAMILY_COUNT);
        gpuParams.push_back({ eGPUFamily::GPU_POWERVR_IOS, PixelFormat::FORMAT_RGBA8888, ImageFormat::IMAGE_FORMAT_PVR });
        gpuParams.push_back({ eGPUFamily::GPU_POWERVR_ANDROID, PixelFormat::FORMAT_RGBA8888, ImageFormat::IMAGE_FORMAT_WEBP });
        gpuParams.push_back({ eGPUFamily::GPU_TEGRA, PixelFormat::FORMAT_RGBA8888, ImageFormat::IMAGE_FORMAT_JPEG });
        gpuParams.push_back({ eGPUFamily::GPU_MALI, PixelFormat::FORMAT_RGBA8888, ImageFormat::IMAGE_FORMAT_TGA });
        gpuParams.push_back({ eGPUFamily::GPU_ADRENO, PixelFormat::FORMAT_ATC_RGB, ImageFormat::IMAGE_FORMAT_DDS });
        gpuParams.push_back({ eGPUFamily::GPU_DX11, PixelFormat::FORMAT_DXT1, ImageFormat::IMAGE_FORMAT_DDS });
        gpuParams.push_back({ eGPUFamily::GPU_ORIGIN, PixelFormat::FORMAT_RGBA8888, ImageFormat::IMAGE_FORMAT_PNG });

        Vector<eGPUFamily> requestedGPUs;
        {
            ScopedPtr<File> flagsFile(File::Create(inputDir + "flags.txt", File::CREATE | File::WRITE));
            for (const GpuParams& params : gpuParams)
            {
                const String& gpuName = GPUFamilyDescriptor::GetGPUName(params.gpu);
                const char* pixelFormatString = PixelFormatDescriptor::GetPixelFormatString(params.pixelFormat);
                const String& imageFormatString = ImageSystem::GetImageFormatInterface(params.imageFormat)->GetName();
                String flagString = Format("--%s %s %s ", gpuName.c_str(), pixelFormatString, imageFormatString.c_str());
                flagsFile->WriteNonTerminatedString(flagString);
                requestedGPUs.push_back(params.gpu);
            }
        }

        ResourcePacker2D packer;
        packer.InitFolders(inputDir, outputDir);
        packer.PackResources(requestedGPUs);

        TEST_VERIFY(packer.IsRunning() == false);
        TEST_VERIFY(packer.GetErrors().empty() == true);

        for (const String& name : srcBaseNames)
        {
            String fullName = name + ".txt";
            TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + fullName) == true);
        }

        for (const GpuParams& params : gpuParams)
        {
            String expectedSheetName = "texture0" + GPUFamilyDescriptor::GetGPUPrefix(params.gpu) + ImageSystem::GetDefaultExtension(params.imageFormat);
            TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + expectedSheetName) == true);
        }
        TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "texture0.tex") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "flags.txt") == false);
    };

    DAVA_TEST (WrongGpuParamsTest)
    {
        using namespace DAVA;

        ClearWorkingFolders();
        CopyPsdSources();

        {
            ScopedPtr<File> flagsFile(File::Create(inputDir + "flags.txt", File::CREATE | File::WRITE));
            flagsFile->WriteLine("--dx11 RGBA8888 dds");
        }

        ResourcePacker2D packer;
        packer.InitFolders(inputDir, outputDir);
        packer.PackResources({ eGPUFamily::GPU_DX11 });

        TEST_VERIFY(packer.IsRunning() == false);
        TEST_VERIFY(packer.GetErrors().empty() == false); // should contain error about incorrect flags for requested GPU
    }

    DAVA_TEST (NoFlagsForGpuTest)
    {
        ClearWorkingFolders();
        CopyPsdSources();

        DAVA::ResourcePacker2D packer;
        packer.InitFolders(inputDir, outputDir);
        packer.PackResources({ DAVA::eGPUFamily::GPU_DX11 });

        TEST_VERIFY(packer.GetErrors().empty() == false); // should contain error about absence of gpu key in flags.txt
    };

    DAVA_TEST (SourcePngTest)
    {
        using namespace DAVA;

        ClearWorkingFolders();
        FileSystem::Instance()->CopyFile(resourcesDir + "air.png", inputDir + "air.png");

        ResourcePacker2D packer;
        packer.InitFolders(inputDir, outputDir);
        packer.PackResources({ eGPUFamily::GPU_ORIGIN });

        TEST_VERIFY(packer.IsRunning() == false);
        TEST_VERIFY(packer.GetErrors().empty() == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "air.txt") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "texture0.png") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "texture0.tex") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "flags.txt") == false);
    }

    DAVA_TEST (TagsTest)
    {
        using namespace DAVA;
        ClearWorkingFolders();

        {
            FileSystem::Instance()->CopyFile(resourcesDir + "arrow_tut.psd", inputDir + "arrow_tut.psd");
            FileSystem::Instance()->CopyFile(resourcesDir + "eye_tut.psd", inputDir + "eye_tut.psd");

            ResourcePacker2D packer;
            packer.InitFolders(inputDir, outputDir);
            packer.PackResources({ eGPUFamily::GPU_ORIGIN });
            TEST_VERIFY(FileSystem::Instance()->Exists(outputDir + "texture0.png") == true);
        }

        FilePath tagsOutputDir = rootDir + "OutputWithTags/";
        FileSystem::Instance()->DeleteDirectory(inputDir, true);
        FileSystem::Instance()->CreateDirectory(inputDir, true);
        FileSystem::Instance()->CopyFile(resourcesDir + "air.psd", inputDir + "arrow_tut.psd");
        FileSystem::Instance()->CopyFile(resourcesDir + "arrow_tut.psd", inputDir + "arrow_tut.china.psd");
        FileSystem::Instance()->CopyFile(resourcesDir + "eye_tut.psd", inputDir + "eye_tut.psd");
        FileSystem::Instance()->CopyFile(resourcesDir + "target_tut.psd", inputDir + "target_tut.japan.psd");

        ResourcePacker2D packer;
        packer.InitFolders(inputDir, tagsOutputDir);
        packer.SetAllTags({ ".china", ".japan" });
        packer.SetTag(".china");
        packer.PackResources({ eGPUFamily::GPU_ORIGIN });

        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "texture0.png") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "texture0.tex") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "arrow_tut.txt") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "eye_tut.txt") == true);
        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "arrow_tut.china.txt") == false);
        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "target_tut.txt") == false);
        TEST_VERIFY(FileSystem::Instance()->Exists(tagsOutputDir + "target_tut.japan.txt") == false);
        TEST_VERIFY(FileSystem::Instance()->CompareBinaryFiles(outputDir + "texture0.png", tagsOutputDir + "texture0.png") == true);
        TEST_VERIFY(FileSystem::Instance()->CompareBinaryFiles(outputDir + "texture0.tex", tagsOutputDir + "texture0.tex") == true);
        TEST_VERIFY(FileSystem::Instance()->CompareTextFiles(outputDir + "arrow_tut.txt", tagsOutputDir + "arrow_tut.txt") == true);
        TEST_VERIFY(FileSystem::Instance()->CompareTextFiles(outputDir + "eye_tut.txt", tagsOutputDir + "eye_tut.txt") == true);
    };

    DAVA_TEST (MissingTagTest)
    {
        using namespace DAVA;

        ClearWorkingFolders();
        CopyPsdSources();

        ResourcePacker2D packer;
        packer.InitFolders(inputDir, outputDir);
        packer.SetAllTags({ ".japan" });
        packer.SetTag(".china");
        packer.PackResources({ eGPUFamily::GPU_ORIGIN });

        TEST_VERIFY(packer.GetErrors().empty() == false); // should contain error about absence of ".china" tag in allTags
    };
};

#endif
