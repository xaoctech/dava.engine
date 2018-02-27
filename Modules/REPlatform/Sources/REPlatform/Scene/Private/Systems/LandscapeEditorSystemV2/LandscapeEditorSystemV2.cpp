#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushInputController.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/ReadBackRingArray.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/TextureRenderBrushApplicant.h"

#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include <TArc/Utils/ReflectionHelpers.h>

#include <Base/BaseTypes.h>
#include <Command/Command.h>
#include <Debug/DVAssert.h>
#include <Engine/Engine.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Qt/RenderWidget.h>
#include <FileSystem/FilePath.h>
#include <Math/Rect.h>
#include <Math/Vector.h>
#include <Math/MathConstants.h>
#include <Math/MathHelpers.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Image/Image.h>
#include <Render/Material/NMaterial.h>
#include <Render/Renderer.h>
#include <Render/RHI/rhi_Public.h>
#include <Render/RuntimeTextures.h>
#include <Render/Texture.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <UI/UIControlSystem.h>

namespace DAVA
{
namespace LandscapeEditorSystemV2Details
{
const char* COVER_FLAG_NAME = "LANDSCAPE_COVER_TEXTURE";
const char* CURSOR_FLAG_NAME = "LANDSCAPE_CURSOR_V2";

struct RGBAPixel
{
    float32 r;
    float32 g;
    float32 b;
    float32 a;
};

const uint32 readbackTextureSize = 8;

class EditedLandscapeSetupGuard : public Command
{
public:
    EditedLandscapeSetupGuard(Scene* scene_, Landscape* landscape_, BaseLandscapeTool* tool_, RefPtr<Texture> cursor_)
        : scene(scene_)
        , landscape(landscape_)
        , tool(tool_)
        , cursor(cursor_)
    {
        DVASSERT(landscape != nullptr);
        DVASSERT(tool != nullptr);
    }

