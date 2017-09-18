/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
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
	* Creates a Cornell Box
	* The size of the box is [-1,-1,-1] to [1,1,1]
	* 
	* inlined to avoid conflicts if linking two files including primitives.h
	*/

inline ILBMeshHandle createCornellBox(ILBManagerHandle bm,
                                      const std::basic_string<TCHAR>& name,
                                      const std::basic_string<TCHAR>& materialName)
{
    ILBMeshHandle mesh;
    apiCall(ILBBeginMesh(bm, name.c_str(), &mesh));

    Vec3 A(-1, -1, -1);
    Vec3 B(-1, -1, 1);
    Vec3 C(1, -1, 1);
    Vec3 D(1, -1, -1);
    Vec3 E(-1, 1, -1);
    Vec3 F(1, 1, -1);
    Vec3 G(1, 1, 1);
    Vec3 H(-1, 1, 1);

    Vec2 UVD(0.f, 0.f);
    Vec2 UVC(1.f / 3.f, 0.f);
    Vec2 UVB(2.f / 3.f, 0.f);
    Vec2 UVA(1.f, 0.f);
    Vec2 UVH(0.f, 1.f / 3.f);
    Vec2 UVG(1.f / 3.f, 1.f / 3.f);
    Vec2 UVF(2.f / 3.f, 1.f / 3.f);
    Vec2 UVE(1.f, 1.f / 3.f);
    Vec2 UVL(0.f, 2.f / 3.f);
    Vec2 UVK(1.f / 3.f, 2.f / 3.f);
    Vec2 UVJ(2.f / 3.f, 2.f / 3.f);
    Vec2 UVI(1.f, 2.f / 3.f);

    ILBLinearRGBA red(.7f, 0.f, 0.f, 1.f);
    ILBLinearRGBA blue(0.f, 0.f, .7f, 1.f);
    ILBLinearRGBA white(.7f, .7f, .7f, 1.f);

    const int vertexCount = 20;
    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<ILBLinearRGBA> colors;
    std::vector<Vec2> uv;
    std::vector<Vec3> tangents;
    std::vector<Vec3> bitangents;

    // Face 1, floor
    positions.push_back(A);
    positions.push_back(B);
    positions.push_back(C);
    positions.push_back(D);
    uv.push_back(UVA);
    uv.push_back(UVB);
    uv.push_back(UVF);
    uv.push_back(UVE);
    normals.insert(normals.end(), 4, Vec3(0, 1, 0));
    colors.insert(colors.end(), 4, white);
    tangents.insert(tangents.end(), 4, Vec3(1, 0, 0));
    bitangents.insert(bitangents.end(), 4, Vec3(0, 0, -1));
    // Face 2, ceiling
    positions.push_back(E);
    positions.push_back(F);
    positions.push_back(G);
    positions.push_back(H);
    uv.push_back(UVB);
    uv.push_back(UVC);
    uv.push_back(UVG);
    uv.push_back(UVF);
    normals.insert(normals.end(), 4, Vec3(0, -1, 0));
    colors.insert(colors.end(), 4, white);
    tangents.insert(tangents.end(), 4, Vec3(1, 0, 0));
    bitangents.insert(bitangents.end(), 4, Vec3(0, 0, 1));
    // Face 3, back wall
    positions.push_back(H);
    positions.push_back(G);
    positions.push_back(C);
    positions.push_back(B);
    uv.push_back(UVC);
    uv.push_back(UVD);
    uv.push_back(UVH);
    uv.push_back(UVG);
    normals.insert(normals.end(), 4, Vec3(0, 0, -1));
    colors.insert(colors.end(), 4, white);
    tangents.insert(tangents.end(), 4, Vec3(1, 0, 0));
    bitangents.insert(bitangents.end(), 4, Vec3(0, -1, 0));
    // Face 4, left wall
    positions.push_back(E);
    positions.push_back(H);
    positions.push_back(B);
    positions.push_back(A);
    uv.push_back(UVE);
    uv.push_back(UVF);
    uv.push_back(UVJ);
    uv.push_back(UVI);
    normals.insert(normals.end(), 4, Vec3(1, 0, 0));
    colors.insert(colors.end(), 4, red);
    tangents.insert(tangents.end(), 4, Vec3(0, 0, 1));
    bitangents.insert(bitangents.end(), 4, Vec3(0, -1, 0));
    // Face 5, right wall
    positions.push_back(G);
    positions.push_back(F);
    positions.push_back(D);
    positions.push_back(C);
    uv.push_back(UVF);
    uv.push_back(UVG);
    uv.push_back(UVK);
    uv.push_back(UVJ);
    normals.insert(normals.end(), 4, Vec3(-1, 0, 0));
    colors.insert(colors.end(), 4, blue);
    tangents.insert(tangents.end(), 4, Vec3(0, 0, -1));
    bitangents.insert(bitangents.end(), 4, Vec3(0, -1, 0));

    // Setup the triangles
    std::vector<int> triangles;
    for (int i = 0; i < 5; i++)
    {
        int base = i * 4;
        triangles.push_back(base + 0);
        triangles.push_back(base + 1);
        triangles.push_back(base + 2);
        triangles.push_back(base + 2);
        triangles.push_back(base + 3);
        triangles.push_back(base + 0);
    }

    apiCall(ILBAddVertexData(mesh, &positions[0], &normals[0], vertexCount));

    // Create a material group that will contain all polygons
    apiCall(ILBBeginMaterialGroup(mesh, materialName.c_str()));
    apiCall(ILBAddTriangleData(mesh, &triangles[0], (int32)triangles.size()));
    apiCall(ILBEndMaterialGroup(mesh));

    // Add a uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv1")));
    apiCall(ILBAddUVData(mesh, &uv[0], vertexCount));
    apiCall(ILBEndUVLayer(mesh));

    // Add a color layer
    apiCall(ILBBeginColorLayer(mesh, _T("colors")));
    apiCall(ILBAddColorData(mesh, &colors[0], vertexCount));
    apiCall(ILBEndColorLayer(mesh));

    // Add a tangent layer
    apiCall(ILBBeginTangents(mesh));
    apiCall(ILBAddTangentData(mesh, &tangents[0], &bitangents[0], vertexCount));
    apiCall(ILBEndTangents(mesh));

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

inline void createPlaneVertices(unsigned int xres,
                                unsigned int yres,
                                std::vector<Vec3>& vertices)
{
    const int vertexCount = xres * yres;
    vertices.resize(vertexCount);
    unsigned int vertexIndex = 0;

    const float x_rcp = 2.0f / ((float)xres - 1.0f);
    const float y_rcp = 2.0f / ((float)yres - 1.0f);
    for (unsigned int y = 0; y < yres; y++)
    {
        float yp = (float)y * y_rcp - 1.0f;
        for (unsigned int x = 0; x < xres; x++)
        {
            float xp = (float)x * x_rcp - 1.0f;
            vertices[vertexIndex++] = Vec3(xp, 0.0f, yp);
        }
    }
}

inline ILBPointCloudHandle createPlanePC(ILBSceneHandle scene,
                                         const std::basic_string<TCHAR>& name,
                                         unsigned int xres,
                                         unsigned int yres,
                                         bex::Matrix4x4& mtx)
{
    const int vertexCount = xres * yres;
    Vec3 normal(0.0f, 1.0f, 0.0f);
    std::vector<Vec3> normals(vertexCount, normal);
    std::vector<Vec3> positions;
    createPlaneVertices(xres, yres, positions);
    for (size_t i = 0; i < positions.size(); i++)
    {
        positions[i] = mulPoint(mtx, positions[i]);
    }
    ILBPointCloudHandle pc;
    apiCall(ILBCreatePointCloud(scene, name.c_str(), &pc));
    apiCall(ILBAddPointCloudData(pc, &positions[0], &normals[0], vertexCount));
    apiCall(ILBEndPointCloud(pc));
    return pc;
}

/**
	* Generates a plane in the x z plane.
	* The number of vertices in x and z can be specified.
	* The size of the plane is -1 - 1 in both x and z directions
	* u is mapped to x, v to z.
	* inlined to avoid conflicts if linking two files including primitives.h
	*/
inline ILBMeshHandle createPlane(ILBManagerHandle bm,
                                 const std::basic_string<TCHAR>& name,
                                 const std::basic_string<TCHAR>& materialName,
                                 unsigned int xres,
                                 unsigned int yres,
                                 std::vector<float>* colors = 0)
{
    ILBMeshHandle mesh;
    apiCall(ILBBeginMesh(bm, name.c_str(), &mesh));
    const int vertexCount = xres * yres;
    const int triangleCount = (xres - 1) * (yres - 1) * 2;

    // Setup vertex data
    Vec3 normal(0.0f, 1.0f, 0.0f);
    Vec3 tangent(1.0f, 0.0f, 0.0f);
    Vec3 bitangent(0.0f, 0.0f, -1.0f);

    std::vector<Vec3> normals(vertexCount, normal);
    std::vector<Vec3> positions;
    std::vector<int32> tris(triangleCount * 3);
    std::vector<Vec2> uvData(vertexCount);
    std::vector<Vec3> tangents(vertexCount, tangent);
    std::vector<Vec3> bitangents(vertexCount, bitangent);

    unsigned int vertexIndex = 0;
    unsigned int triangleIndex = 0;

    createPlaneVertices(xres, yres, positions);

    for (unsigned int y = 0; y < yres; y++)
    {
        for (unsigned int x = 0; x < xres; x++)
        {
            uvData[vertexIndex] = Vec2(positions[vertexIndex].x * 0.5f + 0.5f, positions[vertexIndex].z * 0.5f + 0.5f);
            vertexIndex++;

            if (x != xres - 1 && y != yres - 1)
            {
                unsigned int indexV1 = y * xres + x;
                unsigned int indexV2 = y * xres + x + 1;
                unsigned int indexV3 = y * xres + x + xres;
                unsigned int indexV4 = y * xres + x + xres + 1;
                tris[triangleIndex++] = indexV1;
                tris[triangleIndex++] = indexV4;
                tris[triangleIndex++] = indexV2;
                tris[triangleIndex++] = indexV1;
                tris[triangleIndex++] = indexV3;
                tris[triangleIndex++] = indexV4;
            }
        }
    }
    apiCall(ILBAddVertexData(mesh, &positions[0], &normals[0], vertexCount));

    // Create a material group that will contain all polygons
    apiCall(ILBBeginMaterialGroup(mesh, materialName.c_str()));
    apiCall(ILBAddTriangleData(mesh, &tris[0], triangleCount * 3));
    apiCall(ILBEndMaterialGroup(mesh));

    // Add uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv1")));
    apiCall(ILBAddUVData(mesh, &uvData[0], vertexCount));
    apiCall(ILBEndUVLayer(mesh));

    // Add tangent layer
    apiCall(ILBBeginTangents(mesh));
    apiCall(ILBAddTangentData(mesh, &tangents[0], &bitangents[0], vertexCount));
    apiCall(ILBEndTangents(mesh));

    // Add colors
    if (colors && colors->size() == vertexCount * 4)
    {
        apiCall(ILBBeginColorLayer(mesh, _T("colors")));
        apiCall(ILBAddColorData(mesh, (ILBLinearRGBA*)&(*colors)[0], vertexCount));
        apiCall(ILBEndColorLayer(mesh));
    }

    apiCall(ILBEndMesh(mesh));
    return mesh;
}

/**
	* Generates a plane in the x z plane.
	* The size of the plane is -1 - 1 in both x and z directions
	* We generate four UV sets, uv1-uv4. The each represent one corner of
	* the UV space.
	* inlined to avoid conflicts if linking two files including primitives.h
	*/
inline ILBMeshHandle createPlaneMultiUV(ILBManagerHandle bm, const std::basic_string<TCHAR>& name, const std::basic_string<TCHAR>& materialName)
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

    apiCall(ILBAddVertexData(mesh, &positions[0], &normals[0], 4));

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

    std::vector<Vec2> uvData1(vertexCount);
    std::vector<Vec2> uvData2(vertexCount);
    std::vector<Vec2> uvData3(vertexCount);
    std::vector<Vec2> uvData4(vertexCount);
    std::vector<Vec3> tangents(vertexCount);
    std::vector<Vec3> bitangents(vertexCount);
    for (int i = 0; i < vertexCount; ++i)
    {
        int idx = i & 3;
        float u = (idx < 2) ? 0.0f : 0.5f;
        float v = ((idx == 1) || (idx == 2)) ? 0.0f : 0.5f;
        uvData1[i] = Vec2(u, v);
        uvData2[i] = Vec2(u + 0.5f, v);
        uvData3[i] = Vec2(u, v + 0.5f);
        uvData4[i] = Vec2(u + 0.5f, v + 0.5f);
        tangents[i] = Vec3(1.0f, 0.0f, 0.0f);
        bitangents[i] = Vec3(0.0f, 0.0f, -1.0f);
    }

    // Add first uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv1")));
    apiCall(ILBAddUVData(mesh, &uvData1[0], vertexCount));
    apiCall(ILBEndUVLayer(mesh));

    // Add second uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv2")));
    apiCall(ILBAddUVData(mesh, &uvData2[0], vertexCount));
    apiCall(ILBEndUVLayer(mesh));

    // Add third uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv3")));
    apiCall(ILBAddUVData(mesh, &uvData3[0], vertexCount));
    apiCall(ILBEndUVLayer(mesh));

    // Add fourth uv layer
    apiCall(ILBBeginUVLayer(mesh, _T("uv4")));
    apiCall(ILBAddUVData(mesh, &uvData4[0], vertexCount));
    apiCall(ILBEndUVLayer(mesh));

    // Add a tangent layer
    apiCall(ILBBeginTangents(mesh));
    // Add tangents
    apiCall(ILBAddTangentData(mesh, &tangents[0], &bitangents[0], vertexCount));
    // Finalize the tangent layer
    apiCall(ILBEndTangents(mesh));

    apiCall(ILBEndMesh(mesh));
    return mesh;
}

