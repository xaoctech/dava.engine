#pragma once

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Math/AABBox3.h"
#include "Math/Vector.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
class Texture;

class LandscapePageRenderer : public ReflectionBase
{
public:
    enum class eLandscapeComponent : uint8
    {
        COMPONENT_TERRAIN = 0,
        COMPONENT_DECORATION,

        COMPONENT_COUNT
    };

    struct PageRenderParams
    {
        Vector<Asset<Texture>> pageSrc;
        Vector<Asset<Texture>> pageDst;
        Vector2 relativeCoord0;
        Vector2 relativeCoord1;
        AABBox3 pageBBox;
        uint32 lod = 0;
        uint32 pageSize = 0;

        eLandscapeComponent component = eLandscapeComponent::COMPONENT_COUNT;
    };

public:
    virtual bool RenderPage(const PageRenderParams& params) = 0;
    virtual ~LandscapePageRenderer() = default;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LandscapePageRenderer, ReflectionBase) // TODO: Not in place.
    {
        DAVA::ReflectionRegistrator<LandscapePageRenderer>::Begin()
        .End();
    }
};
}