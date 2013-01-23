#include "Scene3D/Components/RenderComponent.h"

namespace DAVA 
{

RenderComponent::RenderComponent(RenderObject * _object)
{
    renderObject = SafeRetain(_object);
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
    component->renderObject = renderObject->Clone();
    return component;
}


};
