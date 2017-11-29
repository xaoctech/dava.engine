#include "TextureCompression/TextureCompressionModule.h"
#include "TextureCompression/Private/ImageConverterImpl.h"
#include "TextureCompression/PVRConverter.h"

#include <Engine/Engine.h>
#include <Render/Image/ImageConverter.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
TextureCompressionModule::TextureCompressionModule(Engine* engine_)
    : IModule(engine_)
    , engine(engine_)
{
    imageConverter.reset(new ImageConverterImpl());
}

void TextureCompressionModule::Init()
{
    DVASSERT(GetEngineContext()->imageConverter != nullptr);
    GetEngineContext()->imageConverter->SetImplementation(imageConverter.get());

    if (engine->IsConsoleMode())
    {
        const Vector<String>& cmdLine = engine->GetCommandLine();
        if (cmdLine.empty() == false)
        {
            FilePath appPath = cmdLine[0];
            PVRConverter::Instance()->SetPVRTexTool(appPath.GetDirectory() + "Data/PVRTexToolCLI");
        }
        else
        {
            Logger::Error("Cannot setup PVRTexTool pathname");
        }
    }
    else
    {
        PVRConverter::Instance()->SetPVRTexTool("~res:/PVRTexToolCLI");
    }
}

void TextureCompressionModule::Shutdown()
{
    DVASSERT(GetEngineContext()->imageConverter != nullptr);
    GetEngineContext()->imageConverter->SetImplementation(nullptr);
}

DAVA_VIRTUAL_REFLECTION_IMPL(TextureCompressionModule)
{
    ReflectionRegistrator<TextureCompressionModule>::Begin()
    .End();
}
}
