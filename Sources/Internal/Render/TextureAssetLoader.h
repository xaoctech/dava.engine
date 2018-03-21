#pragma once

#include "Asset/AbstractAssetLoader.h"
#include "Asset/Asset.h"

#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Concurrency/Mutex.h"
#include "Render/RHI/rhi_Type.h"

namespace DAVA
{
class TextureAssetLoader final : public AbstractAssetLoader
{
public:
    TextureAssetLoader();

    AssetFileInfo GetAssetFileInfo(const Any& assetKey) const override;
    bool ExistsOnDisk(const Any& assetKey) const override;

    AssetBase* CreateAsset(const Any& assetKey) const override;
    void DeleteAsset(AssetBase* asset) const override;
    void LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const override;
    bool SaveAsset(Asset<AssetBase>, File* file, eSaveMode requestedMode) const override;
    bool SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const override;
    Vector<String> GetDependsOnFiles(const AssetBase* asset) const override;
    Vector<const Type*> GetAssetKeyTypes() const override;

    void SetGPULoadingOrder(const Vector<eGPUFamily>& gpuLoadingOrder);
    const Vector<eGPUFamily>& GetGPULoadingOrder() const;
    eGPUFamily GetPrimaryGPUForLoading() const;
    static uint32 GetBaseMipMap();

private:
    AssetFileInfo GetPathKeyFileInfo(const Any& assetKey) const;
    void LoadPathKeyAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMsg) const;
    Vector<String> GetPathKeyDependsOn(const AssetBase* asset) const;

    AssetFileInfo GetUniqueTextureKeyFileInfo(const Any& assetKey) const;
    void LoadUniqueTextureKeyAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMsg) const;
    Vector<String> GetUniqueTextureKeyDependsOn(const AssetBase* asset) const;

    AssetFileInfo GetRenderTargetTextureKeyFileInfo(const Any& assetKey) const;
    void LoadRenderTargetTextureKeyAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMsg) const;
    Vector<String> GetRenderTargetTextureKeyDependsOn(const AssetBase* asset) const;

    void FlushDataToRenderer(const Asset<Texture>& asset, const Vector<RefPtr<Image>>& images) const;
    void MakePink(const Asset<Texture>& asset, rhi::TextureType requestedType, bool checkers = true) const;

    bool LoadFromImage(const Asset<Texture>& asset, eGPUFamily gpu) const;
    void RestoreRenderResource(Texture* texture) const;

    struct SubKeySupport
    {
        Function<AssetFileInfo(const Any&)> getFileInfo;
        Function<void(Asset<AssetBase>, File*, bool, String&)> loadAsset;
        Function<Vector<String>(const AssetBase*)> getDependOn;
    };

    UnorderedMap<const Type*, SubKeySupport> keySupportMap;

    Mutex gpuOrderMutex;
    Vector<eGPUFamily> gpuLoadingOrder;

    DAVA_VIRTUAL_REFLECTION(TextureAssetLoader, AbstractAssetLoader);
};
} // namespace DAVA
