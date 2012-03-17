#include "TextureConverterCell.h"
#include "ControlsFactory.h"

TextureConverterCell::TextureConverterCell(const Rect &rect, const String &cellIdentifier)
:   UIListCell(Rect(rect.x, rect.y, rect.dx, GetCellHeight()), cellIdentifier)
{
    ControlsFactory::CustomizeListCell(this, L"");
    
    textureName = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureName->SetFont(ControlsFactory::GetFontDark());
    textureName->SetInputEnabled(false);
    AddControl(textureName);
    
    preview = new UIControl(Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, ControlsFactory::TEXTURE_PREVIEW_HEIGHT));
    preview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    preview->SetInputEnabled(false);
    AddControl(preview);
    

    textureFormat = new UIStaticText(Rect(0, ControlsFactory::BUTTON_HEIGHT + ControlsFactory::TEXTURE_PREVIEW_HEIGHT, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureFormat->SetFont(ControlsFactory::GetFontDark());
    textureFormat->SetInputEnabled(false);
    AddControl(textureFormat);
    
    textureDimensions = new UIStaticText(Rect(0, ControlsFactory::BUTTON_HEIGHT*2 + ControlsFactory::TEXTURE_PREVIEW_HEIGHT, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureDimensions->SetFont(ControlsFactory::GetFontDark());
    textureDimensions->SetInputEnabled(false);
    AddControl(textureDimensions);
    
    UIControl *line =  ControlsFactory::CreateLine(Rect(0, GetCellHeight() - 1, rect.dx, 1), Color(0.2f, 0.2f, 0.2f, 0.8f));
    AddControl(line);
    SafeRelease(line);
}

TextureConverterCell::~TextureConverterCell()
{
    SafeRelease(preview);
    SafeRelease(textureName);
    SafeRelease(textureFormat);
    SafeRelease(textureDimensions);
}

void TextureConverterCell::SetTexture(const String &texturePath)
{
    String textureWorkingPath = texturePath;
    String::size_type pos = textureWorkingPath.find(".pvr.png");
    if(String::npos != pos)
    {
        textureWorkingPath = FileSystem::ReplaceExtension(textureWorkingPath, "");
    }
    
    String path, filename;
    FileSystem::SplitPath(textureWorkingPath, path, filename);


    textureName->SetText(StringToWString(filename));
    
    Texture *texture = Texture::CreateFromFile(textureWorkingPath);
    Sprite *s = Sprite::CreateFromTexture(texture, 0, 0, texture->width, texture->height);
    preview->SetSprite(s, 0);
    
    String ext = FileSystem::GetExtension(filename);
    if(".png" == ext)
    {
        String pngFormat = Texture::GetPixelFormatString(texture->format);
        
        String pvrPath = FileSystem::ReplaceExtension(textureWorkingPath, ".pvr");
        Texture *pvrTex = Texture::CreateFromFile(pvrPath);
        if(pvrTex)
        {
            Texture::PixelFormat format = GetPVRFormat(pvrPath);
            
            String pvrFormat = Texture::GetPixelFormatString(format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            SafeRelease(pvrTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pngFormat));
        }
    }
    else if(".pvr" == ext)
    {
        Texture::PixelFormat format = GetPVRFormat(textureWorkingPath);
        String pvrFormat = Texture::GetPixelFormatString(format);

        String pngPath = FileSystem::ReplaceExtension(textureWorkingPath, ".pvr");
        Texture *pngTex = Texture::CreateFromFile(pngPath);
        if(pngTex)
        {
            String pngFormat = Texture::GetPixelFormatString(pngTex->format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            SafeRelease(pngTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pvrFormat));
        }
    }

    textureDimensions->SetText(Format(L"%d x %d", texture->width, texture->height));
    
    SafeRelease(texture);
    SafeRelease(s);
}


int32 TextureConverterCell::GetCellHeight()
{
    return (ControlsFactory::BUTTON_HEIGHT * 3 + ControlsFactory::TEXTURE_PREVIEW_HEIGHT + 1);
}

typedef struct _PVRHeader
{
	uint32 headerLength;
	uint32 height;
	uint32 width;
	uint32 numMipmaps;
	uint32 flags;
	uint32 dataLength;
	uint32 bpp;
	uint32 bitmaskRed;
	uint32 bitmaskGreen;
	uint32 bitmaskBlue;
	uint32 bitmaskAlpha;
	uint32 pvrTag;
	uint32 numSurfs;
} PVRHeader;

#define PVR_TEXTURE_FLAG_TYPE_MASK   0xff

enum
{
    kPVRTextureFlagTypePVRTC_2 = 24,
    kPVRTextureFlagTypePVRTC_4
};


Texture::PixelFormat TextureConverterCell::GetPVRFormat(const String &path)
{
    Texture::PixelFormat retFormat = Texture::FORMAT_INVALID;
    
    File *pvrFile = File::Create(path, File::READ | File::OPEN);
    if(pvrFile)
    {
        PVRHeader header;
        int32 readBytes = pvrFile->Read(&header, sizeof(header));
        if(readBytes == sizeof(header))
        {
            uint32 formatFlags = ConvertLittleToHost(header.flags) & PVR_TEXTURE_FLAG_TYPE_MASK;
            if(kPVRTextureFlagTypePVRTC_4 == formatFlags)
            {
                retFormat = Texture::FORMAT_PVR4;
            }
            else if(kPVRTextureFlagTypePVRTC_2 == formatFlags)
            {
                retFormat = Texture::FORMAT_PVR2;
            }
        }
    }
        
    return retFormat;
}

uint32 TextureConverterCell::ConvertLittleToHost(uint32 value)
{
    uint32 testValue = 1;
    uint8 *testValuePtr = (uint8 *)&testValue;
    
    if(testValuePtr[0] < testValuePtr[1])
    {   //le
        uint8 *valuePtr = (uint8 *)&value;
        
        testValuePtr[0] = valuePtr[3];
        testValuePtr[1] = valuePtr[2];
        testValuePtr[2] = valuePtr[1];
        testValuePtr[3] = valuePtr[0];
        
        return testValue;
    }
    
    return value;    
}



