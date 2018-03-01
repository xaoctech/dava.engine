#include "FeatureManager/FeatureManager.h"

#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "Logger/Logger.h"

namespace DAVA
{
void FeatureManager::InitFromConfig(const FilePath& fileName)
{
    features.clear();

    YamlParser* parser = YamlParser::Create(fileName);
    SCOPE_EXIT
    {
        SafeRelease(parser);
    };

    if (parser)
    {
        YamlNode* node = parser->GetRootNode();
        DVASSERT(node->GetType() == YamlNode::TYPE_MAP);
        auto& map = node->AsMap();
        for (auto pair : map)
        {
            FastName featureName(pair.first);
            bool value = pair.second->AsBool();
            features.insert({ featureName, value });
        }
    }
}

void FeatureManager::Dump()
{
    Logger::Info("***** FeatureManager Dump *****");
    for (auto pair : features)
    {
        DVASSERT(pair.second.GetType() == Type::Instance<bool>());
        Logger::Info("%s:%d", pair.first.c_str(), pair.second.Get<bool>());
    }
    Logger::Info("***** FeatureManager Dump end *****");
}

Any FeatureManager::GetFeatureValue(FastName featureName)
{
    Any res;
    auto pair = features.find(featureName);
    if (pair != features.end())
    {
        res = pair->second;
    }

    return res;
}
}
