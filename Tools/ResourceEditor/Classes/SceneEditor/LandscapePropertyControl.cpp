#include "LandscapePropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"
#include "ControlsFactory.h"
#include "Scene3D/Heightmap.h"

#include "../Qt/Main/QtUtils.h"
#include "SceneValidator.h"

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

    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
	DVASSERT(landscape);

    propertyList->AddSection("property.landscape.landscape", GetHeaderState("property.landscape.landscape", true));
    
    propertyList->AddFloatProperty("property.landscape.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.landscape.height", PropertyList::PROPERTY_IS_EDITABLE); 
    Vector3 size(445.0f, 445.0f, 50.f);
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 emptyBox;
    if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
    {
        AABBox3 transformedBox;
        bbox.GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
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


    propertyList->AddSubsection("Channels");
    propertyList->AddMessageProperty(String("Save Channels"), Message(this, &LandscapePropertyControl::OnSaveChannels));
    propertyList->AddFilepathProperty("Red", TextureDescriptor::GetSupportedTextureExtensions(), true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("Green", TextureDescriptor::GetSupportedTextureExtensions(), true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("Blue", TextureDescriptor::GetSupportedTextureExtensions(), true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("Alpha", TextureDescriptor::GetSupportedTextureExtensions(), true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddMessageProperty(String("Load Channels"), Message(this, &LandscapePropertyControl::OnLoadChannels));
    SetChannelsNames();
    
    
    propertyList->AddBoolProperty("property.landscape.showgrid", PropertyList::PROPERTY_IS_EDITABLE);
    bool showGrid =  (0 != (landscape->GetDebugFlags() & SceneNode::DEBUG_DRAW_GRID));
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
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
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
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        
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
    if ("property.landscape.texture0.tilex" == forKey || "property.landscape.texture0.tiley" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture0.tilex"),
                       propertyList->GetFloatPropertyValue("property.landscape.texture0.tiley"));
        landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE0, tiling);
    }
    if ("property.landscape.texture1.tilex" == forKey || "property.landscape.texture1.tiley" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture1.tilex"),
                       propertyList->GetFloatPropertyValue("property.landscape.texture1.tiley"));
        landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE1, tiling);
    }
	if ("property.landscape.texture2.tilex" == forKey || "property.landscape.texture2.tiley" == forKey)
	{
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture2.tilex"),
			propertyList->GetFloatPropertyValue("property.landscape.texture2.tiley"));
		landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE2, tiling);
	}
	if ("property.landscape.texture3.tilex" == forKey || "property.landscape.texture3.tiley" == forKey)
	{
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture3.tilex"),
			propertyList->GetFloatPropertyValue("property.landscape.texture3.tiley"));
		landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE3, tiling);
	}
    
    if ("property.material.dencity" == forKey)
    {
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
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
			LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
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
			if("property.landscape.texture.tile0" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE0, newValue);
			}
			else if("property.landscape.texture.tile1" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE1, newValue);
			}
			else if("property.landscape.texture.tile2" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE2, newValue);
			}
			else if("property.landscape.texture.tile3" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE3, newValue);
			}
			else if("property.landscape.texture.tilemask" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE_MASK, newValue);
                SetChannelsNames();
			}
			else if("property.landscape.texture.color" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_COLOR, newValue);
			}
			else if("property.landscape.texture.tiledtexture" == forKey)
			{
				SetLandscapeTexture(LandscapeNode::TEXTURE_TILE_FULL, newValue);
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
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
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
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetTiledShaderMode((LandscapeNode::eTiledShaderMode)newItemIndex);
    }
        
    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LandscapePropertyControl::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if("property.landscape.showgrid" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        
        if(newValue)
        {
            landscape->SetDebugFlags(landscape->GetDebugFlags() | SceneNode::DEBUG_DRAW_GRID);
        }
        else 
        {
            landscape->SetDebugFlags(landscape->GetDebugFlags() & ~SceneNode::DEBUG_DRAW_GRID);
        }
    }
    else if (String("property.material.fogenabled") == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetFog(newValue);
    }

    NodesPropertyControl::OnBoolPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("property.material.fogcolor" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        landscape->SetFogColor(newColor);
    }
    
    PropertyListDelegate::OnColorPropertyChanged(forList, forKey, newColor);
}


void LandscapePropertyControl::GenerateFullTiledTexture(DAVA::BaseObject *object, void *userData, void *callerData)
{
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
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
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
    Heightmap * heightmap = landscape->GetHeightmap();
    String heightmapPath = landscape->GetHeightmapPathname();
    heightmapPath = FileSystem::ReplaceExtension(heightmapPath, ".png");
    heightmap->SaveToImage(heightmapPath);
}

