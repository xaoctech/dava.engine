#pragma once

#include <Base/BaseTypes.h>

struct spTrackEntry;

namespace DAVA
{
class SpineTrackEntry
{
public:
    SpineTrackEntry(spTrackEntry* track);

    bool IsLoop() const;

private:
    spTrackEntry* trackPtr = nullptr;
};
}