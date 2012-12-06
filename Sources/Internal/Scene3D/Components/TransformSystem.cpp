#include "Scene3D/Components/TransformSystem.h"
#include "Scene3D/Components/Transform.h"

namespace DAVA
{



TransformSystem::TransformSystem()
:	matrixArray(0),
	parentMatrixArray(0),
	referenceCounter(0),
	matrixCount(0)
{
	matrixArray = new Matrix4[POOL_SIZE];
	parentMatrixArray = new Matrix4*[POOL_SIZE];
	referenceCounter = new uint32[POOL_SIZE];
}

TransformSystem::~TransformSystem()
{
	SafeDeleteArray(referenceCounter);
	SafeDeleteArray(parentMatrixArray);
	SafeDeleteArray(matrixArray);
}

Transform * TransformSystem::CreateTransform()
{
    Transform * transform = new Transform();
    transform->matrix = &matrixArray[matrixCount];
	*(transform->matrix) = Matrix4::IDENTITY;
    transform->parent = 0;
    parentMatrixArray[matrixCount] = 0;
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

void TransformSystem::LinkTransform(Transform * transformParent, Transform * child)
{
    
}

void TransformSystem::Process()
{
	Matrix4 ** parentMatrix = parentMatrixArray;
    for(int32 i = 0; i < matrixCount; ++i)
	{
		if(*parentMatrix)
		{

		}

		parentMatrix++;
	}
}
    
int32 TransformSystem::GetMatrixIndex(Matrix4 * matrix)
{
    int32 index = (matrix - matrixArray) / sizeof(Matrix4);
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

