#include "Render/TextureAssetLoader.h"
#include "Base/Hash.h"
#include "Image/Image.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/Texture.h"

namespace DAVA
{
namespace TextureAssetLoaderDetails
{
size_t PathKeyHash(const Any& v)
{
    const Texture::PathKey& key = v.Get<Texture::PathKey>();

    const String& pathValue = key.path.GetStringValue();
    uint32 pathHash = HashValue_N(pathValue.data(), static_cast<uint32>(pathValue.size()));
    uint32 groupHash = 0;

    size_t seed = 0;
    HashCombine(seed, pathHash);
    HashCombine(seed, groupHash);
    HashCombine(seed, static_cast<size_t>(key.requestedGpu));
    HashCombine(seed, static_cast<size_t>(key.typeHint));
    return seed;
}

size_t UniqueTextureKeyHash(const Any& v)
{
    const Texture::UniqueTextureKey& key = v.Get<Texture::UniqueTextureKey>();
    return key.uniqueKey;
}

size_t RenderTargetTextureKeyHash(const Any& v)
{
    const Texture::RenderTargetTextureKey& key = v.Get<Texture::RenderTargetTextureKey>();

    size_t seed = 0;
    HashCombine(seed, key.width);
    HashCombine(seed, key.height);
    HashCombine(seed, key.sampleCount);
    HashCombine(seed, key.mipLevelsCount);
    HashCombine(seed, static_cast<uint32>(key.format));
    HashCombine(seed, static_cast<uint32>(key.textureType));
    HashCombine(seed, key.isDepth);
    HashCombine(seed, key.needPixelReadback);
    HashCombine(seed, key.ensurePowerOf2);
    uint32 hash = HashValue_N(key.uniqueKey.data(), static_cast<uint32>(key.uniqueKey.size()));
    HashCombine(seed, hash);
    return seed;
}

bool IsFormatHardwareSupported(PixelFormat format)
{
    const PixelFormatDescriptor& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    return formatDescriptor.isHardwareSupported;
}

bool AreImagesSquare(const Vector<Image*>& imageSet)
{
    for (Image* image : imageSet)
    {
        if (!IsPowerOf2(image->GetWidth()) || !IsPowerOf2(image->GetHeight()))
        {
            return false;
        }
    }

    return true;
}

bool AreImagesCorrectForTexture(const Vector<Image*>& imageSet)
{
    if (0 == imageSet.size())
    {
        return false;
    }

    if (imageSet[0]->width < Texture::MINIMAL_WIDTH || imageSet[0]->height < Texture::MINIMAL_HEIGHT)
    {
        Logger::Error("[TextureValidator] Loaded images size is too small. Minimal size for texture is 8x8");
        return false;
    }

    bool isSizeCorrect = AreImagesSquare(imageSet);
    if (!isSizeCorrect)
    {
        Logger::Error("[TextureValidator] Size if loaded images is invalid (not power of 2)");
        return false;
    }

    return true;
}

template <typename T>
struct ImageTraits
{
    static Image* Get(T& img)
    {
        return nullptr;
    }
    static void Release(T& img)
    {
    }
};

template <>
struct ImageTraits<RefPtr<Image>>
{
    static Image* Get(RefPtr<Image>& img)
    {
        return img.Get();
    }
    static void Release(RefPtr<Image>& img)
    {
        img = RefPtr<Image>();
    }
};

template <>
struct ImageTraits<Image*>
{
    static Image* Get(Image*& img)
    {
        return img;
    }
    static void Release(Image*& img)
    {
        img->Release();
    }
};

template <typename TImage>
bool CheckAndFixImageFormat(Vector<TImage>& imageSet)
{
    static_assert(std::is_same<TImage, Image*>::value || std::is_same<TImage, RefPtr<Image>>::value, "");

    PixelFormat format = imageSet[0]->format;
    if (IsFormatHardwareSupported(format))
    {
        return true;
    }

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_WIN_UAP__)
    //we should decode all images for RE/QE
    if (ImageConvert::CanConvertFromTo(format, FORMAT_RGBA8888))
#else
    //We should decode only RGB888 into RGBA8888
    if (format == PixelFormat::FORMAT_RGB888 && ImageConvert::CanConvertFromTo(format, FORMAT_RGBA8888))
#endif
    {
        for (TImage& image : imageSet)
        {
            TImage newImage(Image::Create(image->width, image->height, FORMAT_RGBA8888));
            bool converted = ImageConvert::ConvertImage(ImageTraits<TImage>::Get(image), ImageTraits<TImage>::Get(newImage));
            if (converted)
            {
                newImage->mipmapLevel = image->mipmapLevel;
                newImage->cubeFaceID = image->cubeFaceID;

                ImageTraits<TImage>::Release(image);
                image = newImage;
            }
            else
            {
                ImageTraits<TImage>::Release(newImage);
                return false;
            }
        }

        return true;
    }

