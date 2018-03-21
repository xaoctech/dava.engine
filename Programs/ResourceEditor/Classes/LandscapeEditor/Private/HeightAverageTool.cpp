#include "Classes/LandscapeEditor/Private/HeightAverageTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/Slider.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Base/GlobalEnum.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>

ENUM_DECLARE(HeightAverageTool::eBrushMode)
{
    ENUM_ADD_DESCR(HeightAverageTool::MODE_CONTINUES, "Continues");
    ENUM_ADD_DESCR(HeightAverageTool::MODE_SINGLE, "Single");
}

namespace HeightAverageToolDetail
{
using namespace DAVA;

const float32 coneCircleRadius = 0.1f;
const float32 coneZOffset = 0.2f;
const float32 cilinderRadius = 0.05f;
const float32 planeCircleRadius = 1.0f;
const float32 vertexPerCircle = 32.0f;

struct Vertex
{
    Vertex(const Vector3& pos_, const Vector3& normal_)
        : pos(pos_)
        , normal(normal_)
    {
    }

    Vector3 pos;
    Vector3 normal;
};

void GenerateArc(float32 currentCircleRadius, const Vector3& center, const Function<void(const Vector3&, float32)>& fn)
{
    const float32 deltaRad = PI_2;

    const Vector3 dir(0.0, 0.0, 1.0);
    const Vector3 ortho(1.0, 0.0, 0.0);

    Matrix4 rotationMx;
    float32 angleDelta = deltaRad / vertexPerCircle;
    float32 currentAngle = 0.0;
    while (currentAngle < PI_2 + 0.01)
    {
        rotationMx.BuildRotation(dir, currentAngle);
        fn(center + ((ortho * currentCircleRadius) * rotationMx), currentAngle);
        currentAngle += angleDelta;
    }
}

void GenerateArrowHead(Vector<Vertex>& vertices, Vector<uint16>& indices)
{
    // +1 vertex is head of arrow and +1 circle center
    vertices.reserve(vertices.size() + vertexPerCircle + 2);
    // (pointsPerCircle - 1) = count of triangles on cone + pointsPerCircle on circle. 3 indexes per triangle
    indices.reserve(vertices.size() + 3 * 2 * vertexPerCircle);

    const Vector3 center(0.0f, 0.0f, 0.2f);
    uint16 centerIndex = static_cast<uint16>(vertices.size());
    vertices.push_back(Vertex(center, Vector3(0.0, 0.0, 1.0)));

    uint16 headIndex = static_cast<uint16>(vertices.size());
    vertices.push_back(Vertex(Vector3(center.x, center.y, center.z + coneZOffset), Vector3(0.0, 0.0, 1.0)));

    uint16 startVertex = static_cast<uint16>(vertices.size());

    GenerateArc(coneCircleRadius, center, [&](const Vector3& v, float32 angle) {
        uint16 vertexIndex = static_cast<uint16>(vertices.size());
        vertices.push_back(Vertex(v, Normalize(v - center)));

        if (angle > 0.0)
        {
            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);
            indices.push_back(centerIndex);

            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);
            indices.push_back(headIndex);
        }
    });
}

void GeneratePlane(Vector<Vertex>& vertices, Vector<uint16>& indices)
{
    vertices.reserve(vertices.size() + vertexPerCircle + 1);
    indices.reserve(vertices.size() + 3 * vertexPerCircle);

    uint16 cenrexIndex = static_cast<uint16>(vertices.size());
    const Vector3 center(0.0, 0.0, 0.0);
    const Vector3 normal(0.0, 0.0, 1.0);
    vertices.push_back(Vertex(center, normal));
    uint16 startVertex = static_cast<uint16>(vertices.size());

    GenerateArc(planeCircleRadius, center, [&](const Vector3& v, float32 angle) {
        uint16 vertexIndex = static_cast<uint16>(vertices.size());
        vertices.push_back(Vertex(v, normal));

        if (angle > 0.0)
        {
            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);
            indices.push_back(cenrexIndex);
        }
    });
}

void GenerateCilinder(Vector<Vertex>& vertices, Vector<uint16>& indices, float32 r, float32 h)
{
    vertices.reserve(vertices.size() + 2 * vertexPerCircle);
    indices.reserve(vertices.size() + 3 * 2 * vertexPerCircle);

    const Vector3 center(0.0, 0.0, 0.0);
    uint16 startVertex = static_cast<uint16>(vertices.size());

    GenerateArc(r, center, [&](const Vector3& v, float32 angle) {
        uint16 vertexIndex = static_cast<uint16>(vertices.size());
        Vector3 normal = Normalize(v - center);
        vertices.push_back(Vertex(v, normal));

        Vector3 upV = v + Vector3(0.0, 0.0, h);
        vertices.push_back(Vertex(upV, normal));

        if (angle > 0.0)
        {
            indices.push_back(vertexIndex - 2);
            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);

            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);
            indices.push_back(vertexIndex + 1);
        }
    });
}
} // namespace HeightAverageToolDetail

