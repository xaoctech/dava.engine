#include "Scene3D/Components/TransformSystem.h"
#include "Scene3D/Components/Transform.h"

namespace DAVA
{



TransformSystem::TransformSystem()
:	localMatrices(0),
	parentWorldMatrices(0),
	referenceCounter(0),
	matrixCount(0)
{
	localMatrices = new Matrix4[POOL_SIZE];
	worldMatrices = new Matrix4[POOL_SIZE];
	parentWorldMatrices = new Matrix4*[POOL_SIZE];
	referenceCounter = new uint32[POOL_SIZE];
}

TransformSystem::~TransformSystem()
{
	SafeDeleteArray(referenceCounter);
	SafeDeleteArray(parentWorldMatrices);
	SafeDeleteArray(worldMatrices);
	SafeDeleteArray(localMatrices);
}

Transform * TransformSystem::CreateTransform()
{
    Transform * transform = new Transform();

    transform->localTransform = &localMatrices[matrixCount];
	*(transform->localTransform) = Matrix4::IDENTITY;

	transform->worldTransform = &worldMatrices[matrixCount];
	*(transform->worldTransform) = Matrix4::IDENTITY;

    transform->parent = 0;
	transform->index = matrixCount;

    parentWorldMatrices[matrixCount] = 0;
    referenceCounter[matrixCount] = 1;
    matrixCount++;

	return transform;
}

Transform * TransformSystem::GetTransformWithIncrement(Transform * transform)
{
	return 0;
}
    
void TransformSystem::DeleteTransform(Transform * transform)
{
    if(transform)
	{

	}
}

void TransformSystem::LinkTransform(int32 parentIndex, int32 childIndex)
{
    parentWorldMatrices[childIndex] = &(worldMatrices[parentIndex]);
}

void TransformSystem::UnlinkTransform(int32 childIndex)
{
	parentWorldMatrices[childIndex] = 0;
}

void TransformSystem::Process()
{
	Matrix4 ** parentMatrixPtr = parentWorldMatrices;
	Matrix4 * localMatrix = localMatrices;
	Matrix4 * worldMatrix = worldMatrices;
    for(int32 i = 0; i < matrixCount; ++i)
	{
		if(*parentMatrixPtr)
		{
			*worldMatrix = (*localMatrix) * (**parentMatrixPtr);
		}

		parentMatrixPtr++;
		localMatrix++;
		worldMatrix++;
	}
}
    
int32 TransformSystem::GetMatrixIndex(Matrix4 * matrix)
{
    int32 index = (matrix - localMatrices) / sizeof(Matrix4);
    return index;
}

void TransformSystem::SortAndThreadSplit()
{
    //// SortMultipleArraysByParrentMatrixPtrAsc()
    //for (uint32 k = 0; k < matrixCount; ++k)
    //{
    //    groupIndex[k] = -1;
    //}
    //
    //int32 nextGroupIndex = 0;
    //for (uint32 index = 0; index < matrixCount; ++index)
    //{
    //    if (parentMatrixArray[index] == 0)
    //        groupIndex[index] = -1;
    //    else
    //    {
    //        int32 parentIndex = GetMatrixIndex(parentMatrixArray[index]);
    //        if (groupIndex[parentIndex] == -1)
    //        {
    //            groupIndex[parentIndex] = nextGroupIndex;
    //            groupIndex[index] = nextGroupIndex;
    //            nextGroupIndex++;
    //        }
    //        groupIndex[index] = groupIndex[parentIndex];
    //    }
    //}
    //
    ////
    //SortByGroupIndexAndParentPtr();
    //
    ///* 
    // SplitByThreads
    //int32 groupPerThread = nextGroupIndex / MAX_THREADS;
    //int32 threadIndex = 0;
    //for (uint32 index = 0; index < matrixCount; ++index)
    //{
    //    if (groupIndex[index] >= 0)
    //    {
    //        threads[threadIndex].startIndex = 0;
    //    }
    //}*/
}








};

