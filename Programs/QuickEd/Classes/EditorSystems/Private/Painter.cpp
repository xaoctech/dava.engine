#include "EditorSystems/Painter.h"
#include "EditorSystems/Private/TextPainter.h"


#include <Render/2D/Systems/BatchDescriptor2D.h>
#include <Render/2D/Systems/RenderSystem2D.h>

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
