/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "ParticleRenderObject.h"


namespace DAVA
{
static const int32 POINTS_PER_PARTICLE = 4;
static const int32 INDICES_PER_PARTICLE = 6;

//camera_facing, x_emitter, y_emitter, z_emitter, x_world, y_world, z_world
static Vector3 basisVectors[7*2] = {Vector3(), Vector3(), 
									Vector3(), Vector3(), 
									Vector3(), Vector3(), 
									Vector3(), Vector3(), 
									Vector3(0,1,0), Vector3(0,0,1), 
									Vector3(0,0,1), Vector3(1,0,0), 
									Vector3(0,1,0), Vector3(1,0,0)};

static Vector3 currCamDirection = Vector3(0,0,1);


void ParticleRenderGroup::UpdateRenderBatch()
{
	RenderDataObject *renderDataObject = renderBatch->GetRenderDataObject();
	renderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &vertices.front());
	renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &texcoords.front());
	renderDataObject->SetStream(EVF_COLOR, TYPE_UNSIGNED_BYTE, 4, 0, &colors.front());				
	if (enableFrameBlend)
	{
		renderDataObject->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 2, 0, &texcoords2.front());
		renderDataObject->SetStream(EVF_TIME, TYPE_FLOAT, 1, 0, &times.front());
	}
	else  
	{
		renderDataObject->RemoveStream(EVF_TEXCOORD1);
		renderDataObject->RemoveStream(EVF_TIME);
	}	
}
void ParticleRenderGroup::ResizeArrays(uint32 particlesCount)
{
	vertices.resize(particlesCount * POINTS_PER_PARTICLE * 3); // 4 vertices per each particle, 3 coords per vertex.
	texcoords.resize(particlesCount * POINTS_PER_PARTICLE * 2); // 4 texture coords per particle, 2 values per texture coord.
	colors.resize(particlesCount * POINTS_PER_PARTICLE);	
	
	if (enableFrameBlend)
	{
		texcoords2.resize(particlesCount * POINTS_PER_PARTICLE * 2);
		times.resize(particlesCount * POINTS_PER_PARTICLE); //single time value per vertex
	}
}

void ParticleRenderGroup::ClearArrays()
{
	currParticlesCount = 0;
	vertices.clear();
	texcoords.clear();
	colors.clear();
	texcoords2.clear();
	times.clear();
}



void ParticleRenderObject::PrepareRenderData(Camera * camera)
{
	for (int32 i=0, sz = renderGroupCache.size(); i<sz; ++i)
		renderGroupCache[i]->ClearArrays();
	renderBatchArray.clear();
	
	Matrix4 * transformPtr = GetWorldTransformPtr();	
	if (!transformPtr)
		return;

	currCamDirection = camera->GetDirection();

	/*prepare effect basises*/
	const Matrix4 &mv = camera->GetMatrix();
	basisVectors[0] = Vector3(mv._01, mv._11, mv._21);
	basisVectors[1] = Vector3(mv._00, mv._10, mv._20);
	basisVectors[0].Normalize();
	basisVectors[1].Normalize();	
	Vector3 ex(transformPtr->_00, transformPtr->_01, transformPtr->_02);
	Vector3 ey(transformPtr->_10, transformPtr->_11, transformPtr->_12);
	Vector3 ez(transformPtr->_20, transformPtr->_21, transformPtr->_22);
	ex.Normalize();
	ey.Normalize();
	ez.Normalize();
	basisVectors[2] = ey; basisVectors[3] = ez;
	basisVectors[4] = ez; basisVectors[5] = ex;
	basisVectors[6] = ey; basisVectors[7] = ex;	

	NMaterial *currMaterial = NULL;
	int32 renderGroupCount = 0;	
	ParticleRenderGroup *currRenderGroup = NULL;
	int32 maxParticlesPerBatch = 0;
	for (List<ParticleGroup>::iterator it = effectData->groups.begin(), e=effectData->groups.end(); it!=e; ++it)
	{
		const ParticleGroup& currGroup = (*it);
		if (!currGroup.material) continue; //if no material was set up - dont draw
		if (!currGroup.head) continue; //skip empty group

		//start new batch if needed
		if (currGroup.material!=currMaterial) 
		{
			if (currRenderGroup&&(currRenderGroup->currParticlesCount>maxParticlesPerBatch))
				maxParticlesPerBatch = currRenderGroup->currParticlesCount;
				
			renderGroupCount++;
			if (renderGroupCache.size()<renderGroupCount)
			{
				currRenderGroup = new ParticleRenderGroup();
				currRenderGroup->renderBatch = new RenderBatch();
				currRenderGroup->renderBatch->SetRenderDataObject(new RenderDataObject());
				renderGroupCache.push_back(currRenderGroup);
			}	
			else
				currRenderGroup=renderGroupCache[renderGroupCount-1];
			
			currRenderGroup->enableFrameBlend = currGroup.layer->enableFrameBlend;
			currRenderGroup->renderBatch->SetMaterial(currMaterial);
		}

	}
	
	
	int32 currParticleIndices = indices.size()/INDICES_PER_PARTICLE;
	if (maxParticlesPerBatch>currParticleIndices)
	{
		indices.resize(maxParticlesPerBatch*INDICES_PER_PARTICLE);
		for (;currParticleIndices<maxParticlesPerBatch; currParticleIndices++)
		{
			indices[currParticleIndices*INDICES_PER_PARTICLE+0] = currParticleIndices*POINTS_PER_PARTICLE+0;
			indices[currParticleIndices*INDICES_PER_PARTICLE+1] = currParticleIndices*POINTS_PER_PARTICLE+1;
			indices[currParticleIndices*INDICES_PER_PARTICLE+2] = currParticleIndices*POINTS_PER_PARTICLE+2;
			indices[currParticleIndices*INDICES_PER_PARTICLE+3] = currParticleIndices*POINTS_PER_PARTICLE+2;
			indices[currParticleIndices*INDICES_PER_PARTICLE+4] = currParticleIndices*POINTS_PER_PARTICLE+1;
			indices[currParticleIndices*INDICES_PER_PARTICLE+5] = currParticleIndices*POINTS_PER_PARTICLE+3; //preserve order
		}
	}
	for (int32 i=0; i<renderGroupCount; ++i)
	{
		if (renderGroupCache[i]->currParticlesCount)
		{
			renderGroupCache[i]->UpdateRenderBatch();
			renderGroupCache[i]->renderBatch->SetIndexCount(renderGroupCache[i]->currParticlesCount*INDICES_PER_PARTICLE);		
			renderGroupCache[i]->renderBatch->GetRenderDataObject()->SetIndices(EIF_16, (uint8*)(&indices.front()), renderGroupCache[i]->currParticlesCount*INDICES_PER_PARTICLE);
			renderBatchArray.push_back(renderGroupCache[i]->renderBatch);
		}
		
	}
	
}



