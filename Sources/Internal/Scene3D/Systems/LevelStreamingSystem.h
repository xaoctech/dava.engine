#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/Observer.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Level.h"

namespace DAVA
{
class LevelStreamingSystem : public SceneSystem
{
public:
    LevelStreamingSystem(Scene* scene);
    virtual ~LevelStreamingSystem();

    enum eStatus
    {
        NO_LEVEL = 0,
        LEVEL_BASE_PART_LOADING = 1,
        LEVEL_STREAMING = 2, // base part loaded.
    };

    void LoadLevel(const FilePath& filepath);
    void UnloadLevel();

    void Process(float32 timeElapsed) override;

    eStatus GetStatus() const;

protected:
    void LoadLevelCompleteCallback(Asset<AssetBase> asset);
    void StreamingCallback();
    eStatus status = NO_LEVEL;
    Asset<Level> level = nullptr;
};

} // ns
