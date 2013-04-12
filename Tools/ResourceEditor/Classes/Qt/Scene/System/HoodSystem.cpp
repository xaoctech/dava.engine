#include "Scene/System/HoodSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/SceneEditorProxy.h"

HoodSystem::HoodSystem(DAVA::Scene * scene, SceneCameraSystem *camSys)
	: DAVA::SceneSystem(scene)
	, cameraSystem(camSys)
	, curType(EM_MODE_MOVE)
	, locked(false)
	, invisible(true)
	, moseOverAxis(EM_AXIS_NONE)
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

HoodSystem::~HoodSystem()
{
	RemoveCollObjects();

	delete collWorld;
	delete collDebugDraw;
	delete collBroadphase;
	delete collDispatcher;
	delete collConfiguration;
}

DAVA::Vector3 HoodSystem::GetPosition() const
{
	return (curPos + curOffset);
}

void HoodSystem::SetPosition(const DAVA::Vector3 &pos)
{
	if(curPos != pos)
	{
		curPos = pos;
		curOffset = DAVA::Vector3(0, 0, 0);
		UpdateCollObjectsPos();
	}
}

void HoodSystem::MovePosition(const DAVA::Vector3 &offset)
{
	curOffset = offset;
	UpdateCollObjectsPos();
}

void HoodSystem::SetType(int type)
{
	if(curType != type && type != 0)
	{
		curType = type;
	}
}

void HoodSystem::SetScale(DAVA::float32 scale)
{
	if(curScale != scale)
	{
		curScale = scale;
		UpdateCollObjectsScale();
	}
}

int HoodSystem::GetType() const
{
	return curType;
}

void HoodSystem::Update(float timeElapsed)
{
	if(!locked && !invisible)
	{
		// scale hood depending on current camera position
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		SetScale((GetPosition() - camPosition).Length() / 20.f);
	}
}

void HoodSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(!locked && !invisible)
	{
		// get intersected items in the line from camera to current mouse position
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(event->point);
		DAVA::Vector3 traceTo = camPosition + camToPointDirection * 1000.0f;

		btVector3 btFrom(camPosition.x, camPosition.y, camPosition.z);
		btVector3 btTo(traceTo.x, traceTo.y, traceTo.z);

		btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
		collWorld->rayTest(btFrom, btTo, btCallback);

		// before checking result mark that there is no hood axis under mouse
		moseOverAxis = EM_AXIS_NONE;

		if(btCallback.hasHit())
		{
			const DAVA::Vector<HoodObject*>* curHoodObjects = GetCurrentHood();
			for(size_t i = 0; i < curHoodObjects->size(); ++i)
			{
				HoodObject *hObj = curHoodObjects->operator[](i);
				if(hObj->btObject == btCallback.m_collisionObjects[0])
				{
					// mark that mouse is over one of hood axis
					moseOverAxis = hObj->representAxis;
					break;
				}
			}
		}
	}
}

void HoodSystem::Draw()
{
	if(!invisible)
	{
		int oldState = DAVA::RenderManager::Instance()->GetState();
		DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);

		const DAVA::Vector<HoodObject*>* curHoodObjects = GetCurrentHood();
		int showAsSelected = curAxis;

		if(EM_AXIS_NONE != moseOverAxis)
		{
			showAsSelected = moseOverAxis;
		}

		for (size_t i = 0; i < curHoodObjects->size(); i++)
		{
			HoodObject *obj = curHoodObjects->operator[](i);

			if((showAsSelected & obj->representAxis) == showAsSelected)
			{
				// selected axis - yellow
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1, 1, 0, 1));
			}
			else
			{
				// axis by there color
				if(obj->colorAxis & EM_AXIS_X) DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1, 0, 0, 1));
				else if(obj->colorAxis & EM_AXIS_Y) DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1, 0, 1));
				else if(obj->colorAxis & EM_AXIS_Z) DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 0, 1, 1));
			}

			obj->Draw();
		}

		// debug draw collision objects
		// collWorld->debugDrawWorld();

		DAVA::RenderManager::Instance()->SetState(oldState);
	}
}

void HoodSystem::CreateCollObjects()
{
	HoodObject *hObj;

	RemoveCollObjects();

	// create move hood
	{
		hObj = CreateLine(DAVA::Vector3(1, 0, 0), DAVA::Vector3(4, 0, 0));
		hObj->representAxis = EM_AXIS_X;
		hObj->colorAxis = EM_AXIS_X;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 1, 0), DAVA::Vector3(0, 4, 0));
		hObj->representAxis = EM_AXIS_Y;
		hObj->colorAxis = EM_AXIS_Y;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 0, 1), DAVA::Vector3(0, 0, 4));
		hObj->representAxis = EM_AXIS_Z;
		hObj->colorAxis = EM_AXIS_Z;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(2, 0, 0), DAVA::Vector3(2, 2, 0));
		hObj->representAxis = EM_AXIS_XY;
		hObj->colorAxis = EM_AXIS_X;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 2, 0), DAVA::Vector3(2, 2, 0));
		hObj->representAxis = EM_AXIS_XY;
		hObj->colorAxis = EM_AXIS_Y;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(2, 0, 0), DAVA::Vector3(2, 0, 2));
		hObj->representAxis = EM_AXIS_XZ;
		hObj->colorAxis = EM_AXIS_X;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 0, 2), DAVA::Vector3(2, 0, 2));
		hObj->representAxis = EM_AXIS_XZ;
		hObj->colorAxis = EM_AXIS_Z;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 2, 0), DAVA::Vector3(0, 2, 2));
		hObj->representAxis = EM_AXIS_YZ;
		hObj->colorAxis = EM_AXIS_Y;
		moveHood.push_back(hObj);

		hObj = CreateLine(DAVA::Vector3(0, 0, 2), DAVA::Vector3(0, 2, 2));
		hObj->representAxis = EM_AXIS_YZ;
		hObj->colorAxis = EM_AXIS_Z;
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

