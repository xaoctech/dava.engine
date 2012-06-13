#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "DAVAEngine.h"

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
    
    String NormalizeFolderPath(const String &pathname);
    String RemoveFolderFromPath(const String &pathname, const String &folderPathname);
    
    void RemoveEditorNodes(SceneNode *rootNode);
    
    void ExportMaterials(Scene *scene, Set<String> &errorLog);
    void ExportLandscape(Scene *scene, Set<String> &errorLog);
    void ExportMeshLightmaps(Scene *scene, Set<String> &errorLog);
    void ExportFileDirectly(const String &filePathname, Set<String> &errorLog);
    String ExportTexture(const String &texturePathname, Set<String> &errorLog);
    
protected:
    
    String dataFolder;
    String dataSourceFolder; 
    String workingFolder;

    String format;
};



#endif // __SCENE_EXPORTER_H__