/*
 *  DefinitionFile.h
 *  texturepack
 *
 *  Created by Vitaliy Borodovsky on 10/28/08.
 *  Copyright 2008 DAVA Consulting, LLC. All rights reserved.
 *
 */

#ifndef __DEFINITION_FILE_H__
#define __DEFINITION_FILE_H__

#include "DAVAEngine.h"

using namespace DAVA;


class DefinitionFile 
{
public:
	bool Load(const FilePath & filename);
	bool LoadPNGDef(const FilePath & filename, const FilePath & pathToProcess);
	
	DefinitionFile();
	~DefinitionFile();
	
	void ClearPackedFrames();
	void LoadPNG(const FilePath & fullname, const FilePath & processDirectoryPath);

	FilePath filename;
	int			frameCount;
	int			spriteWidth;
	int			spriteHeight;
	Rect2i		* frameRects;

	Vector<String> pathsInfo;
};

#endif