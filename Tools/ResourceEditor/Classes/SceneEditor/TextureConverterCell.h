#ifndef __TEXTURE_CONVERTER_CELL_H__
#define __TEXTURE_CONVERTER_DIALOG_H__

#include "DAVAEngine.h"

using namespace DAVA;

class TextureConverterCell: public UIListCell
{
public:
    TextureConverterCell(const Rect &rect, const String &cellIdentifier);
    virtual ~TextureConverterCell();

    void SetTexture(const String &texturePath);
    
    static int32 GetCellHeight();
protected:

    static Texture::PixelFormat GetPVRFormat(const String &path);
    static uint32 ConvertLittleToHost(uint32 value);
    
    UIControl *preview;
    UIStaticText *textureName;
    UIStaticText *textureFormat;
    UIStaticText *textureDimensions;
};



#endif // __TEXTURE_CONVERTER_DIALOG_H__