    return false;
}

eGPUFamily GetGPUForLoading(const TextureDescriptor* descriptor, eGPUFamily requestedGPU)
{
    if (descriptor->IsCompressedFile())
        return descriptor->gpu;

    return requestedGPU;
}

bool IsLoadAvailable(const TextureDescriptor* descriptor, eGPUFamily requestedGPU)
{
    if (descriptor->IsCompressedFile())
    {
        return true;
    }

    if (GPUFamilyDescriptor::IsGPUForDevice(requestedGPU) && descriptor->compression[requestedGPU].format == FORMAT_INVALID)
    {
        return false;
    }

    return true;
}

bool LoadImages(const TextureDescriptor* descriptor, eGPUFamily requestedGPU, Vector<RefPtr<Image>>& images)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(requestedGPU != GPU_INVALID);

    if (!IsLoadAvailable(descriptor, requestedGPU))
    {
        Logger::Error("[Texture::LoadImages] Load not available: invalid requested GPU family (%s)", GlobalEnumMap<eGPUFamily>::Instance()->ToString(requestedGPU));

        //GFX_COMPLETE -- hack to load hdr textures on devices
        //return false;
        requestedGPU = GPU_ORIGIN;
    }

    uint32 baseMipMap = TextureAssetLoader::GetBaseMipMap();
    ImageSystem::LoadingParams params;
    params.baseMipmap = baseMipMap;
    params.firstMipmapIndex = 0;
    params.minimalWidth = Texture::MINIMAL_WIDTH;
    params.minimalHeight = Texture::MINIMAL_HEIGHT;

    Vector<Image*> loadedImages;

    if (descriptor->IsCubeMap() && (!GPUFamilyDescriptor::IsGPUForDevice(requestedGPU)))
    {
        Vector<FilePath> facePathes;
        descriptor->GetFacePathnames(facePathes);

        PixelFormat imagesFormat = FORMAT_INVALID;
        for (uint32 faceIndex = 0; faceIndex < Texture::CUBE_FACE_COUNT; ++faceIndex)
        {
            const FilePath& currentfacePath = facePathes[faceIndex];
            if (currentfacePath.IsEmpty())
            {
                continue;
            }

            Vector<Image*> faceImages;
            ImageSystem::Load(currentfacePath, faceImages, params);

            if (faceImages.empty())
            {
                Logger::Error("[Texture::LoadImages] Cannot open file %s", currentfacePath.GetAbsolutePathname().c_str());
                return false;
            }

            if (FORMAT_INVALID == imagesFormat)
            {
                imagesFormat = faceImages.front()->format;
            }
            else if (imagesFormat != faceImages.front()->format)
            {
                Logger::Error("[Texture::LoadImages] Face(%s) has different pixel format(%s)", currentfacePath.GetAbsolutePathname().c_str(),
                              PixelFormatDescriptor::GetPixelFormatString(faceImages.front()->format));
                return false;
            }

            for (Image* img : faceImages)
            {
                img->cubeFaceID = faceIndex;
            }

            if (descriptor->GetGenerateMipMaps())
            {
                Vector<Image*> mipmapsImages = faceImages.front()->CreateMipMapsImages();
                loadedImages.insert(loadedImages.begin(), mipmapsImages.begin(), mipmapsImages.end());

                for (Image* img : faceImages)
                {
                    SafeRelease(img);
                }
            }
            else
            {
                loadedImages.insert(loadedImages.begin(), faceImages.begin(), faceImages.end());
            }
        }
    }
    else
    {
        Vector<FilePath> singleMipFiles;
        bool hasSingleMipFiles = descriptor->CreateSingleMipPathnamesForGPU(requestedGPU, singleMipFiles);
        if (hasSingleMipFiles)
        {
            uint32 singleMipFilesCount = static_cast<uint32>(singleMipFiles.size());
            for (uint32 index = baseMipMap; index < singleMipFilesCount; ++index)
            {
                params.baseMipmap = 0;
                eErrorCode loadingCode = ImageSystem::Load(singleMipFiles[index], loadedImages, params);
                if (loadingCode == eErrorCode::SUCCESS)
                {
                    ++params.firstMipmapIndex;
                }
            }

            params.baseMipmap = Max(static_cast<int32>(baseMipMap) - static_cast<int32>(singleMipFilesCount), 0);
        }

        FilePath multipleMipPathname = descriptor->CreateMultiMipPathnameForGPU(requestedGPU);
        ImageSystem::Load(multipleMipPathname, loadedImages, params);

        ImageSystem::EnsurePowerOf2Images(loadedImages);
    }

