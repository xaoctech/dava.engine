#include "REPlatform/Scene/Systems/TilemaskEditorSystem.h"

#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include "REPlatform/Commands/TilemaskEditorCommands.h"

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Input/Keyboard.h>
#include <Render/DynamicBufferAllocator.h>
#include <Render/Image/ImageConvert.h>
#include <Render/Renderer.h>
#include <UI/UIEvent.h>

namespace DAVA
{
static const FastName TILEMASK_EDITOR_FLAG_DRAW_COPY_PASTE("DRAW_COPY_PASTE");
static const FastName TILEMASK_EDITOR_PARAM_INTENSITY("intensity");
static const FastName TILEMASK_EDITOR_PARAM_INTENSITY_SING("intensitySign");
static const FastName TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET("copypasteOffset");

static const FastName TILEMASK_EDTIOR_TEXTURE_TOOL("toolTexture");

static const FastName TILEMASK_EDITOR_MATERIAL_PASS("2d");

namespace TilemaskEditorSystemDetails
{
const Array<FastName, 4> drawMaskNames =
{ {
FastName("drawMask0"),
FastName("drawMask1"),
FastName("drawMask2"),
FastName("drawMask3")
} };

const Array<FastName, 4> srcTexturesNames =
{ {
FastName("sourceTexture0"),
FastName("sourceTexture1"),
FastName("sourceTexture2"),
FastName("sourceTexture3")
} };
}

TilemaskEditorSystem::TilemaskEditorSystem(Scene* scene)
    : LandscapeEditorSystem(scene, DefaultCursorPath())
    , curToolSize(0)
    , toolImageTexture(nullptr)
    , landscapeTilemaskTexture(nullptr)
    , tileTextureNum(0)
    , drawingType(TILEMASK_DRAW_NORMAL)
    , strength(0.25f)
    , strengthSign(0.25f)
    , toolImagePath("")
    , toolImageIndex(0)
    , copyPasteFrom(-1.f, -1.f)
    , editingIsEnabled(false)
    , toolTexture(NULL)
    , toolSpriteUpdated(false)
    , needCreateUndo(false)
    , textureLevel(Landscape::TEXTURE_TILEMASK)
{
    curToolSize = 120;

    editorMaterial = new NMaterial();
    editorMaterial->SetFXName(FastName("~res:/Materials2/Landscape.Tilemask.Editor.material"));
    editorMaterial->AddFlag(TILEMASK_EDITOR_FLAG_DRAW_COPY_PASTE, 0);
    editorMaterial->AddFlag(NMaterialFlagName::FLAG_LANDSCAPE_LAYERS_COUNT, 1);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_INTENSITY, &strength, rhi::ShaderProp::TYPE_FLOAT1);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_INTENSITY_SING, &strengthSign, rhi::ShaderProp::TYPE_FLOAT1);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET, copyPasteOffset.data, rhi::ShaderProp::TYPE_FLOAT2);
    drawMasks.reserve(4);

    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);

    quadPacket.vertexStreamCount = 1;
    quadPacket.vertexCount = 6;
    quadPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    quadPacket.primitiveCount = 2;

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);

    quadPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);
}

TilemaskEditorSystem::~TilemaskEditorSystem()
{
    SafeRelease(editorMaterial);
}

