#include "ParticleEffectPropertyControl.h"
#include "Scene3D/ParticleEffectNode.h"

ParticleEffectPropertyControl::ParticleEffectPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
}

ParticleEffectPropertyControl::~ParticleEffectPropertyControl()
{

}

void ParticleEffectPropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

	ParticleEffectNode * particleEffect = dynamic_cast<ParticleEffectNode *>(sceneNode);
	DVASSERT(particleEffect);

    propertyList->AddSection("Particle Effect", GetHeaderState("Particle Effect", true));
        
	propertyList->AddMessageProperty(String("Start"), Message(this, &ParticleEffectPropertyControl::OnStart));
	propertyList->AddMessageProperty(String("Stop"), Message(this, &ParticleEffectPropertyControl::OnStop));
	propertyList->AddMessageProperty(String("Restart"), Message(this, &ParticleEffectPropertyControl::OnRestart));
}

void ParticleEffectPropertyControl::OnStart(BaseObject * object, void * userData, void * callerData)
{
	ParticleEffectNode * particleEffect = dynamic_cast<ParticleEffectNode *>(currentSceneNode);
	particleEffect->Start();

}

void ParticleEffectPropertyControl::OnStop(BaseObject * object, void * userData, void * callerData)
{
	ParticleEffectNode * particleEffect = dynamic_cast<ParticleEffectNode *>(currentSceneNode);
}

void ParticleEffectPropertyControl::OnRestart(BaseObject * object, void * userData, void * callerData)
{
	ParticleEffectNode * particleEffect = dynamic_cast<ParticleEffectNode *>(currentSceneNode);

}

