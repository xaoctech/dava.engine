#include "REPlatform/Scene/Utils/Utils.h"

#include <QtTools/ConsoleWidget/PointerSerializer.h>

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Input/Keyboard.h>
#include <Render/2D/Sprite.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Texture.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
String SizeInBytesToString(float32 size)
{
    String retString = "";

    if (1000000 < size)
    {
        retString = Format("%0.2f MB", size / (1024 * 1024));
    }
    else if (1000 < size)
    {
        retString = Format("%0.2f KB", size / 1024);
    }
    else
    {
        retString = Format("%d B", (int32)size);
    }

    return retString;
}

bool IsKeyModificatorPressed(eInputElements key)
{
    Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    return keyboard != nullptr && keyboard->GetKeyState(key).IsPressed();
}

bool IsKeyModificatorsPressed()
{
    return (IsKeyModificatorPressed(eInputElements::KB_LSHIFT) || IsKeyModificatorPressed(eInputElements::KB_LCTRL) || IsKeyModificatorPressed(eInputElements::KB_LALT));
}

String ReplaceInString(const String& sourceString, const String& what, const String& on)
{
    String::size_type pos = sourceString.find(what);
    if (pos != String::npos)
    {
        String newString = sourceString;
        newString = newString.replace(pos, what.length(), on);
        return newString;
    }

    return sourceString;
}

void SaveSpriteToFile(Sprite* sprite, const FilePath& path)
{
    if (sprite)
    {
        SaveTextureToFile(sprite->GetTexture(), path);
    }
}

void SaveTextureToFile(const Asset<Texture>& texture, const FilePath& path)
{
    if (texture)
    {
        Image* img = texture->CreateImageFromRegion();
        SaveImageToFile(img, path);
        img->Release();
    }
}

void SaveImageToFile(Image* image, const FilePath& path)
{
    ImageSystem::Save(path, image);
}

Asset<Texture> CreateSingleMipTexture(const FilePath& imagePath)
{
    RefPtr<Image> image(ImageSystem::LoadSingleMip(imagePath));

    Texture::UniqueTextureKey key(image, false);
    Asset<Texture> result = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
    String baseName = imagePath.GetFilename();
    result->texDescriptor->OverridePathName(Format("memoryfile_%s_%p", baseName.c_str(), result));

    return result;
}

const FilePath& DefaultCursorPath()
{
    static const FilePath path = "~res:/ResourceEditor/LandscapeEditor/Tools/cursor/cursor.png";
    return path;
}
} // namespace DAVA
