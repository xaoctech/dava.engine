/*
 *  TestScreen.h
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __DAVAENGINE_RESOURCEPACKERSCREEN_H__
#define __DAVAENGINE_RESOURCEPACKERSCREEN_H__


#include "DAVAEngine.h"
#include "TexturePacker/DefinitionFile.h"
#include "UISpriteEditor.h"
#include "UIFileTree.h"

using namespace DAVA;

class ResourcePackerScreen : public UIScreen, public UIFileTreeDelegate
{
public:
	ResourcePackerScreen();

	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();

	virtual void Draw(const UIGeometricData &geometricData);
	virtual void Input(UIEvent * touch);
	
	// Packing of resources section
	void PackResources();
	
	FilePath inputGfxDirectory;
	FilePath outputGfxDirectory;
	FilePath excludeDirectory;
	String gfxDirName;

	void RecursiveTreeWalk(const FilePath & inputPath,const FilePath & outputPath);
	bool IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const FilePath & psdName, bool isRecursive);
	bool IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const FilePath & psdName);
	DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const FilePath & psdName);
	void ProcessFlags(const FilePath & flagsPathname);
	FilePath GetProcessFolderName();
	String currentFlags;

	// Resource Tree
	UIFileTree * resourceTree;
	void OnCellSelected(UIFileTree * tree, UIFileTreeCell *selectedCell);
    virtual int32 CellHeight(UIList *forList, int32 index);
	virtual UIFileTreeCell *CellAtIndex(UIFileTree * tree, UITreeItemInfo *entry, int32 index);

	// Sprite Editor Logic
	void OpenSpriteEditor(const FilePath & spriteName);
	UISpriteEditor * spriteEditor;

	
	bool isGfxModified;

	bool isLightmapsPacking;
	bool clearProcessDirectory;
};

#endif // __DAVAENGINE_RESOURCEPACKERSCREEN_H__
