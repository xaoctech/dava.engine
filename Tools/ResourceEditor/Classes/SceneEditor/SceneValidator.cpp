#include "SceneValidator.h"
#include "ErrorNotifier.h"
#include "EditorSettings.h"
#include "SceneInfoControl.h"

#include "PVRUtils.h"

SceneValidator::SceneValidator()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;

    infoControl = NULL;
}

SceneValidator::~SceneValidator()
{
    SafeRelease(infoControl);
}

void SceneValidator::ValidateScene(Scene *scene)
{
    if(!scene) return;

    errorMessages.clear();

    ValidateSceneNodeInternal(scene);
    ValidateLodNodes(scene);

    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateSceneNode(SceneNode *sceneNode)
{
    errorMessages.clear();

    ValidateSceneNodeInternal(sceneNode);
    
    if(errorMessages.size())
    {
        ShowErrors();
    }
    
    for (Set<SceneNode*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
    {
        SceneNode * node = *it;
        if (node->GetParent())
        {
            node->GetParent()->RemoveNode(node);
        }
    }
	for (Set<SceneNode*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
	{
		SceneNode * node = *it;
		SafeRelease(node);
	}
    emptyNodesForDeletion.clear();
}

void SceneValidator::ValidateSceneNodeInternal(SceneNode *sceneNode)
{
    if(!sceneNode) return;
    
    int32 count = sceneNode->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        SceneNode *node = sceneNode->GetChild(i);
        MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode*>(node);
        if(mesh)
        {
            ValidateMeshInstanceInternal(mesh);
        }
        else 
        {
            LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(node);
            if (landscape) 
            {
                ValidateLandscapeInternal(landscape);
            }
            else
            {
                ValidateSceneNodeInternal(node);
            }
        }
        
        KeyedArchive *customProperties = node->GetCustomProperties();
        if(customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
            if(1 < dataSourcePath.length())
            {
                if('/' == dataSourcePath[0])
                {
                    dataSourcePath = dataSourcePath.substr(1);
                }
                
                String referencePath = customProperties->GetString("editor.referenceToOwner");
                String::size_type pos = referencePath.rfind(dataSourcePath);
                if((String::npos != pos) && (1 != pos))
                {
                    referencePath.replace(pos, dataSourcePath.length(), "");
                    customProperties->SetString("editor.referenceToOwner", referencePath);
                    
                    errorMessages.insert(Format("Node %s: referenceToOwner isn't correct. Re-save level.", node->GetName().c_str()));
                }
            }
        }
    }
    
    Set<DataNode*> dataNodeSet;
    sceneNode->GetDataNodes(dataNodeSet);
    if (dataNodeSet.size() == 0)
    {
        SceneNode * parent = sceneNode->GetParent();
        if (parent)
        {
            emptyNodesForDeletion.insert(SafeRetain(sceneNode));
        }
    }
}

void SceneValidator::ValidateTexture(Texture *texture)
{
    errorMessages.clear();

    ValidateTextureInternal(texture);

    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateTextureInternal(Texture *texture)
{
    if(!texture) return;

    if(IsntPower2(texture->GetWidth()) || IsntPower2(texture->GetHeight()))
    {
        String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), texture->GetPathname());
        errorMessages.insert("Wrong size of " + path);
    }
}

void SceneValidator::ValidateLandscape(LandscapeNode *landscape)
{
    errorMessages.clear();
    
    ValidateLandscapeInternal(landscape);
    
    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateLandscapeInternal(LandscapeNode *landscape)
{
    if(!landscape) return;
    
    for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
    {
        ValidateTextureInternal(landscape->GetTexture((LandscapeNode::eTextureLevel)i));
    }
}

bool SceneValidator::IsntPower2(int32 num)
{
    return (num & (num - 1));
}

void SceneValidator::ShowErrors()
{
//    ErrorNotifier::Instance()->ShowError(errorMessages);
	for (Set<String>::iterator it = errorMessages.begin(); it != errorMessages.end(); it++)
	{
		Logger::Error((*it).c_str());
	}
}

void SceneValidator::ValidateMeshInstanceInternal(MeshInstanceNode *meshNode)
{
    meshNode->RemoveFlag(SceneNode::NODE_INVALID);
    
    const Vector<PolygonGroupWithMaterial*> & polygroups = meshNode->GetPolygonGroups();
    //Vector<Material *>materials = meshNode->GetMaterials();
    for(int32 iMat = 0; iMat < polygroups.size(); ++iMat)
    {
        Material * material = polygroups[iMat]->GetMaterial();

        ValidateMaterialInternal(material);

        if (material->Validate(polygroups[iMat]->GetPolygonGroup()) == Material::VALIDATE_INCOMPATIBLE)
        {
            meshNode->AddFlag(SceneNode::NODE_INVALID);
            errorMessages.insert(Format("Material: %s incompatible with node:%s.", material->GetName().c_str(), meshNode->GetFullName().c_str()));
            errorMessages.insert("For lightmapped objects check second coordinate set. For normalmapped check tangents, binormals.");
        }
    }
    
    int32 lightmapCont = meshNode->GetLightmapCount();
    for(int32 iLight = 0; iLight < lightmapCont; ++iLight)
    {
        ValidateTextureInternal(meshNode->GetLightmapDataForIndex(iLight)->lightmap);
    }
}


void SceneValidator::ValidateMaterial(Material *material)
{
    errorMessages.clear();

    ValidateMaterialInternal(material);

    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateMaterialInternal(Material *material)
{
    for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
    {
        ValidateTextureInternal(material->textures[iTex]);
    }
}

void SceneValidator::EnumerateSceneTextures()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;
    
    const Map<String, Texture*> textureMap = Texture::GetTextureMap();
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings(); 
    String projectPath = settings->GetString("ProjectPath");
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
        if(String::npos != t->relativePathname.find(projectPath))
        {
            String::size_type pvrpngPos = t->relativePathname.find(".pvr.png");
            if(String::npos != pvrpngPos)
            {
                String pvrPath = FileSystem::ReplaceExtension(t->relativePathname, "");
                sceneTextureMemory += PVRUtils::Instance()->GetPVRDataLength(pvrPath);
            }
            else 
            {
                sceneTextureMemory += t->GetDataSize();
            }
            
            ++sceneTextureCount;
        }
	}
    
    if(infoControl)
    {
        infoControl->InvalidateTexturesInfo(sceneTextureCount, sceneTextureMemory);
    }
}

void SceneValidator::SetInfoControl(SceneInfoControl *newInfoControl)
{
    SafeRelease(infoControl);
    infoControl = SafeRetain(newInfoControl);
    
    sceneStats.Clear();
}

void SceneValidator::CollectSceneStats(const RenderManager::Stats &newStats)
{
    sceneStats = newStats;
    infoControl->SetRenderStats(sceneStats);
}

void SceneValidator::ReloadTextures()
{
    bool isAlphaPremultiplicationEnabled = Image::IsAlphaPremultiplicationEnabled();
    bool isMipmapsEnabled = Texture::IsMipmapGenerationEnabled();

    const Map<String, Texture*> textureMap = Texture::GetTextureMap();
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *texture = it->second;
        
        Image::EnableAlphaPremultiplication(texture->isAlphaPremultiplied);
        
        if(texture->isMimMapTexture) Texture::EnableMipmapGeneration();
        else Texture::DisableMipmapGeneration();

        Image *image = Image::CreateFromFile(texture->relativePathname);
        if(image)
        {
            texture->TexImage(0, image->GetWidth(), image->GetHeight(), image->GetData());
            if(texture->isMimMapTexture)
            {
                texture->GenerateMipmaps();
            }
            texture->SetWrapMode(texture->wrapModeS, texture->wrapModeT);
                
            SafeRelease(image);
        }
	}
    
    if(isMipmapsEnabled) Texture::EnableMipmapGeneration();
    else Texture::DisableMipmapGeneration();
    
    Image::EnableAlphaPremultiplication(isAlphaPremultiplicationEnabled);
}

