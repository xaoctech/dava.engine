//
//  ParticleForce.cpp
//  FrameworkQt
//
//  Created by Yuri Coder on 2/11/13.
//
//

#include "ParticleForce.h"
using namespace DAVA;

// Particle Force class is needed to store Particle Force data.
ParticleForce::ParticleForce(RefPtr<PropertyLine<Vector3> > force,
							 RefPtr<PropertyLine<Vector3> > forceVariation,
							  RefPtr<PropertyLine<float32> > forceOverLife)
{
	Update(force, forceVariation, forceOverLife);
}
	
ParticleForce::ParticleForce(ParticleForce* forceToCopy)
{
	if (forceToCopy)
	{
		this->force = forceToCopy->force;
		this->forceVariation = forceToCopy->forceVariation;
		this->forceOverLife = forceToCopy->forceOverLife;
	}
	else
	{
		this->force = NULL;
		this->forceVariation = NULL;
		this->forceOverLife = NULL;
	}
}
	
void ParticleForce::Update(RefPtr<PropertyLine<Vector3> > force,
						   RefPtr<PropertyLine<Vector3> > forceVariation,
						   RefPtr<PropertyLine<float32> > forceOverLife)
{
	this->force = force;
	this->forceVariation = forceVariation;
	this->forceOverLife = forceOverLife;
}
