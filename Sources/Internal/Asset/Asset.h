#pragma once
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include <Concurrency/Atomic.h>

namespace DAVA
{
class AssetManager;
class AssetBase;

template <typename T>
using Asset = std::shared_ptr<T>;

class AssetBase : public ReflectionBase
{
public:
    enum eState
    {
        EMPTY,
        QUEUED,
        LOADED,
        OUT_OF_DATE,
        ERROR,
    };

    AssetBase(const Any& assetKey);
    virtual ~AssetBase() = default;

    const Any& GetKey() const;

    eState GetState() const;
    const Any& GetAssetKey() const;

    template <class T>
    const T& GetAssetKey() const;

private:
    DAVA_VIRTUAL_REFLECTION(AssetBase);

    friend class AssetManager;
    friend class AbstractAssetLoader;
    Atomic<eState> state;
    Any assetKey;
};

template <class T>
const T& AssetBase::GetAssetKey() const
{
    return GetAssetKey().Get<T>();
}

template <typename T>
Asset<T> AssetAs(const Asset<AssetBase>& asset)
{
    return std::dynamic_pointer_cast<T>(asset);
}

template <typename T>
T* AssetAs(const AssetBase* asset)
{
    return dynamic_cast<T*>(asset);
}
};