void SceneValidator::ValidateLodNodes(Scene *scene)
{
    Vector<LodNode *> lodnodes;
    scene->GetChildNodes(lodnodes); 
    
    for(int32 index = 0; index < lodnodes.size(); ++index)
    {
        LodNode *ln = lodnodes[index];
        
        int32 layersCount = ln->GetLodLayersCount();
        for(int32 layer = 0; layer < layersCount; ++layer)
        {
            float32 distance = ln->GetLodLayerDistance(layer);
            if(LodNode::INVALID_DISTANCE == distance)
            {
                ln->SetLodLayerDistance(layer, ln->GetDefaultDistance(layer));
                errorMessages.insert(Format("Node %s: lod distances weren't correct. Re-save.", ln->GetName().c_str()));
            }
        }
        
        List<LodNode::LodData *>lodLayers;
        ln->GetLodData(lodLayers);
        
        List<LodNode::LodData *>::const_iterator endIt = lodLayers.end();
        int32 layer = 0;
        for(List<LodNode::LodData *>::iterator it = lodLayers.begin(); it != endIt; ++it, ++layer)
        {
            LodNode::LodData * ld = *it;
            
            if(ld->layer != layer)
            {
                ld->layer = layer;

                errorMessages.insert(Format("Node %s: lod layers weren't correct. Rename childs. Re-save.", ln->GetName().c_str()));
            }
        }
    }
}
