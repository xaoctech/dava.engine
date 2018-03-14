#pragma once

#include "Asset/Asset.h"
#include "Functional/Function.h"

namespace DAVA
{
class AssetListener
{
public:
    virtual ~AssetListener();
    virtual void OnAssetLoaded(const Asset<AssetBase>& /*asset*/)
    {
    }
    virtual void OnAssetReloaded(const Asset<AssetBase>& /*originalAsset*/, const Asset<AssetBase>& /*reloadedAsset*/)
    {
    }
    virtual void OnAssetError(const Asset<AssetBase>& /*asset*/, bool reloaded, const String& /*msg*/)
    {
    }
    virtual void OnAssetUnloaded(const AssetBase* /*asset*/)
    {
    }

private:
    friend class AssetManager;
    bool instanceListener = false;
};

class SimpleAssetListener : public AssetListener
{
public:
    void OnAssetLoaded(const Asset<AssetBase>& asset) override;
    void OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset) override;
    void OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg) override;
    void OnAssetUnloaded(const AssetBase* asset) override;

    Function<void(const Asset<AssetBase>&)> onLoaded;
    Function<void(const Asset<AssetBase>&, const Asset<AssetBase>&)> onReloaded;
    Function<void(const Asset<AssetBase>&, bool, const String&)> onError;
    Function<void(const AssetBase*)> onUnloaded;
};
} // namespace DAVA
