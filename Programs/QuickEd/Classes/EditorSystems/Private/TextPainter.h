#pragma once

#include "EditorSystems/Painter.h"

#include <Base/RefPtr.h>

#include <Render/2D/GraphicFont.h>

namespace DAVA
{
class NMaterial;
}

class TextPainter
{
public:
    TextPainter();
    void Draw(const DrawTextParams& params);
    void PushNextBatch(const DrawTextParams& params);

private:
    using GraphicFontVertexVector = DAVA::Vector<DAVA::GraphicFont::GraphicFontVertex>;

    DAVA::RefPtr<DAVA::GraphicFont> font;
    DAVA::RefPtr<DAVA::NMaterial> fontMaterial;
    GraphicFontVertexVector vertices;
};
