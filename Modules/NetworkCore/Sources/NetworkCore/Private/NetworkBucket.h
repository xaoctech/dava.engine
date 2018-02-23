#pragma once

#include <algorithm>

#include "Base/BaseTypes.h"
#include "Math/Matrix4.h"
#include "Base/List.h"
#include "Base/Vector.h"
#include "Logger/Logger.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
using FramesIter = List<uint32>::const_iterator;
template <typename T>
using BucketIter = typename List<T>::const_iterator;

template <typename T>
class NetworkBucket;

template <typename T>
class NetworkBucketIter
{
public:
    NetworkBucketIter(const FramesIter& framesIt, const BucketIter<T>& bucketIt);
    NetworkBucketIter& operator++();
    bool operator!=(const NetworkBucketIter& rhs);
    uint32 Frame() const;
    const T& Data() const;

private:
    FramesIter framesIt;
    BucketIter<T> bucketIt;
};

template <typename T>
class NetworkBucket
{
public:
    NetworkBucket(uint32 minSize, uint32 maxSize);
    bool Push(uint32 frameId, const T& val);
    void RemoveUntil(uint32 frameId);
    void Clear();
    uint32 GetSize() const;
    const T* GetRawData(uint32& size, uint32& frameId) const;
    uint32 GetMaxFrameId() const;
    NetworkBucketIter<T> Begin() const;
    NetworkBucketIter<T> End() const;

private:
    uint32 minSize;
    uint32 maxSize;
    List<uint32> frames;
    List<T> bucket;
    mutable Vector<T> data;
    uint32 maxFrameId;
};

template <typename T>
NetworkBucketIter<T>::NetworkBucketIter(const FramesIter& framesIt, const BucketIter<T>& bucketIt)
    : framesIt(framesIt)
    , bucketIt(bucketIt)
{
}

template <typename T>
NetworkBucketIter<T>& NetworkBucketIter<T>::operator++()
{
    ++framesIt;
    ++bucketIt;
    return *this;
}

template <typename T>
bool NetworkBucketIter<T>::operator!=(const NetworkBucketIter<T>& rhs)
{
    return rhs.framesIt != framesIt || rhs.bucketIt != bucketIt;
}

template <typename T>
uint32 NetworkBucketIter<T>::Frame() const
{
    return *framesIt;
}

template <typename T>
const T& NetworkBucketIter<T>::Data() const
{
    return *bucketIt;
}

template <typename T>
NetworkBucket<T>::NetworkBucket(uint32 minSize, uint32 maxSize)
    : minSize(minSize)
    , maxSize(maxSize)
    , maxFrameId(~0)
{
}

template <typename T>
bool NetworkBucket<T>::Push(uint32 frameId, const T& val)
{
    int32 diff = frameId - maxFrameId;
    if (diff <= 0)
    {
        return false;
    }
    while (diff > 1 && !bucket.empty())
    {
        --diff;
        bucket.push_back(T());
        frames.push_back(frameId - diff);
    }
    bucket.push_back(val);
    frames.push_back(frameId);
    maxFrameId = frameId;
    while (frames.size() > maxSize)
    {
        frames.pop_front();
        bucket.pop_front();
    }

    return true;
}

template <typename T>
void NetworkBucket<T>::RemoveUntil(uint32 frameId)
{
    if (!frames.empty() && frameId >= frames.front() && frameId <= frames.back())
    {
        while (frames.front() < frameId && frames.size() > minSize)
        {
            frames.pop_front();
            bucket.pop_front();
        }
    }
}

template <typename T>
void NetworkBucket<T>::Clear()
{
    frames.clear();
    bucket.clear();
    maxFrameId = ~0;
}

template <typename T>
uint32 NetworkBucket<T>::GetSize() const
{
    return static_cast<uint32>(frames.size());
}

template <typename T>
uint32 NetworkBucket<T>::GetMaxFrameId() const
{
    return maxFrameId;
}

template <typename T>
const T* NetworkBucket<T>::GetRawData(uint32& size, uint32& frameId) const
{
    frameId = maxFrameId;
    size = static_cast<uint32>(bucket.size());
    data.resize(size);
    std::copy(bucket.begin(), bucket.end(), data.begin());
    return data.data();
}

template <typename T>
NetworkBucketIter<T> NetworkBucket<T>::Begin() const
{
    return NetworkBucketIter<T>(frames.begin(), bucket.begin());
}

template <typename T>
NetworkBucketIter<T> NetworkBucket<T>::End() const
{
    return NetworkBucketIter<T>(frames.end(), bucket.end());
}
}
