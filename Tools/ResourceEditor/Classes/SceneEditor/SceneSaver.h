#ifndef __SCENE_SAVER_H__
#define __SCENE_SAVER_H__

#include "DAVAEngine.h"
#include "SceneUtils.h"

using namespace DAVA;

class SceneSaver: public DAVA::Singleton<SceneSaver>
{
public:
	SceneSaver();
	virtual ~SceneSaver();
    
    void SetInFolder(const String &folderPathname);
    void SetOutFolder(const String &folderPathname);
    
    void SaveFile(const String &fileName, Set<String> &errorLog);
	void ResaveFile(const String &fileName, Set<String> &errorLog);
    void SaveScene(Scene *scene, const String &fileName, Set<String> &errorLog);
    
protected:
    
    void ReleaseTextures();

    void CopyTextures(Scene *scene, Set<String> &errorLog);
    void CopyTexture(const String &texturePathname, Set<String> &errorLog);

	void CopyReferencedObject(Entity *node, Set<String> &errorLog);

protected:
    
    SceneUtils sceneUtils;
    
    Map<String, Texture *> texturesForSave;
};



#endif // __SCENE_SAVER_H__