void HoodSystem::UpdateCollObjectsPos()
{
	DAVA::Vector3 pos = curPos + curOffset;

	for (size_t i = 0; i < moveHood.size(); i++)
	{
		moveHood[i]->UpdatePos(pos);
	}

	for (size_t i = 0; i < rotateHood.size(); i++)
	{
		rotateHood[i]->UpdatePos(pos);
	}

	for (size_t i = 0; i < scaleHood.size(); i++)
	{
		scaleHood[i]->UpdatePos(pos);
	}
}

void HoodSystem::UpdateCollObjectsScale()
{
	for (size_t i = 0; i < moveHood.size(); i++)
	{
		moveHood[i]->UpdateScale(curScale);
	}

	for (size_t i = 0; i < rotateHood.size(); i++)
	{
		rotateHood[i]->UpdateScale(curScale);
	}

	for (size_t i = 0; i < scaleHood.size(); i++)
	{
		scaleHood[i]->UpdateScale(curScale);
	}

	collWorld->updateAabbs();
}

void HoodSystem::RemoveCollObjects()
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

HoodObject* HoodSystem::CreateLine(const DAVA::Vector3 &from, const DAVA::Vector3 &to, DAVA::float32 weight)
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
	ret->btShape = new btCylinderShapeX(btVector3(length / 2, weight / 2, weight / 2)); 
	ret->btObject = new btCollisionObject();
	ret->btObject->setCollisionShape(ret->btShape);
	ret->initialOffset = DAVA::Vector3(length / 2, 0, 0);
	ret->initialPos = from;
	ret->curOffset = curPos;
	ret->rotate.Identity();

	direction.Normalize();
	rotateNormal = axisX.CrossProduct(direction);

	// do we need rotation
	if(!rotateNormal.IsZero())
	{
		rotateNormal.Normalize();
		rotateAngle = acosf(axisX.DotProduct(direction));

		ret->rotate.CreateRotation(rotateNormal, -rotateAngle);
	}

	if(0 != rotateAngle)
	{
		btTransform trasf;
		trasf.setIdentity();
		trasf.setRotation(btQuaternion(btVector3(rotateNormal.x, rotateNormal.y, rotateNormal.z), rotateAngle));
		ret->btObject->setWorldTransform(trasf);
	}

	ret->UpdateScale(1.0f);
	ret->UpdatePos(curPos);

	collWorld->addCollisionObject(ret->btObject);
	return ret;
}

void HoodSystem::Destroy(HoodObject *hoodObject)
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

void HoodSystem::SetSelectedAxis(int axis)
{
	if(EM_AXIS_NONE != axis)
	{
		curAxis = axis;
	}
}

int HoodSystem::GetSelectedAxis() const
{
	return curAxis;
}

int HoodSystem::GetPassingAxis() const
{
	return moseOverAxis;
}

const DAVA::Vector<HoodObject*>* HoodSystem::GetCurrentHood() const
{
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

	return curHoodObjects;
}

void HoodSystem::Lock()
{
	locked = true;
}

void HoodSystem::Unlock()
{
	locked = false;
}

void HoodSystem::Show()
{
	invisible = false;
}

void HoodSystem::Hide()
{
	invisible = true;
}

// -------------------------------------------------------------------------------------------------
// HoodObjects
// -------------------------------------------------------------------------------------------------

void HoodObject::UpdatePos(const DAVA::Vector3 &pos)
{
	curOffset = pos;

	DAVA::Vector3 curPos = scaledOffset + (initialPos * scale) + curOffset;
	btTransform transf = btObject->getWorldTransform();
	transf.setOrigin(btVector3(curPos.x, curPos.y, curPos.z));
	btObject->setWorldTransform(transf);
}

void HoodObject::UpdateScale(const DAVA::float32 &scale)
{
	this->scale = scale;

	btShape->setLocalScaling(btVector3(scale, scale, scale));
	scaledOffset = DAVA::MultiplyVectorMat3x3(initialOffset * scale, rotate);

	UpdatePos(curOffset);
}

void HoodLine::Draw()
{
	DAVA::RenderHelper::Instance()->DrawLine((from * scale) + curOffset, (to * scale) + curOffset);
}