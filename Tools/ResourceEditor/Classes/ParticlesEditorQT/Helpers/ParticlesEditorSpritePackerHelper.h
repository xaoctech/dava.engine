#ifndef __ResourceEditorQt__ParticlesEditorSpritePackerHelper__
#define __ResourceEditorQt__ParticlesEditorSpritePackerHelper__

#include "DAVAEngine.h"
class SceneData;
namespace DAVA {

// Sprite Packer Helper for Particles Editor.
class ParticlesEditorSpritePackerHelper
{
public:
	
	static void UpdateParticleSprites();

protected: 

	static void UpdateParticleSprites(SceneData* sceneData);
};
};
#endif /* defined(__ResourceEditorQt__ParticlesEditorSpritePackerHelper__) */