HeightAverageTool::HeightAverageTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_average.png"), QString("Average height")),
                         "HeightAverageTool")
{
    using namespace DAVA;

    inputController->RegisterVarCallback(DAVA::eInputElements::KB_K, [this](const DAVA::Vector2& delta) {
        kernelSize = DAVA::Saturate(kernelSize + delta.y * 0.1);
    });

    inputController->RegisterVarCallback(DAVA::eInputElements::KB_I, [this](const DAVA::Vector2& delta) {
        kernelStrength = DAVA::Saturate(kernelStrength + delta.y * 0.1);
    });

    rhi::VertexLayout vertexLayout;
    vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vertexLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
    uint32 layoutID = rhi::VertexLayout::UniqueId(vertexLayout);

    normalDebugDrawGeometry.Set(new RenderObject());

    ScopedPtr<RenderBatch> batch(new RenderBatch());
    {
        normalDebugDrawMaterial.Set(new NMaterial());
        normalDebugDrawMaterial->SetMaterialName(FastName("LandscapeNormal"));
        normalDebugDrawMaterial->SetFXName(FastName("~res:/Materials2/LandscapeNormal.material"));

        DAVA::Vector4 params(0.0, 0.0, 0.0, 0.0);
        normalDebugDrawMaterial->AddProperty(FastName("landscapeUV"), params.data, rhi::ShaderProp::TYPE_FLOAT4);
        normalDebugDrawMaterial->AddProperty(FastName("calcNormalUV"), params.data, rhi::ShaderProp::TYPE_FLOAT4);
        normalDebugDrawMaterial->AddProperty(FastName("landscapeParams"), params.data, rhi::ShaderProp::TYPE_FLOAT4);
        normalDebugDrawMaterial->AddProperty(FastName("kernel"), params.data, rhi::ShaderProp::TYPE_FLOAT2);
        normalDebugDrawMaterial->AddProperty(FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4);

        batch->SetMaterial(normalDebugDrawMaterial.Get());
    }
    {
        using namespace HeightAverageToolDetail;
        Vector<Vertex> vertices;
        Vector<uint16> indices;
        GenerateArrowHead(vertices, indices);
        GenerateCilinder(vertices, indices, cilinderRadius, coneZOffset);
        GeneratePlane(vertices, indices);
        GenerateCilinder(vertices, indices, planeCircleRadius, 0.01f);

        RefPtr<PolygonGroup> polygonGroup(new PolygonGroup());
        polygonGroup->SetPrimitiveType(rhi::PRIMITIVE_TRIANGLELIST);
        polygonGroup->AllocateData((eVertexFormat::EVF_VERTEX | eVertexFormat::EVF_NORMAL), static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), static_cast<uint16>(indices.size()) / 3);
        memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vertex));
        memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(uint16));
        polygonGroup->BuildBuffers();
        polygonGroup->RecalcAABBox();
        batch->SetPolygonGroup(polygonGroup.Get());
    }

    batch->vertexLayoutId = layoutID;

    normalDebugDrawGeometry->AddRenderBatch(batch);
    normalDebugDrawGeometry->SetWorldTransformPtr(&DAVA::Matrix4::IDENTITY);
    normalDebugDrawGeometry->AddFlag(DAVA::RenderObject::ALWAYS_CLIPPING_VISIBLE);
}

void HeightAverageTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    mode = toolSettings.Get<eBrushMode>("mode", mode);
    kernelSize = toolSettings.Get<DAVA::float32>("kernelSize", kernelSize);
    kernelStrength = toolSettings.Get<DAVA::float32>("kernelSize", kernelStrength);

    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) > 0)
    {
        DAVA::Asset<DAVA::Texture> heightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);
        normalDebugDrawMaterial->AddTexture(DAVA::FastName("landscapeSourceHeightMap"), heightTexture);
        system->AddDebugDraw(normalDebugDrawGeometry.Get());
    }
}

