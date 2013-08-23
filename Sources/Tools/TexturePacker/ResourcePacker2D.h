#ifndef __DAVAENGINE_RESOURCEPACKER2D_H__
#define __DAVAENGINE_RESOURCEPACKER2D_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class DefinitionFile;
class YamlNode;

class ResourcePacker2D
{
public:
	typedef std::map<String, String> FILESMAP;
	ResourcePacker2D();

	// Packing of resources section
	void InitFolders(const FilePath & inputPath,const FilePath & outputPath);
	void PackResources(eGPUFamily forGPU);
    
	void RecursiveTreeWalk(const FilePath & inputPath,const FilePath & outputPath);
	bool IsModifyDateChagedDir(const FilePath & processDirectoryPath, const FilePath & pathname);
	bool IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName, bool isRecursive);
	bool IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName);
	
    DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName);
	void ProcessFlags(const FilePath & flagsPathname);

	static String GetProcessFolderName();
	bool SaveFileListToYaml(const FilePath & yamlFilePath);
	bool CheckSpriteFilesDates(YamlNode *rootNode);
	void FillSpriteFilesMap(const FilePath & inputPathName);
public:
    
	FilePath inputGfxDirectory;
	FilePath outputGfxDirectory;
	FilePath excludeDirectory;
	String gfxDirName;
	String currentFlags;
    
	bool isGfxModified;
    
	bool isLightmapsPacking;
	bool clearProcessDirectory;
    eGPUFamily requestedGPUFamily;
 	FILESMAP spriteFiles;
};
};


#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
