#ifndef __DAVAENGINE_GRAPHICFONT_H__
#define __DAVAENGINE_GRAPHICFONT_H__

#include "Render/2D/Font.h"
#include "Render/Renderer.h"
#include "Asset/AssetListener.h"

namespace DAVA
{

#define GRAPHIC_FONT_CACHE_SIZE 1000 //text cache size
#define GRAPHIC_FONT_INDEX_BUFFER_SIZE ((GRAPHIC_FONT_CACHE_SIZE)*6)

class GraphicInternalFont;

class GraphicFont : public Font, private AssetListener
{
public:
    class GraphicFontVertex
    {
    public:
        Vector3 position;
        Vector2 texCoord;
    };

    GraphicFont();

protected:
    virtual ~GraphicFont();

public:
    static GraphicFont* Create(const FilePath& descriptorPath, const FilePath& texturePath);

    /**
     \brief Get string size(rect).
     \param[in] str - processed string
     \param[in, out] charSizes - if present(not NULL), will contain widths of every symbol in str
     \returns bounding rect for string in pixels
     */
    Font::StringMetrics GetStringMetrics(const WideString& str, Vector<float32>* charSizes = 0) const override;

    /**
     \brief Checks if symbol is present in font.
     \param[in] ch - tested symbol
     \returns true if symbol is available, false otherwise
     */
    bool IsCharAvaliable(char16 ch) const override;

    /**
     \brief Get height of highest symbol in font.
     \returns height in pixels
     */
    uint32 GetFontHeight() const override;

    /**
     \brief Clone font.
     */
    Font* Clone() const override;

    /**
     \brief Get font texture
     */
    Asset<Texture> GetTexture() const;

    /**
     \brief Tests if two fonts are the same.
     */
    bool IsEqual(const Font* font) const override;

    // Put font properties into YamlNode
    YamlNode* SaveToYamlNode() const override;

    //We need to return font path
    const FilePath& GetFontPath() const;

    Font::StringMetrics DrawStringToBuffer(const WideString& str,
                                           int32 xOffset,
                                           int32 yOffset,
                                           GraphicFontVertex* vertexBuffer,
                                           int32& charDrawed,
                                           Vector<float32>* charSizes = NULL,
                                           int32 justifyWidth = 0,
                                           int32 spaceAddon = 0) const;

    float32 GetSpread() const;

    void OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded) override;

protected:
    // Get the raw hash string (identical for identical fonts).
    String GetRawHashString() override;

private:
    float32 GetSizeScale() const;
    bool LoadTexture(const FilePath& path);

    GraphicInternalFont* fontInternal;
    Asset<Texture> texture;
};

inline Asset<Texture> GraphicFont::GetTexture() const
{
    return texture;
}
}

#endif //__DAVAENGINE_GRAPHICFONT_H__
