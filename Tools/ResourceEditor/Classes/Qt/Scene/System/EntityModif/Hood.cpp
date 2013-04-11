#include "Scene/System/EntityModif/Hood.h"
#include "Scene/System/EntityModifSystem.h"
#include "Scene/System/SceneCollisionSystem.h"
#include "Scene/System/SceneCameraSystem.h"

Hood::Hood()
	: curType(EM_MODE_MOVE)
{
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);

	collConfiguration = new btDefaultCollisionConfiguration();
	collDispatcher = new btCollisionDispatcher(collConfiguration);
	collBroadphase = new btAxisSweep3(worldMin,worldMax);
	collDebugDraw = new SceneCollisionDebugDrawer();
	collDebugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	collWorld = new btCollisionWorld(collDispatcher, collBroadphase, collConfiguration);
	collWorld->setDebugDrawer(collDebugDraw);

	CreateCollObjects();
}

Hood::~Hood()
{
	RemoveCollObjects();

	delete collWorld;
	delete collDebugDraw;
	delete collBroadphase;
	delete collDispatcher;
	delete collConfiguration;
}

void Hood::SetPosition(const DAVA::Vector3 &pos)
{
	if(curPos != pos)
	{
		curPos = pos;
		UpdateCollObjects();
	}
}

void Hood::MovePosition(const DAVA::Vector3 &offset)
{
	curPos += offset;
	UpdateCollObjects();
}

void Hood::SetType(int type)
{
	if(curType != type)
	{
		curType = type;
	}
}

void Hood::SetScale(DAVA::float32 scale)
{
	if(curScale != scale)
	{
		curScale = scale;
		UpdateCollObjects();

		printf("%g\n", scale);
	}
}

int Hood::GetType() const
{
	return curType;
}

void Hood::Draw() const
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);

	//collWorld->debugDrawWorld();

	const DAVA::Vector<HoodObject*>* curHoodObjects = NULL;

	switch (curType)
	{
	case EM_MODE_MOVE:
		curHoodObjects = &moveHood;
		break;
	case EM_MODE_ROTATE:
		curHoodObjects = &rotateHood;
		break;
	case EM_MODE_SCALE:
		curHoodObjects = &scaleHood;
		break;
	default:
		curHoodObjects = &normalHood;
		break;
	}

	for (size_t i = 0; i < curHoodObjects->size(); i++)
	{
		HoodObject *obj = curHoodObjects->operator[](i);

		if(curType & obj->representAxis)
		{
			// selected axis - yellow
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1, 1, 0, 1));
		}
		else
		{
			if(obj->representAxis & EM_AXIS_X)
			{
				// x axis red
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1, 0, 0, 1));
			}
			else if(obj->representAxis & EM_AXIS_Y)
			{
				// y axis green
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1, 0, 1));
			}
			else if(obj->representAxis & EM_AXIS_Z)
			{
				// z axis blue
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0, 1, 1));
			}
		}

		obj->Draw();
	}

	DAVA::RenderManager::Instance()->SetState(oldState);
}

void Hood::RayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	btVector3 btFrom(from.x, from.y, from.z);
	btVector3 btTo(to.x, to.y, to.z);

	btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
	collWorld->rayTest(btFrom, btTo, btCallback);
}

void Hood::CreateCollObjects()
{
	HoodObject *hObj;

	RemoveCollObjects();

	// create move hood
	{
		hObj = CreateLine(DAVA::Vector3(1, 0, 0), DAVA::Vector3(4, 0, 0));
		hObj->representAxis = EM_AXIS_X;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 1, 0), DAVA::Vector3(0, 4, 0));
		hObj->representAxis = EM_AXIS_Y;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 0, 1), DAVA::Vector3(0, 0, 4));
		hObj->representAxis = EM_AXIS_Z;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(2, 0, 0), DAVA::Vector3(2, 2, 0));
		hObj->representAxis = EM_AXIS_XY;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 2, 0), DAVA::Vector3(2, 2, 0));
		hObj->representAxis = EM_AXIS_XY;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(2, 0, 0), DAVA::Vector3(2, 0, 2));
		hObj->representAxis = EM_AXIS_XZ;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 0, 2), DAVA::Vector3(2, 0, 2));
		hObj->representAxis = EM_AXIS_XZ;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 2, 0), DAVA::Vector3(0, 2, 2));
		hObj->representAxis = EM_AXIS_YZ;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 0, 2), DAVA::Vector3(0, 2, 2));
		hObj->representAxis = EM_AXIS_YZ;
		moveHood.push_back(hObj);
	}

	// create rotate hood
	{
		// TODO:
		// ...
		// 
	}

	// create scale hood
	{
		// TODO:
		// ...
		// 
	}
}

