#include "Classes/Painter/Painter.h"
#include "Classes/Painter/Private/TextPainter.h"


#include <Render/2D/Systems/BatchDescriptor2D.h>
#include <Render/2D/Systems/RenderSystem2D.h>

namespace Painting
{
struct Painter::Impl
{
    TextPainter textPainter;
};

Painter::Painter()
    : impl(new Impl())
{
}

Painter::~Painter() = default;

void Painter::Add(const DrawTextParams& params)
{
    impl->textPainter.Add(params);
}

void Painter::Draw(DAVA::Window* window)
{
    impl->textPainter.Draw();
}
}
