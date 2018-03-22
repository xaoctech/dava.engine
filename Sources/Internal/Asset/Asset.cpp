#include "Asset/Asset.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
AssetBase::AssetBase(const Any& assetKey_)
    : assetKey(assetKey_)
    , state(EMPTY)
{
}

const DAVA::Any& AssetBase::GetKey() const
{
    return assetKey;
}

AssetBase::eState AssetBase::GetState() const
{
    return state.Get();
}

const Any& AssetBase::GetAssetKey() const
{
    return assetKey;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AssetBase)
{
    ReflectionRegistrator<AssetBase>::Begin()
    .End();
}
}