LandscapeEditorDrawSystem::eErrorType TilemaskEditorSystem::EnableLandscapeEditing()
{
    if (enabled)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
    if (canBeEnabledError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = drawSystem->EnableTilemaskEditing();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enablingError;
    }

    drawSystem->UpdateTilemaskPathname();
    bool inited = drawSystem->InitTilemaskImageCopy();
    if (!inited)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT;
    }

    uint32 texSize = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(0, Landscape::TEXTURE_TILEMASK)->width;
    for (uint32 i = 1; i < drawSystem->GetLandscapeProxy()->GetLayersCount(); ++i)
    {
        uint32 layerWidth = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK)->width;
        if (texSize != layerWidth)
            return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURES_DIMENSIONS_DOES_NOT_MATCH;
    }

    bool inputLocked = AcquireInputLock(GetScene());
    DVASSERT(inputLocked);
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(true);
    scene->GetSystem<EntityModificationSystem>()->SetLocked(true);

    editorMaterial->SetFlag(NMaterialFlagName::FLAG_LANDSCAPE_LAYERS_COUNT, drawSystem->GetLandscapeProxy()->GetLayersCount());

    landscapeSize = drawSystem->GetTextureSize(0, textureLevel);
    copyPasteFrom = Vector2(-1.f, -1.f);

    if (previousLayerCount != drawSystem->GetLandscapeProxy()->GetLayersCount())
    {
        previousLayerCount = drawSystem->GetLandscapeProxy()->GetLayersCount();
        drawSystem->GetLandscapeProxy()->ReloadLayersCountDependentResources();
    }

    drawSystem->EnableCursor();
    drawSystem->EnableCustomDraw();
    drawSystem->SetCursorTexture(cursorTexture);
    drawSystem->SetCursorSize(cursorSize);
    SetBrushSize(curToolSize);

    InitSprites();

    for (uint32 i = 0; i < drawSystem->GetLandscapeProxy()->GetLayersCount(); ++i)
    {
        Asset<Texture> srcSprite = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
        Asset<Texture> dstSprite = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_DESTINATION);

        srcSprite->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
        dstSprite->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

        editorMaterial->AddTexture(TilemaskEditorSystemDetails::srcTexturesNames[i], srcSprite);
    }
    editorMaterial->AddTexture(TILEMASK_EDTIOR_TEXTURE_TOOL, toolTexture);

    drawSystem->GetBaseLandscape()->SetPagesUpdatePerFrame(128u);

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool TilemaskEditorSystem::DisableLandscapeEdititing()
{
    if (!enabled)
    {
        return true;
    }

    FinishEditing();
    needCreateUndo = false;

    ReleaseInputLock(GetScene());
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(false);
    scene->GetSystem<EntityModificationSystem>()->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();
    drawSystem->DisableTilemaskEditing();

    editorMaterial->RemoveTexture(TILEMASK_EDTIOR_TEXTURE_TOOL);

    for (uint32 i = 0; i < drawSystem->GetLandscapeProxy()->GetLayersCount(); ++i)
        editorMaterial->RemoveTexture(TilemaskEditorSystemDetails::srcTexturesNames[i]);

    drawSystem->GetBaseLandscape()->SetPagesUpdatePerFrame(16u);

    enabled = false;
    return !enabled;
}

void TilemaskEditorSystem::Process(float32 timeElapsed)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (editingIsEnabled && isIntersectsLandscape)
    {
        if (prevCursorPos != cursorPosition)
        {
            prevCursorPos = cursorPosition;

            Vector2 toolSize = Vector2(static_cast<float32>(curToolSize), static_cast<float32>(curToolSize));
            Vector2 toolPos = cursorPosition * landscapeSize - toolSize / 2.f;
            Rect toolRect(std::floor(toolPos.x), std::floor(toolPos.y), std::ceil(toolSize.x), std::ceil(toolSize.y));

            RenderSystem2D::RenderTargetPassDescriptor desc;
            desc.priority = PRIORITY_SERVICE_2D;
            desc.colorAttachment = toolTexture->handle;
            desc.depthAttachment = rhi::HTexture();
            desc.width = toolTexture->width;
            desc.height = toolTexture->height;
            desc.transformVirtualToPhysical = false;
            RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
            RenderSystem2D::Instance()->DrawTexture(toolImageTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, toolRect);
            RenderSystem2D::Instance()->EndRenderTargetPass();

            toolSpriteUpdated = true;

            if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
                editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET, copyPasteOffset.data);

            editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_INTENSITY_SING, &strengthSign);
            for (uint32 i = 0; i < drawMasks.size(); ++i)
            {
                if (editorMaterial->HasLocalProperty(TilemaskEditorSystemDetails::drawMaskNames[i]))
                    editorMaterial->SetPropertyValue(TilemaskEditorSystemDetails::drawMaskNames[i], drawMasks[i].data);
                else
                    editorMaterial->AddProperty(TilemaskEditorSystemDetails::drawMaskNames[i], drawMasks[i].data, rhi::ShaderProp::TYPE_FLOAT4);
            }

            AddRectToAccumulator(toolRect);
        }
    }
}

