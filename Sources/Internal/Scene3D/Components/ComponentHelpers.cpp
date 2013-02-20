#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/LandscapeNode.h"

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


Light * GetLight( SceneNode * fromEntity )
{
	LightComponent * component = static_cast<LightComponent*>(fromEntity->GetComponent(Component::LIGHT_COMPONENT));
	if(component)
	{
		return component->GetLightObject();
	}

	return NULL;
}

LandscapeNode * GetLandscape( SceneNode * fromEntity )
{
	RenderObject * object = GetRenerObject(fromEntity);
	if(object && object->GetType() == RenderObject::TYPE_LANDSCAPE)
	{
		LandscapeNode *landscape = static_cast<LandscapeNode *>(object);
		return landscape;
	}

	return NULL;
}


}