void HeightAverageTool::Process(DAVA::float32 delta)
{
    if (morphTexture == nullptr)
    {
        return;
    }

    DAVA::Vector4 landUV = inputController->GetCurrentCursorUV();
    DAVA::Vector4 calcUV = landUV;
    if (mode == MODE_SINGLE && inputController->IsInOperation() == true)
    {
        calcUV = inputController->GetBeginOperationCursorUV();
    }

    landUV.y = 1.0 - landUV.y;
    calcUV.y = 1.0 - calcUV.y;

    DAVA::Vector4 landscapeParams = GetLandscapeParams();
    DAVA::Vector2 kernelParams = GetKernel();
    DAVA::Vector4 params = GetParams();
    normalDebugDrawMaterial->SetPropertyValue(DAVA::FastName("landscapeUV"), landUV.data);
    normalDebugDrawMaterial->SetPropertyValue(DAVA::FastName("calcNormalUV"), calcUV.data);
    normalDebugDrawMaterial->SetPropertyValue(DAVA::FastName("landscapeParams"), landscapeParams.data);
    normalDebugDrawMaterial->SetPropertyValue(DAVA::FastName("kernel"), kernelParams.data);
    normalDebugDrawMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
}

QWidget* HeightAverageTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    {
        ControlDescriptorBuilder<ComboBox::Fields> fields;
        fields[ComboBox::Fields::Value] = "mode";
        builder.RegisterParam<ComboBox>("Mode", fields);
    }
    CreateRotationSlider(builder);
    CreateBrushSizeControl(builder);
    {
        ControlDescriptorBuilder<Slider::Fields> fields;
        fields[Slider::Fields::Value] = "kernelSize";
        builder.RegisterParam<Slider>("Kernel size (K)", fields);
    }
    {
        ControlDescriptorBuilder<Slider::Fields> fields;
        fields[Slider::Fields::Value] = "kernelStrength";
        builder.RegisterParam<Slider>("Kernel Strength (I)", fields);
    }
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightAverageTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::Asset<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);

        DAVA::Vector4 params = DAVA::Vector4(0.0, 0.0, 0.0, 0.0);

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHT_AVERAGE"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture1"), srcHeightTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("uvPos"), params.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
        descr.phaseMaterial->AddProperty(DAVA::FastName("kernel"), params.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
        descr.phaseMaterial->AddProperty(DAVA::FastName("landscapeParams"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightAverageTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector2 kernel = GetKernel();
        DAVA::Vector2 uvPos = GetUVPos();
        DAVA::Vector4 landscapeParams = GetLandscapeParams();
        DAVA::Vector4 params = GetParams();

        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("uvPos"), uvPos.data);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("kernel"), kernel.data);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("landscapeParams"), landscapeParams.data);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

void HeightAverageTool::Deactivate(DAVA::PropertiesItem& settings)
{
    DAVA::FastName heightTextureSlot("landscapeSourceHeightMap");
    if (normalDebugDrawMaterial->HasLocalTexture(heightTextureSlot))
    {
        normalDebugDrawMaterial->RemoveTexture(heightTextureSlot);
        system->RemoveDebugDraw(normalDebugDrawGeometry.Get());
    }
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("mode", mode);
    toolSettings.Set("kernelSize", kernelSize);
    toolSettings.Set("kernelSize", kernelStrength);
}

DAVA::Vector2 HeightAverageTool::GetUVPos() const
{
    DAVA::Vector4 uv = inputController->GetBeginOperationCursorUV();
    if (mode == MODE_CONTINUES)
    {
        uv = inputController->GetCurrentCursorUV();
    }

    return DAVA::Vector2(uv.x, 1.0f - uv.y);
}

DAVA::Vector2 HeightAverageTool::GetKernel() const
{
    return DAVA::Vector2(kernelSize, kernelStrength);
}

DAVA::Vector4 HeightAverageTool::GetLandscapeParams() const
{
    DAVA::Vector4 result;
    DAVA::Landscape* landscape = system->GetEditedLandscape();
    result.x = landscape->GetLandscapeSize();
    result.y = landscape->GetLandscapeHeight();
    result.z = morphTexture->width;

    return result;
}

DAVA::Vector4 HeightAverageTool::GetParams() const
{
    DAVA::Vector4 params(0.0, 0.0, 0.0, 0.0);
    params.x = strength;
    params.y = brushSize * 0.5;

    return params;
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightAverageTool)
{
    DAVA::ReflectionRegistrator<HeightAverageTool>::Begin()[BaseLandscapeTool::SortKey(40)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("mode", &HeightAverageTool::mode)[DAVA::M::EnumT<HeightAverageTool::eBrushMode>()]
    .Field("kernelSize", &HeightAverageTool::kernelSize)[DAVA::M::Range(0.0f, 1.0f, 0.001f)]
    .Field("kernelStrength", &HeightAverageTool::kernelStrength)[DAVA::M::Range(0.0f, 1.0f, 0.001f)]
    .End();
}
