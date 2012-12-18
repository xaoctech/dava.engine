#include "Scene3D/Components/RenderComponent.h"

namespace DAVA 
{

RenderComponent::RenderComponent()
{
    
}

RenderComponent::~RenderComponent()
{
    
}
    
RenderObject * RenderComponent::GetRenderObject()
{
    return renderObject;
}


};
