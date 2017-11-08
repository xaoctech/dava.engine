#include "TextureCompression/TextureCompressionModule.h"
#include "TextureCompression/Private/ImageConverterImpl.h"

#include <Engine/Engine.h>
#include <Render/Image/ImageConverter.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
TextureCompressionModule::TextureCompressionModule(Engine* engine)
    : IModule(engine)
{
    imageConverter.reset(new ImageConverterImpl());
}

void TextureCompressionModule::Init()
{
    DVASSERT(GetEngineContext()->imageConverter != nullptr);
    GetEngineContext()->imageConverter->SetImplementation(imageConverter.get());
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