    void Redo() override
    {
        using namespace DAVA;

        RenderSystem* renderSystem = scene->GetRenderSystem();
        renderSystem->SetRenderPickingPass(true);
        landscape->AddFlag(RenderObject::VISIBLE_PICKING_PASS);

        NMaterial* material = landscape->GetLandscapeMaterial();

        FastName cursorFlagName(CURSOR_FLAG_NAME);
        DVASSERT(material->HasLocalFlag(cursorFlagName) == false);
        material->AddFlag(cursorFlagName, 1);

        RefPtr<Texture> coverTexture = tool->GetCustomCoverTexture();
        if (coverTexture.Get() != nullptr)
        {
            FastName coverFlagName(COVER_FLAG_NAME);
            DVASSERT(material->HasLocalFlag(coverFlagName) == false);
            material->AddFlag(coverFlagName, 1);

            FastName coverTextureSlotName(TextureRenderBrushApplicant::COVER_TEXTURE_SLOT_NAME);
            DVASSERT(material->HasLocalTexture(coverTextureSlotName) == false);
            material->AddTexture(coverTextureSlotName, coverTexture.Get());
        }

        Color c(0.5f, 0.5f, 1.0f, 1.0f);
        material->AddProperty(FastName("landCursorColor"), c.color, rhi::ShaderProp::TYPE_FLOAT3);
        FastName cursorTextureSlotName(TextureRenderBrushApplicant::CURSOR_TEXTURE_SLOT_NAME);
        DVASSERT(material->HasLocalTexture(cursorTextureSlotName) == false);
        material->AddTexture(cursorTextureSlotName, cursor.Get());

        FastName cursorPositionPropName(TextureRenderBrushApplicant::CURSOR_POS_PROP_NAME);
        DVASSERT(material->HasLocalProperty(cursorPositionPropName) == false);
        Vector4 pos(0.0, 0.0, 0.0, 0.0);
        material->AddProperty(cursorPositionPropName, pos.data, rhi::ShaderProp::TYPE_FLOAT4, 1);

        FastName cursorRotationPropName(TextureRenderBrushApplicant::CURSOR_ROTATION_PROP_NAME);
        DVASSERT(material->HasLocalProperty(cursorRotationPropName) == false);
        Vector2 rotationSinCos(0.0f, 1.0f);
        material->AddProperty(cursorRotationPropName, rotationSinCos.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
    }

    void Undo() override
    {
        using namespace DAVA;

        RenderSystem* renderSystem = scene->GetRenderSystem();
        renderSystem->SetRenderPickingPass(false);
        landscape->RemoveFlag(RenderObject::VISIBLE_PICKING_PASS);

        NMaterial* material = landscape->GetLandscapeMaterial();

        FastName cursorFlagName(CURSOR_FLAG_NAME);
        DVASSERT(material->HasLocalFlag(cursorFlagName) == true);
        material->RemoveFlag(cursorFlagName);

        FastName coverFlagName(COVER_FLAG_NAME);
        if (material->HasLocalFlag(coverFlagName) == true)
        {
            material->RemoveFlag(coverFlagName);
            FastName coverTextureSlotName(TextureRenderBrushApplicant::COVER_TEXTURE_SLOT_NAME);
            DVASSERT(material->HasLocalTexture(coverTextureSlotName) == true);
            material->RemoveTexture(coverTextureSlotName);
        }

        material->RemoveProperty(FastName("landCursorColor"));

        FastName cursorTextureSlotName(TextureRenderBrushApplicant::CURSOR_TEXTURE_SLOT_NAME);
        DVASSERT(material->HasLocalTexture(cursorTextureSlotName) == true);
        material->RemoveTexture(cursorTextureSlotName);

        FastName cursorPositionPropName(TextureRenderBrushApplicant::CURSOR_POS_PROP_NAME);
        DVASSERT(material->HasLocalProperty(cursorPositionPropName) == true);
        material->RemoveProperty(cursorPositionPropName);

        FastName cursorRotationPropName(TextureRenderBrushApplicant::CURSOR_ROTATION_PROP_NAME);
        DVASSERT(material->HasLocalProperty(cursorRotationPropName) == true);
        material->RemoveProperty(cursorRotationPropName);
    }

private:
    Scene* scene = nullptr;
    Landscape* landscape = nullptr;
    BaseLandscapeTool* tool = nullptr;
    RefPtr<Texture> cursor;
};

class InvertEditedLandscapeSetupGuard : public EditedLandscapeSetupGuard
{
public:
    InvertEditedLandscapeSetupGuard(Scene* scene, Landscape* landscape, BaseLandscapeTool* tool, RefPtr<Texture> cursor)
        : EditedLandscapeSetupGuard(scene, landscape, tool, cursor)
    {
    }

    void Redo() override
    {
        EditedLandscapeSetupGuard::Undo();
    }

    void Undo() override
    {
        EditedLandscapeSetupGuard::Redo();
    }
};

void UnpackToolTypes(const Type* baseType, Vector<const ReflectedType*>& result)
{
    const TypeInheritance* inheritance = baseType->GetInheritance();
    if (inheritance == nullptr)
    {
        return;
    }

    for (const TypeInheritance::Info& derived : inheritance->GetDerivedTypes())
    {
        const ReflectedType* derivedType = ReflectedTypeDB::GetByType(derived.type);
        if (derived.type->IsAbstract() == false)
        {
            if (derivedType == nullptr)
            {
                Logger::Warning("Landscape tool without reflection found: %s", derived.type->GetName());
                continue;
            }

            const String& permName = derivedType->GetPermanentName();
            if (permName.empty() == true)
            {
                Logger::Warning("Landscape tool without permanent name found: %s", derived.type->GetName());
                continue;
            }

            const AnyFn* constructor = derivedType->GetCtor<LandscapeEditorSystemV2*>(derived.type->Pointer());
            if (constructor->IsValid() == false)
            {
                Logger::Warning("Landscape tool without reflected ctr found: %s", permName.c_str());
                continue;
            }

            result.push_back(derivedType);
        }

        UnpackToolTypes(derived.type, result);
    }
}

} // namespace LandscapeEditorSystemV2Details

LandscapeEditorSystemV2::LandscapeEditorSystemV2(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<RenderComponent>())
    , defaultInputController(new DefaultBrushInputController())
{
    Vector<const ReflectedType*> tooltypes;
    LandscapeEditorSystemV2Details::UnpackToolTypes(Type::Instance<BaseLandscapeTool>(), tooltypes);

    std::sort(tooltypes.begin(), tooltypes.end(), [](const ReflectedType* left, const ReflectedType* right) {
        const BaseLandscapeTool::SortKey* leftKey = GetReflectedTypeMeta<BaseLandscapeTool::SortKey>(left);
        const BaseLandscapeTool::SortKey* rightKey = GetReflectedTypeMeta<BaseLandscapeTool::SortKey>(right);

        if (leftKey != nullptr && rightKey != nullptr)
        {
            if (leftKey->sortKey == rightKey->sortKey)
            {
                return left->GetPermanentName() < right->GetPermanentName();
            }
            else
            {
                return leftKey->sortKey > rightKey->sortKey;
            }
        }

        if (leftKey == nullptr && rightKey == nullptr)
        {
            return left->GetPermanentName() < right->GetPermanentName();
        }

        if (leftKey == nullptr)
        {
            return false;
        }

        return true;
    });

    for (const ReflectedType* type : tooltypes)
    {
        BaseLandscapeTool* tool = type->CreateObject(ReflectedType::CreatePolicy::ByPointer, this).Cast<BaseLandscapeTool*>(nullptr);
        if (tool == nullptr)
        {
            Logger::Warning("Couldn't create tool: %s", type->GetPermanentName().c_str());
            continue;
        }

        landscapeTools.push_back(tool);
    }

    DVASSERT(Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_UVPICKING) == FORMAT_RGBA32F);

