#include "TArc/PluginsManager/TArcPlugin.h"
#include "TArc/DataProcessing/TArcAnyCasts.h"

#include <Base/FastName.h>
#include <Base/Type.h>
#include <Engine/EngineContext.h>
#include <Engine/Private/EngineBackend.h>
#include <Reflection/ReflectedTypeDB.h>
#include <ReflectionDeclaration/Private/AnyCasts.h>
#include <Render/RHI/rhi_Public.h>
#include <Render/Renderer.h>

namespace DAVA
{
TArcPlugin::TArcPlugin(const EngineContext* context)
{
    DAVA::Private::SetEngineContext(const_cast<EngineContext*>(context));
    TypeDB::GetLocalDB()->SetMasterDB(context->typeDB);
    FastNameDB::GetLocalDB()->SetMasterDB(context->fastNameDB);
    ReflectedTypeDB::GetLocalDB()->SetMasterDB(context->reflectedTypeDB);

    DAVA::RegisterAnyCasts();
    DAVA::RegisterTArcAnyCasts();

    rhi::InitParam params;
    params.maxIndexBufferCount = 8192;
    params.maxVertexBufferCount = 8192;
    params.maxConstBufferCount = 32767;
    params.maxTextureCount = 2048;
    params.maxSamplerStateCount = 32 * 1024;
    params.shaderConstRingBufferSize = 256 * 1024 * 1024;
    Renderer::Initialize(rhi::RHI_NULL_RENDERER, params);
}
} // namespace DAVA