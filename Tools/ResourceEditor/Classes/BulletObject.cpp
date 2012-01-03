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
{
	collWorld = collisionWorld;

//    CollisionCache *coll = CollisionCache::Create(currentConfig->modelName);
    
    btTransform startTransform;
    startTransform.setIdentity();
    float start_x = 0;
    float start_y = 0;
    float start_z = 0;
//	std::vector<float32> debugShapes;
	
    startTransform.setOrigin(btVector3(btScalar(start_x),
                                       btScalar(start_y),
                                       btScalar(start_z)));
    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
	CreateShape(meshNode);
    collisionObject->setCollisionShape(shape);
    collisionWorld->addCollisionObject(collisionObject);
    
	//    collisionPartWorldTransform = &((Matrix4&)pWorldTransform);
    collisionPartTransform = &((Matrix4&)pWorldTransform);
	
//	debugNode = new DebugNode(scene, debugShapes);
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
//	SafeRelease(debugNode);
}

void BulletObject::CreateShape(MeshInstanceNode *meshNode)
{    
	trimesh = new btTriangleMesh();
	
	const Vector<StaticMesh*> & meshes = meshNode->GetMeshes();
	const Vector<int32> & indexes = meshNode->GetPolygonGroupIndexes();
	
	uint32 meshesSize = (uint32)meshes.size();

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
			v = v * meshNode->GetLocalTransform();
			btVector3 vertex0(v.x, v.y, v.z);
			pg->GetCoord(index1, v);
			v = v * meshNode->GetLocalTransform();
			btVector3 vertex1(v.x, v.y, v.z);
			pg->GetCoord(index2, v);
			v = v * meshNode->GetLocalTransform();
			btVector3 vertex2(v.x, v.y, v.z);
			
			trimesh->addTriangle(vertex0,vertex1,vertex2, false);
			
//			debugShapes.push_back(vertex0.x());
//			debugShapes.push_back(vertex0.y());
//			debugShapes.push_back(vertex0.z());
//
//			debugShapes.push_back(vertex1.x());
//			debugShapes.push_back(vertex1.y());
//			debugShapes.push_back(vertex1.z());
//
//			debugShapes.push_back(vertex1.x());
//			debugShapes.push_back(vertex1.y());
//			debugShapes.push_back(vertex1.z());
//			
//			debugShapes.push_back(vertex2.x());
//			debugShapes.push_back(vertex2.y());
//			debugShapes.push_back(vertex2.z());
//
//			debugShapes.push_back(vertex0.x());
//			debugShapes.push_back(vertex0.y());
//			debugShapes.push_back(vertex0.z());
//			
//			debugShapes.push_back(vertex2.x());
//			debugShapes.push_back(vertex2.y());
//			debugShapes.push_back(vertex2.z());
			
		}
	}	
	
	shape = new btBvhTriangleMeshShape(trimesh, true, true);

	
	
	
//	btTriangleMesh* data = new btTriangleMesh();
//	btVector3 A(0.0f,0.0f,0.0f);
//	btVector3 B(100.0f,0.0f,0.0f);
//	btVector3 C(0.0f,0.0f,100.0f);
//	data->addTriangle(A,B,C,false); // false, don’t remove duplicate vertices
//	// true for using quantization; true for building the BVH
//	shape=new btBvhTriangleMeshShape(data,true,true);
//	
//	
//	debugShapes.push_back(A.x());
//	debugShapes.push_back(A.y());
//	debugShapes.push_back(A.z());
//	
//	debugShapes.push_back(B.x());
//	debugShapes.push_back(B.y());
//	debugShapes.push_back(B.z());
//	
//	debugShapes.push_back(B.x());
//	debugShapes.push_back(B.y());
//	debugShapes.push_back(B.z());
//	
//	debugShapes.push_back(C.x());
//	debugShapes.push_back(C.y());
//	debugShapes.push_back(C.z());
//	
//	debugShapes.push_back(A.x());
//	debugShapes.push_back(A.y());
//	debugShapes.push_back(A.z());
//	
//	debugShapes.push_back(C.x());
//	debugShapes.push_back(C.y());
//	debugShapes.push_back(C.z());
	
	
//    btConvexShape* tmpConvexShape = new btConvexTriangleMeshShape(trimesh);
//	
//	//create a hull approximation
//    btShapeHull* hull = new btShapeHull(tmpConvexShape);
//    btScalar margin = tmpConvexShape->getMargin();
//    hull->buildHull(margin);
        
//    shape = new btConvexHullShape();
//    for (i=0;i<hull->numVertices();i++)
//    {
//        shape->addPoint(hull->getVertexPointer()[i]);
//    }
//    
//	shape->
//	for (i=1;i<hull->numIndices();i++)
//	{
//		debugShapes.push_back(hull->getVertexPointer()[hull->getIndexPointer()[i-1]].getX());
//		debugShapes.push_back(hull->getVertexPointer()[hull->getIndexPointer()[i-1]].getY());
//		debugShapes.push_back(hull->getVertexPointer()[hull->getIndexPointer()[i-1]].getZ());
//		
//		debugShapes.push_back(hull->getVertexPointer()[hull->getIndexPointer()[i]].getX());
//		debugShapes.push_back(hull->getVertexPointer()[hull->getIndexPointer()[i]].getY());
//		debugShapes.push_back(hull->getVertexPointer()[hull->getIndexPointer()[i]].getZ());
//	}
//    delete tmpConvexShape;
//    delete hull;
//    delete trimesh;
    
}

void BulletObject::UpdateCollisionObject()
{
    btTransform btt;
    btt.setIdentity();
    btt.setFromOpenGLMatrix(collisionPartTransform->data);
    collisionObject->setWorldTransform(btt);
}



