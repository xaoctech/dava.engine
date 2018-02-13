#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Math/Vector.h>

namespace DAVA
{
class Entity;

/**
		Component that stores replicated information about dynamic body.
		Since only NetworkTransformComponent is predicted on a client,
		this information only used during resimulation to sync local body with remote one
		(i.e. if transform is out of sync and we need to resimulate, we have to have the same state on a server and a client for resimulation to be correct).
	*/
class NetworkDynamicBodyComponent final : public Component
{
public:
    Component* Clone(Entity* toEntity) override;

private:
    DAVA_VIRTUAL_REFLECTION(NetworkDynamicBodyComponent, Component);

private:
    friend class NetworkPhysicsSystem;

private:
    Vector3 linearVelocity;
    Vector3 angularVelocity;
};
}