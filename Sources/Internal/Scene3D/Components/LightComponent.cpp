#include "Scene3D/Components/LightComponent.h"

namespace DAVA 
{

LightComponent::LightComponent(Light * _light)
{
    light = SafeRetain(_light);
}

LightComponent::~LightComponent()
{
    SafeRelease(light);
}
    
void LightComponent::SetLightObject(Light * _light)
{
	SafeRelease(light);
    light = SafeRetain(_light);
}
    
Light * LightComponent::GetLightObject()
{
    return light;
}
    
Component * LightComponent::Clone(SceneNode * toEntity)
{
    LightComponent * component = new LightComponent();
	component->SetEntity(toEntity);
    component->light = (Light*)light->Clone();
    return component;
}


};
