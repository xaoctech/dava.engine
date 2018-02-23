#include "Asset/AssetBase.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
AssetDescriptor::AssetDescriptor(const FilePath& assetName_,
                                 AssetLoadedFunction assetLoadedCallback_)
{
    assetName = assetName_;
    assetLoadedCallback = assetLoadedCallback_;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AssetBase)
{
    ReflectionRegistrator<AssetBase>::Begin()
    .ConstructorByPointer()
    .End();
}

AssetBase::AssetBase()
{
}

void AssetBase::SetState(eState state_)
{
    state = state_;
}

AssetBase::eState AssetBase::GetState() const
{
    return state;
}

const FilePath& AssetBase::GetFilepath() const
{
    return descriptor->assetName;
}

void AssetBase::SetAssetDescriptor(AssetDescriptor* assetDescriptor_)
{
    descriptor.reset(assetDescriptor_);
}

AssetDescriptor* AssetBase::GetAssetDescriptor() const
{
    return descriptor.get();
}

const Vector<AssetBase::Dependency>& AssetBase::GetDependencies() const
{
    return dependencies;
}
}
