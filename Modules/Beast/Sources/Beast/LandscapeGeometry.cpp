#ifdef __DAVAENGINE_BEAST__

#include "LandscapeGeometry.h"

void LandscapeGeometry::ComputeNormals()
{
    normals.resize(vertices.size());

    for (DAVA::size_type i = 0, e = indices.size() / 3; i < e; ++i)
    {
        auto i0 = indices.at(3 * i + 0);
        auto i1 = indices.at(3 * i + 1);
        auto i2 = indices.at(3 * i + 2);
        const DAVA::Vector3& p1 = vertices[i0].position;
        const DAVA::Vector3& p2 = vertices[i1].position;
        const DAVA::Vector3& p3 = vertices[i2].position;
        DAVA::Vector3 normal = (p2 - p1).CrossProduct(p3 - p1);
        DVASSERT(normal.Length() > std::numeric_limits<float>::epsilon(), "Landscape contains degenerate triangles");
        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }

    for (auto& n : normals)
    {
        n.Normalize();
    }
}


#endif //__DAVAENGINE_BEAST__
