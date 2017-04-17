#pragma once

#include <Command/Command.h>

class RECommandNotificationObject;
class REDependentCommandsHolder;
class EditorSceneSystem
{
    friend class SceneEditor2;

public:
    virtual ~EditorSceneSystem() = default;

    virtual void EnableSystem()
    {
        systemIsEnabled = true;
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
