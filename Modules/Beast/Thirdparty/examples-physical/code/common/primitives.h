/*
Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * A set of functions to create primitive meshes
 */ 
#ifndef PRIMITIVES_H
#define PRIMITIVES_H
#include <beastapi/beastmesh.h>
#include <beastapi/beastpointcloud.h>
#include "utils.h"
#include "vecmath.h"
#include <cmath>
#include <vector>

namespace bex
{
/** 
	 * Creates a sphere with radius 1.
	 * Throws an exception if things goes wrong
	 * inlined to avoid conflicts if linking two files including primitives.h
	 * @param bm The Beast manager to create it for
	 * @param name The name of the mesh
	 * @param materialName The name of the material to use
	 * on the mesh
	 * @param segmentsU The number of vertices in each radial segment
	 * @param segmentsV The number of vertical segments on the sphere
	 */
inline ILBMeshHandle createSphere(ILBManagerHandle bm,
                                  const std::basic_string<TCHAR>& name,
                                  const std::basic_string<TCHAR>& materialName,
                                  int segmentsU,
                                  int segmentsV,
                                  std::vector<float>* colors = 0)
{
    ILBMeshHandle mesh;
    apiCall(ILBBeginMesh(bm, name.c_str(), &mesh));
    const float PI = 3.141592f;
    if (segmentsU < 3 || segmentsV < 3)
    {
        throw std::runtime_error("Invalid segment setup");
    }
    // Create the vertex data
    // For simplicity the points on the top and bottom of the
    // sphere is generated as a complete circle
    for (int v = 0; v < segmentsV; ++v)
    {
        float angleTh = (static_cast<float>(v) / static_cast<float>(segmentsV - 1)) * PI;
        std::vector<Vec3> positions;
        std::vector<Vec3> normals;
        positions.reserve(segmentsU);
        normals.reserve(segmentsU);
        for (int u = 0; u <= segmentsU; ++u)
        {
            float anglePh = static_cast<float>(u) * 2.0f * PI / static_cast<float>(segmentsU);
            float x = cosf(anglePh) * sinf(angleTh);
            float y = cosf(angleTh);
            float z = sinf(anglePh) * sinf(angleTh);
            positions.push_back(Vec3(x, y, z));
            // Normalizations might be redundant here
            normals.push_back(normalize(Vec3(x, y, z)));
        }
        // Add vertexdata to the mesh
        // Note this is done per latitude
        apiCall(ILBAddVertexData(mesh, &positions[0], &normals[0], static_cast<int32>(positions.size())));
    }

    // Create a material group that will contain all polygons
    apiCall(ILBBeginMaterialGroup(mesh, materialName.c_str()));
    for (int v = 0; v < segmentsV - 1; ++v)
    {
        std::vector<int32> indices;
        // Each triangle requires 3 indices and each segments needs
        // facet need 2 triangles, hence 6 times the number of segments
        indices.reserve(segmentsU * 6);
        for (int u = 0; u < segmentsU; ++u)
        {
            // Create the 4 indices that defines a sphere facet
            // a-----b
            // |     |
            // |     |
            // d-----c
            int a = u + v * (segmentsU + 1);
            int b = (u + 1) + v * (segmentsU + 1);
            int c = (u + 1) + (v + 1) * (segmentsU + 1);
            int d = u + (v + 1) * (segmentsU + 1);
            // Skip this triangle on the top band since it would be
            // degenerated.
            if (v > 0)
            {
                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(c);
            }
            // Skip this triangle on the bottom band since it would be
            // degenerated.
            if (v < segmentsV - 2)
            {
                indices.push_back(a);
                indices.push_back(c);
                indices.push_back(d);
            }
        }
        apiCall(ILBAddTriangleData(mesh, &indices[0], static_cast<int32>(indices.size())));
    }
    // Finalize the material group
    apiCall(ILBEndMaterialGroup(mesh));

    // Add uv's
    apiCall(ILBBeginUVLayer(mesh, _T("uv1")));
    for (int v = 0; v < segmentsV; ++v)
    {
        std::vector<Vec2> uvs;
        uvs.reserve(segmentsU);
        for (int u = 0; u <= segmentsU; ++u)
        {
            Vec2 uv;
            uv.y = static_cast<float>(v) / static_cast<float>(segmentsV - 1);
            if (v == 0 || v == (segmentsV - 1))
            {
                uv.x = 0.5f;
            }
            else
            {
                uv.x = static_cast<float>(u) / static_cast<float>(segmentsU);
            }
            uvs.push_back(uv);
        }
        // Add uv data to the mesh
        // Note this is done per latitude
        apiCall(ILBAddUVData(mesh, &uvs[0], static_cast<int32>(uvs.size())));
    }
    apiCall(ILBEndUVLayer(mesh));

    // Add a second messes up UV-set to show how multiple UV-sets work
    apiCall(ILBBeginUVLayer(mesh, _T("uv2")));
    for (int v = 0; v < segmentsV; ++v)
    {
        std::vector<Vec2> uvs;
        uvs.reserve(segmentsU);
        for (int u = 0; u <= segmentsU; ++u)
        {
            Vec2 uv;
            uv.y = static_cast<float>(v) / static_cast<float>(segmentsV - 1);
            if (v == 0 || v == (segmentsV - 1))
            {
                uv.x = 0.5f;
            }
            else
            {
                uv.x = static_cast<float>(u) / static_cast<float>(segmentsU);
            }
            uvs.push_back(Vec2(sinf(uv.x * 2.0f * (float)M_PI), cosf(uv.y * 2.0f * (float)M_PI)));
        }
        // Add uv data to the mesh
        // Note this is done per latitude
        apiCall(ILBAddUVData(mesh, &uvs[0], static_cast<int32>(uvs.size())));
    }
    apiCall(ILBEndUVLayer(mesh));

    // Add vertex colors
    apiCall(ILBBeginColorLayer(mesh, _T("color1")));
    if (colors && colors->size() == (segmentsU + 1) * segmentsV * 4)
    {
        apiCall(ILBAddColorData(mesh, (ILBLinearRGBA*)(&(*colors)[0]), static_cast<int32>(colors->size() / 4)));
    }
    else
    {
        std::vector<ColorRGBA> randomColors;
        randomColors.reserve(segmentsU * segmentsV);
        for (int v = 0; v < segmentsV * (segmentsU + 1); ++v)
        {
            randomColors.push_back(randomRGBA(1.0f));
        }
        apiCall(ILBAddColorData(mesh, &randomColors[0], static_cast<int32>(randomColors.size())));
    }
    apiCall(ILBEndColorLayer(mesh));

    apiCall(ILBEndMesh(mesh));
    return mesh;
}

/**
	 * Generates a plane in the x z plane.
	 * The size of the plane is -1 - 1 in both x and z directions
	 * u is mapped to x, v to z.
	 * inlined to avoid conflicts if linking two files including primitives.h
	 */
inline ILBMeshHandle createPlane(ILBManagerHandle bm,
                                 const std::basic_string<TCHAR>& name,
                                 const std::basic_string<TCHAR>& materialName)
{
    ILBMeshHandle mesh;
    apiCall(ILBBeginMesh(bm, name.c_str(), &mesh));
    const int vertexCount = 4;

    // Setup vertex data
    Vec3 normal(0.0f, 1.0f, 0.0f);
    std::vector<Vec3> normals(vertexCount, normal);
    std::vector<Vec3> positions(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
    {
        float x = (i < 2) ? -1.0f : 1.0f;
        float z = ((i == 1) || (i == 2)) ? 1.0f : -1.0f;
        positions[i] = Vec3(x, 0.0f, z);
    }

    apiCall(ILBAddVertexData(mesh, &positions[0], &normals[0], vertexCount));

    // Create a material group that will contain all polygons
    apiCall(ILBBeginMaterialGroup(mesh, materialName.c_str()));

    // Two triangles requires 6 indices
    std::vector<int32> tris(6);
    tris[0] = 0;
    tris[1] = 1;
    tris[2] = 2;
    tris[3] = 0;
    tris[4] = 2;
    tris[5] = 3;
    apiCall(ILBAddTriangleData(mesh, &tris[0], 6));

    // Finalize the material group
    apiCall(ILBEndMaterialGroup(mesh));

    std::vector<Vec2> uvData(vertexCount);
    std::vector<Vec3> tangents(vertexCount);
    std::vector<Vec3> bitangents(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
    {
        int idx = i & 3;
        float u = (idx < 2) ? 0.0f : 1.0f;
        float v = ((idx == 1) || (idx == 2)) ? 1.0f : 0.0f;
        uvData[i] = Vec2(u, v);
        tangents[i] = Vec3(1.0f, 0.0f, 0.0f);
        bitangents[i] = Vec3(0.0f, 0.0f, -1.0f);
    }

    // Add a uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv1")));
    // Add uv data
    apiCall(ILBAddUVData(mesh, &uvData[0], vertexCount));
    // Finalize the uv layer
    apiCall(ILBEndUVLayer(mesh));
#if 0
		// Add a tangent layer
		apiCall(ILBBeginTangents(mesh));
		// Add tangents
		apiCall(ILBAddTangentData(mesh, &tangents[0], &bitangents[0], vertexCount));
		// Finalize the tangent layer
		apiCall(ILBEndTangents(mesh));
#endif
    apiCall(ILBEndMesh(mesh));
    return mesh;
}
}

#endif // PRIMITIVES_H
