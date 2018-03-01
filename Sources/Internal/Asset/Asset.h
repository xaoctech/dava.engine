#pragma once
#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class AssetManager;
class AssetBase;

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

    eState GetState() const;

private:
    DAVA_VIRTUAL_REFLECTION(AssetBase);

    friend class AssetManager;
    eState state = EMPTY;
    Any assetKey;
};

template <typename T>
using Asset = std::shared_ptr<T>;
};
