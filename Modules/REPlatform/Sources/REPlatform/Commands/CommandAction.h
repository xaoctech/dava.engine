#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class CommandAction : public RECommand
{
public:
    CommandAction(const String& text = String());
    void Undo();

private:
    DAVA_VIRTUAL_REFLECTION(CommandAction, RECommand);
};
} // namespace DAVA
