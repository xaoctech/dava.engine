#pragma once

#include "Beast/BeastTypes.h"
#include "Beast/BeastNames.h"

#include <Base/BaseTypes.h>

class TextureTarget
{
public:
    DECLARE_BEAST_NAME(BeastManager);

    TextureTarget(ILBJobHandle job, DAVA::int32 size);
    ~TextureTarget();

    ILBTargetHandle GetHandle();

private:
    ILBTargetHandle handle;
};
