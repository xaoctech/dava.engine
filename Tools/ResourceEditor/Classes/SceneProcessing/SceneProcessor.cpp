#include "SceneProcessor.h"

using namespace DAVA;

namespace
{

typedef std::set<String> StringSet;

}

SceneProcessor::SceneProcessor(EntityProcessorBase *_entityProcessor /*= NULL*/)
    : entityProcessor(_entityProcessor)
{
}

SceneProcessor::~SceneProcessor()
{
    SafeRelease(entityProcessor);
}

void SceneProcessor::SetEntityProcessor(EntityProcessorBase *_entityProcessor)
{
    if (entityProcessor)
    {
        SafeRelease(entityProcessor);
    }

    entityProcessor = _entityProcessor;
}

bool SceneProcessor::Execute(DAVA::Scene *currentScene)
{
    if (!entityProcessor)
    {
        Logger::Warning("%s need to set EntityProcessor", __FUNCTION__);
        return false;
    }

    entityProcessor->Init();

    int32 childrenCount = currentScene->GetChildrenCount();

    StringSet refToOwnerSet;

    const bool needProcessExternal = entityProcessor->NeedProcessExternal();
    bool sceneModified = false;

    for (int32 index = 0; index < childrenCount; index++)
    {
        Entity *currentEntity = currentScene->GetChild(index);

        bool entityModified = entityProcessor->ProcessEntity(currentEntity, currentEntity->GetName(), false);
        sceneModified = sceneModified || entityModified;
        if (entityModified && needProcessExternal)
        {
            CustomPropertiesComponent *customProperties = static_cast<CustomPropertiesComponent*>(currentEntity->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT));
            KeyedArchive *props = customProperties->GetArchive();
            if (!props->IsKeyExists("editor.referenceToOwner"))
            {
                Logger::Error("%s editor.referenceToOwner not found for %s", __FUNCTION__, currentEntity->GetName().c_str());
                continue;
            }

            const String referenceToOwner = props->GetString("editor.referenceToOwner");
            std::pair<StringSet::iterator, bool> insertResult = refToOwnerSet.insert(referenceToOwner);

            if (insertResult.second)
            {
                Scene *newScene = new Scene();
                Entity *root = newScene->GetRootNode(referenceToOwner);
                if (root && root->GetChildrenCount() == 1)
                {
                    root = root->GetChild(0);
                }
                newScene->AddNode(root);
                entityProcessor->ProcessEntity(root, currentEntity->GetName(), true);
                newScene->Save(referenceToOwner);
                SafeRelease(newScene);
            }
        }
    }

    entityProcessor->Finalize();
    return sceneModified;
}
