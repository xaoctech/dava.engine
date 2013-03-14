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
#include "Scene3D/Entity.h"

namespace DAVA {

// Scene Data Helper for Particles Editor. Contains no Qt-related code.
class ParticlesEditorSceneDataHelper
{
public:
	// Add the new node, if it is related to Particles Editor. Returns TRUE if
	// no further processing needed.
	bool AddSceneNode(Entity* node) const;
		
	// Remove the Scene Node, if it is related to Particles Editor.
	void RemoveSceneNode(Entity* node) const;
	
	// Validate the Particle Emitter, generate the error message, if needed.
	static bool ValidateParticleEmitter(ParticleEmitter* emitter, String& validationMsg);
};

};

#endif /* defined(__ResourceEditorQt__ParticlesEditorSceneDataHelper__) */