    if (!AreImagesCorrectForTexture(loadedImages))
    {
        return false;
    }

    if (!CheckAndFixImageFormat(loadedImages))
    {
        Logger::Error("[Texture::LoadImages] cannot create texture from images because of wrong image format");
        return false;
    }

    if (loadedImages.size() == 1 && descriptor->GetGenerateMipMaps())
    {
        Image* img = loadedImages.front();
        loadedImages = img->CreateMipMapsImages(descriptor->dataSettings.GetIsNormalMap());
        SafeRelease(img);

        if (loadedImages.empty())
        {
            Logger::Error("[Texture::LoadImages] Can't create mipmaps for GPU (%s) for %s", GlobalEnumMap<eGPUFamily>::Instance()->ToString(requestedGPU), descriptor->pathname.GetStringValue().c_str());
            return false;
        }
    }

    images.reserve(loadedImages.size());
    for (Image* img : loadedImages)
    {
        images.push_back(RefPtr<Image>::ConstructWithRetain(img));
        SafeRelease(img);
    }

    return true;
}

struct TextureRawData
{
    PixelFormat format;

    uint32 width;
    uint32 height;
    bool generateMipmaps;

    std::shared_ptr<uint8[]> data;
};

struct TextureImageData
{
    Vector<RefPtr<Image>> images;
    bool generateMipMaps;
};

} // namespace TextureAssetLoaderDetails

Texture::PathKey Texture::MakePinkKey(rhi::TextureType type)
{
    PathKey key(FilePath(), type);
    return key;
}

Texture::PathKey::PathKey(const FilePath& filePath_)
    : PathKey(filePath_, GPU_INVALID, rhi::TEXTURE_TYPE_2D)
{
}

Texture::PathKey::PathKey(const FilePath& filePath_, rhi::TextureType typeHint_)
    : PathKey(filePath_, GPU_INVALID, typeHint)
{
}

Texture::PathKey::PathKey(const FilePath& filePath_, eGPUFamily requestedGpu_, rhi::TextureType typeHint_)
    : path(filePath_)
    , requestedGpu(requestedGpu_)
    , typeHint(typeHint_)
{
}

bool Texture::PathKey::operator==(const PathKey& other) const
{
    return path == other.path &&
    typeHint == other.typeHint &&
    requestedGpu == other.requestedGpu;
}

Texture::UniqueTextureKey::UniqueTextureKey(PixelFormat format, uint32 width, uint32 height, bool generateMipMaps)
    : UniqueTextureKey(format, width, height, generateMipMaps, nullptr)
{
}

Texture::UniqueTextureKey::UniqueTextureKey(PixelFormat format, uint32 width, uint32 height, bool generateMipMaps, std::shared_ptr<uint8[]> data)
    : uniqueKey(nextUniqueKey++)
{
    using namespace TextureAssetLoaderDetails;

    TextureRawData texData;
    texData.format = format;
    texData.width = width;
    texData.height = height;
    texData.data = data;
    texData.generateMipmaps = generateMipMaps;

    creationData = texData;
}

Texture::UniqueTextureKey::UniqueTextureKey(RefPtr<Image> image, bool generateMipMaps)
    : UniqueTextureKey(Vector<RefPtr<Image>>{ image }, generateMipMaps)
{
}

Texture::UniqueTextureKey::UniqueTextureKey(Vector<RefPtr<Image>>&& images)
    : UniqueTextureKey(std::move(images), false)
{
}

Texture::UniqueTextureKey::UniqueTextureKey(Vector<RefPtr<Image>>&& images, bool generateMipMaps)
    : uniqueKey(nextUniqueKey++)
{
    using namespace TextureAssetLoaderDetails;
    TextureImageData texData;
    texData.generateMipMaps = generateMipMaps;
    texData.images = std::move(images);

    creationData = texData;
}

size_t Texture::UniqueTextureKey::nextUniqueKey = 0;

bool Texture::RenderTargetTextureKey::operator==(const RenderTargetTextureKey& other) const
{
    return width == other.width &&
    height == other.height &&
    sampleCount == other.sampleCount &&
    mipLevelsCount == other.mipLevelsCount &&
    format == other.format &&
    textureType == other.textureType &&
    isDepth == other.isDepth &&
    needPixelReadback == other.needPixelReadback &&
    ensurePowerOf2 == other.ensurePowerOf2 &&
    uniqueKey == other.uniqueKey;
}

