#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "DAVAEngine.h"
#include "SceneUtils.h"

using namespace DAVA;

class SceneExporter: public DAVA::Singleton<SceneExporter>
{
public:

	SceneExporter();
	virtual ~SceneExporter();
    
    void SetExportingFormat(const String &newFormat);
    
    void CleanFolder(const String &folderPathname, Set<String> &errorLog);
    
    void SetInFolder(const String &folderPathname);
    void SetOutFolder(const String &folderPathname);
    
    void ExportFile(const String &fileName, Set<String> &errorLog);
    void ExportFolder(const String &folderName, Set<String> &errorLog);
    
    void ExportScene(Scene *scene, const String &fileName, Set<String> &errorLog);
    
protected:
    
    void RemoveEditorNodes(Entity *rootNode);
    
    void ExportLandscape(Scene *scene, Set<String> &errorLog);
    void ExportLandscapeFullTiledTexture(Landscape *landscape, Set<String> &errorLog);
    bool ExportTexture(const String &texturePathname, Set<String> &errorLog);
    bool ExportTextureDescriptor(const String &texturePathname, Set<String> &errorLog);
    
    void ExportTextures(Scene *scene, Set<String> &errorLog);
    
    void ReleaseTextures();
    
    void CompressTextureIfNeed(const String &texturePathname, Set<String> &errorLog);
    
    
protected:
    
    SceneUtils sceneUtils;

    ImageFileFormat exportFormat;
    
    Map<String, Texture *> texturesForExport;
};



#endif // __SCENE_EXPORTER_H__