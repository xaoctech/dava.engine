#pragma once

#include "Classes/Commands2/Base/RECommandIDHandler.h"

#include <Command/CommandBatch.h>
#include <Base/BaseTypes.h>

class RECommand;
class RECommandBatch final : public DAVA::CommandBatch, public RECommandIDHandler
{
public:
    RECommandBatch(const DAVA::String& description, DAVA::uint32 commandsCount);

    void RemoveCommands(DAVA::uint32 commandId);

    RECommand* GetCommand(DAVA::uint32 index) const;

    bool MatchCommandID(DAVA::uint32 commandID) const override;
};