Texture::RenderTargetTextureKey::RenderTargetTextureKey()
    : uniqueKey(Format("FBO %u", newUniqueKey++))
{
}

Texture::RenderTargetTextureKey::RenderTargetTextureKey(const String& uniqueName)
    : uniqueKey(uniqueName)
{
}

uint32 Texture::RenderTargetTextureKey::newUniqueKey = 0;

TextureAssetLoader::TextureAssetLoader()
{
    using namespace TextureAssetLoaderDetails;
    AnyHash<Texture::PathKey>::Register(&PathKeyHash);
    AnyHash<Texture::UniqueTextureKey>::Register(&UniqueTextureKeyHash);
    AnyHash<Texture::RenderTargetTextureKey>::Register(&RenderTargetTextureKeyHash);

    // PathKey
    {
        SubKeySupport& keySupport = keySupportMap[Type::Instance<Texture::PathKey>()];
        keySupport.getFileInfo = MakeFunction(this, &TextureAssetLoader::GetPathKeyFileInfo);
        keySupport.loadAsset = MakeFunction(this, &TextureAssetLoader::LoadPathKeyAsset);
        keySupport.getDependOn = MakeFunction(this, &TextureAssetLoader::GetPathKeyDependsOn);
    }

    // UniqueTextureKey
    {
        SubKeySupport& keySupport = keySupportMap[Type::Instance<Texture::UniqueTextureKey>()];
        keySupport.getFileInfo = MakeFunction(this, &TextureAssetLoader::GetUniqueTextureKeyFileInfo);
        keySupport.loadAsset = MakeFunction(this, &TextureAssetLoader::LoadUniqueTextureKeyAsset);
        keySupport.getDependOn = MakeFunction(this, &TextureAssetLoader::GetUniqueTextureKeyDependsOn);
    }

    // RenderTargetTextureKey
    {
        SubKeySupport& keySupport = keySupportMap[Type::Instance<Texture::RenderTargetTextureKey>()];
        keySupport.getFileInfo = MakeFunction(this, &TextureAssetLoader::GetRenderTargetTextureKeyFileInfo);
        keySupport.loadAsset = MakeFunction(this, &TextureAssetLoader::LoadRenderTargetTextureKeyAsset);
        keySupport.getDependOn = MakeFunction(this, &TextureAssetLoader::GetRenderTargetTextureKeyDependsOn);
    }
}

AssetFileInfo TextureAssetLoader::GetAssetFileInfo(const Any& assetKey) const
{
    const Type* keyType = assetKey.GetType();
    auto iter = keySupportMap.find(keyType);
    DVASSERT(iter != keySupportMap.end());
    const SubKeySupport& keySupport = iter->second;
    return keySupport.getFileInfo(assetKey);
}

bool TextureAssetLoader::ExistsOnDisk(const Any& assetKey) const
{
    const Type* keyType = assetKey.GetType();
    if (keyType == Type::Instance<Texture::PathKey>())
    {
        const Texture::PathKey& key = assetKey.Get<Texture::PathKey>();
        FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(key.path);
        std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
        if (nullptr == descriptor)
        {
            return false;
        }
    }

    return true;
}

AssetBase* TextureAssetLoader::CreateAsset(const Any& assetKey) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    Texture* result = new Texture(assetKey);
    Function<void(void)> callback = Bind(&TextureAssetLoader::RestoreRenderResource, this, result);
    Renderer::GetSignals().needRestoreResources.Connect(result, callback);

    return result;
}

void TextureAssetLoader::DeleteAsset(AssetBase* asset) const
{
    Renderer::GetSignals().needRestoreResources.Disconnect(asset);
    delete asset;
}

void TextureAssetLoader::LoadAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMessage) const
{
    const Type* keyType = asset->GetKey().GetType();
    auto iter = keySupportMap.find(keyType);
    DVASSERT(iter != keySupportMap.end());
    const SubKeySupport& keySupport = iter->second;

    keySupport.loadAsset(asset, file, reloading, errorMessage);
}

bool TextureAssetLoader::SaveAsset(Asset<AssetBase> asset, File* file, eSaveMode requestedMode) const
{
    return false;
}

bool TextureAssetLoader::SaveAssetFromData(const Any& data, File* file, eSaveMode requestedMode) const
{
    return false;
}

Vector<String> TextureAssetLoader::GetDependsOnFiles(const AssetBase* asset) const
{
    const Type* keyType = asset->GetKey().GetType();
    auto iter = keySupportMap.find(keyType);
    DVASSERT(iter != keySupportMap.end());
    const SubKeySupport& keySupport = iter->second;

    return keySupport.getDependOn(asset);
}

