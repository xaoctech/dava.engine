#include <DocDirSetup/DocDirSetup.h>

#include <CommandLine/ProgramOptions.h>
#include <Debug/DVAssertDefaultHandlers.h>
#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <Logger/Logger.h>
#include <Logger/TeamcityOutput.h>
#include <Render/TextureDescriptor.h>
#include <Render/TextureDescriptorUtils.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>

#include <iostream>
#include <memory>

namespace TexDependencyDetails
{
enum eReturnCode : DAVA::int32
{
    SUCCESS = 0,
    ERROR = 1
};

void PrintMessage(const DAVA::String& message, bool outputToLog)
{
    if (outputToLog == true)
    {
        DAVA::Logger::Info(message.c_str());
    }
    else
    {
        std::cout << message << std::endl;
    }
}

void PrintError(const DAVA::String& message, bool outputToLog)
{
    if (outputToLog == true)
    {
        DAVA::Logger::Error(message.c_str());
    }
    else
    {
        std::cerr << "[error] " << message << std::endl;
    }
}

DAVA::int32 GetTexturePathnames(const DAVA::FilePath& texturePath, DAVA::Vector<DAVA::FilePath>& imagePathnames)
{
    using namespace DAVA;

    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(texturePath));
    if (descriptor)
    {
        if (descriptor->IsCubeMap())
        {
            descriptor->GetFacePathnames(imagePathnames);
        }
        else
        {
            imagePathnames.push_back(descriptor->GetSourceTexturePathname());
        }
        return eReturnCode::SUCCESS;
    }
    return eReturnCode::ERROR;
};

DAVA::int32 ProcessSelftest(bool enableTeamcityLogging)
{
    using namespace DAVA;
    int32 testResult = eReturnCode::SUCCESS;
    FilePath testDirectory = DocumentsDirectorySetup::GetApplicationDocDirectory(GetEngineContext()->fileSystem, "TexDependency");

    { // set up
        if (enableTeamcityLogging == true)
        {
            GetEngineContext()->logger->SetLogLevel(Logger::LEVEL_INFO);
            GetEngineContext()->logger->AddCustomOutput(new TeamcityOutput());
        }

        //delete data from interrupted tests
        GetEngineContext()->fileSystem->DeleteDirectory(testDirectory, true);

        //create new data
        DocumentsDirectorySetup::CreateApplicationDocDirectory(GetEngineContext()->fileSystem, "TexDependency");
        DocumentsDirectorySetup::SetApplicationDocDirectory(GetEngineContext()->fileSystem, "TexDependency");
    }

    { // test
        FilePath texturePath = testDirectory + "image.tex";
        ScopedPtr<Image> image(Image::Create(16, 16, PixelFormat::FORMAT_RGBA8888));

        { //test for simple texture
            FilePath pngPath = testDirectory + "image.png";
            ImageSystem::Save(pngPath, image.get());

            TextureDescriptorUtils::CreateDescriptor(pngPath);
            Vector<FilePath> imagePathnames;
            GetTexturePathnames(texturePath, imagePathnames);
            if ((imagePathnames.size() == 1 && imagePathnames[0] == pngPath) == false)
            {
                PrintError("[Selftest] Cannot get pathnames for simple texture", enableTeamcityLogging);
                testResult = eReturnCode::ERROR;
            }

            GetEngineContext()->fileSystem->DeleteFile(texturePath);
            GetEngineContext()->fileSystem->DeleteFile(pngPath);
        }

        { //test for cubemap texture
            Vector<FilePath> imagePathnames
            {
              testDirectory + "image_px.png",
              testDirectory + "image_nx.png",
              testDirectory + "image_py.tga",
              testDirectory + "image_ny.tga",
              testDirectory + "image_pz.png",
              testDirectory + "image_nz.png"
            };

            for (const FilePath& path : imagePathnames)
            {
                ImageSystem::Save(path, image.get());
            }

            TextureDescriptorUtils::CreateDescriptorCube(texturePath, imagePathnames);

            Vector<FilePath> newImagePathnames;
            GetTexturePathnames(texturePath, newImagePathnames);
            if (newImagePathnames != imagePathnames)
            {
                PrintError("[Selftest] Cannot get pathnames for cubemap texture", enableTeamcityLogging);
                testResult = eReturnCode::ERROR;
            }

            GetEngineContext()->fileSystem->DeleteFile(texturePath);
            for (const FilePath& path : imagePathnames)
            {
                GetEngineContext()->fileSystem->DeleteFile(path);
            }
        }
    }

    { //clean up
        GetEngineContext()->fileSystem->DeleteDirectory(testDirectory, true);
        if (enableTeamcityLogging == true)
        {
            GetEngineContext()->logger->SetLogLevel(Logger::LEVEL__DISABLE);
        }
    }

    return testResult;
}

DAVA::int32 ProcessTexDependency(DAVA::Engine& e)
{
    using namespace DAVA;

    const Vector<String>& cmdLine = e.GetCommandLine();
    ProgramOptions helpOption("--help");
    if (helpOption.Parse(cmdLine) == true)
    {
        PrintMessage("TexDependency filepath.tex --> to get dependency of *.tex", false);
        PrintMessage("TexDependency --selftest --> to test application by itself", false);
        PrintMessage("TexDependency --selftest --teamcity --> to test application by itself and use logging for Teamcity", false);
        PrintMessage("TexDependency --help --> to see this help", false);
        return eReturnCode::SUCCESS;
    }
    ProgramOptions selfTestOption("--selftest");
    selfTestOption.AddOption("--teamcity", VariantType(false), "Logging in Teamcity format");
    if (selfTestOption.Parse(cmdLine) == true)
    {
        return ProcessSelftest(selfTestOption.GetOption("--teamcity").AsBool());
    }

    if (cmdLine.size() == 2)
    {
        Vector<FilePath> imagePathnames;

        FilePath texturePath = cmdLine[1];
        if (texturePath.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()) == false)
        {
            PrintError(Format("Wrong texture pathname (%s). Should be *.tex", cmdLine[1].c_str()), false);
            return eReturnCode::ERROR;
        }

        if (GetTexturePathnames(texturePath, imagePathnames) == eReturnCode::SUCCESS)
        {
            for (const FilePath& path : imagePathnames)
            {
                PrintMessage(path.GetAbsolutePathname(), false);
            }
            return eReturnCode::SUCCESS;
        }
        else
        {
            PrintError(Format("Cannot get pathnames from %s", cmdLine[1].c_str()), false);
            return eReturnCode::ERROR;
        }
    }

    PrintError("Wrong command line. Call \"TexDependency --help\" to see help", false);
    return eReturnCode::ERROR;
}
}

int DAVAMain(DAVA::Vector<DAVA::String> cmdLine)
{
    using namespace DAVA;

    Assert::AddHandler(Assert::DefaultLoggerHandler);
    Assert::AddHandler(Assert::DefaultDebuggerBreakHandler);

    Engine e;
    Vector<String> modules;
    e.Init(eEngineRunMode::CONSOLE_MODE, modules, nullptr);

    const EngineContext* context = e.GetContext();
    Logger* logger = context->logger;
    logger->SetLogLevel(Logger::LEVEL__DISABLE);
    logger->EnableConsoleMode();

    e.update.Connect([&e](float32)
                     {
                         DAVA::int32 retCode = TexDependencyDetails::ProcessTexDependency(e);
                         e.QuitAsync(retCode);
                     });

    return e.Run();
}
