#include "LDRForwardPass.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
LDRForwardPass::LDRForwardPass()
    : ForwardPass(PASS_FORWARD_LDR)
{
}

void LDRForwardPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::RENDER_PASS_MAIN_3D);

    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);

    if (BeginRenderPass(passConfig))
    {
        PrepareVisibilityArrays(mainCamera, renderSystem);
        DrawLayers(mainCamera, drawLayersMask);
        EndRenderPass();
    }
}

LDRForwardPass::~LDRForwardPass()
{
}
}