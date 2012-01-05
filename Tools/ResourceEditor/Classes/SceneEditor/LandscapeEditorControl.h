#ifndef __LANDSCAPE_EDITOR_CONTROL_H__
#define __LANDSCAPE_EDITOR_CONTROL_H__

#include "DAVAEngine.h"
#include "../UIScrollView.h"
#include "PropertyList.h"

using namespace DAVA;

class PaintTool
{
public:

    enum eBrushType
    {
        EBT_STANDART = 0,
        EBT_SPIKE,
        EBT_CIRCLE,
        EBT_NOISE,
        EBT_ERODE,
        EBT_WATER_ERODE,
        
        EBT_COUNT
    };
    
public:

    PaintTool(eBrushType _type, const String & _spriteName, float32 _solidRadius)
    {
        brushType = _type;
        spriteName = _spriteName;
        
        radius = 0.5f;
        height = 0.5f;
        zoom = 0.5f;
        
        solidRadius = _solidRadius;
    }
    
    eBrushType brushType;
    String spriteName;
    
    float32 radius;
    float32 height;
    float32 zoom;
    float32 solidRadius;
};

class PaintAreaControl: public UIControl
{
public:
    
    enum eTextures
    {
        ET_TEXTURE0 = 0,
        ET_TEXTURE1,
        
        ET_COUNT
    };
    
public:
    PaintAreaControl(const Rect & rect);
    virtual ~PaintAreaControl();
    
    void SetPaintTool(PaintTool *tool);
    
    virtual void Input(UIEvent *currentInput);
    
    virtual void Draw(const UIGeometricData &geometricData);

    
    void SetTextureSideSize(int32 sideSizeW, int32 sideSizeH);
    void SetTextureSideSize(const Vector2 & sideSize);
    
    void SetTexture(eTextures id, const String &path);

protected:

    UIGeometricData savedGeometricData;
    void UpdateMap();
    void GeneratePreview();
    
    PaintTool *usedTool;
    Sprite *spriteForDrawing;
    Sprite *toolSprite;
    
    Vector2 startPoint;
    Vector2 endPoint;
    Vector2 currentMousePos;
    
    Vector2 prevDrawPos;
    
    eBlendMode srcBlendMode;
    eBlendMode dstBlendMode;
    Color paintColor;
    
    Vector2 textureSideSize;
    
    String textures[ET_COUNT];
    
    //
    Shader * blendedShader;
    int32 uniformTexture0;
    int32 uniformTexture1;
    int32 uniformTextureMask;
};



class LandscapeEditorControl : public UIControl, public PropertyListDelegate
{
    enum eConst
    {
        TOOLS_HEIGHT = 40,
        OFFSET = 1,
        TOOL_BUTTON_SIDE = 32,
        SLIDER_WIDTH = 100,
    };
    
public:
    LandscapeEditorControl(const Rect & rect);
    virtual ~LandscapeEditorControl();
    
    virtual void WillAppear();
        
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);

    
protected:

    void CreateLeftPanel();
    void ReleaseLeftPanel();
    
    void CreateRightPanel();
    void ReleaseRightPanel();

    void CreatePaintAreaPanel();
    void ReleasePaintAreaPanel();
    
    bool IsValidPath(const String &path);


    // left side
    UIControl *leftPanel;
    PropertyList *propertyList;

    //right side
    UIControl *rightPanel;

    //paint area
	void OnToolSelected(BaseObject * object, void * userData, void * callerData);

    UIControl *toolsPanel;
    PaintAreaControl *paintArea;
    UIScrollView *scrollView;

    UIControl *toolButtons[PaintTool::EBT_COUNT];
    PaintTool *tools[PaintTool::EBT_COUNT];
    PaintTool *selectedTool;
    
    
    UISlider *radius;
    UISlider *height;
    UISlider *zoom;
    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);
	void OnRadiusChanged(BaseObject * object, void * userData, void * callerData);
	void OnHeightChanged(BaseObject * object, void * userData, void * callerData);
	void OnZoomChanged(BaseObject * object, void * userData, void * callerData);
};



#endif // __LANDSCAPE_EDITOR_CONTROL_H__