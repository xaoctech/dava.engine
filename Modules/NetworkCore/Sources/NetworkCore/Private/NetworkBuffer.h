#pragma once

#include <memory>

#include "Base/BaseTypes.h"
#include "Base/List.h"
#include "Base/Vector.h"
#include "Base/ScopedPtr.h"
#include "Logger/Logger.h"

namespace DAVA
{
template <typename T>
class NetworkBuffer
{
public:
    struct PopResult
    {
        uint32 frameId;
        std::shared_ptr<T> value;
    };

    NetworkBuffer(uint32 minSize, uint32 frameId);
    void Update(const Vector<T>& data, uint32 frameId_);
    PopResult Front() const;
    PopResult Pop();
    void TrimFront();
    uint32 GetSize() const;
    void SetMinSize(uint32 minSize_);
    uint32 GetMinSize() const;
    uint32 GetMaxFrameId() const;
    void Clear();

private:
    uint32 minSize;
    uint32 maxFrameId;
    Deque<std::shared_ptr<T>> buffer;
    std::shared_ptr<T> lastValue;
    int32 misses;

    uint32 GetMinFrameId() const;
    void UpdateRange(const Vector<T>& data, uint32 rangeSize);
};

template <typename T>
NetworkBuffer<T>::NetworkBuffer(uint32 minSize, uint32 frameId)
    : minSize(minSize)
    , maxFrameId(frameId)
    , misses(0)
{
}

template <typename T>
void NetworkBuffer<T>::Update(const Vector<T>& data, uint32 frameId_)
{
    if (frameId_ <= maxFrameId)
    {
        return;
    }

    uint32 diff = frameId_ - maxFrameId;
    buffer.resize(buffer.size() + diff, nullptr);
    uint32 rangeSize = static_cast<uint32>(buffer.size());
    if (buffer.size() > data.size())
    {
        rangeSize = static_cast<uint32>(data.size());
    }
    UpdateRange(data, rangeSize);
    maxFrameId = frameId_;
}

template <typename T>
typename NetworkBuffer<T>::PopResult NetworkBuffer<T>::Front() const
{
    uint32 minFrameId = GetMinFrameId();
    if (buffer.empty())
    {
        return PopResult{ 0, lastValue };
    }
    std::shared_ptr<T> value = buffer.front();
    return PopResult{ minFrameId, value };
}

template <typename T>
typename NetworkBuffer<T>::PopResult NetworkBuffer<T>::Pop()
{
    uint32 minFrameId = GetMinFrameId();
    if (buffer.empty())
    {
        ++misses;
        if (misses == minSize)
        {
            lastValue.reset();
        }
        return PopResult{ 0, lastValue };
    }
    misses = 0;
    std::shared_ptr<T> value = buffer.front();
    buffer.pop_front();
    if (buffer.empty())
    {
        lastValue = value;
    }
    return PopResult{ minFrameId, value };
}

template <typename T>
void NetworkBuffer<T>::TrimFront()
{
    if (buffer.size() > minSize)
    {
        buffer.pop_front();
    }
}

template <typename T>
uint32 NetworkBuffer<T>::GetMinFrameId() const
{
    if (buffer.empty())
    {
        return maxFrameId;
    }
    return maxFrameId + 1 - static_cast<uint32>(buffer.size());
}

template <typename T>
void NetworkBuffer<T>::UpdateRange(const Vector<T>& data, uint32 rangeSize)
{
    for (uint32 i = 0; i < rangeSize; ++i)
    {
        uint32 bufferId = static_cast<uint32>(buffer.size()) - 1 - i;
        uint32 dataId = static_cast<uint32>(data.size()) - 1 - i;
        if (buffer[bufferId] == nullptr)
        {
            buffer[bufferId] = std::make_shared<T>(data[dataId]);
        }
        else if (*buffer[bufferId] != data[dataId])
        {
            Logger::Error("Received different data for a frame");
        }
    }
}

template <typename T>
uint32 NetworkBuffer<T>::GetSize() const
{
    return static_cast<uint32>(buffer.size());
}

template <typename T>
void NetworkBuffer<T>::SetMinSize(uint32 minSize_)
{
    minSize = minSize_;
}

template <typename T>
uint32 NetworkBuffer<T>::GetMinSize() const
{
    return minSize;
}

template <typename T>
uint32 NetworkBuffer<T>::GetMaxFrameId() const
{
    return maxFrameId;
}

template <typename T>
void NetworkBuffer<T>::Clear()
{
    buffer.clear();
    lastValue.reset();
}
}
