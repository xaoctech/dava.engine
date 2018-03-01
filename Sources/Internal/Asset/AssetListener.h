#pragma once

#include "Asset/Asset.h"
#include "Functional/Function.h"

namespace DAVA
{
class AssetListener
{
public:
    virtual ~AssetListener();
    virtual void OnAssetLoaded(const Asset<AssetBase>& asset, bool reloaded) = 0;
    virtual void OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg) = 0;
    virtual void OnAssetUnloaded(const Asset<AssetBase>& asset) = 0;

private:
    friend class AssetManager;
    bool instanceListener = false;
};

class SimpleAssetListener : public AssetListener
{
public:
    void OnAssetLoaded(const Asset<AssetBase>& asset, bool reloaded) override;
    void OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg) override;
    void OnAssetUnloaded(const Asset<AssetBase>& asset) override;

    Function<void(const Asset<AssetBase>&, bool)> onLoaded;
    Function<void(const Asset<AssetBase>&, bool, const String&)> onError;
    Function<void(const Asset<AssetBase>&)> onUnloaded;
};
} // namespace DAVA
