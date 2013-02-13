#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{

RenderObject * GetRenerObject(SceneNode * fromEntity)
{
	RenderObject * object = 0;

	RenderComponent * component = static_cast<RenderComponent*>(fromEntity->GetComponent(Component::RENDER_COMPONENT));
	if(component)
	{
		object = component->GetRenderObject();
	}

	return object;
}

ParticleEmitter * GetEmitter(SceneNode * fromEntity)
{
	RenderObject * object = GetRenerObject(fromEntity);
	ParticleEmitter * emitter = 0;
	if(object && object->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		emitter = static_cast<ParticleEmitter*>(object);
	}

	return emitter;
}

}
