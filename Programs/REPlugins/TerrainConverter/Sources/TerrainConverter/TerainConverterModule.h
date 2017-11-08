#pragma once

#include <REPlatform/Global/CommandLineModule.h>

#include <TArc/Core/ClientModule.h>

#include <Base/Type.h>
#include <FileSystem/FilePath.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class REFileOperation;
} // namespace DAVA

class TerrainConverterGUIModule : public DAVA::ClientModule
{
public:
    TerrainConverterGUIModule();

    void PostInit() override;

protected:
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void OnConvertScene(const DAVA::FilePath& scenePath);

private:
    std::shared_ptr<DAVA::REFileOperation> convertOperation;
    DAVA_VIRTUAL_REFLECTION(TerrainConverterGUIModule, DAVA::ClientModule);
};

class TerrainConverterConsoleModule : public DAVA::CommandLineModule
{
public:
    TerrainConverterConsoleModule(const DAVA::Vector<DAVA::String>& commandLine);

    bool PostInitInternal() override;
    eFrameResult OnFrameInternal() override;
    void ShowHelpInternal() override;

private:
    DAVA::FilePath mapPath;
    DAVA::String projectPath;
    DAVA::String templatesPath;
    DAVA_VIRTUAL_REFLECTION(TerrainConverterConsoleModule, DAVA::CommandLineModule);
};