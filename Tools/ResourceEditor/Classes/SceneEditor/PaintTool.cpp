#include "PaintTool.h"

PaintTool::PaintTool(eBrushType _type, const String & _spriteName, float32 _solidRadius)
{
    brushType = _type;
    spriteName = _spriteName;
    sprite = Sprite::Create(spriteName);
    
    radius = 0.5f;
    intension = 0.5f;
    zoom = 0.5f;
    
    solidRadius = _solidRadius;
}

PaintTool::~PaintTool()
{
    SafeRelease(sprite);
}

