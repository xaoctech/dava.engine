#include "LandscapePropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"
#include "ControlsFactory.h"
#include "Render/Highlevel/Heightmap.h"

#include "../Qt/Main/QtUtils.h"
#include "SceneValidator.h"
#include "Scene3D/Components/DebugRenderComponent.h"

LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
    tiledModes.push_back("Tile mask mode");
    tiledModes.push_back("Texture mode");
    tiledModes.push_back("Mixed mode");
}

LandscapePropertyControl::~LandscapePropertyControl()
{

}

void LandscapePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	LandscapeNode *landscape = GetLandscape();
	if (!landscape)
		return;

    propertyList->AddSection("property.landscape.landscape", GetHeaderState("property.landscape.landscape", true));
    
    propertyList->AddFloatProperty("property.landscape.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.landscape.height", PropertyList::PROPERTY_IS_EDITABLE); 
    Vector3 size(445.0f, 445.0f, 50.f);
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 emptyBox;
    if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
    {
        AABBox3 transformedBox;
        bbox.GetTransformedBox(*landscape->GetWorldTransformPtr(), transformedBox);
        size = transformedBox.max - transformedBox.min;
    }
    propertyList->SetFloatPropertyValue("property.landscape.size", size.x);
    propertyList->SetFloatPropertyValue("property.landscape.height", size.z);

    propertyList->AddComboProperty("property.landscape.tilemode", tiledModes);
    propertyList->SetComboPropertyIndex("property.landscape.tilemode", landscape->GetTiledShaderMode());

    propertyList->AddFilepathProperty("property.landscape.heightmap", ".png;.heightmap", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->SetFilepathPropertyValue("property.landscape.heightmap", landscape->GetHeightmapPathname());
    
    
    propertyList->AddSubsection("property.landscape.subsection.textures");
    AddFilepathProperty(String("property.landscape.texture.color"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_COLOR);
    AddFilepathProperty(String("property.landscape.texture.tile0"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_TILE0);
    AddFilepathProperty(String("property.landscape.texture.tile1"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_TILE1);
    AddFilepathProperty(String("property.landscape.texture.tile2"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_TILE2);
    AddFilepathProperty(String("property.landscape.texture.tile3"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_TILE3);
    AddFilepathProperty(String("property.landscape.texture.tilemask"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_TILE_MASK);
    AddFilepathProperty(String("property.landscape.texture.tiledtexture"), TextureDescriptor::GetSupportedTextureExtensions(), LandscapeNode::TEXTURE_TILE_FULL);
    propertyList->AddMessageProperty(String("property.landscape.generatefulltiled"), 
                                     Message(this, &LandscapePropertyControl::GenerateFullTiledTexture));

    propertyList->AddMessageProperty(String("property.landscape.saveheightmaptopng"),
                                     Message(this, &LandscapePropertyControl::SaveHeightmapToPng));


    propertyList->AddBoolProperty("property.landscape.showgrid", PropertyList::PROPERTY_IS_EDITABLE);
    
    // RETURN TO THIS CODE LATER
    // bool showGrid =  (0 != (landscape->GetDebugFlags() & DebugRenderComponent::DEBUG_DRAW_GRID));
    bool showGrid = false;
    propertyList->SetBoolPropertyValue("property.landscape.showgrid", showGrid);
    

	propertyList->AddIntProperty("lightmap.size");
	propertyList->SetIntPropertyValue("lightmap.size", currentSceneNode->GetCustomProperties()->GetInt32("lightmap.size", 1024));

    propertyList->AddFloatProperty("property.landscape.texture0.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE0).x);
    propertyList->AddFloatProperty("property.landscape.texture0.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE0).y);
    
    propertyList->AddFloatProperty("property.landscape.texture1.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE1).x);
    propertyList->AddFloatProperty("property.landscape.texture1.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE1).y);

    propertyList->AddFloatProperty("property.landscape.texture2.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE2).x);
    propertyList->AddFloatProperty("property.landscape.texture2.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE2).y);
    
    propertyList->AddFloatProperty("property.landscape.texture3.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE3).x);
    propertyList->AddFloatProperty("property.landscape.texture3.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE3).y);
    
    ControlsFactory::AddFogSubsection(propertyList, landscape->IsFogEnabled(), landscape->GetFogDensity(), landscape->GetFogColor());
}

void LandscapePropertyControl::AddFilepathProperty(const String &key, const String &filter, LandscapeNode::eTextureLevel level)
{
	LandscapeNode *landscape = GetLandscape();
	if (!landscape)
		return;
    Texture * t = landscape->GetTexture(level);
    
    propertyList->AddFilepathProperty(key, filter, true, PropertyList::PROPERTY_IS_EDITABLE);
    if(t && !t->isRenderTarget)
    {
        propertyList->SetFilepathPropertyValue(key, landscape->GetTextureName(level));
    }
    else 
    {
        propertyList->SetFilepathPropertyValue(key, String(""));
    }
}


void LandscapePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.landscape.size" == forKey || "property.landscape.height" == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (landscape)
		{
			Vector3 size(
						 propertyList->GetFloatPropertyValue("property.landscape.size"),
						 propertyList->GetFloatPropertyValue("property.landscape.size"),
						 propertyList->GetFloatPropertyValue("property.landscape.height"));
			AABBox3 bbox;
			bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
			bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
			
			Set<String> errorsLog;
			String heightMap = propertyList->GetFilepathPropertyValue("property.landscape.heightmap");
			if(SceneValidator::Instance()->ValidateHeightmapPathname(heightMap, errorsLog) && heightMap.length())
			{
				landscape->BuildLandscapeFromHeightmapImage(heightMap, bbox);
			}
        }
    }
    if ("property.landscape.texture0.tilex" == forKey || "property.landscape.texture0.tiley" == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture0.tilex"),
						   propertyList->GetFloatPropertyValue("property.landscape.texture0.tiley"));
			landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE0, tiling);
		}
    }
    if ("property.landscape.texture1.tilex" == forKey || "property.landscape.texture1.tiley" == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture1.tilex"),
						   propertyList->GetFloatPropertyValue("property.landscape.texture1.tiley"));
			landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE1, tiling);
		}
    }
	if ("property.landscape.texture2.tilex" == forKey || "property.landscape.texture2.tiley" == forKey)
	{
		LandscapeNode *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture2.tilex"),
				propertyList->GetFloatPropertyValue("property.landscape.texture2.tiley"));
			landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE2, tiling);
		}
	}
	if ("property.landscape.texture3.tilex" == forKey || "property.landscape.texture3.tiley" == forKey)
	{
		LandscapeNode *landscape = GetLandscape();
		if (landscape)
		{
			Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture3.tilex"),
				propertyList->GetFloatPropertyValue("property.landscape.texture3.tiley"));
			landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE3, tiling);
		}
	}
    
    if ("property.material.dencity" == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (landscape)
			landscape->SetFogDensity(newValue);
    }

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}


void LandscapePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if("lightmap.size" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetInt32("lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
	Set<String> errorsLog;
	if("property.landscape.heightmap" == forKey)
	{
		bool isValid = SceneValidator::Instance()->ValidateHeightmapPathname(newValue, errorsLog);
		if(isValid)
		{
			LandscapeNode *landscape = GetLandscape();
			if (!landscape)
				return;

			Vector3 size(
				propertyList->GetFloatPropertyValue("property.landscape.size"),
				propertyList->GetFloatPropertyValue("property.landscape.size"),
				propertyList->GetFloatPropertyValue("property.landscape.height"));
			AABBox3 bbox;
			bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
			bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));

			if(newValue.length())
			{
				landscape->BuildLandscapeFromHeightmapImage(newValue, bbox);
			}
		}
	}
	else
	{
		bool isValid = (newValue.empty()) ? true: SceneValidator::Instance()->ValidateTexturePathname(newValue, errorsLog);
		if(isValid)
		{
            String descriptorPathname = String("");
            if(!newValue.empty())
            {
                descriptorPathname = TextureDescriptor::GetDescriptorPathname(newValue);
            }
            
			if("property.landscape.texture.tile0" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE0, descriptorPathname);
			}
			else if("property.landscape.texture.tile1" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE1, descriptorPathname);
			}
			else if("property.landscape.texture.tile2" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE2, descriptorPathname);
			}
			else if("property.landscape.texture.tile3" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE3, descriptorPathname);
			}
			else if("property.landscape.texture.tilemask" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE_MASK, descriptorPathname);
			}
			else if("property.landscape.texture.color" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_COLOR, descriptorPathname);
			}
			else if("property.landscape.texture.tiledtexture" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE_FULL, descriptorPathname);
			}
		}
	}

	if(0 == errorsLog.size())
	{
		NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
	}
	else
	{
		ShowErrorDialog(errorsLog);
	}
}