    Texture::FBODescriptor descriptor;
    descriptor.format = FORMAT_RGBA32F;
    descriptor.height = LandscapeEditorSystemV2Details::readbackTextureSize;
    descriptor.width = LandscapeEditorSystemV2Details::readbackTextureSize;
    descriptor.mipLevelsCount = 1;
    descriptor.needDepth = false;
    descriptor.needPixelReadback = true;
    descriptor.textureType = rhi::TEXTURE_TYPE_2D;
    pickReadBackTextures.reset(new ReadBackRingArray(descriptor, 4));
    pickReadBackTextures->textureReady.Connect(this, &LandscapeEditorSystemV2::ReadPickTexture);
}

LandscapeEditorSystemV2::~LandscapeEditorSystemV2()
{
    for (BaseLandscapeTool* tool : landscapeTools)
    {
        SafeDelete(tool);
    }

    landscapeTools.clear();
}

void LandscapeEditorSystemV2::SetPropertiesCreatorFn(const Function<PropertiesItem(const String& nodeName)>& fn)
{
    DVASSERT(propsCreatorFn == nullptr);
    propsCreatorFn = fn;
}

void LandscapeEditorSystemV2::AddEntity(Entity* entity)
{
    RenderObject* ro = GetRenderObject(entity);
    DVASSERT(ro != nullptr);
    if (ro->GetType() == RenderObject::TYPE_LANDSCAPE)
    {
        landscapeEntities.insert(entity);
    }
}

void LandscapeEditorSystemV2::RemoveEntity(Entity* entity)
{
    Landscape* landscape = GetLandscape(entity);
    if (landscape != nullptr)
    {
        landscapeEntities.erase(entity);
        if (landscape == editedLandscape)
        {
            PrepareForEdit(nullptr);
        }
    }
}

void LandscapeEditorSystemV2::PrepareForRemove()
{
    landscapeEntities.clear();
}

void LandscapeEditorSystemV2::Process(float32 delta)
{
    if (activeTool == nullptr)
    {
        return;
    }

    DVASSERT(editedLandscape != nullptr);
    activeTool->Process(delta);

    NMaterial* landscapeMaterial = editedLandscape->GetLandscapeMaterial();
    {
        // check if cursor texture changed
        Color cursorColor = activeTool->GetCursorColor();
        landscapeMaterial->SetPropertyValue(FastName("landCursorColor"), cursorColor.color);
        RefPtr<Texture> cursorTexture = activeTool->GetCursorTexture();
        if (cursorTexture.Get() != nullptr && currentCursorTexture != cursorTexture)
        {
            currentCursorTexture = cursorTexture;

            FastName cursorTextureSlotName(TextureRenderBrushApplicant::CURSOR_TEXTURE_SLOT_NAME);
            DVASSERT(landscapeMaterial->HasLocalTexture(cursorTextureSlotName) == true);
            landscapeMaterial->SetTexture(cursorTextureSlotName, currentCursorTexture.Get());
        }
    }

    {
        // update cursor position in landscape material to draw it on the screen
        // or not draw if mouse has leaved render widget
        Vector4 cursorPos = GetActiveInputController()->GetCurrentCursorUV();
        if (cursorPos.w == 0.0f && isMouseInRenderWidget == true)
        {
            Vector2 brushSize = activeTool->GetBrushSize();
            cursorPos.z = brushSize.x;
            cursorPos.w = brushSize.y;
        }
        else
        {
            cursorPos.z = cursorPos.w = 0.0f;
        }

        FastName cursorPositionPropName(TextureRenderBrushApplicant::CURSOR_POS_PROP_NAME);
        DVASSERT(landscapeMaterial->HasLocalProperty(cursorPositionPropName) == true);
        landscapeMaterial->SetPropertyValue(cursorPositionPropName, cursorPos.data);

        Vector2 rotationSinCos;
        SinCosFast(PI_2 * activeTool->GetBrushRotation(), rotationSinCos.x, rotationSinCos.y);
        FastName cursorRotationPropName(TextureRenderBrushApplicant::CURSOR_ROTATION_PROP_NAME);
        landscapeMaterial->SetPropertyValue(cursorRotationPropName, rotationSinCos.data);
    }

    RenderSystem* rs = GetScene()->GetRenderSystem();
    {
        // Copy part of pick texture under system's cursor to be ready modify landscape on next frame
        rhi::HSyncObject currentFrameSync = rhi::GetCurrentFrameSyncObject();
        Size2i texSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_UVPICKING);

        Vector2 cursorPos = GetActiveInputController()->GetCursorPos();
        Vector2 targetCenter(cursorPos.x * texSize.dx, cursorPos.y * texSize.dy);

        Rect blitRect;
        blitRect.SetSize(Vector2(LandscapeEditorSystemV2Details::readbackTextureSize, LandscapeEditorSystemV2Details::readbackTextureSize));
        blitRect.SetCenter(targetCenter);
        uvPickBlitter.BlitTextureRect(blitRect, pickReadBackTextures->AcquireTexture(currentFrameSync), rs->GetConfiguration().basePriority);
    }

