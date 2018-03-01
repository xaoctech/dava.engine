#include "Asset/Asset.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
AssetBase::AssetBase(const Any& assetKey_)
    : assetKey(assetKey_)
{
}

AssetBase::eState AssetBase::GetState() const
{
    return state;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AssetBase)
{
    ReflectionRegistrator<AssetBase>::Begin()
    .End();
}
}