void LandscapePropertyControl::SetLandscapeTexture(LandscapeNode::eTextureLevel level, const String &texturePathname)
{
	LandscapeNode *landscape = GetLandscape();
	if (!landscape)
		return;

    landscape->SetTexture(level, texturePathname);
    SceneValidator::Instance()->ValidateTextureAndShowErrors(landscape->GetTexture(level), landscape->GetTextureName(level), Format("Landscape. TextureLevel %d", level));

    if(LandscapeNode::TEXTURE_TILE_FULL != level)
    {
        landscape->UpdateFullTiledTexture();   
    }
}



void LandscapePropertyControl::OnComboIndexChanged(
                                PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("property.landscape.tilemode" == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (!landscape)
			return;

        landscape->SetTiledShaderMode((LandscapeNode::eTiledShaderMode)newItemIndex);
    }
        
    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LandscapePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("property.landscape.showgrid" == forKey)
    {
        // RETURN TO THIS CODE LATER
//        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
//        
//        if(newValue)
//        {
//            landscape->SetDebugFlags(landscape->GetDebugFlags() | DebugRenderComponent::DEBUG_DRAW_GRID);
//        }
//        else 
//        {
//            landscape->SetDebugFlags(landscape->GetDebugFlags() & ~DebugRenderComponent::DEBUG_DRAW_GRID);
//        }
    }
    else if (String("property.material.fogenabled") == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (!landscape)
			return;

        landscape->SetFog(newValue);
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.material.fogcolor" == forKey)
    {
		LandscapeNode *landscape = GetLandscape();
		if (!landscape)
			return;
        landscape->SetFogColor(newColor);
    }
    
    PropertyListDelegate::OnColorPropertyChanged(forList, forKey, newColor);
}


void LandscapePropertyControl::GenerateFullTiledTexture(DAVA::BaseObject *object, void *userData, void *callerData)
{
	LandscapeNode *landscape = GetLandscape();
	if (!landscape)
		return;

    String texPathname = landscape->SaveFullTiledTexture();
    String descriptorPathname = TextureDescriptor::GetDescriptorPathname(texPathname);
    
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
    if(!descriptor)
    {
        descriptor = new TextureDescriptor();
        descriptor->pathname = descriptorPathname;
        descriptor->Save();
    }
    
    propertyList->SetFilepathPropertyValue(String("property.landscape.texture.tiledtexture"), descriptor->pathname);
    landscape->SetTexture(LandscapeNode::TEXTURE_TILE_FULL, descriptor->pathname);
    
    SafeRelease(descriptor);
}

void LandscapePropertyControl::SaveHeightmapToPng(DAVA::BaseObject *object, void *userData, void *callerData)
{
	LandscapeNode *landscape = GetLandscape();
	if (!landscape)
		return;

    Heightmap * heightmap = landscape->GetHeightmap();
    String heightmapPath = landscape->GetHeightmapPathname();
    heightmapPath = FileSystem::ReplaceExtension(heightmapPath, ".png");
    heightmap->SaveToImage(heightmapPath);
}

LandscapeNode* LandscapePropertyControl::GetLandscape() const
{
	RenderComponent* component = cast_if_equal<RenderComponent*>(currentSceneNode->GetComponent(Component::RENDER_COMPONENT));
	LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (component->GetRenderObject());
	DVASSERT(landscape);
	return landscape;
}

