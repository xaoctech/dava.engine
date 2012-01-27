/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "EdgeAdjacency.h"
#include "PolygonGroup.h"
#include "Render/RenderBase.h"
#include "Math/Math2D.h"

namespace DAVA
{


EdgeAdjacency::Adjacency::Adjacency()
{
}

bool EdgeAdjacency::Edge::IsEqual(const Edge & otherEdge)
{
	if(IsPointsEqual(points[0], otherEdge.points[0]) && IsPointsEqual(points[1], otherEdge.points[1]))
	{
		return true;
	}
	else if(IsPointsEqual(points[0], otherEdge.points[1]) && IsPointsEqual(points[1], otherEdge.points[0]))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool EdgeAdjacency::Edge::IsPointsEqual(const Vector3 & p0, const Vector3 & p1)
{
	if(FLOAT_EQUAL(p0.x, p1.x) && FLOAT_EQUAL(p0.y, p1.y) && FLOAT_EQUAL(p0.z, p1.z))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void EdgeAdjacency::InitFromPolygonGroup(PolygonGroup * _polygonGroup)
{
	ePrimitiveType primitiveType = polygonGroup->GetPrimitiveType();
	DVASSERT(PRIMITIVETYPE_TRIANGLELIST == primitiveType);

	polygonGroup = _polygonGroup;


	int32 indexCount = polygonGroup->GetIndexCount();
	int32 trianglesCount = indexCount/3;
	for(int32 i = 0; i < trianglesCount; ++i)
	{
		CreateTriangle(i);
	}
}

void EdgeAdjacency::CreateTriangle(int32 startingVertex)
{
	int32 i = startingVertex;

	Edge edge0;
	FillEdge(edge0, i, i+1);
	int32 edgeIndex0 = GetEdgeIndex(edge0);

	Edge edge1;
	FillEdge(edge1, i+1, i+2);
	int32 edgeIndex1 = GetEdgeIndex(edge1);

	Edge edge2;
	FillEdge(edge2, i+2, i);
	int32 edgeIndex2 = GetEdgeIndex(edge2);

	Triangle triangle;
	triangle.edgeIndices[0] = edgeIndex0;
	triangle.edgeIndices[1] = edgeIndex1;
	triangle.edgeIndices[2] = edgeIndex2;

	triangles.push_back(triangle);
}

void EdgeAdjacency::FillEdge(Edge & edge, int32 point0, int32 point1)
{
	int32 index;
	Vector3 position;

	polygonGroup->GetIndex(point0, index);
	polygonGroup->GetCoord(index, position);
	edge.points[0] = position;

	polygonGroup->GetIndex(point1, index);
	polygonGroup->GetCoord(index, position);
	edge.points[1] = position;
}

int32 EdgeAdjacency::GetEdgeIndex(Edge & edge)
{
	int32 edgesCount = edges.size();
	for(int32 i = 0; i < edgesCount; ++i)
	{
		if(edges[i].IsEqual(edge))
		{
			return i;
		}
	}

	edges.push_back(edge);

	return edgesCount;
}

};
