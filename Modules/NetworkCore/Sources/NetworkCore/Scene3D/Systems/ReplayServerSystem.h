#pragma once

#include <Entity/SceneSystem.h>

#include <fstream>

namespace DAVA
{
struct INetworkEventStorage;
class NetworkServerConnectionsSingleComponent;

class ReplayServerSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ReplayServerSystem, SceneSystem);

    ReplayServerSystem(Scene* scene);
    ~ReplayServerSystem() override;

    enum class Mode
    {
        None,
        Record,
        Replay
    };

    void SetMode(const Mode mode);

    void ProcessFixed(float32 timeElapsed) override;

private:
    enum class Type : uint8
    {
        FastName = 0,
        FastNameIndex,
        VecOfFastName,
        ServerRecvPacket,
        VecOfServerRecvPacket,
        ChannelRecvPacket,
        EndFrame
    };

    friend std::ostream& operator<<(std::ostream& stream, const ReplayServerSystem::Type type);
    friend std::istream& operator>>(std::istream& stream, ReplayServerSystem::Type& type);

    void SerializeNetComponent(const NetworkServerConnectionsSingleComponent& component);
    void DeserializeNetComponent(INetworkEventStorage& eventStorage);

    void SeializeFastNameOrFastIndex(const FastName&);
    void DeserializeFastName(FastName&);

    void SerializeVecOfFastName(const Vector<FastName>&);
    void DeserializeVecOfFastName(Vector<FastName>&);

    void SerializeVecU8(const Vector<uint8>&);
    void DeserializeVecU8(Vector<uint8>&);

    UnorderedMap<uint32, FastName> indexToFastName;
    UnorderedMap<FastName, uint32> fastNameToIndex;

    Mode mode;
    const String replayFilePath;
    std::fstream file;
};
} // end namespace DAVA
