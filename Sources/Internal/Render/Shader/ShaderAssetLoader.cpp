#include "Render/Shader/ShaderAssetLoader.h"
#include "Render/Shader/ShaderDescriptor.h"
#include "Render/RHI/rhi_ShaderCache.h"

#include "Asset/AssetManager.h"
#include "Base/Hash.h"
#include "Concurrency/LockGuard.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace ShaderAssetLoaderDetail
{
size_t KeyHash(const Any& v)
{
    const ShaderAssetLoader::Key& key = v.Get<ShaderAssetLoader::Key>();
    return key.shaderKeyHash;
}
} // namespace ShaderAssetLoaderDetail

ShaderAssetLoader::ShaderAssetLoader()
{
    AnyHash<ShaderAssetLoader::Key>::Register(&ShaderAssetLoaderDetail::KeyHash);
}

AssetFileInfo ShaderAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<Key>());
    const Key& k = assetKey.Get<Key>();

    AssetFileInfo info;
    if (k.name.IsValid() == false)
    {
        return info;
    }

    info.fileName = k.name.c_str();
    info.inMemoryAsset = true;
    return info;
}

AssetBase* ShaderAssetLoader::CreateAsset(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<Key>());
    ShaderDescriptor* res = new ShaderDescriptor(assetKey);
    res->valid = false;
    return res;
}

void ShaderAssetLoader::DeleteAsset(AssetBase* asset) const
{
    delete asset;
}

#define DUMP_SOURCES 0
#define TRACE_CACHE_USAGE 0

#if TRACE_CACHE_USAGE
#define LOG_TRACE_USAGE(usage, ...) Logger::Info(usage, __VA_ARGS__)
#else
#define LOG_TRACE_USAGE(...)
#endif

void ShaderAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const
{
    DVASSERT(file == nullptr);

    Asset<ShaderDescriptor> descr = std::dynamic_pointer_cast<ShaderDescriptor>(asset);
    DVASSERT(descr != nullptr);

    const Key& k = descr->GetKey().Get<Key>();

    Vector<String> progDefines;
    String resName = BuildResourceName(k, progDefines);

    FastName vProgUid, fProgUid;
    vProgUid = FastName(String("vSource: ") + resName);
    fProgUid = FastName(String("fSource: ") + resName);

    const ShaderSourceCode& sourceCode = GetSourceCode(k.name, reloading, errorMessage);
    if (errorMessage.empty() == false)
    {
        return;
    }

    const rhi::ShaderSource* vSource = reloading == true ? nullptr : rhi::ShaderSourceCache::Get(vProgUid, sourceCode.vSrcHash);
    const rhi::ShaderSource* fSource = reloading == true ? nullptr : rhi::ShaderSourceCache::Get(fProgUid, sourceCode.fSrcHash);

    bool isCachedShader = false;
    if (!vSource || !fSource)
    {
        LOG_TRACE_USAGE("building \"%s\"", vProgUid.c_str());
        vSource = rhi::ShaderSourceCache::Add(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str(), vProgUid, rhi::PROG_VERTEX, sourceCode.vertexProgText.data(), progDefines);
        fSource = rhi::ShaderSourceCache::Add(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str(), fProgUid, rhi::PROG_FRAGMENT, sourceCode.fragmentProgText.data(), progDefines);
    }
    else
    {
        LOG_TRACE_USAGE("using cached \"%s\"", vProgUid.c_str());
        isCachedShader = true;
    }

    if (!vSource || !fSource)
    {
        if (!vSource)
        {
            errorMessage = Format("failed to construct vSource for \"%s\"", vProgUid.c_str());
            return;
        }
        if (!fSource)
        {
            errorMessage = Format("failed to construct vSource for \"%s\"", vProgUid.c_str());
            return;
        }
    }

#if DUMP_SOURCES
    Logger::Info("\n\n%s", vProgUid.c_str());
    vSource->Dump();
    Logger::Info("\n\n%s", fProgUid.c_str());
    fSource->Dump();
#endif

    const std::string& vpBin = vSource->GetSourceCode(rhi::HostApi());
    rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vpBin.c_str(), unsigned(vpBin.length()));
    const std::string& fpBin = fSource->GetSourceCode(rhi::HostApi());
    rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fpBin.c_str(), unsigned(fpBin.length()));
    //ShaderDescr
    rhi::PipelineState::Descriptor psDesc;
    psDesc.vprogUid = vProgUid;
    psDesc.fprogUid = fProgUid;
    psDesc.vertexLayout = vSource->ShaderVertexLayout();
    psDesc.blending = fSource->Blending();
    rhi::HPipelineState piplineState = rhi::AcquireRenderPipelineState(psDesc);

    //in case we have broken shaders in cache, replace them with newly compiled
    if ((!piplineState.IsValid()) && isCachedShader)
    {
        Logger::Info("cached shader compilation failed ");
        Logger::Info("  vprog-uid = %s", vProgUid.c_str());
        Logger::Info("  fprog-uid = %s", fProgUid.c_str());
        Logger::Info("trying to replace from source files");

        vSource = rhi::ShaderSourceCache::Add(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str(), vProgUid, rhi::PROG_VERTEX, sourceCode.vertexProgText.data(), progDefines);
        fSource = rhi::ShaderSourceCache::Add(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str(), fProgUid, rhi::PROG_FRAGMENT, sourceCode.fragmentProgText.data(), progDefines);

        const std::string& vpBin = vSource->GetSourceCode(rhi::HostApi());
        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vpBin.c_str(), unsigned(vpBin.length()));
        const std::string& fpBin = fSource->GetSourceCode(rhi::HostApi());
        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fpBin.c_str(), unsigned(fpBin.length()));

        psDesc.vprogUid = vProgUid;
        psDesc.fprogUid = fProgUid;
        psDesc.vertexLayout = vSource->ShaderVertexLayout();
        psDesc.blending = fSource->Blending();
        piplineState = rhi::AcquireRenderPipelineState(psDesc);
    }

    descr->valid = piplineState.IsValid(); //later add another conditions
    if (descr->valid)
    {
        descr->piplineState = piplineState;
        descr->UpdateConfigFromSource(const_cast<rhi::ShaderSource*>(vSource), const_cast<rhi::ShaderSource*>(fSource));
        descr->requiredVertexFormat = GetVertexLayoutRequiredFormat(psDesc.vertexLayout);
    }
    else
    {
        errorMessage = "failed to get pipeline-state";
        Logger::Info("  vprog-uid = %s", vProgUid.c_str());
        Logger::Info("  fprog-uid = %s", fProgUid.c_str());
    }

#if TRACE_SHADER_CONST_BUFFERS
    Logger::Info("Shader vertex const buffers for %s", vProgUid.c_str());
    for (int32 i = 0, sz = vSource->ConstBufferCount(); i < sz; ++i)
    {
        String s;
        for (const rhi::ShaderProp& prop : vSource->Properties())
        {
            if (prop.bufferindex == i)
            {
                s += prop.uid.c_str();
                s += " ";
            }
        }
        Logger::Info(" %d -- %s : [%s]", i, vSource->ConstBufferTag(i).c_str(), s.c_str());
    }

    Logger::Info("Shader fragment const buffers for %s", fProgUid.c_str());
    for (int32 i = 0, sz = fSource->ConstBufferCount(); i < sz; ++i)
    {
        String s;
        for (const rhi::ShaderProp& prop : fSource->Properties())
        {
            if (prop.bufferindex == i)
            {
                s += prop.uid.c_str();
                s += " ";
            }
        }
        Logger::Info(" %d -- %s : [%s]", i, fSource->ConstBufferTag(i).c_str(), s.c_str());
    }
#endif
}

bool ShaderAssetLoader::SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const
{
    return false;
}

bool ShaderAssetLoader::SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const
{
    return false;
}

Vector<String> ShaderAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    const Any& assetKey = asset->GetKey();
    DVASSERT(assetKey.CanGet<Key>());
    const Key& k = assetKey.Get<Key>();

    String error;
    const ShaderSourceCode& source = GetSourceCode(k.name, false, error);
    Vector<String> result;
    result.push_back(source.fragmentProgSourcePath.GetAbsolutePathname());
    result.push_back(source.vertexProgSourcePath.GetAbsolutePathname());

    return result;
}

