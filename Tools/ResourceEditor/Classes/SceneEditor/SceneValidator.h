#ifndef __SCENE_VALIDATOR_H__
#define __SCENE_VALIDATOR_H__

#include "DAVAEngine.h"

using namespace DAVA;

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
    void ValidateTextureAndShowErrors(Texture *texture, const String &textureName, const String &validatedObjectName);

    /*
     \brief Function to validate Texture errors
     \param[in] texture texture for validation
     \param[out] errorsLog set for validation erros
	 */
    
    void ValidateTexture(Texture *texture, const String &texturePathname, const String &validatedObjectName, Set<String> &errorsLog);


    /*
     \brief Function to validate LandscapeNode errors
     \param[in] landscape landscape for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateLandscape(Landscape *landscape, Set<String> &errorsLog);
        
    /*
     \brief Function to validate Entity errors
     \param[in] sceneNode sceneNode for validation
     \param[out] errorsLog set for validation erros
	 */
    void ValidateSceneNode(Entity *sceneNode, Set<String> &errorsLog);
    
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
    void EnumerateNodes(Scene *scene);
    
    static bool IsTextureChanged(const String &texturePathname, ImageFileFormat fileFormat);
    
	bool ValidateTexturePathname(const String &pathForValidation, Set<String> &errorsLog);
	bool ValidateHeightmapPathname(const String &pathForValidation, Set<String> &errorsLog);

    void CreateDefaultDescriptors(const String &folderPathname);

    bool IsPathCorrectForProject(const String &pathname);

	void CreateDescriptorIfNeed(const String &forPathname);
    
    
    
protected:

    void ValidateRenderComponent(Entity *ownerNode, Set<String> &errorsLog);
    void ValidateLodComponent(Entity *ownerNode, Set<String> &errorsLog);
    void ValidateParticleEmitterComponent(Entity *ownerNode, Set<String> &errorsLog);

    void ValidateRenderBatch(Entity *ownerNode, RenderBatch *renderBatch, Set<String> &errorsLog);
    void ValidateInstanceMaterialState(InstanceMaterialState *materialState, Set<String> &errorsLog);

    
    
    int32 EnumerateSceneNodes(Entity *node);
    
	void ValidateScalesInternal(Entity *sceneNode, Set<String> &errorsLog);
    
    bool ValidatePathname(const String &pathForValidation, const String &validatedObjectName);

    bool NodeRemovingDisabled(Entity *node);
    
    bool WasTextureChanged(Texture *texture, ImageFileFormat fileFormat);

	bool IsTextureDescriptorPath(const String &path);
    
    bool IsFBOTexture(Texture *texture);
    
	DAVA_DEPRECATED(void ConvertLightmapSizeFromProperty(Entity *ownerNode, InstanceMaterialState *materialState));

    Set<Entity *> emptyNodesForDeletion;
    Set<String> errorMessages;
    
    int32 sceneTextureCount;
    int32 sceneTextureMemory;
    
    String pathForChecking;
};



#endif // __SCENE_VALIDATOR_H__