//
//  ParticlesEditorSceneDataHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/25/12.
//
//

#ifndef __ResourceEditorQt__ParticlesEditorSceneDataHelper__
#define __ResourceEditorQt__ParticlesEditorSceneDataHelper__

#include "DAVAEngine.h"
#include "Scene3D/SceneNode.h"

namespace DAVA {

// Scene Data Helper for Particles Editor. Contains no Qt-related code.
class ParticlesEditorSceneDataHelper
{
public:
	// Add the new node, if it is related to Particles Editor. Returns TRUE if
	// no further processing needed.
	bool AddSceneNode(SceneNode* node) const;
		
	// Remove the Scene Node, if it is related to Particles Editor.
	void RemoveSceneNode(SceneNode* node) const;
	
	// Validate the Particle Emitter Component, generate the error message, if needed.
	static bool ValidateParticleEmitterComponent(ParticleEmitterComponent* component, String& validationMsg);
};

};

#endif /* defined(__ResourceEditorQt__ParticlesEditorSceneDataHelper__) */
