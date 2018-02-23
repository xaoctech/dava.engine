#include "REPlatform/Scene/Utils/RETextureDescriptorUtils.h"
#include "REPlatform/Scene/Utils/ImageTools.h"
#include "REPlatform/Scene/Utils/Preset.h"

#include <FileSystem/FileList.h>
#include <FileSystem/KeyedArchive.h>
#include <Functional/Function.h>
#include <Logger/Logger.h>
#include <Render/Image/ImageSystem.h>

namespace DAVA
{
namespace RETextureDescriptorUtils
{
namespace Internal
{
bool IsCorrectDirectory(FileList* fileList, const uint32 fileIndex)
{
    if (fileList->IsDirectory(fileIndex))
    {
        String name = fileList->GetFilename(fileIndex);
        if (0 != CompareCaseInsensitive(String(".svn"), name) && !fileList->IsNavigationDirectory(fileIndex))
        {
            return true;
        }
    }

    return false;
}

template <typename FileActionFunctor, typename... Args>
void RecursiveWalk(FileActionFunctor fileAction, const FilePath& folderPath, Args... args)
{
    ScopedPtr<FileList> fileList(new FileList(folderPath));
    for (uint32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath& pathname = fileList->GetPathname(fi);
        if (IsCorrectDirectory(fileList, fi))
        {
            RecursiveWalk(fileAction, pathname, args...);
        }
        else
        {
            fileAction(pathname, args...);
        }
    }
}

bool CreateOrUpdateDescriptor(const FilePath& texturePath, const KeyedArchive* preset)
{
    bool descriptorChanged = false;
    std::unique_ptr<TextureDescriptor> descriptor;

    if (!Texture::CreateDescriptor(texturePath, descriptor, descriptorChanged))
        return false;

    if (preset != nullptr && Preset::ApplyTexturePreset(descriptor.get(), preset))
    {
        descriptorChanged = true;
    }

    if (descriptorChanged)
    {
        FilePath descriptorPath = TextureDescriptor::GetDescriptorPathname(texturePath);
        descriptor->Save(descriptorPath);
    }

    return true;
}

void CreateDescriptorsForFolder(const FilePath& folderPath, const KeyedArchive* preset)
{
    auto fileAction = DAVA::MakeFunction(&Internal::CreateOrUpdateDescriptor);
    RecursiveWalk(fileAction, folderPath, preset);
}

void SetPreset(const FilePath& descriptorPath, const KeyedArchive* preset, bool toConvert, TextureConverter::eConvertQuality quality)
{
    if (false == TextureDescriptor::IsDescriptorExtension(descriptorPath.GetExtension()))
    {
        return;
    }

    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPath));
    if (!descriptor)
    {
        return;
    }

    bool applied = Preset::ApplyTexturePreset(descriptor.get(), preset);
    if (applied)
    {
        if (toConvert)
        {
            for (uint8 gpu = 0; gpu < GPU_FAMILY_COUNT; ++gpu)
            {
                DAVA::eGPUFamily eGPU = static_cast<eGPUFamily>(gpu);
                ImageTools::ConvertImage(descriptor.get(), eGPU, quality);
            }
        }

        descriptor->Save();
    }
}

void SetPresetForFolder(const FilePath& folder, const KeyedArchive* preset, bool toConvert, TextureConverter::eConvertQuality quality)
{
    auto fileAction = DAVA::MakeFunction(&Internal::SetPreset);
    RecursiveWalk(fileAction, folder, preset, toConvert, quality);
}

} // namespace Internal

void ResaveDescriptorsForFolder(const FilePath& folderPath)
{
    auto fileAction = DAVA::MakeFunction(&ResaveDescriptor);
    Internal::RecursiveWalk(fileAction, folderPath);
}

void ResaveDescriptor(const FilePath& descriptorPath)
{
    if (TextureDescriptor::IsDescriptorExtension(descriptorPath.GetExtension()))
    {
        std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPath));
        DVASSERT(descriptor);
        descriptor->Save();
    }
}

void CreateDescriptorsForFolder(const DAVA::FilePath& folder, const DAVA::FilePath& presetPath)
{
    ScopedPtr<KeyedArchive> preset(Preset::LoadArchive(presetPath));
    Internal::CreateDescriptorsForFolder(folder, preset);
}

