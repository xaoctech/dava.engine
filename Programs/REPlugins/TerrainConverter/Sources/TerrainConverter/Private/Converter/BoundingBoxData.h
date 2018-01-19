#ifndef __TERRAIN_CONVERTER_BOUNDING_BOX_DATA_H__
#define __TERRAIN_CONVERTER_BOUNDING_BOX_DATA_H__

#include "Converter/Visitor/ConverterVisitorElement.h"

#include <DAVAEngine.h>

class BoundingBoxData
: public ConverterVisitorElement
{
public:
    BoundingBoxData() = default;

    BoundingBoxData(const DAVA::AABBox2& aBbox)
        : bbox(aBbox)
    {
    }

    ~BoundingBoxData() = default;

    void Accept(ConverterVisitorBase& v) override
    {
        v.Visit(*this);
    }

    DAVA::AABBox2 bbox;
};

#endif // __TERRAIN_CONVERTER_BOUNDING_BOX_DATA_H__