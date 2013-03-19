#include "ParticleEmitterPropertyControl.h"
#include "SceneEditorScreenMain.h"
#include "AppScreens.h"

ParticleEmitterPropertyControl::ParticleEmitterPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

ParticleEmitterPropertyControl::~ParticleEmitterPropertyControl()
{

}

void ParticleEmitterPropertyControl::ReadFrom(Entity * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	ParticleEmitter * emitter = GetEmitter(sceneNode);
	DVASSERT(emitter);

	propertyList->AddSection("Particles emitter");

	propertyList->AddStringProperty("Yaml path", PropertyList::PROPERTY_IS_READ_ONLY);
	propertyList->SetStringPropertyValue("Yaml path", emitter->GetConfigPath());
}