Vector<const Type*> ShaderAssetLoader::GetAssetKeyTypes() const
{
    return Vector<const Type*>{ Type::Instance<Key>() };
}

Vector<const Type*> ShaderAssetLoader::GetAssetTypes() const
{
    return Vector<const Type*>{ Type::Instance<ShaderDescriptor>() };
}

size_t ShaderAssetLoader::GetUniqueFlagKey(FastName flagName)
{
    static_assert(sizeof(size_t) == sizeof(void*), "Cant cast `const char*` into `int`");
    return reinterpret_cast<size_t>(flagName.c_str());
}

String ShaderAssetLoader::BuildResourceName(const Key& key, Vector<String>& progDefines)
{
    progDefines.reserve(key.defines.size() * 2);
    String resName(key.name.c_str());
    resName += "  defines: ";
    for (auto& it : key.defines)
    {
        bool doAdd = true;

        for (size_t i = 0; i != progDefines.size(); i += 2)
        {
            if (strcmp(it.first.c_str(), progDefines[i].c_str()) < 0)
            {
                progDefines.insert(progDefines.begin() + i, String(it.first.c_str()));
                progDefines.insert(progDefines.begin() + i + 1, Format("%d", it.second));
                doAdd = false;
                break;
            }
        }

        if (doAdd)
        {
            progDefines.push_back(String(it.first.c_str()));
            progDefines.push_back(Format("%d", it.second));
        }
    }

    for (size_t i = 0; i != progDefines.size(); i += 2)
    {
        resName += Format("%s = %s, ", progDefines[i + 0].c_str(), progDefines[i + 1].c_str());
    }

    return resName;
}

Vector<size_t> ShaderAssetLoader::BuildFlagsKey(const FastName& name, const UnorderedMap<FastName, int32>& defines)
{
    Vector<size_t> key;

    key.reserve(defines.size() * 2 + 1);
    for (const auto& define : defines)
    {
        key.emplace_back(GetUniqueFlagKey(define.first));
        key.emplace_back(define.second);
    }

    // reinterpret cast to pairs and sort them
    using SizeTPair = std::pair<size_t, size_t>;
    SizeTPair* begin = reinterpret_cast<SizeTPair*>(key.data());
    std::sort(begin, begin + key.size() / 2, [](const SizeTPair& l, const SizeTPair& r) {
        return l.first < r.first;
    });

    key.push_back(GetUniqueFlagKey(name));
    return key;
}

const ShaderAssetLoader::ShaderSourceCode& ShaderAssetLoader::GetSourceCode(const FastName& name, bool reloading, String& error) const
{
    return const_cast<ShaderAssetLoader*>(this)->GetSourceCodeImpl(name, reloading, error);
}

const ShaderAssetLoader::ShaderSourceCode& ShaderAssetLoader::GetSourceCodeImpl(const FastName& name, bool reloading, String& error)
{
    LockGuard<Mutex> guard(const_cast<ShaderAssetLoader*>(this)->mutex);
    if (reloading == true)
    {
        shaderSourceCodes.erase(name);
    }
    else
    {
        auto sourceIt = shaderSourceCodes.find(name);
        if (sourceIt != shaderSourceCodes.end())
        {
            return sourceIt->second;
        }
    }

    LoadFromSource(name.c_str(), shaderSourceCodes[name], error);
    return shaderSourceCodes.at(name);
}

void ShaderAssetLoader::LoadFromSource(const String& source, ShaderAssetLoader::ShaderSourceCode& sourceCode, String& error)
{
    sourceCode.vertexProgSourcePath = FilePath(source + "-vp.sl");
    sourceCode.fragmentProgSourcePath = FilePath(source + "-fp.sl");

    if (LoadShaderSource(sourceCode.vertexProgSourcePath.GetAbsolutePathname(), sourceCode.vertexProgText, error) == false)
    {
        return;
    }

    if (LoadShaderSource(sourceCode.fragmentProgSourcePath.GetAbsolutePathname(), sourceCode.fragmentProgText, error) == false)
    {
        return;
    }

    sourceCode.vSrcHash = HashValue_N(sourceCode.vertexProgText.data(), static_cast<uint32>(strlen(sourceCode.vertexProgText.data())));
    sourceCode.fSrcHash = HashValue_N(sourceCode.fragmentProgText.data(), static_cast<uint32>(strlen(sourceCode.fragmentProgText.data())));
}

