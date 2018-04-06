#pragma once

#include "Scene3D/Systems/BaseStreamingSystem.h"

namespace DAVA
{
class FilePath;
class StreamingSystem final : public BaseStreamingSystem
{
public:
    StreamingSystem(Scene* scene);

    void LoadLevel(const FilePath& filepath);

protected:
    void ChunkBecomeVisible(const Level::ChunkCoord& cord) override;
    void ChunkBecomeInvisible(const Level::ChunkCoord& cord) override;
};
} // namespace DAVA