Vector<const Type*> TextureAssetLoader::GetAssetKeyTypes() const
{
    Vector<const Type*> types;
    types.reserve(keySupportMap.size());
    for (auto& node : keySupportMap)
    {
        types.push_back(node.first);
    }
    return types;
}

void TextureAssetLoader::SetGPULoadingOrder(const Vector<eGPUFamily>& gpuLoadingOrder_)
{
    LockGuard<Mutex> guard(gpuOrderMutex);
    gpuLoadingOrder = gpuLoadingOrder_;
}

const Vector<eGPUFamily>& TextureAssetLoader::GetGPULoadingOrder() const
{
    return gpuLoadingOrder;
}

eGPUFamily TextureAssetLoader::GetPrimaryGPUForLoading() const
{
    if (gpuLoadingOrder.empty())
    {
        return eGPUFamily::GPU_INVALID;
    }

    return gpuLoadingOrder[0];
}

uint32 TextureAssetLoader::GetBaseMipMap()
{
    return QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Textures>().baseLevel;
}

AssetFileInfo TextureAssetLoader::GetPathKeyFileInfo(const Any& assetKey) const
{
    DVASSERT(assetKey.CanGet<Texture::PathKey>());
    const Texture::PathKey& key = assetKey.Get<Texture::PathKey>();

    AssetFileInfo info;
    info.fileName = key.path.GetAbsolutePathname();
    if (info.fileName.empty() == true)
    {
        info.fileName = "pink";
    }
    info.inMemoryAsset = true;
    return info;
}

void TextureAssetLoader::LoadPathKeyAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMsg) const
{
    Asset<Texture> texture = std::static_pointer_cast<Texture>(asset);
    const Texture::PathKey& key = asset->GetKey().Get<Texture::PathKey>();

#if (DAVA_DEBUG_TEXTURE_DISABLE_LOADING)
    MakePink(asset, key.typeHint, true);
    return;
#else

    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (key.path.IsEmpty() || (key.path.GetType() == FilePath::PATH_IN_MEMORY))
    {
        MakePink(texture, key.typeHint);
        return;
    }

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
    {
        MakePink(texture, key.typeHint);
        return;
    }

    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(key.path);
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    if (nullptr == descriptor)
    {
        MakePink(texture, key.typeHint);
        texture->texDescriptor->pathname = (!key.path.IsEmpty()) ? TextureDescriptor::GetDescriptorPathname(key.path) : FilePath();
        return;
    }

    texture->texDescriptor->Initialize(descriptor.get());
    bool loaded = false;
    Vector<eGPUFamily> loadingOrder;
    if (key.requestedGpu == GPU_INVALID)
    {
        loadingOrder = gpuLoadingOrder;
    }
    else
    {
        loadingOrder.push_back(key.requestedGpu);
    }

    for (eGPUFamily gpu : loadingOrder)
    {
        eGPUFamily gpuForLoading = TextureAssetLoaderDetails::GetGPUForLoading(descriptor.get(), gpu);
        bool loadedGpu = LoadFromImage(texture, gpuForLoading);
        if (loadedGpu == true)
        {
            texture->loadedAsFile = gpuForLoading;
            loaded = true;
            break;
        }
    }

    if (loaded == false)
    {
        Logger::Error("[Texture::PureCreate] Cannot create texture. Descriptor: %s, GPU: %s",
                      descriptor->pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(GetPrimaryGPUForLoading()));

        MakePink(texture, descriptor->IsCubeMap() ? rhi::TEXTURE_TYPE_CUBE : key.typeHint);
    }
#endif
}

Vector<String> TextureAssetLoader::GetPathKeyDependsOn(const AssetBase* asset) const
{
    const Texture* texture = static_cast<const Texture*>(asset);
    TextureDescriptor* descriptor = texture->GetDescriptor();
    DVASSERT(descriptor != nullptr);

    Vector<String> files;
    if (texture->IsPinkPlaceholder() == false)
    {
        if (descriptor->IsCubeMap() == true)
        {
            Vector<FilePath> faces;
            descriptor->GetFacePathnames(faces);
            files.reserve(faces.size() + 1);

            for (const FilePath& path : faces)
            {
                files.push_back(path.GetAbsolutePathname());
            }
        }
        else
        {
            files.push_back(descriptor->GetSourceTexturePathname().GetAbsolutePathname());
        }
    }

    files.push_back(descriptor->pathname.GetAbsolutePathname());

    return files;
}

