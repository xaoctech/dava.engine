//
//  ParticleForce.h
//  FrameworkQt
//
//  Created by Yuri Coder on 2/11/13.
//
//

#ifndef __PARTICLE_FORCE_H__
#define __PARTICLE_FORCE_H__

#include "Particles/ParticlePropertyLine.h"

namespace DAVA {

// Particle Force class is needed to store Particle Force data.
class ParticleForce
{
public:
	// Initialization constructor.
	ParticleForce(RefPtr<PropertyLine<Vector3> > force, RefPtr<PropertyLine<Vector3> > forceVariation,
				  RefPtr<PropertyLine<float32> > forceOverLife);

	// Copy constructor.
	ParticleForce(ParticleForce* forceToCopy);

	void Update(RefPtr<PropertyLine<Vector3> > force, RefPtr<PropertyLine<Vector3> > forceVariation,
				RefPtr<PropertyLine<float32> > forceOverLife);

	// Accessors.
	RefPtr<PropertyLine<Vector3> > GetForce() {return force;};
	RefPtr<PropertyLine<Vector3> > GetForceVariation() {return forceVariation; };
	RefPtr<PropertyLine<float32> > GetForceOverlife() { return forceOverLife; };
	
protected:
	RefPtr<PropertyLine<Vector3> > force;
	RefPtr<PropertyLine<Vector3> > forceVariation;
	RefPtr<PropertyLine<float32> > forceOverLife;
};

};

#endif /* defined(__PARTICLE_FORCE_H__) */