bool CreateOrUpdateDescriptor(const DAVA::FilePath& texturePath, const DAVA::FilePath& presetPath)
{
    ScopedPtr<KeyedArchive> preset(Preset::LoadArchive(presetPath));
    return Internal::CreateOrUpdateDescriptor(texturePath, preset);
}

void SetCompressionParamsForFolder(const FilePath& folderPath, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression>& compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps)
{
    auto fileAction = DAVA::MakeFunction(&SetCompressionParams);
    Internal::RecursiveWalk(fileAction, folderPath, compressionParams, convertionEnabled, force, quality, generateMipMaps);
}

void SetCompressionParams(const FilePath& descriptorPath, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression>& compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps)
{
    if (false == TextureDescriptor::IsDescriptorExtension(descriptorPath.GetExtension()))
    {
        return;
    }

    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPath));
    if (!descriptor)
    {
        return;
    }

    descriptor->dataSettings.SetGenerateMipmaps(generateMipMaps);

    DVASSERT(descriptor->compression);

    for (const auto& compressionParam : compressionParams)
    {
        eGPUFamily gpu = compressionParam.first;

        if (force || (descriptor->compression[gpu].format == FORMAT_INVALID))
        {
            PixelFormat dstFormat = static_cast<PixelFormat>(compressionParam.second.format);
            if (dstFormat == FORMAT_PVR2 || dstFormat == FORMAT_PVR4)
            {
                DAVA::FilePath path = descriptor->GetSourceTexturePathname();
                ImageInfo imgInfo = ImageSystem::GetImageInfo(descriptor->GetSourceTexturePathname());
                if (imgInfo.width != imgInfo.height)
                {
                    DAVA::Logger::Error("Can't set %s compression for non-squared texture %s",
                                        GlobalEnumMap<PixelFormat>::Instance()->ToString(dstFormat),
                                        path.GetAbsolutePathname().c_str());
                    continue;
                }
            }

            descriptor->compression[gpu] = compressionParam.second;

            if (convertionEnabled)
            {
                ImageTools::ConvertImage(descriptor.get(), gpu, quality);
            }
        }
    }

    descriptor->Save();
}

void SetPresetForFolder(const FilePath& folder, const FilePath& presetPath, bool toConvert, TextureConverter::eConvertQuality quality)
{
    ScopedPtr<KeyedArchive> preset(Preset::LoadArchive(presetPath));
    if (preset)
    {
        Internal::SetPresetForFolder(folder, preset, toConvert, quality);
    }
}

void SetPreset(const FilePath& descriptorPath, const FilePath& presetPath, bool toConvert, TextureConverter::eConvertQuality quality)
{
    ScopedPtr<KeyedArchive> preset(Preset::LoadArchive(presetPath));
    if (preset)
    {
        Internal::SetPreset(descriptorPath, preset, toConvert, quality);
    }
}

void SavePreset(const DAVA::Vector<DAVA::FilePath>& descriptors, const DAVA::Vector<DAVA::FilePath>& presets)
{
    if (descriptors.size() != presets.size())
    {
        Logger::Error("Descriptors size differs from presets size");
        return;
    }

    size_t count = descriptors.size();
    for (size_t i = 0; i < count; ++i)
    {
        std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptors[i]));
        if (!descriptor)
        {
            Logger::Error("Cannot create descriptor from file %s", descriptors[i].GetStringValue().c_str());
            continue;
        }

        ScopedPtr<KeyedArchive> presetArchive(new KeyedArchive());
        if (descriptor->SerializeToPreset(presetArchive) == false)
        {
            Logger::Error("Can't create preset from descriptor");
            continue;
        }

        FileSystem::Instance()->CreateDirectory(presets[i].GetDirectory(), true);
        if (Preset::SaveArchive(presetArchive, presets[i]) == false)
        {
            Logger::Error("Can't save preset as %s", presets[i].GetStringValue().c_str());
            continue;
        }
    }
}
} // namespace RETextureDescriptorUtils
} // namespace DAVA
