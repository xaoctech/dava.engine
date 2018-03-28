#include "ReplayServerSystem.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerConnectionsSingleComponent.h"
#include "NetworkCore/UDPTransport/Private/ENetUtils.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Logger/Logger.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
using NetComponent = NetworkServerConnectionsSingleComponent;
using Channels = PacketParams::Channels;

DAVA_VIRTUAL_REFLECTION_IMPL(ReplayServerSystem)
{
    ReflectionRegistrator<ReplayServerSystem>::Begin()[M::Tags("tools")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ReplayServerSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 0.9f)]
    .End();
}

ReplayServerSystem::ReplayServerSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkServerConnectionsSingleComponent>())
    , mode(Mode::None)
    , replayFilePath("replay_file.txt")
{
    auto& params = Engine::Instance()->GetCommandLine();
    if (any_of(begin(params), end(params), [this](const String& s) { return s == "--record"; }))
    {
        mode = Mode::Record;
    }
    else if (any_of(begin(params), end(params), [this](const String& s) { return s == "--replay"; }))
    {
        mode = Mode::Replay;
    }

    switch (mode)
    {
    case Mode::None:
        break;
    case Mode::Record:
        file.open(replayFilePath, std::ios::out | std::ios::binary);
        file.exceptions(std::ios::failbit);
        DVASSERT_ALWAYS(file);
        break;
    case Mode::Replay:
        file.open(replayFilePath, std::ios::in | std::ios::binary);
        file.exceptions(std::ios::failbit | std::ios::eofbit);
        DVASSERT_ALWAYS(file);
        break;
    }
}

ReplayServerSystem::~ReplayServerSystem()
{
    file.flush();
    file.close();
}

void ReplayServerSystem::ProcessFixed(float32 /*timeElapsed*/)
{
    switch (mode)
    {
    case Mode::None:
        // do nothing
        break;
    case Mode::Record:
    {
        const auto* component = GetScene()->GetSingleComponentForRead<NetComponent>(this);
        DVASSERT_ALWAYS(component != nullptr);
        SerializeNetComponent(*component);
    }

    break;
    case Mode::Replay:
        auto* component = GetScene()->GetSingleComponentForWrite<NetComponent>(this);
        DVASSERT_ALWAYS(component != nullptr);
        try
        {
            DeserializeNetComponent(*component);
        }
        catch (std::exception& ex)
        {
            if (file.eof())
            {
                Logger::Info("replay finished");
                mode = Mode::None;
            }
            else
            {
                Logger::Error("error: deserialize failed: %s", ex.what());
            }
        }
        break;
    }
}

void ReplayServerSystem::SerializeNetComponent(const NetworkServerConnectionsSingleComponent& component)
{
    const Vector<FastName>& justConnectedTokens = component.GetJustConnectedTokens();
    SerializeVecOfFastName(justConnectedTokens);

    const Vector<FastName>& justDisconnectedTokens = component.GetJustDisconnectedTokens();
    SerializeVecOfFastName(justDisconnectedTokens);

    // TODO serialize it
    size_t numOfChannels = 0;

    for (Channels channel = Channels::DEFAULT_CHANNEL_ID;
         channel != Channels::CHANNELS_COUNT;
         channel = static_cast<Channels>(static_cast<uint8>(channel) + 1))
    {
        const Vector<NetComponent::ServerRecvPacket>& packets = component.GetRecvPackets(channel);
        if (!packets.empty())
        {
            ++numOfChannels;
        }
    }

    file << Type::ChannelRecvPacket;
    file.write(reinterpret_cast<const char*>(&numOfChannels), 4);

    for (Channels channel = Channels::DEFAULT_CHANNEL_ID;
         channel != Channels::CHANNELS_COUNT;
         channel = static_cast<Channels>(static_cast<uint8>(channel) + 1))
    {
        const Vector<NetComponent::ServerRecvPacket>& packets = component.GetRecvPackets(channel);
        if (!packets.empty())
        {
            file.write(reinterpret_cast<const char*>(&channel), 1);
            uint32 size = static_cast<uint32>(packets.size());
            file.write(reinterpret_cast<const char*>(&size), 4);

            for (auto& packet : packets)
            {
                SeializeFastNameOrFastIndex(packet.token);
                SerializeVecU8(packet.data);
            }
        }
    }

    file << Type::EndFrame; // just debug check
}

