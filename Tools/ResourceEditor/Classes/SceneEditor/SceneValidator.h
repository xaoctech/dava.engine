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
     \brief Function to validate Scene errors and Displays errors log at Errors Dialog
     \param[in] scene scene for validation
     \returns true if errors were found
	 */
    bool ValidateSceneAndShowErrors(Scene *scene);
    
    /*
     \brief Function to validate Scene errors
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateScene(Scene *scene, Set<String> &errorsLog);
    
    /*
     \brief Function to find Scales in models transformations
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation erros
	 */
	void ValidateScales(Scene *scene, Set<String> &errorsLog);


    /*
     \brief Function to validate Texture errors
     
     Displays errors log at Errors Dialog
     
     \param[in] texture texture for validation
	 */
    void ValidateTextureAndShowErrors(Texture *texture, const String &validatedObjectName);

    /*
     \brief Function to validate Texture errors
     \param[in] texture texture for validation
     \param[out] errorsLog set for validation erros
	 */
    
    void ValidateTexture(Texture *texture, const String &validatedObjectName, Set<String> &errorsLog);


    /*
     \brief Function to validate LandscapeNode errors
     \param[in] landscape landscape for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateLandscape(LandscapeNode *landscape, Set<String> &errorsLog);
        
    /*
     \brief Function to validate SceneNode errors
     \param[in] sceneNode sceneNode for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateSceneNode(SceneNode *sceneNode, Set<String> &errorsLog);
    
    /*
     \brief Function to validate Material errors
     \param[in] material material for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateMaterial(Material *material, Set<String> &errorsLog);

    
    /*
     \brief Function sets 3d folder path for cheking texture pathnames
     \param[in] pathname path to DataSource/3d folder
     \return old path for checking
	 */
    String SetPathForChecking(const String &pathname);
    
    void EnumerateSceneTextures();
    void CollectSceneStats(const RenderManager::Stats &newStats);
    void EnumerateNodes(Scene *scene);
    
    void SetInfoControl(SceneInfoControl *newInfoControl);
    
    void ReloadTextures(int32 asFile);

    //Need Release returned Texture
    Texture * ReloadTexture(const String &descriptorPathname, Texture *prevTexture, int32 asFile);
    
    static bool IsTextureChanged(const String &texturePathname, ImageFileFormat fileFormat);
    
    void FindTexturesForCompression();
    
	bool ValidateTexturePathname(const String &pathForValidation, Set<String> &errorsLog);
	bool ValidateHeightmapPathname(const String &pathForValidation, Set<String> &errorsLog);

    void CreateDefaultDescriptors(const String &folderPathname);

    void EnumerateTextures(Map<String, Texture *> &textures, Scene *scene);
    void RestoreTextures(Map<String, Texture *> &textures, Scene *scene);
    
    
    bool IsPathCorrectForProject(const String &pathname);

protected:
    
    int32 EnumerateSceneNodes(SceneNode *node);
    
    void ValidateMeshInstance(MeshInstanceNode *meshNode, Set<String> &errorsLog);
    void ValidateLodNodes(Scene *scene, Set<String> &errorsLog);
	void ValidateScalesInternal(SceneNode *sceneNode, Set<String> &errorsLog);

	void CreateDescriptorIfNeed(const String &forPathname);
    
    bool ValidatePathname(const String &pathForValidation, const String &validatedObjectName);

    bool NodeRemovingDisabled(SceneNode *node);
    
    void CompressTextures(const List<Texture *> texturesForCompression, ImageFileFormat fileFormat);
    
    bool WasTextureChanged(Texture *texture, ImageFileFormat fileFormat);

	bool IsTextureDescriptorPath(const String &path);
    
    bool IsFBOTexture(Texture *texture);
    
    Set<SceneNode *> emptyNodesForDeletion;
    Set<String> errorMessages;
    
    SceneInfoControl *infoControl;

    int32 sceneTextureCount;
    int32 sceneTextureMemory;
    
    RenderManager::Stats sceneStats;
    
    String pathForChecking;
};



#endif // __SCENE_VALIDATOR_H__