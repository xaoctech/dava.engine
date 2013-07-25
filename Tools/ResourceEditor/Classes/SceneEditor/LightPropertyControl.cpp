#include "LightPropertyControl.h"


LightPropertyControl::LightPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
	types.push_back("Directional");
	types.push_back("Spot");
	types.push_back("Point");
	types.push_back("Sky");
}

LightPropertyControl::~LightPropertyControl()
{

}

void LightPropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    Light *light = GetLight(sceneNode);
	DVASSERT(light);

    propertyList->AddSection("property.lightnode.light", GetHeaderState("property.lightnode.light", true));
        
    propertyList->AddComboProperty("property.lightnode.type", types);
	propertyList->SetComboPropertyIndex("property.lightnode.type", light->GetType());

    propertyList->AddColorProperty("property.lightnode.ambient.color");
	propertyList->SetColorPropertyValue("property.lightnode.ambient.color", light->GetAmbientColor());

    propertyList->AddColorProperty("property.lightnode.diffuse.color");
	propertyList->SetColorPropertyValue("property.lightnode.diffuse.color", light->GetDiffuseColor());

    propertyList->AddColorProperty("property.lightnode.specular.color");
	propertyList->SetColorPropertyValue("property.lightnode.specular.color", light->GetSpecularColor());
    
    propertyList->AddFloatProperty("property.lightnode.intensity");
	propertyList->SetFloatPropertyValue("property.lightnode.intensity", light->GetIntensity());

    //propertyList->AddFloatProperty("property.lightnode.material.shininess", light->GetShininess())
    
	propertyList->AddSection("property.lightnode.staticlight", GetHeaderState("property.lightnode.staticlight", true));

    propertyList->AddBoolProperty("property.staticlight.enable");
	propertyList->SetBoolPropertyValue("property.staticlight.enable", sceneNode->GetCustomProperties()->GetBool("editor.staticlight.enable", true));

	propertyList->AddBoolProperty("Cast shadows");
	propertyList->SetBoolPropertyValue("Cast shadows", sceneNode->GetCustomProperties()->GetBool("editor.staticlight.castshadows", true));

	propertyList->AddFloatProperty("Intensity");
	propertyList->SetFloatPropertyValue("Intensity", sceneNode->GetCustomProperties()->GetFloat("editor.intensity", 1.f));

	propertyList->AddFloatProperty("Falloff cutoff");
	propertyList->SetFloatPropertyValue("Falloff cutoff", sceneNode->GetCustomProperties()->GetFloat("editor.staticlight.falloffcutoff", 1000.f));

	propertyList->AddFloatProperty("Falloff exponent");
	propertyList->SetFloatPropertyValue("Falloff exponent", sceneNode->GetCustomProperties()->GetFloat("editor.staticlight.falloffexponent", 1.f));

	if(Light::TYPE_DIRECTIONAL == light->GetType())
	{
		propertyList->AddFloatProperty("Shadow angle");
		propertyList->SetFloatPropertyValue("Shadow angle", sceneNode->GetCustomProperties()->GetFloat("editor.staticlight.shadowangle", 0.f));

		propertyList->AddIntProperty("Shadow samples");
		propertyList->SetIntPropertyValue("Shadow samples", sceneNode->GetCustomProperties()->GetInt32("editor.staticlight.shadowsamples", 1));
	}
	else if(Light::TYPE_POINT == light->GetType())
	{
		propertyList->AddFloatProperty("Shadow radius");
		propertyList->SetFloatPropertyValue("Shadow radius", sceneNode->GetCustomProperties()->GetFloat("editor.staticlight.shadowradius", 0.f));
	}

	propertyList->AddSection("property.lightnode.dynamiclight", GetHeaderState("property.lightnode.dynamiclight", true));
	
	propertyList->AddBoolProperty("property.dynamiclight.enable");
	propertyList->SetBoolPropertyValue("property.dynamiclight.enable", sceneNode->GetCustomProperties()->GetBool("editor.dynamiclight.enable", true));
}

void LightPropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("property.lightnode.type" == forKey)
    {
        Light *light = GetLight(currentSceneNode);
        light->SetType((Light::eType)newItemIndex);
        
        if(Light::TYPE_DIRECTIONAL == light->GetType())
        {
            if(propertyList->IsPropertyAvaliable("Shadow angle"))
            {
                currentSceneNode->GetCustomProperties()->SetFloat("editor.staticlight.shadowangle", propertyList->GetFloatPropertyValue("Shadow angle"));
            }
            
            if(propertyList->IsPropertyAvaliable("Shadow samples"))
            {
                currentSceneNode->GetCustomProperties()->SetInt32("editor.staticlight.shadowsamples", propertyList->GetIntPropertyValue("Shadow samples"));
            }
        }
    }

    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LightPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
	Light *light = GetLight(currentSceneNode);

	if("property.staticlight.enable" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetBool("editor.staticlight.enable", newValue);
	}
	else if("property.dynamiclight.enable" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetBool("editor.dynamiclight.enable", newValue);
		light->SetDynamic(newValue);
	}
	else if("Cast shadows" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetBool("editor.staticlight.castshadows", newValue);
	}

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void LightPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
	Light *light = GetLight(currentSceneNode);

	if("Intensity" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetFloat("editor.intensity", newValue);
	}
	else if("Shadow angle" == forKey)
	{
		if(Light::TYPE_DIRECTIONAL == light->GetType())
		{
			currentSceneNode->GetCustomProperties()->SetFloat("editor.staticlight.shadowangle", newValue);
		}
	}
	else if("Shadow radius" == forKey)
	{
		if(Light::TYPE_POINT == light->GetType())
		{
			currentSceneNode->GetCustomProperties()->SetFloat("editor.staticlight.shadowradius", newValue);
		}
	}
	else if("Falloff cutoff" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetFloat("editor.staticlight.falloffcutoff", newValue);
	}
	else if("Falloff exponent" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetFloat("editor.staticlight.falloffexponent", newValue);
	}
	else if("property.lightnode.intensity" == forKey)
	{
		light->SetIntensity(newValue);
	}

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void LightPropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	Light *light = GetLight(currentSceneNode);

	if("Shadow samples" == forKey)
	{
		if(Light::TYPE_DIRECTIONAL == light->GetType())
		{
			currentSceneNode->GetCustomProperties()->SetInt32("editor.staticlight.shadowsamples", newValue);
		}
	}
    
    NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void LightPropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    Light *light = GetLight(currentSceneNode);
    if("property.lightnode.ambient.color" == forKey)
    {
        light->SetAmbientColor(newColor);
    }
    if("property.lightnode.diffuse.color" == forKey)
    {
        light->SetDiffuseColor(newColor);
    }
    if("property.lightnode.specular.color" == forKey)
    {
        light->SetSpecularColor(newColor);
    }
}