    if (isMouseInRenderWidget == false)
    {
        if (operationBegan == true)
        {
            EndOperation(true);
        }
        return;
    }

    if (operationBegan == true && operationCancelRequested == true)
    {
        EndOperation(true);
    }
    else if (operationBegan == true && operationEndRequested == true)
    {
        EndOperation(false);
    }
    else if (operationBeginRequested == true)
    {
        DVASSERT(operationBegan == false);
        BeginOperation();
    }

    if (operationBegan == true)
    {
        ApplyBrush();
    }
}

bool LandscapeEditorSystemV2::Input(UIEvent* uie)
{
    if (activeTool != nullptr)
    {
        GetActiveInputController()->OnInput(uie);

        if (uie->phase == UIEvent::Phase::BEGAN &&
            uie->mouseButton == eMouseButtons::LEFT &&
            operationBegan == false)
        {
            operationBeginRequested = true;
        }
        else if (uie->phase == UIEvent::Phase::ENDED &&
                 ((operationBegan == true) ||
                  (operationBeginRequested == true)))
        {
            operationEndRequested = true;
        }
    }
    return false;
}

void LandscapeEditorSystemV2::InputCancelled(UIEvent* uie)
{
    operationCancelRequested = true;
}

const Vector<BaseLandscapeTool*>& LandscapeEditorSystemV2::GetAvailableTools() const
{
    return landscapeTools;
}

const BaseLandscapeTool* LandscapeEditorSystemV2::GetActiveTool() const
{
    return activeTool;
}

