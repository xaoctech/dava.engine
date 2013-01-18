#ifndef __DAVAENGINE_LOD_COMPONENT_H__
#define __DAVAENGINE_LOD_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

class SceneNode;
class LodComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(LOD_COMPONENT);

	static const int32 MAX_LOD_LAYERS = 4;
	static const int32 INVALID_LOD_LAYER = -1;
	static const float32 MIN_LOD_DISTANCE;
	static const float32 MAX_LOD_DISTANCE;
	static const float32 INVALID_DISTANCE;

	enum eFlags
	{
		NEED_UPDATE_AFTER_LOAD = 1 << 0,
	};

	struct LodDistance
	{
		float32 distance;

		float32 nearDistance;
		float32 farDistance;

		float32 nearDistanceSq;
		float32 farDistanceSq;

		LodDistance();
		void SetDistance(const float32 &newDistance);
        float32 GetDistance() const { return distance; };
        
		void SetNearDistance(const float32 &newDistance);
        float32 GetNearDistance() const {return  nearDistance; };
        
		void SetFarDistance(const float32 &newDistance);
        float32 GetFarDistance() const {return farDistance; };
        
        INTROSPECTION(LodDistance,
            PROPERTY(distance, "Distance", GetDistance, SetDistance, INTROSPECTION_FLAG_SERIALIZABLE)
            PROPERTY(nearDistance, "Near Distance", GetNearDistance, SetNearDistance, INTROSPECTION_FLAG_SERIALIZABLE)
            PROPERTY(farDistance, "Far Distance", GetFarDistance, SetFarDistance, INTROSPECTION_FLAG_SERIALIZABLE)
        );
	};

	struct LodData
	{
		LodData()
		:	layer(INVALID_LOD_LAYER),
			isDummy(false)
		{
		}
		Vector<SceneNode*> nodes;
		Vector<int32> indexes;
		int32 layer;
		bool isDummy;
	};

	LodComponent();
	virtual Component * Clone();

	static float32 GetDefaultDistance(int32 layer);
	void SetCurrentLod(LodData *newLod);

	inline int32 GetLodLayersCount();
	inline float32 GetLodLayerDistance(int32 layerNum);
	inline float32 GetLodLayerNear(int32 layerNum);
	inline float32 GetLodLayerFar(int32 layerNum);
	inline float32 GetLodLayerNearSquare(int32 layerNum);
	inline float32 GetLodLayerFarSquare(int32 layerNum);

	LodData *currentLod;
	List<LodData> lodLayers;
	Vector<LodDistance> lodLayersArray;
	int32 forceLodLayer;
	float32 forceDistance;
	float32 forceDistanceSq;

	int32 flags;
    
    LodDistance testDistance;
    
public:
    
    INTROSPECTION_EXTEND(LodComponent, Component,
                         MEMBER(testDistance, "testDistance", 0)
						 COLLECTION(lodLayersArray, "lodLayersArray", 0)
//                         NULL
                         );

};

int32 LodComponent::GetLodLayersCount()
{
	return (int32)lodLayers.size();
}

float32 LodComponent::GetLodLayerDistance(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArray[layerNum].distance;
}

float32 LodComponent::GetLodLayerNear(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArray[layerNum].nearDistance;
}

float32 LodComponent::GetLodLayerFar(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArray[layerNum].farDistance;
}

float32 LodComponent::GetLodLayerNearSquare(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArray[layerNum].nearDistanceSq;
}

float32 LodComponent::GetLodLayerFarSquare(int32 layerNum)
{
	DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
	return lodLayersArray[layerNum].farDistanceSq;
}

};

#endif //__DAVAENGINE_LOD_COMPONENT_H__
