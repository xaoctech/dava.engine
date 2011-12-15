#ifdef __DAVAENGINE_BEAST__

#ifndef __BEAST_MESH__
#define __BEAST_MESH__

#include "DAVAEngine.h"
#include "BeastTypes.h"
#include "BeastNameGenerator.h"

class BeastMesh
{
public:
	DECLARE_BEAST_NAME(BeastMesh);

	BeastMesh(ILBManagerHandle manager);
	virtual ~BeastMesh();

	void SetStaticMesh(DAVA::StaticMesh * staticMesh);

private:
	ILBMeshHandle mesh;
	ILBManagerHandle manager;

	bool CheckVertexFormat(DAVA::PolygonGroup * polygonGroup);
	void AddVertices(DAVA::PolygonGroup * polygonGroup);
	void AddIndices(DAVA::PolygonGroup * polygonGroup);
	void AddUV(DAVA::PolygonGroup * polygonGroup);
};

#endif //__BEAST_MESH__

#endif //__DAVAENGINE_BEAST__
