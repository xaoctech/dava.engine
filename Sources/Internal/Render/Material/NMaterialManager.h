#pragma once

#include <Base/FastName.h>
#include <Functional/Function.h>

namespace DAVA
{
class NMaterial;
class NMaterialManager
{
public:
    static NMaterialManager& Instance();

public:
    NMaterialManager();
    ~NMaterialManager();

    void RegisterMaterial(NMaterial* material);
    void UnregisterMaterial(NMaterial* material);
    void UpdateMaterialHierarchy(NMaterial* material);

    void SetGlobalFlag(const FastName& name, int32 value);
    void RemoveGlobalFlag(const FastName& name);

    void InvalidateMaterials();

    void MergeGlobalFlags(UnorderedMap<FastName, int32>& flags);

    uint64 RegisterInvalidateCallback(DAVA::Function<void()> cb);
    void UnregisterInvalidateCallback(uint64 callbackId);

private:
    class Private;
    Private* _impl = nullptr;
    char _implStorage[2048];
};
}
