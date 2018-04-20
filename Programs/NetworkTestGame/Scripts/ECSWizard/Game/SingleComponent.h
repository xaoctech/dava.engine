#pragma once

#include <Entity/SingleComponent.h>
#include <Game.h>

class TEMPLATESingleComponent : public DAVA::SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(TEMPLATESingleComponent, DAVA::SingleComponent);

public:
    void Clear() override;
};