bool LandscapeEditorSystemV2::ActivateTool(BaseLandscapeTool* tool)
{
    using namespace LandscapeEditorSystemV2Details;
    using namespace DAVA;

    DVASSERT(editedLandscape != nullptr);
    bool inputLocked = AcquireInputLock(GetScene());
    if (inputLocked == false)
    {
        return false;
    }

    {
        // Render Widget modification
        RenderWidget* renderWidget = PlatformApi::Qt::GetRenderWidget();
        prevCursor = renderWidget->cursor();
        QCursor blankCursor(Qt::BlankCursor);
        renderWidget->setCursor(blankCursor);
        foreach (QWidget* w, renderWidget->findChildren<QWidget*>())
        {
            w->setCursor(blankCursor);
        }
        renderWidget->mouseEntered.Connect(this, &LandscapeEditorSystemV2::OnMouseEnterRenderWidget);
        renderWidget->mouseLeaved.Connect(this, &LandscapeEditorSystemV2::OnMouseLeaveRenderWidget);
    }

    activeTool = tool;
    DVASSERT(propsCreatorFn != nullptr);
    activeTool->Activate(propsCreatorFn("LandscapeEditorProperties"));
    toolInputController = activeTool->GetInputController();
    toolInputController->Init(GetScene());
    toolInputController->UpdateCurrentCursorUV(defaultInputController->GetCurrentCursorUV());
    brushApplicant = activeTool->GetBrushApplicant();

    currentCursorTexture = activeTool->GetCursorTexture();
    if (currentCursorTexture.Get() == nullptr)
    {
        currentCursorTexture = CreateSingleMipTexture(FilePath("~res:/ResourceEditor/LandscapeEditor/Tools/cursor/cursor.png"));
    }

    {
        LandscapeEditorSystemV2Details::EditedLandscapeSetupGuard guard(GetScene(), editedLandscape, activeTool, currentCursorTexture);
        guard.Redo();
    }

    return true;
}

void LandscapeEditorSystemV2::DeactivateTool()
{
    using namespace LandscapeEditorSystemV2Details;
    using namespace DAVA;

    if (editedLandscape == nullptr || activeTool == nullptr)
    {
        return;
    }

    DVASSERT(operationBegan == false);

    {
        // Render Widget modification
        RenderWidget* renderWidget = PlatformApi::Qt::GetRenderWidget();
        renderWidget->setCursor(prevCursor);
        foreach (QWidget* w, renderWidget->findChildren<QWidget*>())
        {
            w->setCursor(prevCursor);
        }
        renderWidget->mouseEntered.Disconnect(this);
        renderWidget->mouseLeaved.Disconnect(this);
    }

    {
        LandscapeEditorSystemV2Details::EditedLandscapeSetupGuard guard(GetScene(), editedLandscape, activeTool, currentCursorTexture);
        guard.Undo();
    }

    RestoreTextureOverrides();

    currentCursorTexture = RefPtr<Texture>();
    overrideMapping.clear();
    DVASSERT(propsCreatorFn != nullptr);
    toolInputController->Reset();
    toolInputController = nullptr;
    brushApplicant = nullptr;
    PropertiesItem propItem = propsCreatorFn("LandscapeEditorProperties");
    activeTool->Deactivate(propItem);
    activeTool = nullptr;
    ReleaseInputLock(GetScene());
}

bool LandscapeEditorSystemV2::IsEditingAllowed() const
{
    return editedLandscape != nullptr && editedLandscape->GetRenderMode() == Landscape::RENDERMODE_INSTANCING_MORPHING;
}

void LandscapeEditorSystemV2::PrepareForEdit(Landscape* landscape)
{
    if (landscape == nullptr)
    {
        DeactivateTool();
    }

    editedLandscape = landscape;
}

int32 LandscapeEditorSystemV2::GetLandscapeTextureCount(Landscape::eLandscapeTexture textureSemantic) const
{
    DVASSERT(editedLandscape != nullptr);
    return editedLandscape->GetTextureCount(textureSemantic);
}

RefPtr<Texture> LandscapeEditorSystemV2::GetOriginalLandscapeTexture(Landscape::eLandscapeTexture textureSemantic, int32 index) const
{
    DVASSERT(editedLandscape != nullptr);
    RTMappingKey key;
    key.semantic = textureSemantic;
    key.index = index;

    auto iter = overrideMapping.find(key);
    if (iter != overrideMapping.end())
    {
        return iter->second;
    }

    return RefPtr<Texture>::ConstructWithRetain(editedLandscape->GetTexture(textureSemantic, index));
}

