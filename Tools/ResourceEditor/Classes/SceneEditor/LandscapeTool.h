#ifndef __PAINT_TOOL_H__
#define __PAINT_TOOL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool: public BaseObject
{
    
public:

    LandscapeTool(int32 _toolID, const String & _imageName);
    virtual ~LandscapeTool();

    static float32 SizeColorMin();
    static float32 SizeColorMax();

    static float32 StrengthColorMin();
    static float32 StrengthColorMax();
    
    static float32 StrengthHeightMax();
    static float32 SizeHeightMax();

    static float32 DefaultStrengthHeight();
    static float32 DefaultSizeHeight();

    int32 toolID;
    
    String imageName;
    Image *image;
    Sprite *sprite;
    
    //height
    float32 strength;
    float32 size;
    float32 maxStrength;
    float32 maxSize;
    float32 height;
    
    bool relativeDrawing;
    bool averageDrawing;
};

#endif // __PAINT_TOOL_H__
