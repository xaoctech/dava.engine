#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"
#include "Render/Highlevel/Camera.h"
#include "Utils/Random.h"

namespace DAVA
{

REGISTER_CLASS(ParticleEmitter3D);

ParticleEmitter3D::ParticleEmitter3D()
{
	is3D = true;
}

void ParticleEmitter3D::AddLayer(ParticleLayer * layer)
{
	// Only ParticleLayer3Ds are allowed on this level.
	if (dynamic_cast<ParticleLayer3D*>(layer))
	{
		ParticleEmitter::AddLayer(layer);
	}
}
	
void ParticleEmitter3D::AddLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove)
{
	// Only ParticleLayer3Ds are allowed on this level.
	if (dynamic_cast<ParticleLayer3D*>(layer))
	{
		ParticleEmitter::AddLayer(layer, layerToMoveAbove);
	}
}

void ParticleEmitter3D::RenderUpdate(Camera *camera, float32 timeElapsed)
{
	eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
	eBlendMode destMode = RenderManager::Instance()->GetDestBlend();

	Draw(camera);
    
	RenderManager::Instance()->SetBlendMode(srcMode, destMode);
}

void ParticleEmitter3D::Draw(Camera * camera)
{
	//Dizz: now layer->Draw is called from ParticleLayerBatch
}

void ParticleEmitter3D::PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex)
{
	Vector3 tempPosition = Vector3();
	Matrix4 * worldTransformPtr = GetWorldTransformPtr();
	Matrix3 rotationMatrix;
	rotationMatrix.Identity();

	if(worldTransformPtr)
	{
		tempPosition = worldTransformPtr->GetTranslationVector();
		rotationMatrix = Matrix3(*worldTransformPtr);;
	}

	//Vector3 tempPosition = particlesFollow ? Vector3() : position;
    if (emitterType == EMITTER_POINT)
    {
        particle->position = tempPosition;
    }
    else if (emitterType == EMITTER_RECT)
    {
        // TODO: add emitter angle support
        float32 rand05_x = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_y = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_z = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = Vector3(size->GetValue(time).x * rand05_x, size->GetValue(time).y * rand05_y, size->GetValue(time).z * rand05_z);
        particle->position = tempPosition + lineDirection;
    }

    if (emitterType == EMITTER_ONCIRCLE)
    {
		// For "Circle" emitters the calculation is different.
		PrepareEmitterParametersOnCircle(particle, velocity, emitIndex, tempPosition, rotationMatrix);
	}
	else
	{
		PrepareEmitterParametersGeneric(particle, velocity, emitIndex, tempPosition, rotationMatrix);
	}

	if(worldTransformPtr)
	{
		Matrix4 newTransform = *worldTransformPtr;
		newTransform._30 = newTransform._31 = newTransform._32 = 0;
		particle->direction = particle->direction*newTransform;
	}
}
	
void ParticleEmitter3D::PrepareEmitterParametersOnCircle(Particle * particle, float32 velocity,
														 int32 emitIndex, const Vector3& tempPosition,
														 const Matrix3& rotationMatrix)
{
	// Emit ponts from the circle in the XY plane.
	float32 curRadius = 1.0f;
	if (radius)
	{
		curRadius = radius->GetValue(time);
	}

	float32 curAngle = PI_2 * (float32)Random::Instance()->RandFloat();
	float sinAngle = 0.0f;
	float cosAngle = 0.0f;
	SinCosFast(curAngle, sinAngle, cosAngle);

	Vector3 directionVector(curRadius * cosAngle,
							curRadius * sinAngle,
							0.0f);

	particle->position = (tempPosition + directionVector) * rotationMatrix;
	particle->speed = velocity;

	// Calculate Z value.
	const float TANGENT_EPSILON = 1E-4;
	if (this->emissionRange)
	{
		float32 emissionRangeValue = DegToRad(emissionRange->GetValue(time));
		SinCosFast(emissionRangeValue, sinAngle, cosAngle);
		if (fabs(cosAngle) < TANGENT_EPSILON)
		{
			// Reset the direction vector.
			directionVector.x = 0;
			directionVector.y = 0;
			directionVector.z = -curRadius / 2 + (float32)Random::Instance()->RandFloat() * curRadius;
		}
		else
		{
			float32 zValue = (curRadius * sinAngle / cosAngle);
			directionVector.z = -zValue / 2 + (float32)Random::Instance()->RandFloat() * zValue;
		}
	}

	particle->direction = directionVector;
}

