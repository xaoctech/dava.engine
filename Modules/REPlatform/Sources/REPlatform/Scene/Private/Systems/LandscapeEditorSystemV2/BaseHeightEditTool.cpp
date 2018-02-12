#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/TextureRenderBrushApplicant.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include "REPlatform/Commands/HeightmapEditorCommands2.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"

#include <TArc/Controls/Slider.h>
#include <TArc/Controls/SpinSlider.h>
#include <TArc/Controls/ControlDescriptor.h>

#include <FileSystem/FileList.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Render/Highlevel/Landscape.h>

namespace DAVA
{
namespace BaseHeightEditToolDetail
{
const char* BRUSH_TEXTURES_DIR = "~res:/ResourceEditor/LandscapeEditorV2/BrushTextures/";
} // namespace BaseHeightEditTool

BaseHeightEditTool::BaseHeightEditTool(LandscapeEditorSystemV2* system, const ButtonInfo& buttonInfo_, const String& toolName_)
    : BaseTextureRenderLandscapeTool(system)
    , toolName(toolName_)
    , buttonInfo(buttonInfo_)
{
    ResetInputController(std::make_unique<KeyboardInputController>());
    blitConvertMaterial.Set(new NMaterial());
    blitConvertMaterial->SetFXName(NMaterialName::LANDSCAPE_BRUSH);
}

void BaseHeightEditTool::Activate(const PropertiesItem& settings)
{
    RefPtr<FileList> fileList(new FileList(BaseHeightEditToolDetail::BRUSH_TEXTURES_DIR, false));
    FilePath firstTexturePath;
    for (uint32 i = 0; i < fileList->GetCount(); ++i)
    {
        if (fileList->IsDirectory(i) == false)
        {
            firstTexturePath = fileList->GetPathname(i);
            break;
        }
    }

    DVASSERT(firstTexturePath.IsEmpty() == false);
    DVASSERT(firstTexturePath.IsDirectoryPathname() == false);

    PropertiesItem toolSettings = settings.CreateSubHolder("BaseHeightEditTool");
    String brushPath = toolSettings.Get<String>("brushTexture", firstTexturePath.GetAbsolutePathname());
    SetBrushPath(brushPath);

    rotation = toolSettings.Get<float32>("brushRotation", rotation);
    brushSize = toolSettings.Get<float32>("brushSize", brushSize);
    strength = toolSettings.Get<float32>("strength", strength);

    brushApplicant.reset(new TextureRenderBrushApplicant(this));
}

BrushInputController* BaseHeightEditTool::GetInputController() const
{
    return inputController.get();
}

BaseBrushApplicant* BaseHeightEditTool::GetBrushApplicant() const
{
    return brushApplicant.get();
}

BaseLandscapeTool::ButtonInfo BaseHeightEditTool::GetButtonInfo() const
{
    return buttonInfo;
}

RefPtr<Texture> BaseHeightEditTool::GetCustomCoverTexture() const
{
    return RefPtr<Texture>();
}

RefPtr<Texture> BaseHeightEditTool::GetCursorTexture() const
{
    return cursorTexture;
}

const String& BaseHeightEditTool::GetBrushPath() const
{
    return cursorPath;
}

void BaseHeightEditTool::SetBrushPath(const String& brushPath)
{
    cursorPath = brushPath;
    cursorTexture = CreateSingleMipTexture(FilePath(brushPath));
    cursorTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
}

Vector2 BaseHeightEditTool::GetBrushSize() const
{
    return Vector2(brushSize, brushSize);
}

float32 BaseHeightEditTool::GetRotationAngle() const
{
    return rotation * 360.0f;
}

void BaseHeightEditTool::SetRotationAngle(const float32& angle)
{
    rotation = angle / 360.0f;
}

float32 BaseHeightEditTool::GetBrushRadius() const
{
    Landscape* landscape = system->GetEditedLandscape();
    return landscape->GetLandscapeSize() * brushSize;
}

void BaseHeightEditTool::SetBrushRadius(const float32& radius)
{
    Landscape* landscape = system->GetEditedLandscape();
    brushSize = DAVA::Saturate(radius / landscape->GetLandscapeSize());
}

float32 BaseHeightEditTool::GetStrengthInMeters() const
{
    Landscape* landscape = system->GetEditedLandscape();
    return landscape->GetLandscapeHeight() * strength;
}

void BaseHeightEditTool::SetStrengthInMeters(const float32& v)
{
    Landscape* landscape = system->GetEditedLandscape();
    strength = DAVA::Saturate(v / landscape->GetLandscapeHeight());
}

float32 BaseHeightEditTool::GetBrushRotation() const
{
    return rotation;
}

void BaseHeightEditTool::Deactivate(PropertiesItem& settings)
{
    brushApplicant.reset();
    cursorTexture = RefPtr<Texture>();
    morphTexture = RefPtr<Texture>();
    floatTexture = RefPtr<Texture>();
    heightSnapshot = RefPtr<Texture>();

    PropertiesItem toolSettings = settings.CreateSubHolder("BaseHeightEditTool");

    DAVA::FilePath brushPath = GetBrushPath();
    DAVA::String brushAbsPath = brushPath.GetAbsolutePathname();

    const DAVA::Vector<DAVA::FilePath>& resFolders = DAVA::FilePath::GetResFolders();
    for (const DAVA::FilePath& folder : resFolders)
    {
        DAVA::String absFolder = folder.GetAbsolutePathname();
        size_t pos = brushAbsPath.find(absFolder);
        if (pos != DAVA::String::npos)
        {
            brushPath = DAVA::FilePath("~res:/" + brushPath.GetRelativePathname(absFolder));
        }
    }

    toolSettings.Set("brushTexture", brushPath);
    toolSettings.Set("brushRotation", rotation);
    toolSettings.Set("brushSize", brushSize);
    toolSettings.Set("strength", strength);
}

void BaseHeightEditTool::StoreSnapshots()
{
    RefPtr<Texture> heightTexture = system->GetOriginalLandscapeTexture(Landscape::HEIGHTMAP_TEXTURE, 0);
    if (heightSnapshot.Get() == nullptr)
    {
        Texture::FBODescriptor descr;
        descr.format = FORMAT_RGBA8888;
        descr.height = heightTexture->height;
        descr.width = heightTexture->width;
        descr.mipLevelsCount = 1;
        descr.needDepth = false;
        descr.needPixelReadback = true;
        descr.textureType = rhi::TEXTURE_TYPE_2D;
        heightSnapshot.Set(Texture::CreateFBO(descr));
    }

    blitConvertMaterial->AddFlag(FastName("COPY_TEXTURE_LOD"), 1);
    blitConvertMaterial->AddTexture(FastName("texture0"), heightTexture.Get());
    float32 lodLevel = 0;
    blitConvertMaterial->AddProperty(FastName("lodLevel"), &lodLevel, rhi::ShaderProp::TYPE_FLOAT1, 1);

    blitConvertMaterial->SetPropertyValue(FastName("lodLevel"), &lodLevel);
    if (blitConvertMaterial->PreBuildMaterial(PASS_FORWARD))
    {
        TextureBlitter::TargetInfo info;
        info.renderTarget = heightSnapshot;
        info.textureLevel = 0;
        blitter.BlitTexture(info, blitConvertMaterial, PRIORITY_SERVICE_3D);
    }
    blitConvertMaterial->RemoveTexture(FastName("texture0"));
    blitConvertMaterial->RemoveFlag(FastName("COPY_TEXTURE_LOD"));
    blitConvertMaterial->RemoveProperty(FastName("lodLevel"));
}

void BaseHeightEditTool::CreateBrushSelector(BrushWidgetBuilder& builder) const
{
    ControlDescriptorBuilder<DAVA::BrushTextureSelector::Fields> fields;
    fields[DAVA::BrushTextureSelector::Fields::Value] = "brush";
    builder.RegisterParam<DAVA::BrushTextureSelector>("Brush", fields);
}

void BaseHeightEditTool::CreateRotationSlider(BrushWidgetBuilder& builder) const
{
    ControlDescriptorBuilder<DAVA::SpinSlider::Fields> fields;
    fields[DAVA::SpinSlider::Fields::SliderValue] = "rotation";
    fields[DAVA::SpinSlider::Fields::SpinValue] = "rotationAngle";
    builder.RegisterParam<DAVA::SpinSlider>("Brush rotation (Y)", fields);
}

void BaseHeightEditTool::CreateBrushSizeControl(BrushWidgetBuilder& builder) const
{
    ControlDescriptorBuilder<DAVA::SpinSlider::Fields> fields;
    fields[DAVA::SpinSlider::Fields::SliderValue] = "size";
    fields[DAVA::SpinSlider::Fields::SpinValue] = "brushRadius";
    builder.RegisterParam<DAVA::SpinSlider>("Brush radius (H)", fields);
}

void BaseHeightEditTool::CreateStrengthSlider(BrushWidgetBuilder& builder) const
{
    ControlDescriptorBuilder<DAVA::SpinSlider::Fields> fields;
    fields[DAVA::SpinSlider::Fields::SliderValue] = "strength";
    fields[DAVA::SpinSlider::Fields::SpinValue] = "strengthInMeters";
    builder.RegisterParam<DAVA::SpinSlider>("Brush strength (N)", fields);
}

void BaseHeightEditTool::ResetInputController(std::unique_ptr<KeyboardInputController>&& controller)
{
    inputController = std::move(controller);

    inputController->RegisterVarCallback(eInputElements::KB_Y, [this](const Vector2& delta) {
        rotation += delta.y;
        rotation = Saturate(rotation);
    });

    inputController->RegisterVarCallback(eInputElements::KB_H, [this](const Vector2& delta) {
        brushSize += delta.y;
        brushSize = Saturate(brushSize);
    });

    inputController->RegisterVarCallback(eInputElements::KB_N, [this](const Vector2& delta) {
        strength += delta.y * 0.1;
        strength = Saturate(strength);
    });
}

void BaseHeightEditTool::InitTextures()
{
    floatTexture = CreateFloatHeightTexture();
    morphTexture = CreateHeightTextureCopy();
    normalMap = CreateNormalTextureCopy();

    system->SetOverrideTexture(Landscape::HEIGHTMAP_TEXTURE, 0, morphTexture);
    system->SetOverrideTexture(Landscape::TANGENT_TEXTURE, 0, normalMap);
}

std::unique_ptr<Command> BaseHeightEditTool::CreateDiffCommand(const Rect& operationRect) const
{
    using namespace DAVA;

    DVASSERT(heightSnapshot->width == morphTexture->width);
    DVASSERT(heightSnapshot->height == morphTexture->height);
    DVASSERT(heightSnapshot->GetFormat() == DAVA::FORMAT_RGBA8888);
    DVASSERT(morphTexture->GetFormat() == DAVA::FORMAT_RGBA8888);

    uint32 x = static_cast<uint32>(std::floor(operationRect.x * heightSnapshot->width));
    uint32 y = static_cast<uint32>(std::floor(operationRect.y * heightSnapshot->height));
    uint32 dx = static_cast<uint32>(std::ceil(operationRect.dx * heightSnapshot->width));
    uint32 dy = static_cast<uint32>(std::ceil(operationRect.dy * heightSnapshot->width));
    Rect2i updatedRect(x, y, dx, dy);

    uint32 diffSize = static_cast<uint32>(dx * dy);
    Vector<uint32> srcRegion(diffSize, 0);
    Vector<uint32> dstRegion(diffSize, 0);
    rhi::ReadTextureRegion(heightSnapshot->handle, srcRegion.data(), 4 * diffSize, updatedRect);
    rhi::ReadTextureRegion(morphTexture->handle, dstRegion.data(), 4 * diffSize, updatedRect);

    Vector<uint16> srcData(diffSize, 0);
    Vector<uint16> dstData(diffSize, 0);

    auto copyHeightInfo = [&](Vector<uint16>& data, const Vector<uint32>& img) {
        const uint32* imgData = img.data();
        size_t dataOffset = 0;
        for (uint32 yIndex = 0; yIndex < dy; ++yIndex)
        {
            for (uint32 xIndex = 0; xIndex < dx; ++xIndex)
            {
                uint32 pixelValue = imgData[yIndex * dx + xIndex];
                uint16 heightValue = static_cast<uint16>(pixelValue & 0xFFFF);
                data[dataOffset++] = heightValue;
            }
        }
    };

    copyHeightInfo(srcData, srcRegion);
    copyHeightInfo(dstData, dstRegion);

    return std::make_unique<ModifyHeightmapCommandV2>(system, system->GetEditedLandscape(), srcData, dstData, updatedRect);
}

void BaseHeightEditTool::OnCommandExecuted(const RECommandNotificationObject& notif)
{
    if (notif.MatchCommandTypes<ModifyHeightmapCommandV2>())
    {
        RefPtr<Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(Landscape::HEIGHTMAP_TEXTURE, 0);
        CopyHeightTextureToFloat(srcHeightTexture, floatTexture);
        CopyTextureWithMips(srcHeightTexture, morphTexture);
        CopyTextureWithMips(system->GetOriginalLandscapeTexture(Landscape::TANGENT_TEXTURE, 0), normalMap);
    }
}

RefPtr<Texture> BaseHeightEditTool::CreateFloatHeightTexture()
{
    RefPtr<Texture> heightTexture = system->GetOriginalLandscapeTexture(Landscape::HEIGHTMAP_TEXTURE, 0);
    RefPtr<Texture> result;

    Texture::FBODescriptor descr;
    descr.format = FORMAT_R32F;
    descr.height = heightTexture->height;
    descr.width = heightTexture->width;
    descr.mipLevelsCount = 1;
    descr.needDepth = false;
    descr.needPixelReadback = true;
    descr.textureType = rhi::TEXTURE_TYPE_2D;
    result.Set(Texture::CreateFBO(descr));
    result->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
    result->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    result->maxAnisotropyLevel = 1;

    CopyHeightTextureToFloat(heightTexture, result);
    return result;
}

RefPtr<Texture> BaseHeightEditTool::CreateHeightTextureCopy()
{
    RefPtr<Texture> heightTexture = system->GetOriginalLandscapeTexture(Landscape::HEIGHTMAP_TEXTURE, 0);
    RefPtr<Texture> result;

    Texture::FBODescriptor descr;
    descr.format = heightTexture->GetFormat();
    descr.height = heightTexture->height;
    descr.width = heightTexture->width;
    descr.mipLevelsCount = heightTexture->GetMipLevelsCount();
    descr.needDepth = false;
    descr.needPixelReadback = true;
    descr.textureType = rhi::TEXTURE_TYPE_2D;
    result.Set(Texture::CreateFBO(descr));
    result->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NEAREST);
    result->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    result->maxAnisotropyLevel = 1;

