#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/Components/SoundComponent.h"

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
	case PARTICLE_EFFECT_COMPONENT:
		return new ParticleEffectComponent();
	case BULLET_COMPONENT:
		return new BulletComponent();
	case UPDATABLE_COMPONENT:
		return new UpdatableComponent();
	case CAMERA_COMPONENT:
		return new CameraComponent();
		break;
	case LIGHT_COMPONENT:
		return new LightComponent();
		break;
	case SWITCH_COMPONENT:
		return new SwitchComponent();
		break;
	case USER_COMPONENT:
		return new UserComponent();
		break;
	case SOUND_COMPONENT:
		return new SoundComponent();
		break;
	case ANIMATION_COMPONENT:
	case COLLISION_COMPONENT:
	case ACTION_COMPONENT:
	case SCRIPT_COMPONENT:
	default:
		DVASSERT(0);
		return 0;
	}

}

Component::Component()
:	entity(0)
{

}

Component::~Component()
{ }

void Component::SetEntity(Entity * _entity)
{
	entity = _entity;
}

Entity* Component::GetEntity() 
{ 
	return entity;
};

void Component::GetDataNodes(Set<DAVA::DataNode *> &dataNodes)
{
    //Empty as default
}

void Component::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive) archive->SetUInt32("comp.type", GetType());
}

void Component::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		uint32 type = 0xFFFFFFFF;

		if(archive->IsKeyExists("comp.type")) type = archive->GetUInt32("comp.type");

		DVASSERT(type == GetType());
	}
}

}