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
	ResourcePacker();
	// Set packing properties and lanch process of packing
	void PackResources(const String & inputDir, const String & outputDir);
	
private:
	// Packing of resources section
	void StartPacking();
	
	String inputGfxDirectory;
	String outputGfxDirectory;
	String excludeDirectory;
	String gfxDirName;

	void RecursiveTreeWalk(const String & inputPath, const String & outputPath);
	bool IsMD5ChangedDir(const String & processDirectoryPath, const String & pathname, const String & psdName, bool isRecursive);
	bool IsMD5ChangedFile(const String & processDirectoryPath, const String & pathname, const String & psdName);
	DefinitionFile * ProcessPSD(const String & processDirectoryPath, const String & psdPathname, const String & psdName);
	void ProcessFlags(const String & flagsPathname);
	String GetProcessFolderName();
	String currentFlags;
	
	bool isGfxModified;
	bool isLightmapsPacking;
	bool clearProcessDirectory;
};

#endif // __DAVAENGINE_RESOURCEPACKER_H__
