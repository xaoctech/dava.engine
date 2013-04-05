//
//  ResourcePacker.h
//  UIEditor
//
//  Created by Denis Bespalov on 1/10/13.
//  Originaly Created by Vitaliy  Borodovsky on 3/21/10  for class ResourcePackerScreen.
//
//

#ifndef __DAVAENGINE_RESOURCEPACKER_H__
#define __DAVAENGINE_RESOURCEPACKER_H__

#include "DAVAEngine.h"
#include "TexturePacker/DefinitionFile.h"

using namespace DAVA;

class ResourcePacker
{
public:
	typedef std::map<String, String> FILESMAP;
	ResourcePacker();
	// Set packing properties and lanch process of packing
	void PackResources(const FilePath & inputDir, const FilePath & outputDir);
	
private:
	// Packing of resources section
	void StartPacking();
	
	FilePath inputGfxDirectory;
	FilePath outputGfxDirectory;
	FilePath excludeDirectory;
	String gfxDirName;

	void RecursiveTreeWalk(const FilePath & inputPath, const FilePath & outputPath);
	bool IsModifyDateChagedDir(const FilePath & processDirectoryPath, const FilePath & pathname);
	bool IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName, bool isRecursive);
	bool IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const FilePath & psdName);
	DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const FilePath & psdName);
	void ProcessFlags(const FilePath & flagsPathname);
	String GetProcessFolderName();
	bool SaveFileListToYaml(const FilePath & yamlFilePath);
	bool CheckSpriteFilesDates(YamlNode *rootNode);
	void FillSpriteFilesMap(const FilePath & inputPathName);
	
	String currentFlags;
	bool isGfxModified;
	bool isLightmapsPacking;
	bool clearProcessDirectory;
	FILESMAP spriteFiles;
};

#endif // __DAVAENGINE_RESOURCEPACKER_H__
