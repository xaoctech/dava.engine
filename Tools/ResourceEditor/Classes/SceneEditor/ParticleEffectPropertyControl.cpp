#include "ParticleEffectPropertyControl.h"
#include "Scene3D/ParticleEffectNode.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

ParticleEffectPropertyControl::ParticleEffectPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

ParticleEffectPropertyControl::~ParticleEffectPropertyControl()
{

}

void ParticleEffectPropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	Component* component = sceneNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT);
	DVASSERT(component);

    propertyList->AddSection("Particle Effect", GetHeaderState("Particle Effect", true));
        
	propertyList->AddMessageProperty(String("Start"), Message(this, &ParticleEffectPropertyControl::OnStart));
	propertyList->AddMessageProperty(String("Stop"), Message(this, &ParticleEffectPropertyControl::OnStop));
	propertyList->AddMessageProperty(String("Restart"), Message(this, &ParticleEffectPropertyControl::OnRestart));
}

void ParticleEffectPropertyControl::OnStart(BaseObject * object, void * userData, void * callerData)
{
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(currentSceneNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
	effectComponent->Start();

}

void ParticleEffectPropertyControl::OnStop(BaseObject * object, void * userData, void * callerData)
{
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(currentSceneNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
	effectComponent->Stop();
}

void ParticleEffectPropertyControl::OnRestart(BaseObject * object, void * userData, void * callerData)
{
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(currentSceneNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
	effectComponent->Restart();
}

