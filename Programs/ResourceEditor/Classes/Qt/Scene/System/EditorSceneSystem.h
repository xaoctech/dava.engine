#pragma once

#include <Command/Command.h>

class RECommandNotificationObject;
class REDependentCommandsHolder;
class SceneData;
class EditorSceneSystem
{
    friend class SceneEditor2;

public:
    virtual ~EditorSceneSystem() = default;

    virtual void EnableSystem()
    {
        systemIsEnabled = true;
    }

    virtual void DisableSystem()
    {
        systemIsEnabled = false;
    }

    virtual bool LoadLocalProperties(SceneData *data)
    {
        return true;
    }

    virtual bool SaveLocalProperties(SceneData* data)
    {
        return true;
    }

    bool IsSystemEnabled() const
    {
        return systemIsEnabled;
    }

protected:
    virtual void Draw()
    {
    }
    virtual void AccumulateDependentCommands(REDependentCommandsHolder& holder)
    {
    }
    virtual void ProcessCommand(const RECommandNotificationObject& commandNotification)
    {
    }

    virtual std::unique_ptr<DAVA::Command> PrepareForSave(bool saveForGame)
    {
        return nullptr;
    }

    bool systemIsEnabled = false;
};