void LandscapeEditorSystemV2::SetOverrideTexture(Landscape::eLandscapeTexture textureSemantic, int32 index, RefPtr<Texture> texture)
{
    RTMappingKey key;
    key.semantic = textureSemantic;
    key.index = index;
    DVASSERT(overrideMapping.count(key) == 0);

    RefPtr<Texture> sourceTexture = RefPtr<Texture>::ConstructWithRetain(editedLandscape->GetTexture(textureSemantic, index));
    editedLandscape->SetTexture(textureSemantic, index, texture.Get());
    overrideMapping.emplace(key, sourceTexture);
}

Landscape* LandscapeEditorSystemV2::GetEditedLandscape() const
{
    DVASSERT(editedLandscape != nullptr);
    return editedLandscape;
}

void LandscapeEditorSystemV2::UpdateHeightmap(Landscape* landscape, const Vector<uint16>& data, const Rect2i& rect)
{
    using namespace DAVA;
    Map<RTMappingKey, RefPtr<Texture>> overrideTargetMapping;
    for (const auto& node : overrideMapping)
    {
        RefPtr<Texture> textureOverride = RefPtr<Texture>::ConstructWithRetain(editedLandscape->GetTexture(node.first.semantic, node.first.index));
        overrideTargetMapping.emplace(node.first, textureOverride);
        editedLandscape->SetTexture(node.first.semantic, node.first.index, node.second.Get());
    }

    landscape->UpdateHeightmap(data, rect);

    for (const auto& node : overrideTargetMapping)
    {
        editedLandscape->SetTexture(node.first.semantic, node.first.index, node.second.Get());
    }
}

void LandscapeEditorSystemV2::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (brushApplicant != nullptr)
    {
        brushApplicant->OnCommandExecuted(commandNotification);
    }
}

void LandscapeEditorSystemV2::AddDebugDraw(RenderObject* ro)
{
    GetScene()->renderSystem->RenderPermanent(ro);
}

void LandscapeEditorSystemV2::RemoveDebugDraw(RenderObject* ro)
{
    GetScene()->renderSystem->RemoveFromRender(ro);
}

std::unique_ptr<Command> LandscapeEditorSystemV2::PrepareForSave(bool saveForGame)
{
    if (activeTool == nullptr)
    {
        return nullptr;
    }

    return std::make_unique<LandscapeEditorSystemV2Details::InvertEditedLandscapeSetupGuard>(GetScene(), editedLandscape, activeTool, currentCursorTexture);
}

DAVA::Scene* LandscapeEditorSystemV2::GetEditedScene() const
{
    return GetScene();
}

BrushInputController* LandscapeEditorSystemV2::GetActiveInputController() const
{
    if (toolInputController == nullptr)
    {
        return defaultInputController.get();
    }

    return toolInputController;
}

void LandscapeEditorSystemV2::BeginOperation()
{
    DVASSERT(editedLandscape != nullptr);
    DVASSERT(operationBegan == false);

    operationBegan = true;
    operationBeginRequested = false;
    StoreSnapshots();
    BrushInputController* controller = GetActiveInputController();
    controller->BeginOperation(controller->GetCurrentCursorUV());
}

void LandscapeEditorSystemV2::ApplyBrush()
{
    DVASSERT(operationBegan == true);
    Vector4 lastCursorPosInLandscapeSpace = GetActiveInputController()->GetCurrentCursorUV();
    if (lastCursorPosInLandscapeSpace.w == 0.0f)
    {
        Vector2 cursorSize = activeTool->GetBrushSize();
        Rect applyRect;
        applyRect.SetSize(cursorSize);
        applyRect.SetCenter(Vector2(lastCursorPosInLandscapeSpace.x, lastCursorPosInLandscapeSpace.y));

        applyRect.x = Saturate(applyRect.x);
        applyRect.y = Saturate(applyRect.y);
        if (applyRect.x + applyRect.dx > 1.0f)
        {
            applyRect.dx = 1.0f - applyRect.x;
        }
        if (applyRect.y + applyRect.dy > 1.0f)
        {
            applyRect.dy = 1.0f - applyRect.y;
        }

        if (operationRect == Rect())
        {
            operationRect = applyRect;
        }
        else
        {
            operationRect = operationRect.Combine(applyRect);
        }

        Vector4 cursorPosSize(lastCursorPosInLandscapeSpace.x, 1.0f - lastCursorPosInLandscapeSpace.y, cursorSize.x, cursorSize.y);
        Vector2 rotationSinCos;
        SinCosFast(PI_2 * activeTool->GetBrushRotation(), rotationSinCos.x, rotationSinCos.y);

        brushApplicant->SetCursorUV(cursorPosSize);
        brushApplicant->SetRotation(rotationSinCos);
        brushApplicant->SetInvertionFactor(1.0);
        brushApplicant->SetCursorTexture(currentCursorTexture);
        brushApplicant->ApplyBrush(GetScene(), applyRect);
    }
}