AssetFileInfo TextureAssetLoader::GetUniqueTextureKeyFileInfo(const Any& assetKey) const
{
    const Texture::UniqueTextureKey& key = assetKey.Get<Texture::UniqueTextureKey>();

    AssetFileInfo info;
    info.inMemoryAsset = true;
    info.fileName = Format("%d", key.uniqueKey);

    return info;
}

void TextureAssetLoader::LoadUniqueTextureKeyAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMsg) const
{
    DVASSERT(reloading == false);

    using namespace TextureAssetLoaderDetails;

    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    const Any& assetKey = asset->GetKey();

    Vector<RefPtr<Image>> images;
    bool generateMipMaps = false;

    Texture::UniqueTextureKey key = assetKey.Get<Texture::UniqueTextureKey>();
    if (key.creationData.CanGet<TextureRawData>())
    {
        TextureRawData texData = key.creationData.Get<TextureRawData>();
        images.push_back(RefPtr<Image>(Image::CreateFromData(texData.width, texData.height, texData.format, texData.data.get())));
        generateMipMaps = texData.generateMipmaps;
        texData.data.reset();
        key.creationData = texData;
    }
    else if (key.creationData.CanGet<TextureImageData>())
    {
        TextureImageData texData = key.creationData.Get<TextureImageData>();
        images = std::move(texData.images);
        generateMipMaps = texData.generateMipMaps;
        key.creationData = texData;
    }
    MofidyAssetKey(asset, std::move(key));

    DVASSERT(images.empty() == false);
    { // check that input data is valid
        Image* img = images[0].Get();

        if ((img->width < Texture::MINIMAL_WIDTH || img->height < Texture::MINIMAL_HEIGHT) &&
            (img->format == FORMAT_PVR2 || img->format == FORMAT_PVR4))
        {
            const char* formatName = img->format == FORMAT_PVR2 ? "PVR2" : "PVR4";
            errorMsg = Format("Requested size of texture for format %s is less than minimal : %u x %u", formatName, img->width, img->height);
            return;
        }
    }

    Asset<Texture> textureAsset = std::static_pointer_cast<Texture>(asset);

    textureAsset->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, generateMipMaps);

    TextureAssetLoaderDetails::CheckAndFixImageFormat(images);

    FlushDataToRenderer(textureAsset, images);
}

Vector<String> TextureAssetLoader::GetUniqueTextureKeyDependsOn(const AssetBase* asset) const
{
    return Vector<String>{};
}

AssetFileInfo TextureAssetLoader::GetRenderTargetTextureKeyFileInfo(const Any& assetKey) const
{
    const Texture::RenderTargetTextureKey& key = assetKey.Get<Texture::RenderTargetTextureKey>();

    AssetFileInfo info;
    info.inMemoryAsset = true;
    if (key.uniqueKey.empty() == false)
    {
        info.fileName = key.uniqueKey;
    }
    else
    {
        info.fileName = "FBO";
    }

    return info;
}

void TextureAssetLoader::LoadRenderTargetTextureKeyAsset(Asset<AssetBase> asset, File* file, bool reloading, String& errorMsg) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    const Texture::RenderTargetTextureKey& key = asset->GetKey().Get<Texture::RenderTargetTextureKey>();

    uint32 w = key.width;
    uint32 h = key.height;
    PixelFormat format = key.format;
    bool isDepth = key.isDepth;
    rhi::TextureType requestedType = key.textureType;

    int32 dx = Max(static_cast<int32>(w), 8);
    int32 dy = Max(static_cast<int32>(h), 8);

    if (key.ensurePowerOf2)
    {
        EnsurePowerOf2(dx);
        EnsurePowerOf2(dy);
    }

    Asset<Texture> tx = std::static_pointer_cast<Texture>(asset);

    tx->width = dx;
    tx->height = dy;
    tx->textureType = requestedType;
    tx->texDescriptor->format = format;
    tx->samplerState.mipFilter = tx->texDescriptor->drawSettings.mipFilter = rhi::TEXMIPFILTER_NONE;
    tx->samplerStateHandle = Texture::CreateSamplerStateHandle(tx->samplerState);
    tx->isDepth = key.isDepth;

    const PixelFormatDescriptor& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    rhi::Texture::Descriptor descriptor;
    descriptor.width = tx->width;
    descriptor.height = tx->height;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = true;
    descriptor.needRestore = false;
    descriptor.cpuAccessWrite = false;
    descriptor.type = (key.isDepth == false) ? requestedType : rhi::TEXTURE_TYPE_2D;
    descriptor.format = (key.isDepth == false) ? formatDescriptor.format : rhi::TEXTURE_FORMAT_D16;
    descriptor.sampleCount = key.sampleCount;
    descriptor.levelCount = key.mipLevelsCount;
    descriptor.cpuAccessRead = key.needPixelReadback;

    DVASSERT(descriptor.format != static_cast<rhi::TextureFormat>(-1)); //unsupported format
    tx->autoGeneratedMips = descriptor.autoGenMipmaps;
    tx->levelsCount = descriptor.levelCount;
    tx->handle = rhi::CreateTexture(descriptor);

    if (key.isDepth)
    {
        rhi::TextureSetDescriptor textureSetDesc;
        textureSetDesc.fragmentTexture[0] = tx->handle;
        textureSetDesc.fragmentTextureCount = 1;
        tx->singleTextureSet = rhi::AcquireTextureSet(textureSetDesc);
    }

    tx->isRenderTarget = true;
    tx->texDescriptor->pathname = key.uniqueKey;
}

