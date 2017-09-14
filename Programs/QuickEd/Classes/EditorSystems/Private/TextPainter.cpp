#include "EditorSystems/Private/TextPainter.h"

#include <Render/Material/NMaterial.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/2D/TextBlockGraphicRender.h>
#include <Utils/UTF8Utils.h>

namespace TextPainterDetails
{

}

TextPainter::TextPainter()
{
    using namespace DAVA;

    FilePath fntPath = FilePath("~res:/QuickEd/Fonts/DejaVuSans.fnt");
    FilePath texPath = FilePath("~res:/QuickEd/Fonts/DejaVuSans.tex");
    font = GraphicFont::Create(fntPath, texPath);
    DVASSERT(font != nullptr);

    if (font->GetFontType() == Font::TYPE_DISTANCE)
    {
        cachedSpread = font->GetSpread();
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

void TextPainter::Add(const DrawTextParams& params)
{
    drawItems.push_back(params);
    ApplyParamPos(drawItems.back());
}

void TextPainter::Draw()
{
    using namespace DAVA;
    for (DrawTextParams& params : drawItems)
    {
        font->SetSize(params.textSize);
        vertices.resize(4 * params.text.length());

        int32 charactersDrawn = 0;
        font->DrawStringToBuffer(UTF8Utils::EncodeToWideString(params.text), 0, 0, vertices.data(), charactersDrawn);

        PushNextBatch(params);
    }
    drawItems.clear();
}

void TextPainter::PushNextBatch(const DrawTextParams& params)
{
    using namespace DAVA;

    uint32 vertexCount = static_cast<uint32>(vertices.size());
    uint32 indexCount = 6 * vertexCount / 4;

    if (font->GetFontType() == Font::TYPE_DISTANCE)
    {
        float32 spread = font->GetSpread();
        if (!FLOAT_EQUAL(cachedSpread, spread))
        {
            cachedSpread = spread;
            fontMaterial->SetPropertyValue(FastName("smoothing"), &cachedSpread);
        }
    }

    //NOTE: correct affine transformations
    Matrix4 offsetMatrix;
    offsetMatrix.BuildTranslation(Vector3(params.pos.x, params.pos.y, 0.f));

    Matrix4 rotateMatrix;
    rotateMatrix.BuildRotation(Vector3(0.f, 0.f, 1.f), -1.0f * params.angle);

    Matrix4 scaleMatrix;
    scaleMatrix.BuildScale(Vector3(1.0f, 1.f, 1.0f));

    Matrix4 worldMatrix;
    worldMatrix.BuildTranslation(Vector3(params.pos.x, params.pos.y, 0.f));

    Matrix4 resultMatrix = rotateMatrix * worldMatrix;

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
    batchDescriptor.worldMatrix = &resultMatrix;

    RenderSystem2D::Instance()->PushBatch(batchDescriptor);
}

void TextPainter::ApplyParamPos(DrawTextParams& params) const
{
    using namespace DAVA;

    if (params.size.IsZero())
    {
        Font::StringMetrics metrics = font->GetStringMetrics(UTF8Utils::EncodeToWideString(params.text));
        //while we using hard-coded font we need to fix it base line manually
        //DejaVuSans have a very big height which is invalid for digits. So while we use only digits, and font DejaVuSans and GraphicsFont have no GetBaseLine member function - i will change metrics height manually
        float32 padding = 6.0f;
        params.size = Vector2(metrics.width, metrics.height - padding);
        //params.scale /= params.scale.y;
    }

    params.size /= params.scale;
    params.margin /= params.scale;

    if (params.direction & ALIGN_LEFT)
    {
        params.pos.x -= (params.size.dx + params.margin.x);
    }
    else if (params.direction & ALIGN_HCENTER)
    {
        params.pos.x -= params.size.dx / 2.0f;
    }
    else if (params.direction & ALIGN_RIGHT)
    {
        params.pos.x += params.margin.x;
    }

    if (params.direction & ALIGN_TOP)
    {
        params.pos.y -= (params.size.dy + params.margin.y);
    }
    else if (params.direction & ALIGN_VCENTER)
    {
        params.pos.y -= params.size.dy / 2.0f;
    }
    else if (params.direction & ALIGN_BOTTOM)
    {
        params.pos.y += params.margin.y;
    }

    params.pos = Rotate(params.pos, params.angle);
    params.pos *= params.scale;
    params.pos += params.parentPos;
}
