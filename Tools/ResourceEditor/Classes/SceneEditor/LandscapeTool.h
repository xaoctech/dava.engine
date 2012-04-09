#ifndef __PAINT_TOOL_H__
#define __PAINT_TOOL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool: public BaseObject
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
        
        EBT_COUNT_COLOR
    };
    
public:

    LandscapeTool(eBrushType _type, const String & _spriteName, const String & _imageName);
    virtual ~LandscapeTool();

    static float32 ZoomMin();
    static float32 ZoomMax();

    static float32 IntensionMin();
    static float32 IntensionMax();
    
    static float32 DefaultStrength();
    static float32 DefaultSize();

    eBrushType brushType;
    String spriteName;
    Sprite *sprite;
    
    String imageName;
    Image *image;
    
    //color
    float32 intension;
    float32 zoom;
    
    //height
    float32 strength;
    float32 size;
    float32 maxStrength;
    float32 maxSize;
    
    bool releativeDrawing;
};

#endif // __PAINT_TOOL_H__
