#ifndef __PAINT_TOOL_H__
#define __PAINT_TOOL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class PaintTool: public BaseObject
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

    PaintTool(eBrushType _type, const String & _spriteName, float32 _solidRadius);
    virtual ~PaintTool();
    
    eBrushType brushType;
    String spriteName;
    Sprite *sprite;
    
    float32 radius;
    float32 intension;
    float32 zoom;
    float32 solidRadius;
};

#endif // __PAINT_TOOL_H__
