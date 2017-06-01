#include <Debug/DVAssert.h>
#include <FileSystem/File.h>
#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Texture.h>

#include <spine/spine.h>
#include <spine/extension.h>

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path)
{
    using namespace DAVA;
    FilePath imagePath = path;
    Vector<Image*> images;
    ImageSystem::Load(imagePath, images);
    DVASSERT(images.size() > 0 && images[0] != nullptr, "Failed to load image!");
    Texture* texture = Texture::CreateFromData(images[0], 0);
    images[0]->Release();
    DVASSERT(texture, "Failed to create texture!");
    self->rendererObject = texture;
    self->width = texture->GetWidth();
    self->height = texture->GetHeight();
}

void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
    using namespace DAVA;
    if (self->rendererObject)
    {
        static_cast<Texture*>(self->rendererObject)->Release();
        self->rendererObject = nullptr;
    }
}

char* _spUtil_readFile(const char* path, int* length)
{
    using namespace DAVA;
    File* fp = File::Create(path, File::READ | File::OPEN);
    DVASSERT(fp != nullptr, "Failed to read file!");
    *length = static_cast<uint32>(fp->GetSize());
    char* bytes = MALLOC(char, *length);
    fp->Read(bytes, *length);
    fp->Release();

    return bytes;
}