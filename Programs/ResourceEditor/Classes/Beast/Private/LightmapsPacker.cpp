#if defined(__DAVAENGINE_BEAST__)

#include "Classes/Beast/Private/LightmapsPacker.h"

#include <REPlatform/Scene/Utils/Utils.h>

#include <FileSystem/File.h>
#include <FileSystem/FileList.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/TextureDescriptor.h>

void LightmapsPacker::ParseSpriteDescriptors(const DAVA::FilePath& inputFolder)
{
    DAVA::ScopedPtr<DAVA::FileList> fileList(new DAVA::FileList(inputFolder));

    DAVA::UnorderedMap<DAVA::String, DAVA::Vector2> texturesSize;

    DAVA::int32 itemsCount = fileList->GetCount();
    for (DAVA::int32 i = 0; i < itemsCount; ++i)
    {
        const DAVA::FilePath& filePath = fileList->GetPathname(i);
        if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
        {
            ParseSpriteDescriptors(filePath);
        }
        else if (filePath.IsEqualToExtension(".txt"))
        {
            DAVA::char8 buf[512] = {};
            DAVA::String fileName;
            {
                DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(filePath, DAVA::File::OPEN | DAVA::File::READ));
                file->ReadLine(buf, sizeof(buf)); // textures count

                DAVA::uint32 fileNameSize = file->ReadLine(buf, sizeof(buf)); // texture name
                fileName = DAVA::String(buf, DAVA::Min(fileNameSize, DAVA::uint32(strlen(buf))));

                file->ReadLine(buf, sizeof(buf)); // image size
                file->ReadLine(buf, sizeof(buf)); // frames count
                file->ReadLine(buf, sizeof(buf)); // frame rect
            }
            DAVA::FileSystem::Instance()->DeleteFile(filePath);

            Beast::LightmapAtlasingData data;
            data.meshInstanceName = filePath.GetBasename();
            data.textureName = outputDir + fileName;

            DAVA::int32 x, y, dx, dy, unused0, unused1, unused2;
            sscanf(buf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &unused0, &unused1, &unused2);

            DAVA::String absoluteFileName = (inputFolder + fileName).GetAbsolutePathname();
            if (texturesSize.count(absoluteFileName) == 0)
            {
                texturesSize.emplace(absoluteFileName, GetTextureSize(absoluteFileName));
            }

            DAVA::Vector2 textureSize = texturesSize[absoluteFileName];
            data.uvOffset = DAVA::Vector2(static_cast<DAVA::float32>(x) / textureSize.x, static_cast<DAVA::float32>(y) / textureSize.y);
            data.uvScale = DAVA::Vector2(static_cast<DAVA::float32>(dx) / textureSize.x, static_cast<DAVA::float32>(dy) / textureSize.y);

            atlasingData.push_back(data);
        }
    }
}

void LightmapsPacker::ParseSpriteDescriptors()
{
    ParseSpriteDescriptors(outputDir);
}

DAVA::Vector2 LightmapsPacker::GetTextureSize(const DAVA::FilePath& filePath)
{
    DAVA::Vector2 ret;

    DAVA::FilePath sourceTexturePathname = DAVA::FilePath::CreateWithNewExtension(filePath, DAVA::TextureDescriptor::GetLightmapTextureExtension());

    DAVA::ScopedPtr<DAVA::Image> image(DAVA::ImageSystem::LoadSingleMip(sourceTexturePathname));
    if (image)
    {
        ret.x = static_cast<DAVA::float32>(image->GetWidth());
        ret.y = static_cast<DAVA::float32>(image->GetHeight());
    }

    return ret;
}

DAVA::Vector<Beast::LightmapAtlasingData>* LightmapsPacker::GetAtlasingData()
{
    return &atlasingData;
}

void LightmapsPacker::CreateDescriptors(const DAVA::FilePath& inputFolder)
{
    DAVA::ScopedPtr<DAVA::FileList> fileList(new DAVA::FileList(inputFolder));

    DAVA::int32 itemsCount = fileList->GetCount();
    for (DAVA::int32 i = 0; i < itemsCount; ++i)
    {
        const DAVA::FilePath& filePath = fileList->GetPathname(i);
        if (fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
        {
            CreateDescriptors(filePath);
        }
        else if (filePath.IsEqualToExtension(".png"))
        {
            DAVA::TextureDescriptor descriptor;
            descriptor.Save(DAVA::TextureDescriptor::GetDescriptorPathname(filePath));
        }
    }
}

void LightmapsPacker::CreateDescriptors()
{
    CreateDescriptors(outputDir);
}

#endif //#if defined(__DAVAENGINE_BEAST__)
