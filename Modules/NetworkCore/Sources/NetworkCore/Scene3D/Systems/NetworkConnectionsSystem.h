#pragma once

#include "Entity/SceneSystem.h"

namespace DAVA
{
class NetworkServerConnectionsSingleComponent;

class NetworkConnectionsSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkConnectionsSystem, SceneSystem);

    NetworkConnectionsSystem(Scene* scene);

    void PrepareForRemove() override{};
};
}
