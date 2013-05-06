#include "Scene3D/Components/LightComponent.h"

namespace DAVA 
{
    
REGISTER_CLASS(LightComponent)

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
    
Component * LightComponent::Clone(Entity * toEntity)
{
    LightComponent * component = new LightComponent();
	component->SetEntity(toEntity);
    component->light = (Light*)light->Clone();
    return component;
}

void LightComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);

	if(NULL != archive && NULL != light)
	{
		KeyedArchive *lightArch = new KeyedArchive();
		light->Save(lightArch, sceneFile);

		archive->SetArchive("lc.light", lightArch);

		lightArch->Release();
	}
}

void LightComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		KeyedArchive *lightArch = archive->GetArchive("lc.light");
		if(NULL != lightArch)
		{
			Light* l = new Light();
			l->Load(lightArch, sceneFile);
			SetLightObject(l);
			l->Release();
		}
	}

	Component::Deserialize(archive, sceneFile);
}


};
