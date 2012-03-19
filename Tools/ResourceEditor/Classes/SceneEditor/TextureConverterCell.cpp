#include "TextureConverterCell.h"
#include "ControlsFactory.h"

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

uint32 ConvertLittleToHost(uint32 value)
{
    uint32 testValue = 1;
    uint8 *testValuePtr = (uint8 *)&testValue;
    
    if(testValuePtr[0] < testValuePtr[1])
    {   //be
        uint8 *valuePtr = (uint8 *)&value;
        
        testValuePtr[0] = valuePtr[3];
        testValuePtr[1] = valuePtr[2];
        testValuePtr[2] = valuePtr[1];
        testValuePtr[3] = valuePtr[0];
        
        return testValue;
    }
    
    return value;    
}

bool GetPVRHeader(PVRHeader *header, const String &path)
{
    File *pvrFile = File::Create(path, File::READ | File::OPEN);
    if(pvrFile)
    {
        int32 readBytes = pvrFile->Read(header, sizeof(PVRHeader));
        SafeRelease(pvrFile);

        return (sizeof(PVRHeader) == readBytes);
    }
    return false;
}

WideString SizeToReadableForm(float32 size)
{
    WideString retString = L"";
    
    if(1000000 < size)
    {
        retString = Format(L"%0.2f MB", size / (1024 * 1024) );
    }
    else if(1000 < size)
    {
        retString = Format(L"%0.2f KB", size / 1024);
    }
    else 
    {
        retString = Format(L"%d B", (int32)size);
    }
    
    return  retString;
}

Texture::PixelFormat GetPVRFormat(int32 format)
{
    Texture::PixelFormat retFormat = Texture::FORMAT_INVALID;
    uint32 formatFlags = ConvertLittleToHost(format) & PVR_TEXTURE_FLAG_TYPE_MASK;
    if(kPVRTextureFlagTypePVRTC_4 == formatFlags)
    {
        retFormat = Texture::FORMAT_PVR4;
    }
    else if(kPVRTextureFlagTypePVRTC_2 == formatFlags)
    {
        retFormat = Texture::FORMAT_PVR2;
    }
    
    return retFormat;
}



TextureConverterCell::TextureConverterCell(const Rect &rect, const String &cellIdentifier)
:   UIListCell(Rect(rect.x, rect.y, rect.dx, GetCellHeight()), cellIdentifier)
{
    ControlsFactory::CustomizeListCell(this, L"");
    
    textureName = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureName->SetFont(ControlsFactory::GetFontDark());
    textureName->SetInputEnabled(false);
    AddControl(textureName);
    
    preview = new UIControl(Rect(0, textureName->GetRect().y + textureName->GetRect().dy, 
                                 rect.dx, ControlsFactory::TEXTURE_PREVIEW_HEIGHT));
    preview->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_PROPORTIONAL);
    preview->SetInputEnabled(false);
    AddControl(preview);
    

    textureFormat = new UIStaticText(Rect(0, preview->GetRect().y + preview->GetRect().dy, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureFormat->SetFont(ControlsFactory::GetFontDark());
    textureFormat->SetInputEnabled(false);
    AddControl(textureFormat);
    
    textureDimensions = new UIStaticText(Rect(0, textureFormat->GetRect().y + textureFormat->GetRect().dy, 
                                          rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureDimensions->SetFont(ControlsFactory::GetFontDark());
    textureDimensions->SetInputEnabled(false);
    AddControl(textureDimensions);

    textureSize = new UIStaticText(Rect(0, textureDimensions->GetRect().y + textureDimensions->GetRect().dy, 
                                              rect.dx, ControlsFactory::BUTTON_HEIGHT));
    textureSize->SetFont(ControlsFactory::GetFontDark());
    textureSize->SetInputEnabled(false);
    AddControl(textureSize);

    
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
    SafeRelease(textureSize);
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

    textureFormat->SetText(L"");
    textureSize->SetText(L"");
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
            PVRHeader pvrHeader = {0};
            GetPVRHeader(&pvrHeader, pvrPath);

            Texture::PixelFormat format = GetPVRFormat(pvrHeader.flags);

            String pvrFormat = Texture::GetPixelFormatString(format);
            textureFormat->SetText(StringToWString(pngFormat + "/" + pvrFormat));
            
            textureSize->SetText(SizeToReadableForm(pvrHeader.dataLength));
            
            SafeRelease(pvrTex);
        }
        else 
        {
            textureFormat->SetText(StringToWString(pngFormat));
        }
    }
    else if(".pvr" == ext)
    {
        PVRHeader pvrHeader = {0};
        GetPVRHeader(&pvrHeader, textureWorkingPath);
        Texture::PixelFormat format = GetPVRFormat(pvrHeader.flags);
        String pvrFormat = Texture::GetPixelFormatString(format);
        textureSize->SetText(SizeToReadableForm(pvrHeader.dataLength));

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
    return (ControlsFactory::BUTTON_HEIGHT * 4 + ControlsFactory::TEXTURE_PREVIEW_HEIGHT + 1);
}

