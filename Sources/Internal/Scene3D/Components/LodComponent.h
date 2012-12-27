#ifndef __DAVAENGINE_LOD_COMPONENT_H__
#define __DAVAENGINE_LOD_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

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

	struct LodDistance
	{
		float32 distance;

		float32 nearDistance;
		float32 farDistance;

		float32 nearDistanceSq;
		float32 farDistanceSq;

		LodDistance();
		void SetDistance(float32 newDistance);
		void SetNearDistance(float32 newDistance);
		void SetFarDistance(float32 newDistance);
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

	LodData *currentLod;
	List<LodData> lodLayers;
	LodDistance lodLayersArray[MAX_LOD_LAYERS];
};

};

#endif //__DAVAENGINE_LOD_COMPONENT_H__
