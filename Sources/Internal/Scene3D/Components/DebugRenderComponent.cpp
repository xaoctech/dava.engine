#include "Scene3D/Components/DebugRenderComponent.h"

namespace DAVA 
{

DebugRenderComponent::DebugRenderComponent()
    : debugFlags(0)
{
}

DebugRenderComponent::~DebugRenderComponent()
{
}
    
void DebugRenderComponent::SetDebugFlags(uint32 _debugFlags)
{
    debugFlags = _debugFlags;
}
    
uint32 DebugRenderComponent::GetDebugFlags()
{
    return debugFlags;
}
    
Component * DebugRenderComponent::Clone(SceneNode * toEntity)
{
    DebugRenderComponent * component = new DebugRenderComponent();
	SetEntity(toEntity);
    component->debugFlags = debugFlags;
    return component;
}


};
