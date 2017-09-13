#include "EditorSystems/Private/TextPainter.h"

#include <Render/Material/NMaterial.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/2D/TextBlockGraphicRender.h>
#include <Utils/UTF8Utils.h>

namespace TextPainterDetails
{
DAVA::Vector2 GetPos(const DrawTextParams& params)
{
    using namespace DAVA;
    Vector2 pos;
    switch (params.align)
    {
    case ALIGN_TOP | ALIGN_LEFT:
        break;

    //     case Align::TopCenter:
    //         x -= sSize.dx / 2;
    //         break;
    //
    //     case Align::TopRight:
    //         x -= sSize.dx;
    //         break;
    //
    //     case Align::Left:
    //         y -= sSize.dy / 2;
    //         break;
    //
    //     case Align::Center:
    //         x -= sSize.dx / 2;
    //         y -= sSize.dy / 2;
    //         break;
    //
    //     case Align::Right:
    //         x -= sSize.dx;
    //         y -= sSize.dy / 2;
    //         break;
    //
    //     case Align::BottomLeft:
    //         y -= sSize.dy;
    //         break;
    //
    //     case Align::BottomCenter:
    //         x -= sSize.dx / 2;
    //         y -= sSize.dy;
    //         break;
    //
    //     case Align::BottomRight:
    //         x -= sSize.dx;
    //         y -= sSize.dy;
    //         break;

    default:
        break;
    }

    return pos;
}
}

TextPainter::TextPainter()
{
    using namespace DAVA;

    FilePath fntPath = FilePath("~res:/ResourceEditor/Fonts/DejaVuSans.fnt");
    FilePath texPath = FilePath("~res:/ResourceEditor/Fonts/DejaVuSans.tex");
    font = GraphicFont::Create(fntPath, texPath);
    DVASSERT(font != nullptr);

    if (font->GetFontType() == Font::TYPE_DISTANCE)
    {
        float32 cachedSpread = font->GetSpread();
        fontMaterial = new NMaterial();
        fontMaterial->SetFXName(FastName("~res:/Materials/2d.DistanceFont.material"));
        fontMaterial->SetMaterialName(FastName("DistanceFontMaterial"));
        fontMaterial->AddProperty(FastName("smoothing"), &cachedSpread, rhi::ShaderProp::TYPE_FLOAT1);
        fontMaterial->PreBuildMaterial(RenderSystem2D::RENDER_PASS_NAME);
    }
    else
    {
        fontMaterial = SafeRetain(RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL);
    }
}

void TextPainter::Draw(const DrawTextParams& params)
{
    using namespace DAVA;

    font->SetSize(params.textSize);
    vertices.resize(4 * params.text.length());

    Vector2 pos = TextPainterDetails::GetPos(params);

    int32 charactersDrawn = 0;
    font->DrawStringToBuffer(UTF8Utils::EncodeToWideString(params.text), static_cast<int32>(pos.x), static_cast<int32>(pos.y), vertices.data(), charactersDrawn);

    PushNextBatch(params);
}

void TextPainter::PushNextBatch(const DrawTextParams& params)
{
    using namespace DAVA;

    uint32 vertexCount = static_cast<uint32>(vertices.size());
    uint32 indexCount = 6 * vertexCount / 4;

    BatchDescriptor2D batchDescriptor;
    batchDescriptor.singleColor = params.color;
    batchDescriptor.vertexCount = vertexCount;
    batchDescriptor.indexCount = DAVA::Min(TextBlockGraphicRender::GetSharedIndexBufferCapacity(), indexCount);
    batchDescriptor.vertexPointer = vertices.front().position.data;
    batchDescriptor.vertexStride = TextBlockGraphicRender::TextVerticesDefaultStride;
    batchDescriptor.texCoordPointer[0] = vertices.front().texCoord.data;
    batchDescriptor.texCoordStride = TextBlockGraphicRender::TextVerticesDefaultStride;
    batchDescriptor.indexPointer = TextBlockGraphicRender::GetSharedIndexBuffer();
    batchDescriptor.material = fontMaterial.Get();
    batchDescriptor.textureSetHandle = font->GetTexture()->singleTextureSet;
    batchDescriptor.samplerStateHandle = font->GetTexture()->samplerStateHandle;
    batchDescriptor.worldMatrix = &Matrix4::IDENTITY;
    RenderSystem2D::Instance()->PushBatch(batchDescriptor);
}