void Hood::UpdateCollObjects()
{
	for (size_t i = 0; i < moveHood.size(); i++)
	{
		moveHood[i]->UpdatePos(curPos);
	}

	for (size_t i = 0; i < rotateHood.size(); i++)
	{
		rotateHood[i]->UpdatePos(curPos);
	}

	for (size_t i = 0; i < scaleHood.size(); i++)
	{
		scaleHood[i]->UpdatePos(curPos);
	}
}

void Hood::RemoveCollObjects()
{
	for (size_t i = 0; i < moveHood.size(); i++)
	{
		Destroy(moveHood[i]);
	}
	moveHood.clear();

	for (size_t i = 0; i < rotateHood.size(); i++)
	{
		Destroy(rotateHood[i]);
	}
	rotateHood.clear();

	for (size_t i = 0; i < scaleHood.size(); i++)
	{
		Destroy(scaleHood[i]);
	}
	scaleHood.clear();
}

HoodObject* Hood::CreateLine(const DAVA::Vector3 &from, const DAVA::Vector3 &to, DAVA::float32 weight)
{
	HoodLine* ret = new HoodLine();

	DAVA::Vector3 direction = (to - from);
	DAVA::float32 length = direction.Length();
	DAVA::Vector3 axisX(1, 0, 0);
	DAVA::Vector3 rotateNormal;
	DAVA::float32 rotateAngle = 0;

	// initially create object on x axis
	ret->from = from;
	ret->to = to;
	ret->btShape = new btBoxShape(btVector3(length / 2, weight / 2, weight / 2)); 
	ret->btObject = new btCollisionObject();
	ret->btObject->setCollisionShape(ret->btShape);
	ret->initialOffset = DAVA::Vector3(length / 2, 0, 0);

	direction.Normalize();
	rotateNormal = axisX.CrossProduct(direction);

	// do we need rotation
	if(!rotateNormal.IsZero())
	{
		rotateNormal.Normalize();
		rotateAngle = acosf(axisX.DotProduct(direction));

		DAVA::Matrix4 rotate;
		rotate.Identity();
		rotate.CreateRotation(rotateNormal, -rotateAngle);

		ret->initialOffset = DAVA::MultiplyVectorMat3x3(ret->initialOffset, rotate);
	}

	ret->initialOffset += from;

	btTransform trasf;
	trasf.setIdentity();

	if(0 != rotateAngle)
	{
		trasf.setRotation(btQuaternion(btVector3(rotateNormal.x, rotateNormal.y, rotateNormal.z), rotateAngle));
	}

	trasf.setOrigin(btVector3(ret->initialOffset.x + curPos.x, ret->initialOffset.y + curPos.y, ret->initialOffset.z + curPos.z));

	ret->btObject->setWorldTransform(trasf);
	collWorld->addCollisionObject(ret->btObject);

	return ret;
}

void Hood::Destroy(HoodObject *hoodObject)
{
	if(NULL != hoodObject)
	{
		if(NULL != hoodObject->btObject)
		{
			collWorld->removeCollisionObject(hoodObject->btObject);
			delete hoodObject->btObject;
		}

		if(NULL != hoodObject->btShape)
		{
			delete hoodObject->btShape;
		}

		delete hoodObject;
	}
}

// -------------------------------------------------------------------------------------------------
// HoodObjects
// -------------------------------------------------------------------------------------------------

void HoodObject::UpdatePos(const DAVA::Vector3 &pos)
{
	curOffset = pos;

	DAVA::Vector3 curPos = initialOffset + curOffset;
	btTransform transf = btObject->getWorldTransform();
	transf.setOrigin(btVector3(curPos.x, curPos.y, curPos.z));
	btObject->setWorldTransform(transf);
}

void HoodLine::Draw()
{
	DAVA::RenderHelper::Instance()->DrawLine(from + curOffset, to + curOffset);
}