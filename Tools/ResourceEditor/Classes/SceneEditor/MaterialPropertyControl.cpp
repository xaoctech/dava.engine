#include "MaterialPropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"

static const String textureNames[] = 
{
    "Diffuse texture", 
    "Decal texture",
    "Detail texture",
    "Normal map"
};

static const int32 textureTypes[] = 
{
    Material::TEXTURE_DIFFUSE,
    Material::TEXTURE_DECAL,
    Material::TEXTURE_DETAIL,
    Material::TEXTURE_NORMALMAP
};

MaterialPropertyControl::MaterialPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

MaterialPropertyControl::~MaterialPropertyControl()
{

}

void MaterialPropertyControl::ReadFrom(DataNode * dataNode)
{
	NodesPropertyControl::ReadFrom(dataNode);

    Material *material = dynamic_cast<Material*> (dataNode);
	DVASSERT(material);

    propertyList->AddSection("property.material", GetHeaderState("property.material", true));
    
    Vector<String> materialTypes;
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        materialTypes.push_back(Material::GetTypeName((Material::eType)i));
    }

    int32 materialType = material->type;
    propertyList->AddComboProperty("property.material.type", materialTypes);
    propertyList->SetComboPropertyIndex("property.material.type", materialType);

    
    propertyList->AddFilepathProperty(textureNames[ETT_DIFFUSE], ".png;.pvr");
    SetFilepathValue(material, ETT_DIFFUSE);
    
    if (    (Material::MATERIAL_UNLIT_TEXTURE_DECAL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DECAL == materialType))
    {
        propertyList->AddFilepathProperty(textureNames[ETT_DECAL], ".png;.pvr");
        SetFilepathValue(material, ETT_DECAL);
    }
    
    if (    (Material::MATERIAL_UNLIT_TEXTURE_DETAIL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DETAIL == materialType))
    {
        propertyList->AddFilepathProperty(textureNames[ETT_DETAIL], ".png;.pvr");
        SetFilepathValue(material, ETT_DETAIL);
    }
    
    if (    (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == materialType))
    {
        propertyList->AddFilepathProperty(textureNames[ETT_NORMAL_MAP], ".png;.pvr");
        SetFilepathValue(material, ETT_NORMAL_MAP);
    }

    propertyList->AddBoolProperty("materialeditor.isopaque");
    propertyList->SetBoolPropertyValue("materialeditor.isopaque", material->GetOpaque());
    
    propertyList->AddBoolProperty("materialeditor.twosided");
    propertyList->SetBoolPropertyValue("materialeditor.twosided", material->GetTwoSided());
    
    if (    (Material::MATERIAL_VERTEX_LIT_TEXTURE == materialType) 
        ||  (Material::MATERIAL_VERTEX_LIT_DECAL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DETAIL == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == materialType))
    {
        propertyList->AddColorProperty("materialeditor.ambientcolor");
        propertyList->SetColorPropertyValue("materialeditor.ambientcolor", material->GetAmbientColor());
        
        propertyList->AddColorProperty("materialeditor.diffusecolor");
        propertyList->SetColorPropertyValue("materialeditor.diffusecolor", material->GetDiffuseColor());
    }
    
    if (    (Material::MATERIAL_VERTEX_LIT_TEXTURE == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DECAL == materialType)
        ||  (Material::MATERIAL_VERTEX_LIT_DETAIL == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR == materialType)
        ||  (Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP == materialType))
    {
        propertyList->AddColorProperty("materialeditor.specularcolor");
        propertyList->SetColorPropertyValue("materialeditor.specularcolor", material->GetSpecularColor());
        
        propertyList->AddFloatProperty("materialeditor.specularshininess");
        propertyList->SetFloatPropertyValue("materialeditor.specularshininess", material->GetShininess());
    }
}

void MaterialPropertyControl::SetFilepathValue(Material *material, int32 type)
{
    if (material->textures[textureTypes[type]])
    {
        propertyList->SetFilepathPropertyValue(textureNames[type], material->names[textureTypes[type]]);
    }
    else 
    {
        propertyList->SetFilepathPropertyValue(textureNames[type], "");
    }

}


void MaterialPropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("materialeditor.isopaque" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetOpaque(newValue);
    }
    else if("materialeditor.twosided" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetTwoSided(newValue);
    }
    else
    {
        NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
    }
}

void MaterialPropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("materialeditor.ambientcolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetAmbientColor(newColor);
    }
    else if("materialeditor.diffusecolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetDiffuseColor(newColor);
    }
    else if("materialeditor.specularcolor" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetSpecularColor(newColor);
    }
    else 
    {
        PropertyListDelegate::OnColorPropertyChanged(forList, forKey, newColor);
    }
}

void MaterialPropertyControl::OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if ("property.material.type" == forKey) 
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetType((Material::eType)newItemIndex);
        
        ReadFrom(currentDataNode);
    }
    else 
    {
        NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
    }
}

void MaterialPropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if ("materialeditor.specularshininess" == forKey)
    {
        Material *material = dynamic_cast<Material *> (currentDataNode);
        material->SetShininess(newValue);
    }
    else 
    {
        NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
    }
}

void MaterialPropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    for (int i = 0; i < ME_TEX_COUNT; i++) 
    {
        if (forKey == textureNames[i]) 
        {
            Material *material = dynamic_cast<Material *> (currentDataNode);

            material->SetTexture((Material::eTextureLevel)textureTypes[i], newValue);
            Texture *tx = material->textures[textureTypes[i]];
            if(tx)
            {
                SceneValidator::Instance()->ValidateTexture(tx);
            }
            else 
            {
                propertyList->SetFilepathPropertyValue(textureNames[i], "");
            }
            return;
        }
    }
    
    NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
}

