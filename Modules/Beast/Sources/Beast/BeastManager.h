#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastNames.h"
#include "Beast/BeastConstants.h"
#include "Beast/SceneParser.h"

namespace DAVA
{
class Scene;
class RenderBatch;
}

struct LightmapAtlasingData;

class BeastMaterial;
class BeastManager
{
public:
    DECLARE_BEAST_NAME(BeastManager);

    BeastManager();
    virtual ~BeastManager();

    void Run(DAVA::Scene* davaScene);

    void Update();
    bool IsJobDone();
    void UpdateAtlas(DAVA::Vector<LightmapAtlasingData>* atlasData);

    void SetLightmapsDirectory(const DAVA::String& path);
    void ParseScene(DAVA::Scene* davaScene);
    bool GenerateLightmaps();
    void SetMode(eBeastMode mode);
    eBeastMode GetMode() const;
    void SetLodLevel(int32 lodLevel);
    int32 GetLodLevel();
    void SetSwitchIndex(int32 switchIndex);
    int32 GetSwitchIndex();

    ILBManagerHandle GetILBManager();
    ILBSceneHandle GetILBScene();

    void OnJobCompleted();

    int32 GetCurTaskProcess() const;
    const DAVA::String& GetCurTaskName() const;

    void Cancel();
    bool WasCancelled() const;

private:
    static bool shouldInit;
    static void StaticInit();

    void BeginScene();
    void EndScene();
    void RetriveProgress();
    void SaveResults();
    void ClearLightmapsDirectory();
    void StartLodPass();
    void EndLodPass();
    void RunLodIteration();
    bool IsLightmapGenerated(DAVA::RenderBatch* batch);
    void SetLightmapGenerated(DAVA::RenderBatch* batch);

    void OnSceneParsingCompleted();

    enum eState : DAVA::uint32
    {
        STATE_IDLE = 0,
        STATE_GENERATING,
        STATE_DONE,
        STATE_CANCELED
    };

private:
    ILBManagerHandle handle = nullptr;
    ILBSceneHandle scene = nullptr;
    ILBJobHandle job = nullptr;
    ILBRenderPassHandle passLM = nullptr;
    ILBRenderPassHandle passSH = nullptr;

    MaxLodMaxSwitch maxLodMaxSwitch;
    eBeastMode mode = eBeastMode::MODE_LIGHTMAPS;
    SceneParser* sceneParser = nullptr;

    eState state = STATE_IDLE;

    DAVA::Scene* davaScene = nullptr;
    DAVA::Map<DAVA::RenderBatch*, bool> generatedLightmaps;
    DAVA::String curTaskName;
    DAVA::String lightmapsDirectory;
    DAVA::int32 lodLevel = 0;
    DAVA::int32 switchIndex = 0;
    DAVA::int32 curTaskProgress = 0;
};
