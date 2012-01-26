#ifndef __SCENE_VALIDATOR_H__
#define __SCENE_VALIDATOR_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ErrorDialog;
class SceneValidator: public Singleton<SceneValidator>
{
    
public:
    SceneValidator();
    virtual ~SceneValidator();

    void ValidateScene(Scene *scene);
    void ValidateTexture(Texture *texture);
    void ValidateLandscape(LandscapeNode *landscape);
    void ValidateSceneNode(SceneNode *sceneNode);
    
protected:

    bool IsntPower2(int32 num);
    
    void ValidateTexture(Texture *texture, bool singleMessage);
    void ValidateLandscape(LandscapeNode *landscape, bool singleMessage);
    void ValidateSceneNode(SceneNode *sceneNode, bool singleMessage);
    
    void ShowErrors();
    
    Vector<String> errorMessages;
    ErrorDialog *errorDialog;
    
};



#endif // __SCENE_VALIDATOR_H__