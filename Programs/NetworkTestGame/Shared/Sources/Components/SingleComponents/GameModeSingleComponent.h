#pragma once

#include "Entity/SingletonComponent.h"

namespace DAVA
{
class Entity;
}

class GameModeSingleComponent : public DAVA::SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(GameModeSingleComponent, DAVA::SingletonComponent);

public:
    bool IsMapLoaded() const;
    void SetIsMapLoaded(bool value);

    DAVA::Entity* GetPlayer() const;
    void SetPlayer(DAVA::Entity* entity);

    void Clear() override;

private:
    bool isMapLoaded = false;

    DAVA::Entity* player = nullptr;
};
