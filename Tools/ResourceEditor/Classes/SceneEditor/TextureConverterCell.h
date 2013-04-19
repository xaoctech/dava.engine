#ifndef __TEXTURE_CONVERTER_CELL_H__
#define __TEXTURE_CONVERTER_DIALOG_H__

#include "DAVAEngine.h"

using namespace DAVA;

class TextureConverterCell: public UIListCell
{
public:
    TextureConverterCell(const Rect &rect, const String &cellIdentifier);
    virtual ~TextureConverterCell();

    void SetTexture(const FilePath &texturePath);
    
    static int32 GetCellHeight();
protected:

    UIControl *preview;
    UIStaticText *textureName;
    UIStaticText *textureFormat;
    UIStaticText *textureDimensions;
    UIStaticText *textureSize;
};



#endif // __TEXTURE_CONVERTER_DIALOG_H__