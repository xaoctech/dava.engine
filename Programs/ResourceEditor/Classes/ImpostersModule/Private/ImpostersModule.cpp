#include "Classes/ImpostersModule/ImpostersModule.h"
#include "Classes/Qt/Tools/PathDescriptor/PathDescriptor.h"

#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/DataNodes/SelectionData.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/Systems/EditorMaterialSystem.h>
#include <REPlatform/Scene/Systems/VisibilityCheckSystem.h>
#include <REPlatform/Scene/Utils/RETextureDescriptorUtils.h>

#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

#include <FileSystem/FileSystem.h>
#include <Render/RhiUtils.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Render/3D/MeshUtils.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Image/ImageSystem.h>
#include <Scene3D/Lod/LodComponent.h>
#include <Scene3D/Systems/ReflectionSystem.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Debug/MessageBox.h>
#include <Time/DateTime.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ImpostersModule)
{
    DAVA::ReflectionRegistrator<ImpostersModule>::Begin()
    .ConstructorByPointer()
    .End();
}

#define IMPOSTER_FRAME_COUNT 16

void ImpostersModule::PostInit()
{
    using namespace DAVA;

    QtAction* bakeAction = new QtAction(GetAccessor(), QIcon(":/QtIcons/sphere.png"), QString("Create Imposter LOD..."));

    FieldDescriptor selectionField;
    selectionField.type = ReflectedTypeDB::Get<SelectionData>();
    selectionField.fieldName = FastName(SelectionData::selectionPropertyName);

    FieldDescriptor fieldDescriptor(ReflectedTypeDB::Get<ProjectManagerData>(), FastName(ProjectManagerData::ProjectPathProperty));
    bakeAction->SetStateUpdationFunction(QtAction::Enabled, selectionField, [](const Any& fieldValue) -> Any {

        if (!fieldValue.CanGet<SelectableGroup>())
            return false;

        const SelectableGroup& selection = fieldValue.Get<SelectableGroup>();
        if (selection.GetSize() != 1)
            return false;

        Entity* e = selection.GetContent().front().AsEntity();
        if (e == nullptr)
            return false;

        RenderObject* ro = GetRenderObject(e);
        if (ro == nullptr)
            return false;

        return true;
    });

    ActionPlacementInfo menuPlacement(CreateMenuPoint("Scene", InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "actionEnableCameraLight")));
    GetUI()->AddAction(mainWindowKey, menuPlacement, bakeAction);

    connections.AddConnection(bakeAction, &QAction::triggered, MakeFunction(this, &ImpostersModule::Bake));
}

void ImpostersModule::Bake()
{
    using namespace DAVA;

    SelectionData* selectionData = GetAccessor()->GetActiveContext()->GetData<SelectionData>();
    if (selectionData == nullptr)
        return;

    const SelectableGroup& selection = selectionData->GetSelection();
    if (selection.GetSize() != 1)
        return;

    Entity* e = selection.GetContent().front().AsEntity();
    if (e == nullptr)
        return;

    RenderObject* ro = GetRenderObject(e);
    if (ro == nullptr)
        return;

    SceneData* sceneData = GetAccessor()->GetActiveContext()->GetData<SceneData>();
    FileDialogParams params;
    params.dir = QString::fromStdString(sceneData->GetScenePath().GetDirectory().GetAbsolutePathname());
    params.filters = PathDescriptor::GetPathDescriptor(PathDescriptor::PATH_IMAGE).fileFilter;
    params.title = "Imposter texture";

    String destination = GetUI()->GetSaveFileName(mainWindowKey, params).toStdString();
    // Logger::Info("Baking imposter for %s to %s", e->GetName().c_str(), destination.c_str());

    BakeRenderObject(e, ro, destination);
}

