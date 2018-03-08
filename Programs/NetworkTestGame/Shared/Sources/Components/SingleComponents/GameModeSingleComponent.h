#pragma once

#include "Entity/SingleComponent.h"

namespace DAVA
{
class Entity;
}

class GameModeSingleComponent : public DAVA::SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(GameModeSingleComponent, DAVA::SingleComponent);

public:
    bool IsMapLoaded() const;
    void SetIsMapLoaded(bool value);

    DAVA::Entity* GetPlayer() const;
    void SetPlayer(DAVA::Entity* entity);

private:
    bool isMapLoaded = false;

    DAVA::Entity* player = nullptr;
};