bool TilemaskEditorSystem::Input(UIEvent* event)
{
    if (!IsLandscapeEditingEnabled())
    {
        return false;
    }

    UpdateCursorPosition();

    if (event->mouseButton == eMouseButtons::LEFT)
    {
        Vector3 point;

        switch (event->phase)
        {
        case UIEvent::Phase::BEGAN:
            if (isIntersectsLandscape && !needCreateUndo)
            {
                Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
                bool altPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LALT).IsPressed() || kb->GetKeyState(eInputElements::KB_RALT).IsPressed());
                bool shiftPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || kb->GetKeyState(eInputElements::KB_RSHIFT).IsPressed());
                if (drawingType == TILEMASK_DRAW_COPY_PASTE)
                {
                    if (altPressed == true)
                    {
                        copyPasteFrom = cursorPosition;
                        copyPasteOffset = Vector2();
                        return false;
                    }
                    else
                    {
                        if (copyPasteFrom == Vector2(-1.f, -1.f))
                        {
                            return false;
                        }
                        copyPasteOffset = copyPasteFrom - cursorPosition;
                    }
                }

                static const Vector4 normalDrawMask[4] = {
                    Vector4(1.f, 0.f, 0.f, 0.f),
                    Vector4(0.f, 1.f, 0.f, 0.f),
                    Vector4(0.f, 0.f, 1.f, 0.f),
                    Vector4(0.f, 0.f, 0.f, 1.f)
                };
                static const Vector4 invertDrawMask[4] = {
                    Vector4(0.f, 1.f, 1.f, 1.f),
                    Vector4(1.f, 0.f, 1.f, 1.f),
                    Vector4(1.f, 1.f, 0.f, 1.f),
                    Vector4(1.f, 1.f, 1.f, 0.f)
                };

                static const Vector4 maskNothing(0.0f, 0.0f, 0.0f, 0.0f);
                static const Vector4 maskAll(1.0f, 1.0f, 1.0f, 1.0f);

                drawMasks.clear();
                for (uint32 i = 0; i < drawSystem->GetLandscapeProxy()->GetLayersCount(); ++i)
                {
                    if (i == tileTextureNum / 4)
                        drawMasks.push_back((shiftPressed == true) ? invertDrawMask[tileTextureNum % 4] : normalDrawMask[tileTextureNum % 4]);
                    else
                        drawMasks.push_back((shiftPressed == true) ? maskAll : maskNothing);
                }

                strengthSign = (shiftPressed == true || altPressed == true) ? -1.f : 1.f;

                ResetAccumulatorRect();
                editingIsEnabled = true;
                activeDrawingType = drawingType;
            }
            break;

        case UIEvent::Phase::DRAG:
            break;

        case UIEvent::Phase::ENDED:
            FinishEditing();
            break;

        default:
            break;
        }
    }
    return false;
}

void TilemaskEditorSystem::InputCancelled(UIEvent* event)
{
    if (IsLandscapeEditingEnabled() && (event->mouseButton == eMouseButtons::LEFT))
    {
        FinishEditing();
    }
}

void TilemaskEditorSystem::FinishEditing()
{
    if (editingIsEnabled)
    {
        needCreateUndo = true;
        editingIsEnabled = false;
    }
    prevCursorPos = Vector2(-1.f, -1.f);
}

void TilemaskEditorSystem::SetBrushSize(int32 brushSize)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = static_cast<float32>(brushSize) / landscapeSize;
        drawSystem->SetCursorSize(cursorSize);

        UpdateToolImage();
    }
}

void TilemaskEditorSystem::SetStrength(float32 strength_)
{
    if (strength_ >= 0)
    {
        strength = strength_;
        editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_INTENSITY, &strength);
    }
}

void TilemaskEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
    this->toolImagePath = toolImagePath;
    this->toolImageIndex = index;
    UpdateToolImage();
}

void TilemaskEditorSystem::SetTileTexture(uint32 tileTexture)
{
    tileTextureNum = tileTexture;
}