void ParticleEmitter3D::PrepareEmitterParametersGeneric(Particle * particle, float32 velocity,
														int32 emitIndex, const Vector3& tempPosition,
														const Matrix3& rotationMatrix)
{
    Vector3 vel = Vector3(1.0f, 0.0f, 0.0f);
    if(emissionVector)
	{
		// Yuri Coder, 2013/04/12. Need to invert the directions in the emission vector, since
		// their coordinates are in the opposite directions for the Particles Editor.
        vel = emissionVector->GetValue(0) * -1.0f;
		vel = vel*rotationMatrix;
	}

    Vector3 rotVect(0, 0, 1);
    float32 phi = PI*2*(float32)Random::Instance()->RandFloat();
    if(vel.x != 0)
    {
        rotVect.y = sinf(phi);
        rotVect.z = cosf(phi);
        rotVect.x = - rotVect.y*vel.y/vel.x - rotVect.z*vel.z/vel.x;
    }
    else if(vel.y != 0)
    {
        rotVect.x = cosf(phi);
        rotVect.z = sinf(phi);
        rotVect.y = - rotVect.z*vel.z/vel.y;
    }
    else if(vel.z != 0)
    {
        rotVect.x = cosf(phi);
        rotVect.y = sinf(phi);
        rotVect.z = 0;
    }
    rotVect.Normalize();
	
    float32 range = 0;
    if(emissionRange)
        range = DegToRad(emissionRange->GetValue(time) + angle);
    float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f;
	
    Vector3 q_v(rotVect*sinf(range*rand05/2));
    float32 q_w = cosf(range*rand05/2);
	
    Vector3 q1_v(q_v);
    float32 q1_w = -q_w;
    q1_v /= (q_v.SquareLength() + q_w*q_w);
    q1_w /= (q_v.SquareLength() + q_w*q_w);
	
    Vector3 v_v(vel);
	
    Vector3 qv_v = q_v.CrossProduct(v_v) + q_w*v_v;
    float32 qv_w = - q_v.DotProduct(v_v);
	
    Vector3 qvq1_v = qv_v.CrossProduct(q1_v) + qv_w*q1_v + q1_w*qv_v;
	
	Vector3 speed = qvq1_v * velocity;
	particle->speed = speed.Length();
    particle->direction = speed/particle->speed;
	if (particle->direction.x <= EPSILON && particle->direction.x >= -EPSILON)
		particle->direction.x = 0.f;
	if (particle->direction.y <= EPSILON && particle->direction.y >= -EPSILON)
		particle->direction.y = 0.f;
	if (particle->direction.z <= EPSILON && particle->direction.z >= -EPSILON)
		particle->direction.z = 0.f;

	// Yuri Coder, 2013/03/26. After discussion with Ivan it appears this angle
	// calculation is incorrect. TODO: return to this code later on.
    particle->angle = atanf(particle->direction.z/particle->direction.x);
}

void ParticleEmitter3D::LoadParticleLayerFromYaml(YamlNode* yamlNode, bool isLong)
{
	ParticleLayer3D* layer = new ParticleLayer3D(this);
	layer->SetLong(isLong);

	AddLayer(layer);
	layer->LoadFromYaml(configPath, yamlNode);
	SafeRelease(layer);
}

bool ParticleEmitter3D::Is3DFlagCorrect()
{
	// For ParticleEmitter3D is3D flag must be set to TRUE.
	return (is3D == true);
}

RenderObject * ParticleEmitter3D::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitter3D>(this), "Can clone only ParticleEmitter3D");
		newObject = new ParticleEmitter3D();
	}

	((ParticleEmitter3D*)newObject)->LoadFromYaml(configPath);

	return newObject;
}

}

