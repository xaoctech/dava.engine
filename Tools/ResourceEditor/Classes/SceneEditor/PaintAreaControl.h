#ifndef __PAINT_AREA_CONTROL_H__
#define __PAINT_AREA_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class TextureRenderObject: public BaseObject
{
public:
    TextureRenderObject();
    ~TextureRenderObject();
    
    void Set(const String &texturepath);
    void CreateMeshFromSprite(Sprite *spr, int32 frameToGen);
    
    void Draw();
    
    Vector<float32> vertexes;
    Vector<float32> textureCoords;
    RenderDataObject *renderData;
    Texture *texture;
    Sprite *sprite;
};


class PaintTool;
class PaintAreaControl: public UIControl
{
public:
    
    enum eConst
    {
        ZOOM_MULTIPLIER = 2,
    };
    
    enum eTextureRenderObjectIDs
    {
        ETROID_LIGHTMAP_RGB = 0,
        ETROID_TEXTURE_TEXTURE0,
        ETROID_TEXTURE_TEXTURE1,
        
        ETROID_COUNT
    };
    
    
    enum eDrawingMask
    {
        EDM_NONE = 0,
        
        EDM_RED = 1,
        EDM_GREEN = 1 << 1,
        EDM_BLUE = 1 << 2,
        EDM_ALPHA = 1 << 3
    };
    
public:
    PaintAreaControl(const Rect & rect);
    virtual ~PaintAreaControl();
    
    void SetPaintTool(PaintTool *tool);
    
    virtual void Input(UIEvent *currentInput);
    
    virtual void Draw(const UIGeometricData &geometricData);

    
    void SetTextureSideSize(const Vector2 & sideSize);
    
    void SetTexture(eTextureRenderObjectIDs id, const String &path);
    
    void ShowResultTexture(bool show);

    int32 GetDrawingMask();
    void SetDrawingMask(int32 newMask);
    
    void SaveMask(const String &pathToFile);
    
protected:
    
    void Recreate();
    
    void DrawCursor();
    void DrawLightmap();
    
    void DrawShader();
    void InitShader();
    void ReleaseShader();
    
    
    UIGeometricData savedGeometricData;
    void UpdateMap();
    
    PaintTool *usedTool;
    Sprite *spriteForAlpha;
    Sprite *spriteForResult;
    Sprite *toolSprite;
    
    Vector2 startPoint;
    Vector2 endPoint;
    Vector2 currentMousePos;
    
    Vector2 prevDrawPos;
    
    eBlendMode srcBlendMode;
    eBlendMode dstBlendMode;
    Color paintColor;
    
    Vector2 textureSideSize;
    
    Shader * blendedShader;
    int32 uniformTexture0;
    int32 uniformTexture1;
    int32 uniformTextureMask;
    
    Vector<float32> vertexes;
    Vector<float32> textureCoords;
    RenderDataObject *renderData;

    int32 drawingMask;
    bool showResultSprite;

    TextureRenderObject *textureRenderObjects[ETROID_COUNT];
    void InitTextureRenderObjects();
    void ReleaseTextureRenderObjects();
};



#endif // __PAINT_AREA_CONTROL_H__