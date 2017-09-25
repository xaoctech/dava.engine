#pragma once

#include <Base/RefPtr.h>
#include <Base/BaseTypes.h>
#include <Math/Color.h>
#include <Render/2D/GraphicFont.h>
#include <UI/UIGeometricData.h>

namespace DAVA
{
class Window;
class NMaterial;
}

namespace Painting
{
struct DrawTextParams
{
    DAVA::String text;
    DAVA::float32 textSize = 10.0f;
    //text color
    DAVA::Color color = DAVA::Color::Black;
    //position of item in screen coordinates
    DAVA::Vector2 pos = DAVA::Vector2(0.0f, 0.0f);
    //direction to draw text item relative to it position.
    //As an example if direction is equal to ALIGN_LEFT item X position will be equal to params.pos.x - params.size.x - margin.x
    DAVA::int32 direction = DAVA::ALIGN_RIGHT | DAVA::ALIGN_BOTTOM;
    //margin between params position and actual text item position.
    //As an example if direction is equal to ALIGN_LEFT item X position will be equal to params.pos.x - params.size.x - margin.x
    DAVA::Vector2 margin = DAVA::Vector2(0.0f, 0.0f);

    DAVA::Vector2 scale = DAVA::Vector2(1.0f, 1.0f);

    DAVA::float32 angle = 0.0f;
    DAVA::Matrix3 transformMatrix;
};

struct DrawLineParams
{
    //line color
    DAVA::Color color = DAVA::Color::Black;
    //position of item in screen coordinates
    DAVA::Vector2 startPos = DAVA::Vector2(0.0f, 0.0f);
    //size of item in screen coordinates. If size is zero it will be calculated from text metrics
    DAVA::Vector2 endPos = DAVA::Vector2(0.0f, 0.0f);

    //line width in pixels
    DAVA::float32 width = 1.0f;

    DAVA::Matrix3 transformMatrix;

    enum eType
    {
        SOLID,
        DOT
    };

    eType type = SOLID;
};

class Painter final
{
public:
    Painter();
    ~Painter();

    void Add(DAVA::uint32 order, const DrawTextParams& params);
    void Add(DAVA::uint32 order, const DrawLineParams& params);

    void Draw(DAVA::Window* window);

private:
    void Draw(const DrawTextParams& params);
    void Draw(const DrawLineParams& params);

    void ApplyParamPos(DrawTextParams& params) const;

    using GraphicFontVertexVector = DAVA::Vector<DAVA::GraphicFont::GraphicFontVertex>;

    DAVA::RefPtr<DAVA::GraphicFont> font;
    DAVA::RefPtr<DAVA::NMaterial> fontMaterial;
    DAVA::RefPtr<DAVA::NMaterial> textureMaterial;
    GraphicFontVertexVector vertices;

    struct DrawItems
    {
        DAVA::Vector<DrawTextParams> drawTextItems;
        DAVA::Vector<DrawLineParams> drawLineItems;
    };

    DAVA::Map<DAVA::uint32, DrawItems> drawItems;

    DAVA::float32 cachedSpread = 0.0f;
};
}
