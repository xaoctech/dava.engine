#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "DAVAEngine.h"
#include "CommandLine/SceneUtils/SceneUtils.h"

using namespace DAVA;

class SceneExporter
{
public:

	SceneExporter();
	virtual ~SceneExporter();
    
    void SetGPUForExporting(const String &newGPU);
    void SetGPUForExporting(const eGPUFamily newGPU);
    
    void CleanFolder(const FilePath &folderPathname, Set<String> &errorLog);
    
    void SetInFolder(const FilePath &folderPathname);
    void SetOutFolder(const FilePath &folderPathname);
    
    void ExportFile(const String &fileName, Set<String> &errorLog);
    void ExportFolder(const String &folderName, Set<String> &errorLog);
    
    void ExportScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog);
    
protected:
    
    void RemoveEditorNodes(Entity *rootNode);
    
    void ExportDescriptors(Scene *scene, Set<String> &errorLog);
    bool ExportTextureDescriptor(const FilePath &pathname, Set<String> &errorLog);
    bool ExportTexture(const TextureDescriptor * descriptor, Set<String> &errorLog);
    void CompressTextureIfNeed(const TextureDescriptor * descriptor, Set<String> &errorLog);

    void ExportLandscape(Scene *scene, Set<String> &errorLog);
    void ExportLandscapeFullTiledTexture(Landscape *landscape, Set<String> &errorLog);

    
    
    
    
protected:
    
    SceneUtils sceneUtils;

    eGPUFamily exportForGPU;
};



#endif // __SCENE_EXPORTER_H__