void ImpostersModule::BakeRenderObject(DAVA::Entity* sourceEntity, DAVA::RenderObject* sourceObject, const DAVA::String& baseFileName)
{
    using namespace DAVA;

    const int32 baseFrameSize = 128; // better size deduction ?
    const int32 superSamplingScale = 4;

    FilePath albedoPath = baseFileName;
    albedoPath.ReplaceBasename(albedoPath.GetBasename() + "_am");

    FilePath normalPath = baseFileName;
    normalPath.ReplaceBasename(normalPath.GetBasename() + "_nm");

#define IMPOSTER_MODULE_CREATE_DEBUG_OBJECT 0

#if (IMPOSTER_MODULE_CREATE_DEBUG_OBJECT)
    ScopedPtr<Entity> newEntity(sourceEntity->Clone());
    {
        Matrix4 m = newEntity->GetLocalTransform();
        m.SetTranslationVector(m.GetTranslationVector() + Vector3(10.0f, 0.0f, 0.0f));
        newEntity->SetLocalTransform(m);
        newEntity->SetName(DAVA::Format("%s Imposter Debug", newEntity->GetName().c_str()).c_str());
    }
    sourceEntity->GetParent()->AddNode(newEntity);
    RenderObject* clonedObject = GetRenderObject(newEntity);
#endif

    int32 maxLodIndex = -1;
    for (uint32 i = 0; i < sourceObject->GetRenderBatchCount(); ++i)
    {
        int32 lodIndex = -1;
        int32 switchIndex = -1;
        sourceObject->GetRenderBatch(i, lodIndex, switchIndex);
        maxLodIndex = std::max(maxLodIndex, lodIndex);
    }

    if (maxLodIndex == 3)
    {
        GetUI()->ShowMessage(DAVA::mainWindowKey, "Unable to add LOD ");
        return;
    }

    bool createLods = (maxLodIndex == -1);

    Vector<RenderBatch*> sourceBatches;
    for (uint32 i = 0; i < sourceObject->GetRenderBatchCount(); ++i)
    {
        int32 lodIndex = -1;
        int32 switchIndex = -1;
        RenderBatch* batch = sourceObject->GetRenderBatch(i, lodIndex, switchIndex);
        if (lodIndex == maxLodIndex)
            sourceBatches.emplace_back(SafeRetain(batch));
    }

    AABBox3 sourceBBox;
    for (RenderBatch* batch : sourceBatches)
        sourceBBox.AddAABBox(batch->GetBoundingBox());

    Vector3 center = sourceBBox.GetCenter();
    Vector3 size = sourceBBox.GetSize() * 0.5f;
    size.x = std::max(size.x, size.y);

    ScopedPtr<PolygonGroup> planeGeometry(new PolygonGroup);
    {
        Vector3 dx = Vector3(std::numeric_limits<float>::epsilon(), 0.0f, 0.0f);
        Vector3 dy = Vector3(0.0f, std::numeric_limits<float>::epsilon(), 0.0f);
        planeGeometry->AllocateData(EVF_VERTEX | EVF_PIVOT4, 4, 6);
        planeGeometry->SetCoord(0, center - dx - dy);
        planeGeometry->SetCoord(1, center + dx - dy);
        planeGeometry->SetCoord(2, center - dx + dy);
        planeGeometry->SetCoord(3, center + dx + dy);
        planeGeometry->SetPivot(0, Vector4(-size.x, -size.z, 0.0, 1.0));
        planeGeometry->SetPivot(1, Vector4(+size.x, -size.z, 1.0, 1.0));
        planeGeometry->SetPivot(2, Vector4(-size.x, +size.z, 0.0, 0.0));
        planeGeometry->SetPivot(3, Vector4(+size.x, +size.z, 1.0, 0.0));
        planeGeometry->SetIndex(0, 0);
        planeGeometry->SetIndex(1, 1);
        planeGeometry->SetIndex(2, 2);
        planeGeometry->SetIndex(3, 1);
        planeGeometry->SetIndex(4, 3);
        planeGeometry->SetIndex(5, 2);
        planeGeometry->BuildBuffers();
        planeGeometry->RecalcAABBox();
    }

    ScopedPtr<NMaterial> material(new NMaterial);
    {
        material->SetMaterialName(FastName("Imposter material"));
        material->SetFXName(FastName("~res:/Materials2/Imposter.material"));
    }

    ScopedPtr<NMaterial> dilationMaterial(new NMaterial);
    {
        static Vector2 dummy;
        dilationMaterial->AddProperty(FastName("srcTexSize"), dummy.data, rhi::ShaderProp::TYPE_FLOAT2);
        dilationMaterial->SetMaterialName(FastName("Dilation material"));
        dilationMaterial->SetFXName(FastName("~res:/Materials2/Dilation.material"));
        dilationMaterial->PreBuildMaterial(PASS_FORWARD);
    }

    ScopedPtr<RenderBatch> batch(new RenderBatch);
    {
        batch->SetPolygonGroup(planeGeometry);
        batch->SetMaterial(material);
    }

    float cameraAspect = size.x / size.z;
    float cameraDistance = 1.0f + 2.0f * std::max(size.x, size.z);
    float randomDistanceScale = 100.0f * std::sqrt(2.0f);

    ScopedPtr<Camera> camera(new Camera);
    camera->SetTarget(center);
    camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
    camera->SetupOrtho(2.0f * size.x, cameraAspect, 0.0f, 2.0f * randomDistanceScale * cameraDistance);

    float cameraInvAspect = std::max(1.0f, std::floor(1.0f / cameraAspect));

    Size2i frameSize = Size2i(baseFrameSize, baseFrameSize * std::min(4, int32(std::exp2(cameraInvAspect - 1.0f))));

    ScopedPtr<Texture> maskTexture(Texture::CreateFBO(superSamplingScale * frameSize.dx * IMPOSTER_FRAME_COUNT, superSamplingScale * frameSize.dy, PixelFormat::FORMAT_RGBA8888, true));
    ScopedPtr<Texture> buffer0Texture(Texture::CreateFBO(superSamplingScale * frameSize.dx * IMPOSTER_FRAME_COUNT, superSamplingScale * frameSize.dy, PixelFormat::FORMAT_RGBA8888, false));
    ScopedPtr<Texture> buffer1Texture(Texture::CreateFBO(superSamplingScale * frameSize.dx * IMPOSTER_FRAME_COUNT, superSamplingScale * frameSize.dy, PixelFormat::FORMAT_RGBA8888, false));

    ScopedPtr<Texture> albedoTexture(Texture::CreateFBO(frameSize.dx * IMPOSTER_FRAME_COUNT, frameSize.dy, PixelFormat::FORMAT_RGBA8888, false));
    ScopedPtr<Texture> normalTexture(Texture::CreateFBO(frameSize.dx * IMPOSTER_FRAME_COUNT, frameSize.dy, PixelFormat::FORMAT_RGBA8888, false));
    buffer0Texture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    buffer1Texture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    rhi::HSyncObject sync = rhi::CreateSyncObject();

    Renderer::RegisterSyncCallback(sync, [albedoPath, normalPath, albedoTexture, normalTexture, material](rhi::HSyncObject sync) mutable {
        {
            ScopedPtr<Image> image(albedoTexture->CreateImageFromMemory());
            ImageSystem::Save(albedoPath, image);
            RETextureDescriptorUtils::CreateOrUpdateDescriptor(albedoPath);
        }
        material->AddTexture(FastName("albedo"), ScopedPtr<Texture>(Texture::CreateFromFile(albedoPath)));
        {
            ScopedPtr<Image> image(normalTexture->CreateImageFromMemory());
            ImageSystem::Save(normalPath, image);
            RETextureDescriptorUtils::CreateOrUpdateDescriptor(normalPath);
        }
        material->AddTexture(FastName("normalmap"), ScopedPtr<Texture>(Texture::CreateFromFile(normalPath)));
        
        material.reset(nullptr);
        albedoTexture.reset(nullptr);
        normalTexture.reset(nullptr);
    });

    rhi::RenderPassConfig passConfig;
    passConfig.name = "ImposterPass";
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.texture = maskTexture->handleDepthStencil;
    passConfig.colorBuffer[0].texture = maskTexture->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 0.0f;
    passConfig.colorBuffer[1].texture = buffer0Texture->handle;
    passConfig.colorBuffer[1].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[2].texture = buffer1Texture->handle;
    passConfig.colorBuffer[3].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.viewport = rhi::Viewport(0, 0, superSamplingScale * frameSize.dx, superSamplingScale * frameSize.dy);
    passConfig.priority = PRIORITY_MAIN_3D + 5;

    rhi::HPacketList packetList;
    rhi::HRenderPass renderPass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    rhi::BeginRenderPass(renderPass);
    rhi::BeginPacketList(packetList);

    const Matrix4* currentWorldTransform = sourceObject->GetWorldTransformPtr();
    sourceObject->SetWorldTransformPtr(&Matrix4::IDENTITY);

    bool inverseProjection = rhi::IsInvertedProjectionRequired(true, false);
    auto reversedCullMode = [](rhi::CullMode cm) {
        switch (cm)
        {
        case rhi::CULL_CW:
            return rhi::CULL_CCW;
        case rhi::CULL_CCW:
            return rhi::CULL_CW;
        default:
            return rhi::CULL_NONE;
        }
    };

    for (uint32 frame = 0; frame < IMPOSTER_FRAME_COUNT; ++frame)
    {
        float phi = 2.0f * PI * float(frame) / float(IMPOSTER_FRAME_COUNT);
        camera->SetPosition(center - randomDistanceScale * cameraDistance * Vector3(std::cos(phi), std::sin(phi), 0.0f));
        camera->SetupDynamicParameters(inverseProjection, false);
        sourceObject->PrepareToRender(camera);

        for (RenderBatch* batch : sourceBatches)
        {
            sourceObject->BindDynamicParameters(camera, batch);
            if (batch->GetMaterial()->PreBuildMaterial(FastName("ImposterPass")))
            {
                rhi::Packet packet;
                batch->GetMaterial()->BindParams(packet);
                batch->BindGeometryData(packet);
                packet.options |= rhi::Packet::OPT_OVERRIDE_VIEWPORT;
                packet.viewportRect = rhi::Viewport(superSamplingScale * frame * frameSize.dx, 0, superSamplingScale * frameSize.dx, superSamplingScale * frameSize.dy);
                packet.cullMode = inverseProjection ? reversedCullMode(packet.cullMode) : packet.cullMode;
                rhi::AddPacket(packetList, packet);
            }
        }
    }
    sourceObject->SetWorldTransformPtr(currentWorldTransform);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(renderPass);

    {
        QuadRenderer quadRenderer;
        QuadRenderer::Options options;
        options.material = dilationMaterial;
        options.dstTextures[0] = albedoTexture->handle;
        options.dstLoadActions[0] = rhi::LoadAction::LOADACTION_CLEAR;
        options.dstTextures[1] = normalTexture->handle;
        options.dstLoadActions[1] = rhi::LoadAction::LOADACTION_CLEAR;
        options.dstTexSize = Vector2(albedoTexture->width, albedoTexture->height);
        options.dstRect = Rect2f(0.0f, 0.0f, albedoTexture->width, albedoTexture->height);
        options.srcTexture = buffer0Texture->handle;
        options.srcTexSize = Vector2(buffer0Texture->width, buffer0Texture->height);
        options.srcRect = Rect2f(0.0f, 0.0f, buffer0Texture->width, buffer0Texture->height);
        options.textureSet = RhiUtils::FragmentTextureSet({ maskTexture->handle, buffer0Texture->handle, buffer1Texture->handle });
        options.renderPassPriority = 4;
        options.renderPassName = "Imposter filtering";
        options.syncObject = sync;
        quadRenderer.Render(options);
    }

#if (IMPOSTER_MODULE_CREATE_DEBUG_OBJECT)

    while (clonedObject->GetRenderBatchCount() > 0)
        clonedObject->RemoveRenderBatch(0u);
    clonedObject->AddRenderBatch(batch, -1, -1);
    
#else

    if (createLods)
    {
        while (sourceObject->GetRenderBatchCount() > 0)
            sourceObject->RemoveRenderBatch(0u);

        for (RenderBatch* batch : sourceBatches)
            sourceObject->AddRenderBatch(batch, 0, -1);

        if (sourceEntity->GetComponent<LodComponent>() == nullptr)
            sourceEntity->AddComponent(new LodComponent());

        maxLodIndex = 0;
    }
    sourceObject->AddRenderBatch(batch, maxLodIndex + 1, -1);
    
#endif

    for (RenderBatch*& b : sourceBatches)
        SafeRelease(b);
}

DECL_TARC_MODULE(ImpostersModule);
