#pragma once

#include "Base/FastName.h"
#include "Engine/Engine.h" //for GetEngineContext
#include "FileSystem/FilePath.h"

namespace DAVA
{
namespace Private
{
class EngineBackend;
}

class FeatureManager final
{
public:
    void InitFromConfig(const FilePath& fileName);
    Any GetFeatureValue(FastName featureName);
    void Dump();

private:
    UnorderedMap<FastName, Any> features;

    ~FeatureManager() = default;
    friend class Private::EngineBackend;
};
}

#define DAVA_CONCATENATE_IMPL__(s1, s2) s1##s2
#define DAVA_CONCATENATE__(s1, s2) DAVA_CONCATENATE_IMPL__(s1, s2)
#define DAVA_ANONYMOUS_VARIABLE__(str) DAVA_CONCATENATE__(str, __LINE__)

#define DAVA_IF_FEATURE(feature) \
static FastName DAVA_ANONYMOUS_VARIABLE__(feature)(#feature); if (GetEngineContext()->featureManager->GetFeatureValue(DAVA_ANONYMOUS_VARIABLE__(feature)).Get<bool>())
