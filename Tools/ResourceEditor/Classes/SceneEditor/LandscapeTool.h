#ifndef __PAINT_TOOL_H__
#define __PAINT_TOOL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LandscapeTool: public BaseObject
{
    
public:

    LandscapeTool(int32 _toolID, const String & _imageName);
    virtual ~LandscapeTool();

    static float32 ZoomMin();
    static float32 ZoomMax();

    static float32 IntensionMin();
    static float32 IntensionMax();
    
    static float32 DefaultStrength();
    static float32 DefaultSize();

    int32 toolID;
    
    String imageName;
    Image *image;
    Sprite *sprite;
    
    //color
    float32 intension;
    float32 zoom;
    
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
