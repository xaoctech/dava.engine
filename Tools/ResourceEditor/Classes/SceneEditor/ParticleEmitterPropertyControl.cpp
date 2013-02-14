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

void ParticleEmitterPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	//Andrii Makovii 2013/01/16 This codeis outdated - new particle emitter editor should be used.
	NodesPropertyControl::ReadFrom(sceneNode);

	ParticleEmitter * emitter = GetEmitter(sceneNode);
    
	DVASSERT(emitter);

	propertyList->AddSection("Particles emitter");

	propertyList->AddStringProperty("Yaml path", PropertyList::PROPERTY_IS_READ_ONLY);
	propertyList->SetStringPropertyValue("Yaml path", emitter->GetConfigPath());

	propertyList->AddMessageProperty("Open editor", Message(this, &ParticleEmitterPropertyControl::OnOpenEditor));
}

void ParticleEmitterPropertyControl::OnOpenEditor(BaseObject * object, void * userData, void * callerData)
{
	SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
	ParticleEmitter * emitter = GetEmitter(currentSceneNode);
    DVASSERT(emitter);
	screen->EditParticleEmitter(currentSceneNode);
}