void LandscapePropertyControl::OnLoadChannels(DAVA::BaseObject *object, void *userData, void *callerData)
{
    LoadChannels();
}

void LandscapePropertyControl::OnSaveChannels(DAVA::BaseObject *object, void *userData, void *callerData)
{
    SaveChannels();
}

void LandscapePropertyControl::SaveChannels()
{
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);

    Texture *tileMaskTexture = landscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK);
    if(!tileMaskTexture)
    {
        return;
    }
    
    Image *tileMask = tileMaskTexture->CreateImageFromMemory();
    DVASSERT(tileMask->format == FORMAT_RGBA8888);
    
    Image *red = Image::Create(tileMask->width, tileMask->height, FORMAT_A8);
    Image *green = Image::Create(tileMask->width, tileMask->height, FORMAT_A8);
    Image *blue = Image::Create(tileMask->width, tileMask->height, FORMAT_A8);
    Image *alpha = Image::Create(tileMask->width, tileMask->height, FORMAT_A8);
    
    int32 size = tileMask->width * tileMask->height;
    int32 pixelSize = Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    for(int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSize;
        red->data[i] = tileMask->data[offset];
        green->data[i] = tileMask->data[offset + 1];
        blue->data[i] = tileMask->data[offset + 2];
        alpha->data[i] = tileMask->data[offset + 3];
    }
    
    String tileMaskPathname = landscape->GetTextureName(LandscapeNode::TEXTURE_TILE_MASK);
    String redPathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".r.png");
    String greenPathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".g.png");
    String bluePathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".b.png");
    String alphaPathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".a.png");
    
    ImageLoader::Save(red, redPathname);
    ImageLoader::Save(green, greenPathname);
    ImageLoader::Save(blue, bluePathname);
    ImageLoader::Save(alpha, alphaPathname);
    
    SafeRelease(red);
    SafeRelease(green);
    SafeRelease(blue);
    SafeRelease(alpha);
    SafeRelease(tileMask);
}

void LandscapePropertyControl::LoadChannels()
{
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
    
    Image *red = CreateTopLevelImage(propertyList->GetFilepathPropertyValue("Red"));
    Image *green = CreateTopLevelImage(propertyList->GetFilepathPropertyValue("Green"));
    Image *blue = CreateTopLevelImage(propertyList->GetFilepathPropertyValue("Blue"));
    Image *alpha = CreateTopLevelImage(propertyList->GetFilepathPropertyValue("Alpha"));

    if( (red->width == green->width && red->width == blue->width && red->width == alpha->width)
       && (red->height == green->height && red->height == blue->height && red->height == alpha->height))
    {

        Image *tileMask = Image::Create(red->width, red->height, FORMAT_RGBA8888);
        int32 size = tileMask->width * tileMask->height;
        int32 pixelSize = Texture::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
        for(int32 i = 0; i < size; ++i)
        {
            int32 offset = i * pixelSize;
            tileMask->data[offset] = red->data[i];
            tileMask->data[offset + 1] = green->data[i];
            tileMask->data[offset + 2] = blue->data[i];
            tileMask->data[offset + 3] = alpha->data[i];
        }
        
        String tileDescriptorName = landscape->GetTextureName(LandscapeNode::TEXTURE_TILE_MASK);
        String tileMaskPathname = FileSystem::Instance()->ReplaceExtension(tileDescriptorName, ".png");
        ImageLoader::Save(tileMask, tileMaskPathname);
        SafeRelease(tileMask);
        
        landscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, tileDescriptorName);
    }
    
    SafeRelease(red);
    SafeRelease(green);
    SafeRelease(blue);
    SafeRelease(alpha);
}

void LandscapePropertyControl::SetChannelsNames()
{
    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);

    String tileMaskPathname = landscape->GetTextureName(LandscapeNode::TEXTURE_TILE_MASK);
    String redPathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".r.png");
    String greenPathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".g.png");
    String bluePathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".b.png");
    String alphaPathname = FileSystem::Instance()->ReplaceExtension(tileMaskPathname, ".a.png");

    propertyList->SetFilepathPropertyValue("Red", redPathname);
    propertyList->SetFilepathPropertyValue("Green", greenPathname);
    propertyList->SetFilepathPropertyValue("Blue", bluePathname);
    propertyList->SetFilepathPropertyValue("Alpha", alphaPathname);
}