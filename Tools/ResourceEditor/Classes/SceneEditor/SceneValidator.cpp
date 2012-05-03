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
    
    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateSceneNode(DAVA::SceneNode *sceneNode)
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

void SceneValidator::ValidateSceneNodeInternal(DAVA::SceneNode *sceneNode)
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
                    
                    errorMessages.insert("ReferenceToOwner isn't correct. Re-save level.");
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
    ErrorNotifier::Instance()->ShowError(errorMessages);
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


void SceneValidator::ValidateMaterial(DAVA::Material *material)
{
    errorMessages.clear();

    ValidateMaterialInternal(material);

    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateMaterialInternal(DAVA::Material *material)
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
    Logger::Info("_____************ RELOAD **********____________");
    
    bool isAlphaPremultiplicationEnabled = Image::IsAlphaPremultiplicationEnabled();
    bool isMipmapsEnabled = Texture::IsMipmapGenerationEnabled();

    RenderManager::Instance()->LockNonMain();

    const Map<String, Texture*> textureMap = Texture::GetTextureMap();
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *texture = it->second;
        Image *image = Image::CreateFromFile(texture->relativePathname);
        if(image)
        {
            Image::EnableAlphaPremultiplication(texture->isAlphaPremultiplied);
            
            if(texture->isMimMapTexture) Texture::EnableMipmapGeneration();
            else Texture::DisableMipmapGeneration();


//Delete texture data            
            texture->ReleaseTextureData();
            
//Load texture data            
            texture->width = image->GetWidth();
            texture->height = image->GetHeight();
            texture->format = image->GetPixelFormat();
            
#if defined(__DAVAENGINE_OPENGL__)
            for (int32 i = 0; i < 10; ++i) 
            {
                RENDER_VERIFY(glGenTextures(1, &texture->id));
                if(texture->id != 0)
                {
                    break;
                }
                Logger::Error("TEXTURE %d GENERATE ERROR: %d", i, glGetError());
            }	
            
            
            int saveId = GetSavedTextureID();
            BindTexture(texture->id);
            RENDER_VERIFY(glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ));
            
            switch(texture->format) 
            {
                case FORMAT_RGBA8888:
                    RENDER_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->GetData()));
                    break;
                case FORMAT_RGB565:
                    RENDER_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, image->GetData()));
                    break;
                case FORMAT_A8:
                    RENDER_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture->width, texture->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, image->GetData()));
                    break;
                case FORMAT_RGBA4444:
                    RENDER_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, image->GetData()));
                    break;
                case FORMAT_A16:
                    RENDER_VERIFY(glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, texture->width, texture->height, 0, GL_ALPHA, GL_UNSIGNED_SHORT, image->GetData()));
                    break;
                default:
                    DVASSERT(false && "Wrong forma");
                    return;
            }
            

            GLint wrapMode = 0;
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
            wrapMode = GL_CLAMP_TO_EDGE;
#else //Non ES platforms
            wrapMode = GL_CLAMP;
#endif //PLATFORMS
            if (Texture::IsMipmapGenerationEnabled())
            {
                RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
                RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
                texture->GenerateMipmaps();
            }else
            {
                RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode));
                RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode));
                RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            }
            
            if (saveId != 0)
            {
                BindTexture(saveId);
            }
            
#elif defined(__DAVAENGINE_DIRECTX9__)
            
            texture->id = CreateTextureNative(Vector2((float32)texture->width, (float32)texture->height), texture->format, false, 0);
            texture->TexImage(0, texture->width, texture->height, image->GetData());
            
            // allocate only 2 levels, and reuse buffers for generation of every mipmap level
            uint8 *mipMapData = new uint8[(texture->width / 2) * (texture->height / 2) * GetPixelFormatSize(texture->format) / 8];
            uint8 *mipMapData2 = new uint8[(texture->width / 4) * (texture->height / 4) * GetPixelFormatSize(texture->format) / 8];
            
            const uint8 * prevMipData = image->GetData();
            uint8 * currentMipData = mipMapData;
            
            int32 mipMapWidth = texture->width / 2;
            int32 mipMapHeight = texture->height / 2;
            
            for (uint32 i = 1; i < texture->id->GetLevelCount(); ++i)
            {
                ImageConvert::DownscaleTwiceBillinear(texture->format, texture->format, 
                                                      prevMipData, mipMapWidth << 1, mipMapHeight << 1, (mipMapWidth << 1) * GetPixelFormatSize(texture->format) / 8,
                                                      currentMipData, mipMapWidth, mipMapHeight, mipMapWidth * GetPixelFormatSize(texture->format) / 8);
                
                texture->TexImage(i, mipMapWidth, mipMapHeight, currentMipData);
                
                mipMapWidth  >>= 1;
                mipMapHeight >>= 1;
                
                prevMipData = currentMipData;
                currentMipData = (i & 1) ? (mipMapData2) : (mipMapData); 
            }
            
            SafeDeleteArray(mipMapData2);
            SafeDeleteArray(mipMapData);
            
#endif //#if defined(__DAVAENGINE_OPENGL__)
            
            texture->SetWrapMode(texture->wrapModeS, texture->wrapModeT);
                
            SafeRelease(image);
        }
	}
    
    RenderManager::Instance()->UnlockNonMain();

    
    if(isMipmapsEnabled) Texture::EnableMipmapGeneration();
    else Texture::DisableMipmapGeneration();
    
    Image::EnableAlphaPremultiplication(isAlphaPremultiplicationEnabled);
}