void TilemaskEditorSystem::UpdateBrushTool()
{
    struct QuadVertex
    {
        Vector3 position;
        Vector2 texCoord;
    };

    if (drawingType == TILEMASK_DRAW_COPY_PASTE && (copyPasteFrom == Vector2(-1.f, -1.f)))
        return;

    Vector<Asset<Texture>> dstTexs;
    for (uint32 i = 0; i < drawSystem->GetLandscapeProxy()->GetLayersCount(); ++i)
    {
        Asset<Texture> srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
        editorMaterial->SetTexture(TilemaskEditorSystemDetails::srcTexturesNames[i], srcTexture);
        dstTexs.push_back(drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_DESTINATION));
    }

    rhi::RenderPassConfig passConf;
    passConf.name = "TilemaskEditor_UpdateBrushTool";
    for (uint32 i = 0; i < dstTexs.size(); ++i)
    {
        passConf.colorBuffer[i].texture = dstTexs[i]->handle;
        passConf.colorBuffer[i].loadAction = rhi::LOADACTION_CLEAR;
        passConf.colorBuffer[i].storeAction = rhi::STOREACTION_STORE;
        Memset(passConf.colorBuffer[i].clearColor, 0, 4 * sizeof(float32));
    }
    passConf.priority = PRIORITY_SERVICE_2D;
    passConf.viewport.width = dstTexs[0]->width;
    passConf.viewport.height = dstTexs[0]->height;

    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
    editorMaterial->BindParams(quadPacket);

    DynamicBufferAllocator::AllocResultVB quadBuffer = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(QuadVertex), 4);
    QuadVertex* quadVertices = reinterpret_cast<QuadVertex*>(quadBuffer.data);

    quadVertices[0].position = Vector3(-1.f, -1.f, .0f);
    quadVertices[1].position = Vector3(-1.f, 1.f, .0f);
    quadVertices[2].position = Vector3(1.f, -1.f, .0f);
    quadVertices[3].position = Vector3(1.f, 1.f, .0f);

    if (rhi::DeviceCaps().isCenterPixelMapping)
    {
        const float32 pixelOffset = 1.f / dstTexs[0]->width;
        for (uint32 i = 0; i < 4; ++i)
        {
            quadVertices[i].position.x -= pixelOffset;
            quadVertices[i].position.y -= pixelOffset;
        }
    }

    if (rhi::DeviceCaps().isUpperLeftRTOrigin)
    {
        for (uint32 i = 0; i < 4; ++i)
            quadVertices[i].position.y = -quadVertices[i].position.y;
    }

    quadVertices[0].texCoord = Vector2(0.f, 0.f);
    quadVertices[1].texCoord = Vector2(0.f, 1.f);
    quadVertices[2].texCoord = Vector2(1.f, 0.f);
    quadVertices[3].texCoord = Vector2(1.f, 1.f);

    quadPacket.vertexStream[0] = quadBuffer.buffer;
    quadPacket.baseVertex = quadBuffer.baseVertex;
    quadPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);

    rhi::HPacketList pList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConf, 1, &pList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(pList);

    rhi::AddPacket(pList, quadPacket);

    rhi::EndPacketList(pList);
    rhi::EndRenderPass(pass);

    for (uint32 i = 0; i < dstTexs.size(); ++i)
    {
        drawSystem->SetTileMaskTexture(i, dstTexs[i]);
    }
    drawSystem->GetLandscapeProxy()->SwapTilemaskDrawTextures();

    Rect rect = GetUpdatedRect();
    rect.x /= toolTexture->width;
    rect.dx /= toolTexture->width;
    rect.y /= toolTexture->height;
    rect.dy /= toolTexture->height;
    rect.y = 1.f - (rect.y + rect.dy);

    drawSystem->GetBaseLandscape()->InvalidatePages(rect);
}

void TilemaskEditorSystem::UpdateToolImage()
{
    Vector<Image*> images;
    ImageSystem::Load(toolImagePath, images);
    if (images.size())
    {
        DVASSERT(images.size() == 1);
        DVASSERT(images[0]->GetPixelFormat() == FORMAT_RGBA8888);

        RefPtr<Image> toolImage(Image::Create(curToolSize, curToolSize, FORMAT_RGBA8888));
        ImageConvert::ResizeRGBA8Billinear(reinterpret_cast<uint32*>(images[0]->data), images[0]->GetWidth(), images[0]->GetHeight(),
                                           reinterpret_cast<uint32*>(toolImage->data), curToolSize, curToolSize);

        SafeRelease(images[0]);

        Texture::UniqueTextureKey key(toolImage, false);
        toolImageTexture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
    }
}

