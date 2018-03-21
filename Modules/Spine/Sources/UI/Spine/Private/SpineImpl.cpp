#include <Debug/DVAssert.h>
#include <FileSystem/File.h>
#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <Render/2D/Sprite.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Texture.h>
#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

#include <spine/spine.h>
#include <spine/extension.h>

namespace SpineImplDetails
{
DAVA::UnorderedMap<void*, DAVA::Asset<DAVA::Texture>> spineTextures;
} // namespace SpineImplDetails

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path_)
{
    using namespace DAVA;

    FilePath path(path_);
    // Check file as is
    if (!path.Exists())
    {
        // Try find sprite file
        path = FilePath::CreateWithNewExtension(path, ".txt");
        if (!path.Exists())
        {
            // Try find texture descriptor
            path = FilePath::CreateWithNewExtension(path, ".tex");
            if (!path.Exists())
            {
                Logger::Error("Spine atlas texture file '%s' not found!", path_ != nullptr ? path_ : "nullptr");
                return;
            }
        }
    }

    AssetManager* assetManager = GetEngineContext()->assetManager;

    Asset<Texture> texture;
    if (path.GetExtension() == ".tex")
    {
        Texture::PathKey key(path);
        if (assetManager->ExistsOnDisk(key) == true)
        {
            // Try open atlas as Texture
            texture = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        }
    }
    else if (path.GetExtension() == ".txt")
    {
        // Try open atlas as Sprite
        Sprite* s = Sprite::Create(path);
        DVASSERT(s, "Create sprite failure!");
        texture = s->GetTexture();
        SafeRelease(s);
    }
    else
    {
        // Try open atlas as Image
        Vector<Image*> images;
        ImageSystem::Load(path, images);
        DVASSERT(images.size() > 0 && images[0] != nullptr, "Failed to load image!");
        if (images.size() > 0 && images[0] != nullptr)
        {
            Vector<RefPtr<Image>> refImages;
            refImages.reserve(images.size());
            for (Image* img : images)
            {
                refImages.push_back(RefPtr<Image>::ConstructWithRetain(img));
            }

            Texture::UniqueTextureKey key(std::move(refImages));
            texture = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
        }
        for (Image* image : images)
        {
            image->Release();
        }
    }

    DVASSERT(texture, "Failed to create texture!");
    if (texture != nullptr)
    {
        SpineImplDetails::spineTextures.emplace(texture.get(), texture);
        self->rendererObject = texture.get();
    }
    else
    {
        self->rendererObject = nullptr;
    }
    self->width = texture ? texture->width : 0;
    self->height = texture ? texture->height : 0;
}

void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
    using namespace DAVA;
    if (self->rendererObject)
    {
        auto iter = SpineImplDetails::spineTextures.find(self->rendererObject);
        DVASSERT(iter != SpineImplDetails::spineTextures.end());

        SpineImplDetails::spineTextures.erase(iter);
        self->rendererObject = nullptr;
    }
}

char* _spUtil_readFile(const char* path, int* length)
{
    using namespace DAVA;
    File* fp = File::Create(path, File::READ | File::OPEN);
    DVASSERT(fp != nullptr, "Failed to read file!");
    if (fp != nullptr)
    {
        *length = static_cast<int>(fp->GetSize());

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

        char* bytes = MALLOC(char, *length);

#ifdef __clang__
#pragma clang diagnostic pop
#endif

        fp->Read(bytes, *length);
        fp->Release();
        return bytes;
    }
    *length = 0;
    return nullptr;
}