bool ShaderAssetLoader::LoadShaderSource(const String& source, Vector<char>& shaderText, String& error)
{
    ScopedPtr<File> fp(File::Create(source, File::OPEN | File::READ));
    if (fp)
    {
        uint32 fileSize = static_cast<uint32>(fp->GetSize());
        shaderText.resize(fileSize + 1);
        shaderText[fileSize] = 0;
        uint32 dataRead = fp->Read(shaderText.data(), fileSize);
        if (dataRead == fileSize)
        {
            return true;
        }
        else
        {
            shaderText.resize(1);
            shaderText.shrink_to_fit();
        }
    }
    error = Format("Failed to open vertex shader source file: %s", source.c_str());
    return false;
}

ShaderAssetLoader::Key::Key(const FastName& name_, const UnorderedMap<FastName, int32>& inputDefines)
    : name(name_)
    , defines(inputDefines)
{
    Renderer::GetRuntimeFlags().AddToDefines(defines);
    shaderKey = BuildFlagsKey(name, inputDefines);

    for (size_t v : shaderKey)
    {
        HashCombine<size_t>(shaderKeyHash, v);
    }
}

void ShaderAssetListener::Init()
{
    listener.onLoaded = MakeFunction(this, &ShaderAssetListener::OnAssetLoaded);
    listener.onReloaded = MakeFunction(this, &ShaderAssetListener::OnAssetReloaded);
    listener.onError = MakeFunction(this, &ShaderAssetListener::OnAssetError);

    GetEngineContext()->assetManager->RegisterListener(&listener, Type::Instance<ShaderDescriptor>());
}

void ShaderAssetListener::Shoutdown()
{
    SetShadersCleanupEnabled(false);
    GetEngineContext()->assetManager->UnregisterListener(&listener);
    shaders.clear();
}

void ShaderAssetListener::ClearDynamicBindigs()
{
    for (Asset<ShaderDescriptor> shader : shaders)
    {
        shader->ClearDynamicBindings();
    }
}

void ShaderAssetListener::SetShadersCleanupEnabled(bool enabled)
{
    if (cleanupEnabled == enabled)
    {
        return;
    }

    Engine* engine = Engine::Instance();

    if (cleanupEnabled == true)
    {
        engine->endFrame.Disconnect(this);
    }

    cleanupEnabled = enabled;

    if (cleanupEnabled == true)
    {
        engine->endFrame.Connect(this, &ShaderAssetListener::Cleanup);
    }
}

void ShaderAssetListener::SetLoadingNotifyEnabled(bool enable)
{
    loadingNotify = enable;
}

void ShaderAssetListener::OnAssetLoaded(const Asset<AssetBase>& asset)
{
    shaders.emplace(std::dynamic_pointer_cast<ShaderDescriptor>(asset));
}

void ShaderAssetListener::OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset)
{
    shaders.emplace(std::dynamic_pointer_cast<ShaderDescriptor>(reloadedAsset));
}

void ShaderAssetListener::OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg)
{
    shaders.emplace(std::dynamic_pointer_cast<ShaderDescriptor>(asset));
    if (loadingNotify == true)
    {
        ShaderAssetLoader::Key k = asset->GetKey().Get<ShaderAssetLoader::Key>();

        Vector<String> progDefines;
        String resName = ShaderAssetLoader::BuildResourceName(k, progDefines);
        Logger::Error("Forbidden call to GetShaderDescriptor %s", resName.c_str());
    }
}

void ShaderAssetListener::Cleanup()
{
    auto iter = shaders.begin();
    while (iter != shaders.end())
    {
        Asset<AssetBase> shader = *iter;
        if (shader.unique() == true)
        {
            iter = shaders.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

template <>
bool AnyCompare<ShaderAssetLoader::Key>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<ShaderAssetLoader::Key>().shaderKey == v2.Get<ShaderAssetLoader::Key>().shaderKey;
}
} // namespace DAVA
