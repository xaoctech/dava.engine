#include "Scene3D/Components/RenderComponent.h"

namespace DAVA 
{

RenderComponent::RenderComponent()
{
    renderObject = 0;
}

RenderComponent::~RenderComponent()
{
    SafeRelease(renderObject);
}
    
void RenderComponent::SetRenderObject(RenderObject * _renderObject)
{
    renderObject = SafeRetain(_renderObject);
}
    
RenderObject * RenderComponent::GetRenderObject()
{
    return renderObject;
}
    
Component * RenderComponent::Clone()
{
    RenderComponent * component = new RenderComponent();
    component->renderObject = SafeRetain(renderObject);
    return component;
}


};
