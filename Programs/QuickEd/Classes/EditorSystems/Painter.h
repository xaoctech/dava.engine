#pragma once

#include <Base/BaseTypes.h>
#include <Math/Color.h>

struct DrawTextParams
{
    DAVA::float32 textSize = 10.0f;
    DAVA::String text;
    DAVA::eAlign align = static_cast<DAVA::eAlign>(DAVA::ALIGN_TOP | DAVA::ALIGN_LEFT);
    DAVA::Color color = DAVA::Color::Black;
};

class Painter
{
public:
    Painter();
    ~Painter();

    void Draw(const DrawTextParams& params);

private:
    struct Impl;
    //std::unique_ptr<Impl> impl;
};
