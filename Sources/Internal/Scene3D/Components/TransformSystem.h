#ifndef __DAVAENGINE_TRANSFORM_SYSTEM_H__
#define __DAVAENGINE_TRANSFORM_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"
#include "Base/Singleton.h"

namespace DAVA 
{

class SceneNode;
class Transform;

class TransformSystem : public Singleton<TransformSystem>
{
public:
	static const int32 POOL_SIZE = 5000;

	TransformSystem();
	~TransformSystem();

    Transform * CreateTransform();
    Transform * GetTransformWithIncrement(Transform * transform);
    void DeleteTransform(Transform * transform);
    void LinkTransform(int32 parentIndex, int32 childIndex);
	void UnlinkTransform(int32 childIndex);
	int32 GetMatrixIndex(Matrix4 * matrix);
    
    void Process();

private:
    void SortAndThreadSplit();
    
    Matrix4 * localMatrices;
	Matrix4 * worldMatrices;
    Matrix4 ** parentWorldMatrices;
    uint32 * referenceCounter;
    uint32 matrixCount;

    //static const uint32 MAX_THREADS = 4;
    //ThreadBatch threads[MAX_THREADS];
    
    //struct ThreadBatch
    //{
    //    uint32 startIndex;
    //    uint32 count;
    //};
    
};

};

#endif //__DAVAENGINE_TRANSFORM_SYSTEM_H__