void ReplayServerSystem::DeserializeNetComponent(NetworkServerConnectionsSingleComponent& component)
{
    Vector<FastName> justConnectedTokens;
    DeserializeVecOfFastName(justConnectedTokens);

    for (auto& token : justConnectedTokens)
    {
        component.AddConnectedToken(token);
    }

    Vector<FastName> justDisconnectedTokens;
    DeserializeVecOfFastName(justDisconnectedTokens);

    for (auto& token : justDisconnectedTokens)
    {
        component.RemoveConnectedToken(token);
    }

    ReplayServerSystem::Type typeChannels;
    file >> typeChannels;
    DVASSERT_ALWAYS(typeChannels == Type::ChannelRecvPacket);
    uint32 numOfChannels = 0;
    file.read(reinterpret_cast<char*>(&numOfChannels), 4);

    for (; numOfChannels > 0; --numOfChannels)
    {
        uint8 channel = 0;
        file.read(reinterpret_cast<char*>(&channel), 1);
        uint32 packetsCount = 0;
        file.read(reinterpret_cast<char*>(&packetsCount), 4);

        NetComponent::ServerRecvPacket packet;

        for (; packetsCount > 0; --packetsCount)
        {
            DeserializeFastName(packet.token);
            DeserializeVecU8(packet.data);

            component.StoreRecvPacket(channel, packet.token, packet.data.data(), packet.data.size());
        }
    }

    ReplayServerSystem::Type endMarker;
    file >> endMarker;
    DVASSERT_ALWAYS(endMarker == Type::EndFrame);
}

std::ostream& operator<<(std::ostream& stream, const ReplayServerSystem::Type type)
{
    char value = static_cast<char>(type);
    stream.write(&value, 1);
    return stream;
}

std::istream& operator>>(std::istream& stream, ReplayServerSystem::Type& type)
{
    char value = 0;
    stream.read(&value, 1);
    DVASSERT_ALWAYS(value >= 0 && value <= 6);
    type = static_cast<ReplayServerSystem::Type>(value);
    return stream;
}

void ReplayServerSystem::SeializeFastNameOrFastIndex(const FastName& fastName)
{
    auto it = fastNameToIndex.find(fastName);
    if (it == end(fastNameToIndex))
    {
        uint32& fastIndex = fastNameToIndex[fastName];
        fastIndex = static_cast<uint32>(fastNameToIndex.size());
        indexToFastName.emplace(fastIndex, fastName);

        file << Type::FastName;
        uint32 size = static_cast<uint32>(fastName.size());
        file.write(reinterpret_cast<const char*>(&size), 4);
        file.write(fastName.c_str(), fastName.size());
        file.write(reinterpret_cast<const char*>(&fastIndex), 4);
    }
    else
    {
        uint32 fastIndex = it->second;
        file << Type::FastNameIndex;
        file.write(reinterpret_cast<const char*>(&fastIndex), 4);
    }
}

void ReplayServerSystem::DeserializeFastName(FastName& fastNameOut)
{
    ReplayServerSystem::Type typeVal = Type::FastName;
    file >> typeVal;

    if (typeVal == Type::FastName)
    {
        uint32 strLen = 0;
        file.read(reinterpret_cast<char*>(&strLen), 4);
        String str(strLen, '\0');
        file.read(&str[0], str.size());
        uint32 fastIndex = 0;
        file.read(reinterpret_cast<char*>(&fastIndex), 4);
        FastName fastName(str);

        indexToFastName.emplace(fastIndex, fastName);
        fastNameToIndex[fastName] = fastIndex;

        fastNameOut = fastName;
    }
    else if (typeVal == Type::FastNameIndex)
    {
        uint32 fastIndex = 0;
        file.read(reinterpret_cast<char*>(&fastIndex), 4);
        auto it = indexToFastName.find(fastIndex);
        DVASSERT_ALWAYS(it != end(indexToFastName));

        fastNameOut = it->second;
    }
    else
    {
        DVASSERT_ALWAYS(false);
    }
}

void ReplayServerSystem::SerializeVecOfFastName(const Vector<FastName>& vec)
{
    file << Type::VecOfFastName;
    uint32 v = static_cast<uint32>(vec.size());
    file.write(reinterpret_cast<const char*>(&v), 4);

    std::for_each(begin(vec), end(vec), [this](const FastName& n) { SeializeFastNameOrFastIndex(n); });
}

void ReplayServerSystem::DeserializeVecOfFastName(Vector<FastName>& vec)
{
    ReplayServerSystem::Type type;
    file >> type;
    DVASSERT_ALWAYS(type == ReplayServerSystem::Type::VecOfFastName);
    uint32 size = 0;
    file.read(reinterpret_cast<char*>(&size), 4);
    vec.resize(size);

    std::for_each(begin(vec), end(vec), [this](FastName& n) { DeserializeFastName(n); });
}

void ReplayServerSystem::SerializeVecU8(const Vector<uint8>& vec)
{
    uint32 size = static_cast<uint32>(vec.size());
    file.write(reinterpret_cast<const char*>(&size), 4);

    file.write(reinterpret_cast<const char*>(vec.data()), vec.size());
}

void ReplayServerSystem::DeserializeVecU8(Vector<uint8>& vec)
{
    uint32 size = 0;
    file.read(reinterpret_cast<char*>(&size), 4);
    vec.resize(size);
    file.read(reinterpret_cast<char*>(vec.data()), vec.size());
}

} // end namespace DAVA
