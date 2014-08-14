#include <QMetaType>

#include "EntityProcessors.h"
#include "Project/ProjectManager.h"
#include "Tools/QtFileDialog/QtFileDialog.h"

using namespace DAVA;

namespace
{

enum eModelPreset
{
    EMP_NO_COLLISION = 0,
    EMP_TREE,
    EMP_BUSH,
    EMP_FRAGILE_PR,
    EMP_FRAGILE_NPR,
    EMP_FALLING_ATOM,
    EMP_BUILDING,
    EMP_INVISIBLE_WALL,
    EMP_SPEED_TREE,
    EMP_WATER
};

FastName SFX_SOUNDGROUP_FX("FX");

}

bool TreeActionAdder::ProcessEntity(DAVA::Entity *entity, const DAVA::FastName& entityName, bool isExternal)
{
    CustomPropertiesComponent *customProperties = static_cast<CustomPropertiesComponent*>(entity->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT));
    
    if (!customProperties)
    {
        return false;
    }

    KeyedArchive *props = customProperties->GetArchive();

    if (!props->IsKeyExists("CollisionType"))
    {
        return false;
    }

    int32 collisionType = props->GetInt32("CollisionType");

    if (EMP_TREE == collisionType || EMP_FALLING_ATOM == collisionType || EMP_SPEED_TREE == collisionType)
    {
        ActionComponent *actionComponent = static_cast<ActionComponent*>(entity->GetComponent(Component::ACTION_COMPONENT));
        if (actionComponent)
        {
            return false;
        }

        actionComponent = new ActionComponent();
        entity->AddComponent(actionComponent);

        ActionComponent::Action action;
        action.eventType = ActionComponent::Action::EVENT_CUSTOM;
        action.type = ActionComponent::Action::TYPE_PARTICLE_EFFECT_START;
        action.entityName = FastName("start");
        action.userEventId = FastName("start");
        action.stopAfterNRepeats = 1;
        actionComponent->Add(action);

        action.type = ActionComponent::Action::TYPE_SOUND_START;
        actionComponent->Add(action);

        action.userEventId = FastName("touch");
        action.type = ActionComponent::Action::TYPE_SOUND_STOP;
        actionComponent->Add(action);

        action.entityName = FastName("touch");
        action.type = ActionComponent::Action::TYPE_PARTICLE_EFFECT_START;
        actionComponent->Add(action);

        action.type = ActionComponent::Action::TYPE_SOUND_START;
        actionComponent->Add(action);

        return true;
    }

    return false;

}

DestructibleSoundAdder::DestructibleSoundAdder()
    : settingsYaml(NULL)
    , processDestructibles(false)
    , processTrees(false)
{ }

DestructibleSoundAdder::~DestructibleSoundAdder()
{
    SafeRelease(settingsYaml);
}

void DestructibleSoundAdder::Init()
{
    QString path = QtFileDialog::getOpenFileName(NULL, "Open sound settings file", ProjectManager::Instance()->CurProjectDataSourcePath().GetAbsolutePathname().c_str(), "YAML File (*.yaml)");
    if (path.isEmpty())
    {
        return;
    }

    ScopedPtr<YamlParser> parser(YamlParser::Create(path.toStdString()));

    settingsYaml = SafeRetain(parser->GetRootNode());

    processDestructibles = (settingsYaml->Get("destructibles") != NULL);
    processTrees = (settingsYaml->Get("trees") != NULL);
}

bool DestructibleSoundAdder::ProcessEntity(DAVA::Entity *entity, const DAVA::FastName& entityName, bool isExternal)
{
    CustomPropertiesComponent *customProperties = static_cast<CustomPropertiesComponent*>(entity->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT));

    KeyedArchive *props = customProperties->GetArchive();
    if (!props->IsKeyExists("CollisionType"))
    {
        return false;
    }

    int32 collisionType = props->GetInt32("CollisionType");

    if (processDestructibles && (EMP_FRAGILE_PR == collisionType || EMP_FRAGILE_NPR == collisionType))
    {
        const YamlNode *destructibles = settingsYaml->Get("destructibles");
        const YamlNode *setting = destructibles->Get(entityName.c_str());

        if (!setting)
        {
            Logger::Error("%s setting does not exist for %s", __FUNCTION__, entityName.c_str());
            return false;
        }

        AddSoundToEntity(entity, setting->AsString());
        return true;
    }
    else if (processTrees && (EMP_TREE == collisionType || EMP_FALLING_ATOM == collisionType || EMP_SPEED_TREE == collisionType))
    {
        const YamlNode *trees = settingsYaml->Get("trees");
        const YamlNode *setting = trees->Get(entityName.c_str());

        if (!setting)
        {
            Logger::Error("%s setting does not exist for %s", __FUNCTION__, entityName.c_str());
            return false;
        }

        const YamlNode *startNode = setting->Get("start");
        if (startNode)
        {
            AddSoundToEntity(entity->FindByName("start"), startNode->AsString());
        }

        const YamlNode *touchNode = setting->Get("touch");
        if (touchNode)
        {
            AddSoundToEntity(entity->FindByName("touch"), touchNode->AsString());
        }

        return true;
    }
    else
    {
        return false;
    }
}

void DestructibleSoundAdder::Finalize()
{
    SafeRelease(settingsYaml);
}


void DestructibleSoundAdder::AddSoundToEntity(DAVA::Entity* entity, const String &soundEventName, bool removeExisting)
{
    if (!entity)
    {
        return;
    }

    SoundComponent *soundComponent = static_cast<SoundComponent*>(entity->GetComponent(Component::SOUND_COMPONENT));
    if (!soundComponent)
    {
        soundComponent = static_cast<SoundComponent*>(Component::CreateByType(Component::SOUND_COMPONENT));
        entity->AddComponent(soundComponent);
        soundComponent->SetLocalDirection(Vector3(0.f, 1.f, 0.f));
    }
    else if (removeExisting)
    {
        soundComponent->RemoveAllEvents();
    }

    SoundEvent *soundEvent = SoundSystem::Instance()->CreateSoundEventByID(FastName(soundEventName), SFX_SOUNDGROUP_FX);
    soundComponent->AddSoundEvent(soundEvent);

    SafeRelease(soundEvent);
}