Vector<String> TextureAssetLoader::GetRenderTargetTextureKeyDependsOn(const AssetBase* asset) const
{
    return Vector<String>();
}

void TextureAssetLoader::FlushDataToRenderer(const Asset<Texture>& asset, const Vector<RefPtr<Image>>& images) const
{
    DVASSERT(images.size() != 0);

    RefPtr<Image> img = images.front();
    asset->width = img->width;
    asset->height = img->height;
    asset->texDescriptor->format = img->format;

    asset->textureType = (img->cubeFaceID != Texture::INVALID_CUBEMAP_FACE) ? rhi::TEXTURE_TYPE_CUBE : rhi::TEXTURE_TYPE_2D;
    asset->state = Texture::STATE_DATA_LOADED;

    const PixelFormatDescriptor& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(asset->texDescriptor->format);
    rhi::Texture::Descriptor descriptor;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = false;
    descriptor.width = images[0]->width;
    descriptor.height = images[0]->height;
    descriptor.type = (images[0]->cubeFaceID == Texture::INVALID_CUBEMAP_FACE) ? rhi::TEXTURE_TYPE_2D : rhi::TEXTURE_TYPE_CUBE;
    descriptor.format = formatDescriptor.format;
    descriptor.levelCount = static_cast<uint32>((descriptor.type == rhi::TEXTURE_TYPE_CUBE) ? images.size() / 6 : images.size());

    const uint32 oldLevelCountVerify = descriptor.levelCount; //to notify about wrong images
    for (const RefPtr<Image>& img : images)
    { // hack for some wrong data
        descriptor.levelCount = Max(descriptor.levelCount, img->mipmapLevel + 1);
    }

    if (oldLevelCountVerify != descriptor.levelCount)
    {
        Logger::Error("Something wrong with image mipmap levels at %s", asset->texDescriptor->pathname.GetStringValue().c_str());
    }

    DVASSERT(descriptor.format != static_cast<rhi::TextureFormat>(-1)); //unsupported format

    if (descriptor.type == rhi::TEXTURE_TYPE_2D)
    {
        for (size_t i = 0, sz = images.size(); i < sz; ++i)
            descriptor.initialData[i] = images[i]->data;
    }
    else if (descriptor.type == rhi::TEXTURE_TYPE_CUBE)
    {
        rhi::TextureFace face[] = { rhi::TEXTURE_FACE_POSITIVE_X, rhi::TEXTURE_FACE_NEGATIVE_X, rhi::TEXTURE_FACE_POSITIVE_Y, rhi::TEXTURE_FACE_NEGATIVE_Y, rhi::TEXTURE_FACE_POSITIVE_Z, rhi::TEXTURE_FACE_NEGATIVE_Z };
        void** data = descriptor.initialData;

        for (unsigned f = 0; f != Texture::CUBE_FACE_COUNT; ++f)
        {
            for (unsigned m = 0; m != descriptor.levelCount; ++m)
            {
                *data = nullptr;

                for (size_t i = 0, sz = images.size(); i < sz; ++i)
                {
                    const RefPtr<Image>& img = images[i];

                    if (img->cubeFaceID == face[f] && img->mipmapLevel == m)
                    {
                        *data = img->data;
                        break;
                    }
                }

                ++data;
            }
        }
    }

    asset->autoGeneratedMips = descriptor.autoGenMipmaps;
    asset->levelsCount = descriptor.levelCount;
    asset->handle = rhi::CreateTexture(descriptor);
    if (asset->handle != rhi::InvalidHandle)
    {
        rhi::TextureSetDescriptor textureSetDesc;
        textureSetDesc.fragmentTexture[0] = asset->handle;
        textureSetDesc.fragmentTextureCount = 1;
        asset->singleTextureSet = rhi::AcquireTextureSet(textureSetDesc);
    }
    else
    {
        asset->singleTextureSet = rhi::HTextureSet();
    }

    asset->samplerState.addrU = asset->texDescriptor->drawSettings.wrapModeS;
    asset->samplerState.addrV = asset->texDescriptor->drawSettings.wrapModeT;
    asset->samplerState.minFilter = asset->texDescriptor->drawSettings.minFilter;
    asset->samplerState.magFilter = asset->texDescriptor->drawSettings.magFilter;
    asset->samplerState.mipFilter = asset->texDescriptor->drawSettings.mipFilter;

    rhi::ReleaseSamplerState(asset->samplerStateHandle);
    asset->samplerStateHandle = Texture::CreateSamplerStateHandle(asset->samplerState);

    asset->state = Texture::STATE_VALID;
}

