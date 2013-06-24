#ifndef __RESOURCE_EDITOR_MATERIAL_VIEW_OPTIONS_COMMANDS_H__
#define __RESOURCE_EDITOR_MATERIAL_VIEW_OPTIONS_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "../Constants.h"
#include "../Qt/DockSetSwitchIndex/SetSwitchIndexHelper.h"

class CommandChangeMaterialViewOption: public Command
{
public:
	DAVA_DEPRECATED(CommandChangeMaterialViewOption(DAVA::Material::eViewOptions value));// DEPRECATED : using SceneDataManager(QOBJECT)

protected:
	DAVA::Material::eViewOptions	value;

    virtual void Execute();
};

#endif // #ifndef __RESOURCE_EDITOR_MATERIAL_VIEW_OPTIONS_COMMANDS_H__