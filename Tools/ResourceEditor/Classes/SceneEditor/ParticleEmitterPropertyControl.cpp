#include "ParticleEmitterPropertyControl.h"
#include "SceneEditorScreenMain.h"
#include "AppScreens.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"

ParticleEmitterPropertyControl::ParticleEmitterPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

ParticleEmitterPropertyControl::~ParticleEmitterPropertyControl()
{

}

void ParticleEmitterPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	//Andrii Makovii 2013/01/16 This codeis outdated - new particle emitter editor should be used.
	NodesPropertyControl::ReadFrom(sceneNode);

	ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(sceneNode->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    
	DVASSERT(emitterComponent);

	propertyList->AddSection("Particles emitter");

	propertyList->AddStringProperty("Yaml path", PropertyList::PROPERTY_IS_READ_ONLY);
	propertyList->SetStringPropertyValue("Yaml path", emitterComponent->GetYamlPath());

	propertyList->AddMessageProperty("Open editor", Message(this, &ParticleEmitterPropertyControl::OnOpenEditor));
}

void ParticleEmitterPropertyControl::OnOpenEditor(BaseObject * object, void * userData, void * callerData)
{
	SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
	ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(currentSceneNode->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    DVASSERT(emitterComponent);
	screen->EditParticleEmitter(currentSceneNode);
}