void TextureAssetLoader::MakePink(const Asset<Texture>& asset, rhi::TextureType requestedType, bool checkers) const
{
    if (rhi::TEXTURE_TYPE_CUBE == requestedType)
    {
        asset->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, true);
        asset->texDescriptor->dataSettings.cubefaceFlags = 0x000000FF;
    }
    else
    {
        asset->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, false);
    }

    Vector<RefPtr<Image>> images;
    if (asset->texDescriptor->IsCubeMap())
    {
        for (uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
        {
            RefPtr<Image> img(Image::CreatePinkPlaceholder(checkers));
            img->cubeFaceID = i;
            img->mipmapLevel = 0;

            images.push_back(img);
        }
    }
    else
    {
        images.push_back(RefPtr<Image>(Image::CreatePinkPlaceholder(checkers)));
    }

    FlushDataToRenderer(asset, images);

    asset->isPink = true;

    asset->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
}

bool TextureAssetLoader::LoadFromImage(const Asset<Texture>& asset, eGPUFamily gpu) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector<RefPtr<Image>> images;
    if (TextureAssetLoaderDetails::LoadImages(asset->texDescriptor, gpu, images) == false)
    {
        return false;
    }

    asset->isPink = false;
    asset->state = Texture::STATE_DATA_LOADED;
    FlushDataToRenderer(asset, images);

    if (!asset->singleTextureSet.IsValid())
    {
        Logger::Error
        (
        "[Texture::CreateFromImage] Cannot create rhi.texture from image. Descriptor: %s, GPU: %s",
        asset->texDescriptor->pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu)
        );
        return false;
    }

    return true;
}

void TextureAssetLoader::RestoreRenderResource(Texture* texture) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    using namespace TextureAssetLoaderDetails;

    if ((!texture->handle.IsValid()) || (!NeedRestoreTexture(texture->handle)))
    {
        return;
    }

    Vector<RefPtr<Image>> images;

    const FilePath& relativePathname = texture->texDescriptor->GetSourceTexturePathname();
    FilePath::ePathType pathType = relativePathname.GetType();

    bool shouldMakePink = texture->isPink || (pathType == FilePath::PATH_EMPTY);

    if ((pathType == FilePath::PATH_IN_FILESYSTEM) || (pathType == FilePath::PATH_IN_RESOURCES) || (pathType == FilePath::PATH_IN_DOCUMENTS))
    {
        eGPUFamily gpuForLoading = GetGPUForLoading(texture->texDescriptor, texture->loadedAsFile);
        LoadImages(texture->texDescriptor, gpuForLoading, images);
        if (images.empty())
        {
            String absolutePath = relativePathname.GetAbsolutePathname();
            Logger::Error("Unable to restore texture from file: %s", absolutePath.c_str());
            shouldMakePink = true;
        }
    }

    if (shouldMakePink)
    {
        DVASSERT(images.empty());

        if (texture->texDescriptor->IsCubeMap())
        {
            for (uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
            {
                RefPtr<Image> img(Image::Create(texture->width, texture->height, FORMAT_RGBA8888));
                img->MakePink(true);
                img->cubeFaceID = i;
                img->mipmapLevel = 0;
                images.push_back(img);
            }
        }
        else
        {
            RefPtr<Image> img(Image::Create(texture->width, texture->height, FORMAT_RGBA8888));
            img->MakePink(true);
            images.push_back(img);
        }
    }

    for (uint32 i = 0, sz = static_cast<uint32>(images.size()); i < sz; ++i)
    {
        RefPtr<Image> img = images[i];
        texture->TexImage((img->mipmapLevel != static_cast<uint32>(-1)) ? img->mipmapLevel : i, img->width, img->height, img->data, img->dataSize, img->cubeFaceID);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(TextureAssetLoader)
{
}

} // namespace DAVA
