/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/SystemTimer.h"
#include "Base/TemplateHelpers.h"

namespace DAVA
{


void ParticleEmitterSystem::AddIfEmitter(RenderObject * maybeEmitter)
{
	if(maybeEmitter->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(maybeEmitter);
		emitters.push_back(emitter);
	}
}

void ParticleEmitterSystem::RemoveIfEmitter(RenderObject * maybeEmitter)
{
	if(maybeEmitter->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(maybeEmitter);
		uint32 size = emitters.size();
		for(uint32 i = 0; i < size; ++i)
		{
			if(emitters[i] == emitter)
			{
				emitters[i] = emitters[size-1];
				emitters.pop_back();
				return;
			}
		}
		DVASSERT(0);
	}
}

void ParticleEmitterSystem::Update(float32 timeElapsed)
{
	uint32 size = emitters.size();
	for(uint32 i = 0; i < size; ++i)
	{
		
		// Yuri Coder, 2013/05/15. Have to update all emitters, even non-visible ones because
		// of DF-1140 issue.
        emitters[i]->Update(timeElapsed);
	}
}

}
