#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MANAGER__
#define __BEAST_MANAGER__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastNames.h"
#include "SceneParser.h"
#include "Beast/BeastProxy.h"

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
    void SetMode(BeastProxy::eBeastMode mode);
    BeastProxy::eBeastMode GetMode() const;
    void SetLodLevel(DAVA_BEAST::int32 lodLevel);
    DAVA_BEAST::int32 GetLodLevel();
    void SetSwitchIndex(DAVA_BEAST::int32 switchIndex);
    DAVA_BEAST::int32 GetSwitchIndex();

    DAVA_BEAST::ILBManagerHandle GetILBManager();
    DAVA_BEAST::ILBSceneHandle GetILBScene();

    void OnJobCompleted();

    DAVA_BEAST::int32 GetCurTaskProcess() const;
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
    DAVA_BEAST::ILBManagerHandle handle = nullptr;
    DAVA_BEAST::ILBSceneHandle scene = nullptr;
    DAVA_BEAST::ILBJobHandle job = nullptr;
    DAVA_BEAST::ILBRenderPassHandle passLM = nullptr;
    DAVA_BEAST::ILBRenderPassHandle passSH = nullptr;

    MaxLodMaxSwitch maxLodMaxSwitch;
    BeastProxy::eBeastMode mode = BeastProxy::eBeastMode::MODE_LIGHTMAPS;
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

#endif //__BEAST_MANAGER__

#endif //__DAVAENGINE_BEAST__
