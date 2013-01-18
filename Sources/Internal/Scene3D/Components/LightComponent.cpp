#include "Scene3D/Components/LightComponent.h"

namespace DAVA 
{

LightComponent::LightComponent(LightNode * _light)
{
    light = SafeRetain(_light);
}

LightComponent::~LightComponent()
{
    SafeRelease(light);
}
    
void LightComponent::SetLightObject(LightNode * _light)
{
    light = SafeRetain(_light);
}
    
LightNode * LightComponent::GetLightObject()
{
    return light;
}
    
Component * LightComponent::Clone()
{
    LightComponent * component = new LightComponent();
    component->light = SafeRetain(light);
    return component;
}


};
