#include "Render/Material/NMaterialManager.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterial.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
class NMaterialManager::Private
{
public:
    Set<NMaterial*> allMaterials;
    UnorderedMap<FastName, int32> globalFlags;
    Map<uint64, DAVA::Function<void()>> callbacks;
};

NMaterialManager& NMaterialManager::Instance()
{
    static NMaterialManager instance;
    return instance;
}

NMaterialManager::NMaterialManager()
{
    _impl = new (_implStorage) NMaterialManager::Private();
}

NMaterialManager::~NMaterialManager()
{
    _impl->~Private();
    _impl = nullptr;
}

void NMaterialManager::RegisterMaterial(NMaterial* material)
{
    _impl->allMaterials.insert(material);
    UpdateMaterialHierarchy(material);
}

void NMaterialManager::UnregisterMaterial(NMaterial* material)
{
    _impl->allMaterials.erase(material);
}

void NMaterialManager::UpdateMaterialHierarchy(NMaterial* material)
{
    /* TODO or NOT TODO */
}

void NMaterialManager::SetGlobalFlag(const FastName& name, int32 value)
{
    //GFX_COMPLETE NMaterialManager singleton thats sets flags for all materials in all scenes seems something wired
    if (_impl->globalFlags[name] != value)
    {
        _impl->globalFlags[name] = value;
        InvalidateMaterials();
    }
}

void NMaterialManager::RemoveGlobalFlag(const FastName& name)
{
    _impl->globalFlags.erase(name);
    InvalidateMaterials();
}

void NMaterialManager::InvalidateMaterials()
{
    for (NMaterial* m : _impl->allMaterials)
        m->InvalidateRenderVariants();

    for (const auto& cb : _impl->callbacks)
        (cb.second)();
}

void NMaterialManager::MergeGlobalFlags(UnorderedMap<FastName, int32>& flags)
{
    for (const auto& kv : _impl->globalFlags)
    {
        flags[kv.first] = kv.second;
    }
}

uint64 NMaterialManager::RegisterInvalidateCallback(DAVA::Function<void()> cb)
{
    uint64 cbHash = reinterpret_cast<uint64>(&cb) ^ reinterpret_cast<uint64>(this);
    cbHash ^= static_cast<uint64>(SystemTimer::GetUs());
    _impl->callbacks.emplace(cbHash, cb);
    return cbHash;
}

void NMaterialManager::UnregisterInvalidateCallback(uint64 callbackId)
{
    _impl->callbacks.erase(callbackId);
}
}