void ParticleRenderObject::AppendParticleGroup(ParticleGroup &group, ParticleRenderGroup *renderGroup)
{
	//prepare basis indexes
	int32 basisCount = 0;
	int32 basises[4]; //4 basises max per particle
	bool worldAlign = group.layer->particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_WORLD_ALIGN;
	if (group.layer->particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
		basises[basisCount++] = 0;
	if (group.layer->particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_X_FACING)
		basises[basisCount++] = worldAlign?4:1;
	if (group.layer->particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_Y_FACING)
		basises[basisCount++] = worldAlign?5:2;
	if (group.layer->particleOrientation&ParticleLayer::PARTICLE_ORIENTATION_Z_FACING)
		basises[basisCount++] = worldAlign?6:3;

	//prepare arrays in group
	renderGroup->ResizeArrays(renderGroup->currParticlesCount + group.activeParticleCount*basisCount);

	int32 currVerticesCount = renderGroup->currParticlesCount*POINTS_PER_PARTICLE;
	Particle *current = group.head;
	Vector3 particlePos[4];
	while (current)
	{
		for (int32 i=0; i<basisCount; i++)
		{
			Vector3 ex = basisVectors[basises[i]*2];
			Vector3 ey = basisVectors[basises[i]*2+1];
			//TODO: rethink this code - it should be easier
			if (group.layer->isLong) //note that for now it's just a copy of long implementatio - later rethink it;
			{
				ex = current->speed;
				float32 vel = ex.Length();
				ey = ex.CrossProduct(currCamDirection);
				ey.Normalize();				
				ex *=(group.layer->scaleVelocityBase/vel+group.layer->scaleVelocityFactor); //optimized ex=(svBase+svFactor*vel)/vel
			}			

			float32 sin_angle;
			float32 cos_angle;
			SinCosFast(current->angle, sin_angle, cos_angle);
			Vector3 left = ex*cos_angle+ey*sin_angle;
			Vector3 right = -left;
			Vector3 top = ey*cos_angle + ex*sin_angle;
			Vector3 bot = -top;
			
			left*=0.5f*current->currSize.x*(1+group.layer->layerPivotPoint.x);
			right*=0.5f*current->currSize.x*(1-group.layer->layerPivotPoint.x);
			top*=0.5f*current->currSize.y*(1+group.layer->layerPivotPoint.y);
			bot*=0.5f*current->currSize.y*(1-group.layer->layerPivotPoint.y);			

			particlePos[0] = current->position+left+top;
			particlePos[1] = current->position+right+top;
			particlePos[2] = current->position+left+bot;			
			particlePos[3] = current->position+right+bot;
			
			memcpy(&renderGroup->vertices[currVerticesCount*3], &particlePos[0], sizeof(Vector3) * 4);
			
			float32 *pT = group.layer->sprite->GetTextureVerts(current->frame);
			memcpy(&renderGroup->texcoords[currVerticesCount*2], pT, sizeof(float32) * 8);
			
			Color currColor = current->color;
			if (group.layer->colorOverLife)
				currColor = group.layer->colorOverLife->GetValue(current->life/current->lifeTime);
			if (group.layer->alphaOverLife)
				currColor.a = group.layer->alphaOverLife->GetValue(current->life/current->lifeTime);

			uint32 color = (((uint32)(currColor.a*255.f))<<24) |  (((uint32)(currColor.b*255.f))<<16) |
				(((uint32)(currColor.g*255.f))<<8) | ((uint32)(currColor.r*255.f));
			
			renderGroup->colors[currVerticesCount] = color;
			renderGroup->colors[currVerticesCount+1] = color;
			renderGroup->colors[currVerticesCount+2] = color;
			renderGroup->colors[currVerticesCount+3] = color;

			if (renderGroup->enableFrameBlend)
			{
				int32 nextFrame = current->frame+1;
				if (nextFrame >= group.layer->sprite->GetFrameCount())
				{
					if (group.layer->loopSpriteAnimation)
						nextFrame = 0;
					else
						nextFrame = group.layer->sprite->GetFrameCount()-1;
				}
				pT = group.layer->sprite->GetTextureVerts(nextFrame);

				memcpy(&renderGroup->texcoords2[currVerticesCount], pT, sizeof(float32) * 8);				
				renderGroup->times[currVerticesCount] = current->animTime;
				renderGroup->times[currVerticesCount+1] = current->animTime;
				renderGroup->times[currVerticesCount+2] = current->animTime;
				renderGroup->times[currVerticesCount+3] = current->animTime;
			}
			renderGroup->currParticlesCount++;
			currVerticesCount+=4;			
		}
	}
}


}//namespace