void LandscapeEditorSystemV2::EndOperation(bool canceled)
{
    DVASSERT(operationBegan == true);
    if (canceled == false)
    {
        CreateDiffCommand();
    }

    operationBeginRequested = false;
    operationCancelRequested = false;
    operationEndRequested = false;
    operationBegan = false;
    operationRect = Rect();
    GetActiveInputController()->EndOperation();
}

void LandscapeEditorSystemV2::StoreSnapshots()
{
    DVASSERT(activeTool != nullptr);
    brushApplicant->StoreSnapshots();
}

void LandscapeEditorSystemV2::ReadPickTexture(RefPtr<Texture> texture)
{
    using namespace LandscapeEditorSystemV2Details;

    RefPtr<Image> image(texture->CreateImageFromMemory());
    DVASSERT(image->width == LandscapeEditorSystemV2Details::readbackTextureSize);
    DVASSERT(image->height == LandscapeEditorSystemV2Details::readbackTextureSize);
    DVASSERT(image->GetPixelFormat() == FORMAT_RGBA32F);

    RGBAPixel* colorData = reinterpret_cast<RGBAPixel*>(image->data);

    RGBAPixel topLeft = colorData[3 + 3 * image->width];
    RGBAPixel topRight = colorData[4 + 3 * image->width];
    RGBAPixel bottomLeft = colorData[3 + 4 * image->width];
    RGBAPixel bottomRight = colorData[4 + 4 * image->width];

    Vector4 unnormalizedColor = Vector4((topLeft.r + topRight.r + bottomLeft.r + bottomRight.r) / 4.0f,
                                        (topLeft.g + topRight.g + bottomLeft.g + bottomRight.g) / 4.0f,
                                        (topLeft.b + topRight.b + bottomLeft.b + bottomRight.b) / 4.0f,
                                        (topLeft.a + topRight.a + bottomLeft.a + bottomRight.a) / 4.0f);

    GetActiveInputController()->UpdateCurrentCursorUV(unnormalizedColor);
}

void LandscapeEditorSystemV2::CreateDiffCommand()
{
    using namespace DAVA;
    DVASSERT(activeTool != nullptr);

    operationRect.y = Saturate(1.0 - operationRect.y - operationRect.dy);
    std::unique_ptr<Command> command = brushApplicant->CreateDiffCommand(operationRect);

    Map<RTMappingKey, RefPtr<Texture>> overrideTargetMapping;
    for (const auto& node : overrideMapping)
    {
        RefPtr<Texture> textureOverride = RefPtr<Texture>::ConstructWithRetain(editedLandscape->GetTexture(node.first.semantic, node.first.index));
        overrideTargetMapping.emplace(node.first, textureOverride);
        editedLandscape->SetTexture(node.first.semantic, node.first.index, node.second.Get());
    }

    static_cast<SceneEditor2*>(GetScene())->Exec(std::move(command));

    for (const auto& node : overrideTargetMapping)
    {
        editedLandscape->SetTexture(node.first.semantic, node.first.index, node.second.Get());
    }
}

void LandscapeEditorSystemV2::RestoreTextureOverrides()
{
    DVASSERT(editedLandscape != nullptr);
    for (const auto& node : overrideMapping)
    {
        editedLandscape->SetTexture(node.first.semantic, node.first.index, node.second.Get());
    }
}

void LandscapeEditorSystemV2::OnMouseLeaveRenderWidget()
{
    isMouseInRenderWidget = false;
}

void LandscapeEditorSystemV2::OnMouseEnterRenderWidget()
{
    isMouseInRenderWidget = true;
}
} // namespace DAVA
