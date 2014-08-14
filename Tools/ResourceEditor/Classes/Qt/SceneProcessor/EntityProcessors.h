#include "DAVAEngine.h"

#include "SceneProcessor/SceneProcessor.h"

class TreeActionAdder : public EntityProcessorBase
{
public:
    TreeActionAdder()
    {}

    virtual bool ProcessEntity(DAVA::Entity *entity, const DAVA::FastName& entityName, bool isExternal) override;
    
    virtual bool NeedProcessExternal() const { return true; }

protected:
    ~TreeActionAdder() {};
};

class DestructibleSoundAdder : public EntityProcessorBase
{
public:
    DestructibleSoundAdder();

    virtual void Init() override;
    virtual bool ProcessEntity(DAVA::Entity *entity, const DAVA::FastName& entityName, bool isExternal) override;
    virtual void Finalize() override;

    virtual bool NeedProcessExternal() const { return true; }

protected:
    ~DestructibleSoundAdder();

private:
    void AddSoundToEntity(DAVA::Entity *entity, const DAVA::String &soundEventName, bool removeExisting = true);
private:
    DAVA::YamlNode *settingsYaml;
    
    bool processDestructibles;
    bool processTrees;
};