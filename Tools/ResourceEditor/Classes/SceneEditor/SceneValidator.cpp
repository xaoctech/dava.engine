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

    int32 materialCount = scene->GetMaterialCount();
    for(int32 iMat = 0; iMat < materialCount; ++iMat)
    {
        Material *m = scene->GetMaterial(iMat);
        for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
        {
            ValidateTexture(m->textures[iTex], false);
        }
    }
    
    ValidateSceneNode(scene, false);
    
    if(errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateSceneNode(DAVA::SceneNode *sceneNode)
{
    ValidateSceneNode(sceneNode, true);
}

void SceneValidator::ValidateSceneNode(DAVA::SceneNode *sceneNode, bool singleMessage)
{
    if(!sceneNode) return;
    
    int32 count = sceneNode->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        SceneNode *node = sceneNode->GetChild(i);
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(node);
        if (landscape) 
        {
            ValidateLandscape(landscape, false);
        }
        
        ValidateSceneNode(node, false);
    }
}

void SceneValidator::ValidateTexture(Texture *texture)
{
    ValidateTexture(texture, true);
}

void SceneValidator::ValidateTexture(Texture *texture, bool singleMessage)
{
    if(!texture) return;
    
    if(singleMessage)
    {
        errorMessages.clear();
    }

    if(IsntPower2(texture->GetWidth()) || IsntPower2(texture->GetHeight()))
    {
        String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), texture->GetPathname());
        errorMessages.push_back("Wrongsize of " + path);
    }
    
    if(singleMessage && errorMessages.size())
    {
        ShowErrors();
    }
}

void SceneValidator::ValidateLandscape(LandscapeNode *landscape)
{
    ValidateLandscape(landscape, true);
}

void SceneValidator::ValidateLandscape(LandscapeNode *landscape, bool singleMessage)
{
    if(!landscape) return;
    
    if(singleMessage)
    {
        errorMessages.clear();
    }

    for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
    {
        ValidateTexture(landscape->GetTexture((LandscapeNode::eTextureLevel)i), false);
    }
    
    if(singleMessage && errorMessages.size())
    {
        ShowErrors();
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
