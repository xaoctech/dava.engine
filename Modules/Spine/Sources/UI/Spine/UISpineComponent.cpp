#include "UISpineComponent.h"

namespace DAVA
{

UISpineComponent::UISpineComponent(const UISpineComponent& copy)
    : skeletonPath(copy.skeletonPath)
    , atlasPath(copy.atlasPath)
{
}

UISpineComponent* UISpineComponent::Clone() const
{
    return new UISpineComponent(*this);
}

}
