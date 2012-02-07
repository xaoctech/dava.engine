#include "SceneValidator.h"
#include "ErrorDialog.h"
#include "EditorSettings.h"
#include "SceneInfoControl.h"

SceneValidator::SceneValidator()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;

    errorDialog = NULL;
    infoControl = NULL;
}

SceneValidator::~SceneValidator()
{
    SafeRelease(errorDialog);
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
            emptyNodesForDeletion.insert(sceneNode);
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
        errorMessages.insert("Wrongsize of " + path);
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
    if(!errorDialog)
    {
        errorDialog = new ErrorDialog();
    }
    errorDialog->Show(errorMessages);
}

void SceneValidator::ValidateMeshInstanceInternal(MeshInstanceNode *meshNode)
{
    const Vector<PolygonGroupWithMaterial*> & polygroups = meshNode->GetPolygonGroups();
    //Vector<Material *>materials = meshNode->GetMaterials();
    for(int32 iMat = 0; iMat < polygroups.size(); ++iMat)
    {
        ValidateMaterialInternal(polygroups[iMat]->GetMaterial());
    }
    
    int32 lightmapCont = meshNode->GetLightmapCount();
    for(int32 iLight = 0; iLight < lightmapCont; ++iLight)
    {
        ValidateTextureInternal(meshNode->GetLightmapForIndex(iLight));
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
            sceneTextureMemory += t->GetDataSize();
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
