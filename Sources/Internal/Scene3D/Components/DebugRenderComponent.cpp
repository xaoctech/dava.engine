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
	component->SetEntity(toEntity);
    component->debugFlags = debugFlags;
    return component;
}

void DebugRenderComponent::Serialize(KeyedArchive *archive)
{
	Component::Serialize(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("drc.flags", debugFlags);
	}
}

void DebugRenderComponent::Deserialize(KeyedArchive *archive)
{
	if(NULL != archive)
	{
		if(archive->IsKeyExists("drc.flags")) debugFlags = archive->GetUInt32("drc.flags");
	}

	Component::Deserialize(archive);
}

};
