#include "SceneValidator.h"
#include "ErrorDialog.h"
#include "EditorSettings.h"

SceneValidator::SceneValidator()
{
    errorDialog = NULL;
}

SceneValidator::~SceneValidator()
{
    SafeRelease(errorDialog);
}

void SceneValidator::ValidateScene(Scene *scene)
{
    if(!scene) return;

    errorMessages.clear();

//    int32 materialCount = scene->GetMaterialCount();
//    for(int32 iMat = 0; iMat < materialCount; ++iMat)
//    {
//        ValidateMaterialInternal(scene->GetMaterial(iMat));
//    }
    
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
    Vector<Material *>materials = meshNode->GetMaterials();
    for(int32 iMat = 0; iMat < materials.size(); ++iMat)
    {
        ValidateMaterialInternal(materials[iMat]);
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

