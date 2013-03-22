/*
 *  TexturePacker.h
 *  texturepack
 *
 *  Created by Vitaliy Borodovsky on 10/28/08.
 *  Copyright 2008 DAVA Consulting, LLC. All rights reserved.
 *
 */
#ifndef __DAVAENGINE_TEXTURE_PACKER_H__
#define __DAVAENGINE_TEXTURE_PACKER_H__

#include <string>
#include <list>
#include <vector>
#include "DefinitionFile.h"
#include "ImagePacker.h"


struct SizeSortItem
{
	int					imageSize;
	DefinitionFile *	defFile;
	int					frameIndex;
};

class DAVA::TextureDescriptor;
class PngImageExt;
class TexturePacker 
{
public:
	TexturePacker();
	
	// pack textures to single texture
	void PackToTextures(const FilePath & excludeFolder, const FilePath & outputPath, std::list<DefinitionFile*> & defsList);
	// page each PSD file to separate texture
	void PackToTexturesSeparate(const FilePath & excludeFolder, const FilePath & outputPath, std::list<DefinitionFile*> & defsList);
	// pack one sprite and use several textures if more than one needed
	void PackToMultipleTextures(const FilePath & excludeFolder, const FilePath & outputPath, std::list<DefinitionFile*> & remainingList);

	bool TryToPack(const Rect2i & textureRect, std::list<DefinitionFile*> & defsList);
	bool WriteDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const FilePath & textureName, DefinitionFile * defFile);
	bool WriteMultipleDefinition(const FilePath & excludeFolder, const FilePath & outputPath, const FilePath & _textureName, DefinitionFile * defFile);

	int TryToPackFromSortVector(ImagePacker * packer, std::vector<SizeSortItem> & tempSortVector);
	float TryToPackFromSortVectorWeight(ImagePacker * packer,std::vector<SizeSortItem> & tempSortVector);

	Rect2i ReduceRectToOriginalSize(const Rect2i & _input);

	void UseOnlySquareTextures();

	void SetMaxTextureSize(int32 maxTextureSize);
	
private:
    
    void ExportImage(PngImageExt *image, const FilePath &exportedPathname);
    DAVA::TextureDescriptor * CreateDescriptor();
    PixelFormat DetectPixelFormatFromFlags();
    
    
	ImagePacker *			lastPackedPacker;
	Vector<ImagePacker*> usedPackers;

	Vector<SizeSortItem> sortVector;
	int32 maxTextureSize;

	bool onlySquareTextures;
};

#endif // __DAVAENGINE_TEXTURE_PACKER_H__

