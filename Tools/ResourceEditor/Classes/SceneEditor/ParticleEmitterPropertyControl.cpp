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
	NodesPropertyControl::ReadFrom(sceneNode);

	ParticleEmitterNode * emitter = dynamic_cast<ParticleEmitterNode*> (sceneNode);
	DVASSERT(emitter);

	propertyList->AddSection("Particles emitter");

	propertyList->AddStringProperty("Yaml path", PropertyList::PROPERTY_IS_READ_ONLY);
	propertyList->SetStringPropertyValue("Yaml path", emitter->GetYamlPath());

	propertyList->AddMessageProperty("Open editor", Message(this, &ParticleEmitterPropertyControl::OnOpenEditor));
}

void ParticleEmitterPropertyControl::OnOpenEditor(BaseObject * object, void * userData, void * callerData)
{
	SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
	ParticleEmitterNode * emitter = dynamic_cast<ParticleEmitterNode*> (currentSceneNode);
	screen->EditParticleEmitter(emitter);
}
