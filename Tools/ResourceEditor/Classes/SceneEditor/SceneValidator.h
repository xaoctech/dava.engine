#ifndef __SCENE_VALIDATOR_H__
#define __SCENE_VALIDATOR_H__

#include "DAVAEngine.h"

using namespace DAVA;

class SceneInfoControl;
class SceneValidator: public Singleton<SceneValidator>
{
    
public:
    SceneValidator();
    virtual ~SceneValidator();

    /*
     \brief Function to validate Scene errors

     Displays errors log at Errors Dialog
     
     \param[in] scene scene for validation
	 */
    void ValidateScene(Scene *scene);
    
    /*
     \brief Function to validate Scene errors
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateScene(Scene *scene, Set<String> &errorsLog);
    
    
    /*
     \brief Function to validate Texture errors
     
     Displays errors log at Errors Dialog
     
     \param[in] texture texture for validation
	 */
    void ValidateTexture(Texture *texture);

    /*
     \brief Function to validate Texture errors
     \param[in] texture texture for validation
     \param[out] errorsLog set for validation erros
	 */
    
    void ValidateTexture(Texture *texture, Set<String> &errorsLog);

    /*
     \brief Function to validate LandscapeNode errors
     
     Displays errors log at Errors Dialog
     
     \param[in] landscape landscape for validation
	 */
    void ValidateLandscape(LandscapeNode *landscape);

    /*
     \brief Function to validate LandscapeNode errors
     \param[in] landscape landscape for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateLandscape(LandscapeNode *landscape, Set<String> &errorsLog);
    
    /*
     \brief Function to validate SceneNode errors
     
     Displays errors log at Errors Dialog
     
     \param[in] sceneNode sceneNode for validation
	 */
    void ValidateSceneNode(SceneNode *sceneNode);
    
    /*
     \brief Function to validate SceneNode errors
     \param[in] sceneNode sceneNode for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateSceneNode(SceneNode *sceneNode, Set<String> &errorsLog);
    
    /*
     \brief Function to validate Material errors

     Displays errors log at Errors Dialog
     
     \param[in] material material for validation
	 */
    void ValidateMaterial(Material *material);

    /*
     \brief Function to validate Material errors
     \param[in] material material for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateMaterial(Material *material, Set<String> &errorsLog);

    
    /*
     \brief Function sets 3d folder path for cheking texture pathnames
     \param[in] pathname path to DataSource/3d folder
	 */
    void SetPathForChecking(const String &pathname);
    
    
    void EnumerateSceneTextures();
    void CollectSceneStats(const RenderManager::Stats &newStats);
    
    void SetInfoControl(SceneInfoControl *newInfoControl);
    
    static bool IsntPower2(int32 num);

    void ReloadTextures();
    
protected:
    
    void ValidateMeshInstance(MeshInstanceNode *meshNode, Set<String> &errorsLog);
    void ValidateLodNodes(Scene *scene, Set<String> &errorsLog);
    
    void ShowErrors();
    
    bool ValidatePathname(const String &pathForValidation);
    
    Set<SceneNode*> emptyNodesForDeletion;
    Set<String> errorMessages;
    
    SceneInfoControl *infoControl;

    int32 sceneTextureCount;
    int32 sceneTextureMemory;
    
    RenderManager::Stats sceneStats;
    
    String pathForChecking;
};



#endif // __SCENE_VALIDATOR_H__