/**
	* Creates a Point Cloud in the shape of a grid. It will cover the space
	* between minCorner and maxCorner. It will contain count^3 points.
	* Inlined to avoid conflicts if linking two files including primitives.h
	*/
inline ILBPointCloudHandle createPointCloudGrid(ILBSceneHandle scene,
                                                const std::basic_string<TCHAR>& name,
                                                const ILBVec3& minCorner,
                                                const ILBVec3& maxCorner,
                                                int32 count)
{
    if (count < 2)
    {
        return 0;
    }

    ILBPointCloudHandle pch;
    apiCall(ILBCreatePointCloud(scene, name.c_str(), &pch));

    std::vector<ILBVec3> normals;
    normals.resize(count, ILBVec3(1, 0, 0));

    ILBVec3 sideLength(maxCorner.x - minCorner.x, maxCorner.y - minCorner.y, maxCorner.z - minCorner.z);

    for (int32 z = 0; z < count; z++)
    {
        for (int32 y = 0; y < count; y++)
        {
            std::vector<ILBVec3> points;
            points.reserve(count);
            for (int x = 0; x < count; ++x)
            {
                float xx = minCorner.x + sideLength.x * (float)x / (float)(count - 1);
                float yy = minCorner.y + sideLength.y * (float)y / (float)(count - 1);
                float zz = minCorner.z + sideLength.z * (float)z / (float)(count - 1);
                points.push_back(ILBVec3(xx, yy, zz));
            }
            apiCall(ILBAddPointCloudData(pch, &points[0], &normals[0], count));
        }
    }
    apiCall(ILBEndPointCloud(pch));
    return pch;
}
}

#endif // PRIMITIVES_H
