#include "Qt/Scene/System/CollisionSystem/CollisionLandscape.h"
#include "Render/Highlevel/Heightmap.h"

CollisionLandscape::CollisionLandscape(DAVA::Entity *entity, btCollisionWorld *word, DAVA::Landscape *landscape)
	: CollisionBaseObject(entity, word)
{
	if(NULL != landscape && NULL != word)
	{
		DAVA::Heightmap *heightmap = landscape->GetHeightmap();
		if(NULL != heightmap)
		{
			DAVA::Vector3 landSize;
			DAVA::AABBox3 landTransformedBox;
			DAVA::Matrix4 landWorldTransform = entity->GetWorldTransform();

			landscape->GetBoundingBox().GetTransformedBox(landWorldTransform, landTransformedBox);
			landSize = landTransformedBox.max - landTransformedBox.min;

			DAVA::float32 landWidth = landSize.x;
			DAVA::float32 landScaleW = landWidth / heightmap->Size();
			DAVA::float32 landHeight = landSize.z;
			DAVA::float32 landScaleH = landHeight / 65535.f;

			DAVA::uint16 *heightData = heightmap->Data();
			btHMap.resize(heightmap->Size() * heightmap->Size());

			for(DAVA::int32 y = 0; y < heightmap->Size(); ++y)
			{
				for (DAVA::int32 x = 0; x < heightmap->Size(); ++x)
				{
					DAVA::int32 heightIndex = x + y * heightmap->Size();
					btHMap[heightIndex] = heightData[heightIndex] * landScaleH;
				}
			}

			btTerrain = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size(),	&btHMap.front(), landScaleH, 0, landHeight, 2, PHY_FLOAT, true);
			btTerrain->setLocalScaling(btVector3(landScaleW, landScaleW, 1.0f));
			
			btTransform landTransform;
			landTransform.setIdentity();
			landTransform.setOrigin(btVector3(0, 0, landHeight / 2.0f));

			btObject = new btCollisionObject();
			btObject->setWorldTransform(landTransform);
			btObject->setCollisionShape(btTerrain);
			btWord->addCollisionObject(btObject);
		}
	}
}

CollisionLandscape::~CollisionLandscape()
{
	if(NULL != btObject)
	{
		btWord->removeCollisionObject(btObject);
		DAVA::SafeDelete(btObject);
		DAVA::SafeDelete(btTerrain);
	}
}
