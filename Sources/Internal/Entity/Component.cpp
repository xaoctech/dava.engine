#include "Entity/Component.h"
#include "Scene3D/SceneNode.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/CameraComponent.h"

namespace DAVA
{

Component * Component::CreateByType(uint32 componentType)
{
	switch(componentType)
	{
	case TRANSFORM_COMPONENT:
		return new TransformComponent();
	case RENDER_COMPONENT:
		return new RenderComponent();
	case DEBUG_RENDER_COMPONENT: 
		return new DebugRenderComponent();
	case LOD_COMPONENT:
		return new LodComponent();
	case PARTICLE_EMITTER_COMPONENT:
		return new ParticleEmitterComponent();
	case PARTICLE_EFFECT_COMPONENT:
		return new ParticleEffectComponent();
	case BULLET_COMPONENT:
		return new BulletComponent();
	case UPDATABLE_COMPONENT:
		return new UpdatableComponent();
	case CAMERA_COMPONENT:
		return new CameraComponent();
		break;
	case ANIMATION_COMPONENT:
	case COLLISION_COMPONENT:
	case ACTION_COMPONENT:
	case SCRIPT_COMPONENT:
	case LIGHT_COMPONENT:
	default:
		DVASSERT(0);
		return 0;
	}

}

Component::Component()
:	entity(0)
{

}

void Component::SetEntity(SceneNode * _entity)
{
	entity = _entity;
}

void Component::GetDataNodes(Set<DAVA::DataNode *> &dataNodes)
{
    //Empty as default
}


}