#include "Scene/System/EntityModif/Hood.h"
#include "Scene/System/SceneCollisionSystem.h"
#include "Scene/System/SceneCameraSystem.h"

Hood::Hood(SceneCameraSystem *camSys)
	: curType(HOOD_NORMAL)
	, cameraSystem(camSys)
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

	UpdateCollObjects();
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

void Hood::SetPosition(DAVA::Vector3 pos)
{
	if(curPos != pos)
	{
		curPos = pos;
		UpdateCollObjects();
	}
}

void Hood::SetType(int type)
{
	if(curType != type)
	{
		curType = type;
		UpdateCollObjects();
	}
}

void Hood::SetScale(DAVA::float32 scale)
{
	if(curScale != scale)
	{
		curScale = scale;
		UpdateCollObjects();
	}
}

int Hood::GetType() const
{
	return curType;
}

void Hood::Draw() const
{
	DAVA::Vector3 c(0, 0, 0);
	DAVA::Vector3 x(3.0f, 0, 0);
	DAVA::Vector3 y(0, 3.0f, 0);
	DAVA::Vector3 z(0, 0, 3.0f);

	c += curPos;
	x += curPos;
	y += curPos;
	z += curPos;

	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);

	DAVA::RenderManager::Instance()->SetColor(1, 0, 0, 1);
	DAVA::RenderHelper::Instance()->DrawLine(c, x);

	DAVA::RenderManager::Instance()->SetColor(0, 1, 0, 1);
	DAVA::RenderHelper::Instance()->DrawLine(c, y);

	DAVA::RenderManager::Instance()->SetColor(0, 0, 1, 1);
	DAVA::RenderHelper::Instance()->DrawLine(c, z);

	DAVA::RenderManager::Instance()->SetState(oldState);
	
	collWorld->debugDrawWorld();
}

void Hood::RayTest(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	btVector3 btFrom(from.x, from.y, from.z);
	btVector3 btTo(to.x, to.y, to.z);

	btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
	collWorld->rayTest(btFrom, btTo, btCallback);

	if(btCallback.hasHit()) 
	{
		intersected = true;
	}
	else
	{
		intersected = false;
	}
}

void Hood::CreateCollObjects()
{
	RemoveCollObjects();


	AddLineShape(DAVA::Vector3(0, 0, 0), DAVA::Vector3(3, 0, 0));
	AddLineShape(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 3, 0));
	AddLineShape(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, 3));
	//AddLineShape(DAVA::Vector3(3, 0, 0), DAVA::Vector3(0, 0, 3));
	//AddLineShape(DAVA::Vector3(0, 3, 0), DAVA::Vector3(0, 0, 3));
	//AddLineShape(DAVA::Vector3(3, 0, 0), DAVA::Vector3(0, 3, 0));

	/*
	switch(curType)
	{
	case HOOD_MOVE:
		{
		}
		break;

	default:
		break;
	}
	*/
}

void Hood::RemoveCollObjects()
{
	QMapIterator<btCollisionObject*, int> i(collObjects);
	while(i.hasNext())
	{
		i.next();

		collWorld->removeCollisionObject(i.key());
		delete i.key();
	}

	for(size_t j = 0; j < collShapes.size(); ++j)
	{
		delete collShapes[j];
	}

	collShapes.clear();
	collObjects.clear();
}

void Hood::AddLineShape(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	// we will create shape on x axis
	DAVA::Vector3 initial(1, 0, 0);

	DAVA::Vector3 dir = (to - from);
	DAVA::float32 length = dir.Length();
	DAVA::Vector3 normal;
	DAVA::float32 angle = 0;
	DAVA::Vector3 drawAround(length / 2, 0.05f, 0.05f);

	// create shape on x axis
	btCollisionShape *shape = new btBoxShape(btVector3(drawAround.x, drawAround.y, drawAround.z));
	btCollisionObject *obj = new btCollisionObject();
	obj->setCollisionShape(shape);

	dir.Normalize();
	normal = initial.CrossProduct(dir);
	if(!normal.IsZero())
	{
		DAVA::Vector3 fromN = from;

		if(!fromN.IsZero())
		{
			fromN.Normalize();
		}

		DAVA::float32 angleCos = initial.DotProduct(dir);
		angle = acosf(angleCos);
		normal.Normalize();

		DAVA::Matrix4 rotate;
		rotate.Identity();
		rotate.CreateRotation(normal, -angle);

		drawAround = DAVA::MultiplyVectorMat3x3(drawAround, rotate);
	}

	btTransform trasf;
	trasf.setIdentity();
	if(NULL != angle)
	{
		trasf.setIdentity();
		trasf.setRotation(btQuaternion(btVector3(normal.x, normal.y, normal.z), angle));
		obj->setWorldTransform(trasf);
	}
	trasf.setOrigin(btVector3(from.x + drawAround.x + curPos.x, from.y + drawAround.y + curPos.y, from.z + drawAround.z + curPos.z));
	obj->setWorldTransform(trasf);

	collWorld->addCollisionObject(obj);
}