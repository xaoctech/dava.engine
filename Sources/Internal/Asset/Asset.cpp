#include "Asset/Asset.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
AssetBase::AssetBase(const Any& assetKey_)
    : assetKey(assetKey_)
    , state(EMPTY)
{
}

AssetBase::eState AssetBase::GetState() const
{
    return state.Get();
}

DAVA_VIRTUAL_REFLECTION_IMPL(AssetBase)
{
    ReflectionRegistrator<AssetBase>::Begin()
    .End();
}
}
