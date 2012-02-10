#ifndef __SCENE_VALIDATOR_H__
#define __SCENE_VALIDATOR_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ErrorDialog;
class SceneInfoControl;
class SceneValidator: public Singleton<SceneValidator>
{
    
public:
    SceneValidator();
    virtual ~SceneValidator();

    void ValidateScene(Scene *scene);
    void ValidateTexture(Texture *texture);
    void ValidateLandscape(LandscapeNode *landscape);
    void ValidateSceneNode(SceneNode *sceneNode);
    void ValidateMaterial(Material *material);
    
    void EnumerateSceneTextures();
    void CollectSceneStats(const RenderManager::Stats &newStats);
    
    void SetInfoControl(SceneInfoControl *newInfoControl);
    
protected:

    bool IsntPower2(int32 num);
    
    void ValidateTextureInternal(Texture *texture);
    void ValidateLandscapeInternal(LandscapeNode *landscape);
    void ValidateSceneNodeInternal(SceneNode *sceneNode);
    void ValidateMeshInstanceInternal(MeshInstanceNode *meshNode);
    void ValidateMaterialInternal(Material *material);
    
    void ShowErrors();
    
    Set<SceneNode*> emptyNodesForDeletion;
    Set<String> errorMessages;
    ErrorDialog *errorDialog;
    
    SceneInfoControl *infoControl;

    int32 sceneTextureCount;
    int32 sceneTextureMemory;
    
    RenderManager::Stats sceneStats;
};



#endif // __SCENE_VALIDATOR_H__