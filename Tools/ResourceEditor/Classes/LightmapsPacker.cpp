#include "LightmapsPacker.h"

#include "Qt/Main/QtUtils.h"

LightmapsPacker::LightmapsPacker()
{
}

void LightmapsPacker::ParseSpriteDescriptors()
{
	FileList * fileList = new FileList(outputDir);

	char8 buf[512];
	uint32 readSize; 

	int32 itemsCount = fileList->GetCount();
	for(int32 i = 0; i < itemsCount; ++i)
	{
		const FilePath & filePath = fileList->GetPathname(i);
		if(fileList->IsDirectory(i) || !filePath.IsEqualToExtension(".txt"))
		{
			continue;
		}

		LightmapAtlasingData data;

        FilePath meshname = FilePath(filePath);
        meshname.TruncateExtension();
		data.meshInstanceName = meshname.GetAbsolutePathname();
        
		File * file = File::Create(filePath, File::OPEN | File::READ);
		
		file->ReadLine(buf, sizeof(buf)); //textures count

		readSize = file->ReadLine(buf, sizeof(buf)); //texture name
		FilePath originalTextureName = outputDir + FilePath(String(buf, readSize));
		data.textureName = originalTextureName;

		file->ReadLine(buf, sizeof(buf)); //image size

		file->ReadLine(buf, sizeof(buf)); //frames count

		file->ReadLine(buf, sizeof(buf)); //frame rect
		int32 x, y, dx, dy, unused0, unused1, unused2;
		sscanf(buf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &unused0, &unused1, &unused2);
		dx++;//cause TexturePacker::ReduceRectToOriginalSize removed one pixel by default
		dy++;

		Vector2 textureSize = GetTextureSize(originalTextureName);
		data.uvOffset = Vector2((float32)x/textureSize.x, (float32)y/textureSize.y);
		data.uvScale = Vector2((float32)dx/textureSize.x, (float32)dy/textureSize.y);
		
		file->Release();

		atlasingData.push_back(data);

		FileSystem::Instance()->DeleteFile(filePath);
	}

	fileList->Release();
}

Vector2 LightmapsPacker::GetTextureSize(const FilePath & filePath)
{
	Vector2 ret;

	FilePath sourceTexturePathname(filePath);
    sourceTexturePathname.ReplaceExtension(TextureDescriptor::GetSourceTextureExtension());
    Image * image = CreateTopLevelImage(sourceTexturePathname);
    if(image)
    {
        ret.x = (float32)image->GetWidth();
        ret.y = (float32)image->GetHeight();
        
        SafeRelease(image);
    }

	return ret;
}

Vector<LightmapAtlasingData> * LightmapsPacker::GetAtlasingData()
{
	return &atlasingData;
}

void LightmapsPacker::Compress()
{
	FileList * fileList = new FileList(outputDir);

	int32 itemsCount = fileList->GetCount();
	for(int32 i = 0; i < itemsCount; ++i)
	{
		const FilePath & filePath = fileList->GetPathname(i);
		if(fileList->IsDirectory(i) || !filePath.IsEqualToExtension(".png"))
		{
			continue;
		}

		TextureDescriptor descriptor;
        descriptor.Save(TextureDescriptor::GetDescriptorPathname(filePath));
	}

	fileList->Release();
}
