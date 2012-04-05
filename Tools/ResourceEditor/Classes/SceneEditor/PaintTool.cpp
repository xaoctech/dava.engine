#include "PaintTool.h"

PaintTool::PaintTool(eBrushType _type, const String & _spriteName)
{
    brushType = _type;
    spriteName = _spriteName;
    sprite = Sprite::Create(spriteName);
    
    intension = (IntensionMax() + IntensionMin()) / 2.0f;
    zoom = (ZoomMax() + ZoomMin()) / 2.0f;
}

PaintTool::~PaintTool()
{
    SafeRelease(sprite);
}

float32 PaintTool::ZoomMin()
{
    return 0.2f;
}

float32 PaintTool::ZoomMax()
{
    return 2.0f;
}

float32 PaintTool::IntensionMin()
{
    return 0.0f;
}

float32 PaintTool::IntensionMax()
{
    return 0.50f;
}
