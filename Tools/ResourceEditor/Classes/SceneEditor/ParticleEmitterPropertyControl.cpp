#include "ParticleEmitterPropertyControl.h"
#include "SceneEditor/SceneEditorScreenMain.h"
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

	propertyList->AddFilepathProperty("Yaml path", ".yaml");
	propertyList->SetFilepathPropertyValue("Yaml path", emitter->GetYamlPath());

	propertyList->AddMessageProperty("Open editor", Message(this, &ParticleEmitterPropertyControl::OnOpenEditor));
}

void ParticleEmitterPropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
	if("Yaml path" == forKey)
	{
		ParticleEmitterNode * emitter = dynamic_cast<ParticleEmitterNode *> (currentSceneNode);
		emitter->LoadFromYaml(newValue);
	}
}

void ParticleEmitterPropertyControl::OnOpenEditor(BaseObject * object, void * userData, void * callerData)
{
	SceneEditorScreenMain *screen = (SceneEditorScreenMain *)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
	ParticleEmitterNode * emitter = dynamic_cast<ParticleEmitterNode*> (currentSceneNode);
	screen->EditParticleEmitter(emitter);
}
