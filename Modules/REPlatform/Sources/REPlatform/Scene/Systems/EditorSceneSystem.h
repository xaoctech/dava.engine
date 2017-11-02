#pragma once

#include <Command/Command.h>

namespace DAVA
{
class RECommandNotificationObject;
class REDependentCommandsHolder;
class PropertiesHolder;

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

    virtual void LoadLocalProperties(DAVA::PropertiesHolder* holder)
    {
    }

    virtual void SaveLocalProperties(DAVA::PropertiesHolder* holder)
    {
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

    virtual std::unique_ptr<Command> PrepareForSave(bool saveForGame)
    {
        return nullptr;
    }

    bool systemIsEnabled = false;
};
} // namespace DAVA
