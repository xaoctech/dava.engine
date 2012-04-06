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

    PaintTool(eBrushType _type, const String & _spriteName);
    virtual ~PaintTool();

    static float32 ZoomMin();
    static float32 ZoomMax();

    static float32 IntensionMin();
    static float32 IntensionMax();
    
    
    
    eBrushType brushType;
    String spriteName;
    Sprite *sprite;
    
    float32 intension;
    float32 zoom;
};

#endif // __PAINT_TOOL_H__
