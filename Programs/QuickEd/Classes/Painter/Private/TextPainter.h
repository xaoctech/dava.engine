#pragma once

#include "Classes/Painter/Painter.h"

#include <Base/RefPtr.h>

#include <Render/2D/GraphicFont.h>

namespace DAVA
{
class NMaterial;
}

namespace Painting
{
class TextPainter final
{
public:
    TextPainter();
    void Add(const DrawTextParams& params);
    void Draw();

private:
    void PushNextBatch(const DrawTextParams& params);
    void ApplyParamPos(DrawTextParams& params) const;

    using GraphicFontVertexVector = DAVA::Vector<DAVA::GraphicFont::GraphicFontVertex>;

    DAVA::RefPtr<DAVA::GraphicFont> font;
    DAVA::RefPtr<DAVA::NMaterial> fontMaterial;
    GraphicFontVertexVector vertices;
    DAVA::Vector<DrawTextParams> drawItems;
    DAVA::float32 cachedSpread = 0.0f;
};
}