void TilemaskEditorSystem::AddRectToAccumulator(const Rect& rect)
{
    updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

void TilemaskEditorSystem::ResetAccumulatorRect()
{
    float32 inf = std::numeric_limits<float32>::infinity();
    updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

Rect TilemaskEditorSystem::GetUpdatedRect()
{
    Rect r = updatedRectAccumulator;
    drawSystem->ClampToTexture(textureLevel, r);

    return r;
}

uint32 TilemaskEditorSystem::GetTileTextureCount() const
{
    return 4;
}

void TilemaskEditorSystem::CreateMaskTexture()
{
    for (uint32 i = 0; i < drawSystem->GetLandscapeProxy()->GetLayersCount(); ++i)
    {
        Asset<Texture> tilemask = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK);
        Asset<Texture> srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_SOURCE);

        if (tilemask != srcTexture)
        {
            landscapeTilemaskTexture = tilemask;

            Rect destRect(0.0f, 0.0f, landscapeTilemaskTexture->width, landscapeTilemaskTexture->height);
            Rect sourceRect(0.0f, 0.0f, 1.0f, 1.0f);

            RenderSystem2D::RenderTargetPassDescriptor desc;
            desc.priority = PRIORITY_SERVICE_2D;
            desc.colorAttachment = srcTexture->handle;
            desc.depthAttachment = rhi::HTexture();
            desc.width = srcTexture->width;
            desc.height = srcTexture->height;
            desc.transformVirtualToPhysical = false;
            desc.clearTarget = true;

            RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
            RenderSystem2D::Instance()->DrawTextureWithoutAdjustingRects(landscapeTilemaskTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL,
                                                                         Color::White, destRect, sourceRect);
            RenderSystem2D::Instance()->EndRenderTargetPass();

            drawSystem->SetTileMaskTexture(i, srcTexture);
        }
    }
}

void TilemaskEditorSystem::Draw()
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (toolSpriteUpdated)
    {
        UpdateBrushTool();
        toolSpriteUpdated = false;
    }

    if (needCreateUndo)
    {
        CreateUndoPoint();
        needCreateUndo = false;
    }
}

void TilemaskEditorSystem::CreateUndoPoint()
{
    Rect rect = GetUpdatedRect();
    if (rect.dx > 0 && rect.dy > 0)
    {
        SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
        scene->Exec(std::make_unique<ModifyTilemaskCommand>(drawSystem->GetLandscapeProxy(), rect));
    }
}

int32 TilemaskEditorSystem::GetBrushSize()
{
    return curToolSize;
}

float32 TilemaskEditorSystem::GetStrength()
{
    return strength;
}

int32 TilemaskEditorSystem::GetToolImage()
{
    return toolImageIndex;
}

uint32 TilemaskEditorSystem::GetTileTextureIndex()
{
    return tileTextureNum;
}

void TilemaskEditorSystem::InitSprites()
{
    // GFX_COMPLETE
    int32 texSize = static_cast<int32>(drawSystem->GetTextureSize(0, textureLevel));
    if (toolTexture != nullptr && texSize != toolTexture->width)
    {
        toolTexture.reset();
    }

    if (toolTexture == nullptr)
    {
        Texture::RenderTargetTextureKey key;
        key.width = texSize;
        key.height = texSize;
        key.format = FORMAT_RGBA8888;
        key.isDepth = false;

        toolTexture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
    }

    drawSystem->GetLandscapeProxy()->InitTilemaskDrawTextures();
    CreateMaskTexture();
}

void TilemaskEditorSystem::SetDrawingType(eTilemaskDrawType type)
{
    if (type >= TILEMASK_DRAW_NORMAL && type < TILEMASK_DRAW_TYPES_COUNT)
    {
        drawingType = type;

        editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_COPY_PASTE, (type == TILEMASK_DRAW_COPY_PASTE) ? 1 : 0);
        editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
    }
}

TilemaskEditorSystem::eTilemaskDrawType TilemaskEditorSystem::GetDrawingType()
{
    return drawingType;
}
} // namespace DAVA
