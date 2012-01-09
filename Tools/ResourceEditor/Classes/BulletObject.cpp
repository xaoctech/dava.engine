/*
 *  BulletObject.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "BulletObject.h"
//#include "bullet/btBulletCollisionCommon.h"
#include "bullet/BulletCollision/CollisionShapes/btShapeHull.h"

BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *meshNode, const Matrix4 &pWorldTransform)
:collWorld(collisionWorld),
collisionPartTransform(&((Matrix4&)pWorldTransform))
{    
    btTransform startTransform;
    startTransform.setIdentity();
    float start_x = 0;
    float start_y = 0;
    float start_z = 0;

    startTransform.setOrigin(btVector3(btScalar(start_x),
                                       btScalar(start_y),
                                       btScalar(start_z)));
    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
	CreateShape(meshNode);
    collisionObject->setCollisionShape(shape);
    collisionWorld->addCollisionObject(collisionObject);
}

BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, LightNode *lightNode, const Matrix4 &pWorldTransform)
:collWorld(collisionWorld),
collisionPartTransform(&((Matrix4&)pWorldTransform))
{
    btTransform startTransform;
    startTransform.setIdentity();
    float start_x = 0;
    float start_y = 0;
    float start_z = 0;
	
    startTransform.setOrigin(btVector3(btScalar(start_x),
                                       btScalar(start_y),
                                       btScalar(start_z)));
    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
	CreateLightShape(lightNode->GetRadius());
    collisionObject->setCollisionShape(shape);
    collisionWorld->addCollisionObject(collisionObject);	
}


BulletObject::~BulletObject()
{
	if (collisionObject) 
	{
		collWorld->removeCollisionObject(collisionObject);
		SafeDelete(collisionObject);
    }
	collWorld = 0;
	collisionPartTransform = 0;
	SafeDelete(shape);
	SafeDelete(trimesh);
}

void BulletObject::CreateShape(MeshInstanceNode *meshNode)
{    
	trimesh = new btTriangleMesh();
	
	const Vector<StaticMesh*> & meshes = meshNode->GetMeshes();
	const Vector<int32> & indexes = meshNode->GetPolygonGroupIndexes();
	
	uint32 meshesSize = (uint32)meshes.size();

	if (meshesSize > 0)
	{
		for (uint32 k = 0; k < meshesSize; ++k)
		{
			PolygonGroup * pg = meshes[k]->GetPolygonGroup(indexes[k]);    
			
			int i;
			for (i = 0; i < pg->indexCount / 3; i++)
			{
				int index0 = pg->indexArray[i*3];
				int index1 = pg->indexArray[i*3+1];
				int index2 = pg->indexArray[i*3+2];
				Vector3 v;
				pg->GetCoord(index0, v);
//				v = v * meshNode->GetLocalTransform();
				btVector3 vertex0(v.x, v.y, v.z);
				pg->GetCoord(index1, v);
//				v = v * meshNode->GetLocalTransform();
				btVector3 vertex1(v.x, v.y, v.z);
				pg->GetCoord(index2, v);
//				v = v * meshNode->GetLocalTransform();
				btVector3 vertex2(v.x, v.y, v.z);
				
				trimesh->addTriangle(vertex0,vertex1,vertex2, false);
				triangles.push_back(Vector3(vertex0.x(),vertex0.y(),vertex0.z()));
				triangles.push_back(Vector3(vertex1.x(),vertex1.y(),vertex1.z()));
				triangles.push_back(Vector3(vertex2.x(),vertex2.y(),vertex2.z()));
			}
		}	
		shape = new btBvhTriangleMeshShape(trimesh, true, true);    
	}
}	
void BulletObject::CreateLightShape(float32 radius)
{
	trimesh = 0;
	shape = new btSphereShape(radius);
}

void BulletObject::UpdateCollisionObject()
{
    btTransform btt;
    btt.setIdentity();
	
	//scale
	Vector3 scale = collisionPartTransform->GetScaleVector();
	shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
	
	//origin
	Vector3 origin = collisionPartTransform->GetTranslationVector();
	btt.setOrigin(btVector3(origin.x, origin.y, origin.z));
	
	//rotation
	Quaternion qt;
	qt.Construct(*collisionPartTransform);
	btt.setRotation(btQuaternion(qt.x, qt.y, qt.z, qt.w));
					
//    btt.setFromOpenGLMatrix(collisionPartTransform->data);
    collisionObject->setWorldTransform(btt);
}

void BulletObject::Draw(const Matrix4 & worldTransform)
{
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 finalMatrix = worldTransform * prevMatrix;
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

//	RenderManager::Instance()->SetState(RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_CULL); 
//	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
//	RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
	
	int sz = triangles.size() / 3;
	for (int i = 0; i < sz; i++)
	{
		Vector3 & p0 = triangles[i*3];
		Vector3 & p1 = triangles[i*3 + 1];
		Vector3 & p2 = triangles[i*3 + 2];
		RenderHelper::Instance()->DrawLine(p0, p1);
		RenderHelper::Instance()->DrawLine(p1, p2);
		RenderHelper::Instance()->DrawLine(p0, p2);
	}
	
//	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);	
	
}