    CopyTextureWithMips(heightTexture, result);
    return result;
}

RefPtr<Texture> BaseHeightEditTool::CreateNormalTextureCopy()
{
    RefPtr<Texture> sourceNormalMap = system->GetOriginalLandscapeTexture(Landscape::TANGENT_TEXTURE, 0);
    RefPtr<Texture> result;

    Texture::FBODescriptor descr;
    descr.format = sourceNormalMap->GetFormat();
    descr.height = sourceNormalMap->height;
    descr.width = sourceNormalMap->width;
    descr.mipLevelsCount = sourceNormalMap->GetMipLevelsCount();
    descr.needDepth = false;
    descr.needPixelReadback = true;
    descr.textureType = rhi::TEXTURE_TYPE_2D;
    result.Set(Texture::CreateFBO(descr));
    result->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NEAREST);
    result->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    result->maxAnisotropyLevel = 1;

    CopyTextureWithMips(sourceNormalMap, result);
    return result;
}

void BaseHeightEditTool::CreateBasePhases(Vector<BrushPhaseDescriptor>& phases) const
{
    for (uint32 mip = 0; mip < morphTexture->GetMipLevelsCount(); ++mip)
    {
        BrushPhaseDescriptor descr;
        descr.internalId = mip;
        descr.passName = PASS_FORWARD;
        descr.phaseMaterial.Set(new NMaterial());
        descr.phaseMaterial->SetFXName(NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(FastName("R32F_TO_MORPH"), 1);
        descr.phaseMaterial->AddTexture(FastName("texture0"), floatTexture.Get());

        float32 mipTextureSize = floatTexture->width >> mip;
        descr.phaseMaterial->AddProperty(FastName("mipTextureSize"), &mipTextureSize, rhi::ShaderProp::TYPE_FLOAT1, 1);
        descr.renderTarget = morphTexture;
        descr.renderTargetLevel = mip;

        phases.push_back(descr);
    }

    Landscape* landscape = system->GetEditedLandscape();
    Vector4 landscapeParams = Vector4(landscape->GetLandscapeSize(), landscape->GetLandscapeHeight(), 0.0, 0.0);
    for (uint32 mip = 0; mip < normalMap->GetMipLevelsCount(); ++mip)
    {
        BrushPhaseDescriptor descr;
        descr.internalId = mip;
        descr.passName = PASS_FORWARD;
        descr.phaseMaterial.Set(new NMaterial());
        descr.phaseMaterial->SetFXName(NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(FastName("GENERATE_TANGENT_MAP"), 1);
        descr.phaseMaterial->AddTexture(FastName("texture1"), morphTexture.Get());

        float32 floatTextureSize = static_cast<float32>(floatTexture->width);
        descr.phaseMaterial->AddProperty(FastName("floatTextureSize"), &floatTextureSize, rhi::ShaderProp::TYPE_FLOAT1, 1);

        landscapeParams.z = floatTexture->width >> mip;
        descr.phaseMaterial->AddProperty(FastName("landscapeParams"), landscapeParams.data, rhi::ShaderProp::TYPE_FLOAT4, 1);

        descr.renderTarget = normalMap;
        descr.renderTargetLevel = mip;

        phases.push_back(descr);
    }
}

void BaseHeightEditTool::CopyHeightTextureToFloat(RefPtr<Texture> heightTexture, RefPtr<Texture> target)
{
    blitConvertMaterial->AddFlag(FastName("MORPH_TO_R32F"), 1);
    blitConvertMaterial->AddTexture(FastName("texture0"), heightTexture.Get());
    if (blitConvertMaterial->PreBuildMaterial(PASS_FORWARD) == true)
    {
        TextureBlitter::TargetInfo info;
        info.renderTarget = target;
        blitter.BlitTexture(info, blitConvertMaterial, PRIORITY_SERVICE_3D);
    }
    blitConvertMaterial->RemoveTexture(FastName("texture0"));
    blitConvertMaterial->RemoveFlag(FastName("MORPH_TO_R32F"));
}

void BaseHeightEditTool::CopyTextureWithMips(RefPtr<Texture> source, RefPtr<Texture> target)
{
    blitConvertMaterial->AddFlag(FastName("COPY_TEXTURE_LOD"), 1);
    blitConvertMaterial->AddTexture(FastName("texture0"), source.Get());
    float32 lodLevel = 0;
    blitConvertMaterial->AddProperty(FastName("lodLevel"), &lodLevel, rhi::ShaderProp::TYPE_FLOAT1, 1);

    for (uint32 i = 0; i < source->GetMipLevelsCount(); ++i)
    {
        lodLevel = i;
        blitConvertMaterial->SetPropertyValue(FastName("lodLevel"), &lodLevel);
        if (blitConvertMaterial->PreBuildMaterial(PASS_FORWARD))
        {
            TextureBlitter::TargetInfo info;
            info.renderTarget = target;
            info.textureLevel = i;
            blitter.BlitTexture(info, blitConvertMaterial, PRIORITY_SERVICE_3D);
        }
    }
    blitConvertMaterial->RemoveTexture(FastName("texture0"));
    blitConvertMaterial->RemoveFlag(FastName("COPY_TEXTURE_LOD"));
    blitConvertMaterial->RemoveProperty(FastName("lodLevel"));
}

DAVA_VIRTUAL_REFLECTION_IMPL(BaseHeightEditTool)
{
    ReflectionRegistrator<BaseHeightEditTool>::Begin()
    .Field("brush", &BaseHeightEditTool::GetBrushPath, &BaseHeightEditTool::SetBrushPath)
    .Field("size", &BaseHeightEditTool::brushSize)[M::Range(0.0f, 1.0f, 0.0001f)]
    .Field("strength", &BaseHeightEditTool::strength)[M::Range(0.0f, 1.0f, 0.0001f)]
    .Field("rotation", &BaseHeightEditTool::rotation)[M::Range(0.0f, 1.0f, 0.0001f)]
    .Field("brushRadius", &BaseHeightEditTool::GetBrushRadius, &BaseHeightEditTool::SetBrushRadius)[M::Range(0.0f, Any(), 0.5f)]
    .Field("strengthInMeters", &BaseHeightEditTool::GetStrengthInMeters, &BaseHeightEditTool::SetStrengthInMeters)[M::Range(0.0f, Any(), 0.5f)]
    .Field("rotationAngle", &BaseHeightEditTool::GetRotationAngle, &BaseHeightEditTool::SetRotationAngle)[M::Range(0.0f, 360.0f, 0.1f)]
    .End();
}
} // namespace DAVA
