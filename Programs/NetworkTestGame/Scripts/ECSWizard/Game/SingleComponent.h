#pragma once

#include <Entity/SingletonComponent.h>
#include <Game.h>

class TEMPLATESingleComponent : public DAVA::SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(TEMPLATESingleComponent, DAVA::SingletonComponent);

public:
    void Clear() override;
};
