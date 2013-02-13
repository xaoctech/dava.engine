#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"

namespace DAVA
{

ParticleEmitter * GetEmitter(SceneNode * fromEntity)
{
	RenderComponent * component = static_cast<RenderComponent*>(fromEntity->GetComponent(Component::RENDER_COMPONENT));
	if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
		return emitter;
	}
	else
	{
		return 0;
	}
}

}