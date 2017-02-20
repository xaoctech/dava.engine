#include "UISpineComponent.h"

namespace DAVA
{

UISpineComponent* UISpineComponent::Clone() const
{
    return new UISpineComponent(*this);
}

}
