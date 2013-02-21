#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/LandscapeNode.h"
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

Camera * GetCamera(SceneNode * fromEntity)
{
    CameraComponent *component = static_cast<CameraComponent *>(fromEntity->GetComponent(Component::CAMERA_COMPONENT));
    if(component)
    {
        return component->GetCamera();
    }
    
    return NULL;
}
    
LodComponent * GetLodComponent(SceneNode *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<LodComponent*>(fromEntity->GetComponent(Component::LOD_COMPONENT));
    }
    
    return NULL;
}



}
