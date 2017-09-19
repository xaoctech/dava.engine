#pragma once

#include <Base/BaseTypes.h>
#include <Math/Color.h>

namespace DAVA
{
class Window;
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
    //size of item in screen coordinates. If size is zero it will be calculated from text metrics
    DAVA::Vector2 size = DAVA::Vector2(0.0f, 0.0f);
    //angle of text item in radians
    DAVA::float32 angle = 0.0f;
    //direction to draw text item relative to it position.
    //As an example if direction is equal to ALIGN_LEFT item X position will be equal to params.pos.x - params.size.x - margin.x
    DAVA::int32 direction = DAVA::ALIGN_RIGHT | DAVA::ALIGN_BOTTOM;
    //margin between params position and actual text item position.
    //As an example if direction is equal to ALIGN_LEFT item X position will be equal to params.pos.x - params.size.x - margin.x
    DAVA::Vector2 margin = DAVA::Vector2(0.0f, 0.0f);

    DAVA::Vector2 scale = DAVA::Vector2(1.0f, 1.0f);
    DAVA::Vector2 parentPos = DAVA::Vector2(0.0f, 0.0f);
};

class Painter final
{
public:
    Painter();
    ~Painter();

    void Add(const DrawTextParams& params);
    void Draw(DAVA::Window* window);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
}
