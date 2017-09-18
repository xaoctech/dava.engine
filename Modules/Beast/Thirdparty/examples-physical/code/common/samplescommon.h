/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * Functions that generate a test scene
 */

#ifndef SAMPLESCOMMON_H
#define SAMPLESCOMMON_H

#include <beastapi/beastmanager.h>
#include <beastapi/beastscene.h>
#include <beastapi/beastinstance.h>
#include <beastapi/beastlightsource.h>
#include <beastapi/beastcamera.h>
#include <beastapi/beastmaterial.h>
#include <beastapi/beasttarget.h>
#include <beastapi/beastrenderpass.h>

#include "vecmath.h"
#include "utils.h"
#include "primitives.h"
#include "textures.h"
#include <map>

namespace bex
{
typedef std::map<tstring, ILBInstanceHandle> InstanceMap;

inline ILBManagerHandle setupBeastManager(const tstring& testName)
{
    ILBManagerHandle bmh;

    // Route errors to stderr
    bex::apiCall(ILBSetLogTarget(ILB_LT_ERROR, ILB_LS_STDERR, 0));

#if defined(WIN32)
    // Route general info to debug output (i.e output window in
    // visual studio)
    bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_DEBUG_OUTPUT, 0));
#else
    // Route general info to std output
    bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_STDOUT, 0));
#endif

    tstringstream ss;
    ss << _T("../../../temp/") << testName;

    // Setup our beast manager
    bex::apiCall(ILBCreateManager(ss.str().c_str(), ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

    // Set the path to the Beast binaries
    bex::apiCall(ILBSetBeastPath(bmh, _T("../../../bin")));

    // Waste the cache from previous runs if present
    bex::apiCall(ILBClearCache(bmh));

    return bmh;
}

inline void setupTestScene(ILBManagerHandle bmh, ILBSceneHandle scene, InstanceMap& createdInstances)
{
    // Create the floor material
    tstring defaultMaterialName(_T("DefaultMaterial"));

    ILBShaderHandle defaultShader;
    bex::apiCall(ILBCreateShader(bmh, _T("DefaultShader"), "../../data/default.osl", &defaultShader));

    ILBMaterialHandle defaultMat;
    bex::apiCall(ILBCreateMaterial(scene, defaultMaterialName.c_str(), &defaultMat));
    bex::apiCall(ILBSetShader(defaultMat, defaultShader));

    // Create ball and a plane meshes
    ILBMeshHandle sphereMesh, floorMesh;

    tstringstream dummyStream;

    if (!findCachedMesh(bmh, _T("Sphere"), sphereMesh, dummyStream))
    {
        sphereMesh = bex::createSphere(bmh, _T("Sphere"), defaultMaterialName, 50, 30);
    }
    if (!findCachedMesh(bmh, _T("Floor"), floorMesh, dummyStream))
    {
        floorMesh = bex::createPlane(bmh, _T("Floor"), defaultMaterialName);
    }

    const int numSpheres = 7;
    const float sphereRad = 2.0f;
    const float sphereDist = 5.0f;
    const float sphereLineWidth = (numSpheres - 1) * sphereDist + sphereRad * 2.0f;

    // Create an instance of the plane that will be a floor
    ILBInstanceHandle floorInstance;
    bex::Matrix4x4 floorTrans = bex::scaleTranslation(bex::Vec3(sphereLineWidth / 2.0f + 5.0f, 1.0f, 8.0f),
                                                      bex::Vec3(0.0f, -5.0f, 0.0f));
    bex::apiCall(ILBCreateInstance(scene, floorMesh, _T("FloorInstance"), &floorTrans, &floorInstance));

    createdInstances[_T("FloorInstance")] = floorInstance;

    // Create a line of 7 spheres
    std::vector<ILBInstanceHandle> sphereInstances;
    for (int gx = 0; gx < numSpheres; ++gx)
    {
        const float x = -sphereLineWidth / 2.0f + gx * sphereDist + sphereRad;

        bex::Matrix4x4 trans = bex::scaleTranslation(bex::Vec3(sphereRad, sphereRad, sphereRad),
                                                     bex::Vec3(x, -3.0f, 0.0f));
        ILBInstanceHandle tempInstance;
        tstringstream sphereName;
        sphereName << _T("SphereInstance_") << gx;
        bex::apiCall(ILBCreateInstance(scene, sphereMesh, sphereName.str().c_str(), &trans, &tempInstance));

        // Store away the generated instance to set render stats
        sphereInstances.push_back(tempInstance);

        createdInstances[sphereName.str()] = tempInstance;
    }

    // Create a directional light that casts soft shadows
    ILBLightHandle light;
    bex::apiCall(ILBCreateDirectionalLight(scene,
                                           _T("Sun"),
                                           &bex::directionalLightOrientation(bex::Vec3(1.0, -1.0f, -1.0f)),
                                           &bex::ColorRGB(0.9f, 0.9f, .7f),
                                           &light));
    bex::apiCall(ILBSetShadowAngle(light, 0.1f * (float)M_PI));

    // Create a sky light
    ILBLightHandle skyLight;
    bex::apiCall(ILBCreateSkyLight(scene,
                                   _T("SkyLight"),
                                   &bex::identity(),
                                   &bex::ColorRGB(0.21f, 0.21f, 0.3f),
                                   &skyLight));

    // Create sphere shaders
    ILBShaderHandle emissiveShader;
    bex::apiCall(ILBCreateShader(bmh, _T("EmissiveShader"), "../../data/emissive.osl", &emissiveShader));

    ILBShaderHandle phongishShader;
    bex::apiCall(ILBCreateShader(bmh, _T("PhongishShader"), "../../data/phongish.osl", &phongishShader));

    ILBShaderHandle mirrorShader;
    bex::apiCall(ILBCreateShader(bmh, _T("MirrorShader"), "../../data/mirror.osl", &mirrorShader));

    ILBShaderHandle slickShader;
    bex::apiCall(ILBCreateShader(bmh, _T("SlickShader"), "../../data/slick.osl", &slickShader));

    // Create textures
    ILBTextureHandle xorTexture = bex::createXorTexture(bmh, _T("Tex1"), bex::ColorRGB(1.0f, 1.0f, 1.0f));

    // Sphere 0 - Red emissive
    ILBMaterialHandle sm0;
    bex::apiCall(ILBCreateMaterial(scene, _T("RedEmissive"), &sm0));
    bex::apiCall(ILBSetShader(sm0, emissiveShader));
    bex::apiCall(ILBSetShaderParamColor(sm0, _T("Ke"), &ILBLinearRGB(5.0f, 0.0f, 0.0f)));
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[0], &sm0, 1));

    // Sphere 1 - Textured lambert
    ILBMaterialHandle sm1;
    bex::apiCall(ILBCreateMaterial(scene, _T("TexturedLambert"), &sm1));
    bex::apiCall(ILBSetShader(sm1, phongishShader));
    bex::apiCall(ILBSetShaderParamTexture(sm1, _T("DiffuseTexture"), xorTexture));
    bex::apiCall(ILBSetShaderParamColor(sm1, _T("DiffuseColor"), &ILBLinearRGB(0.5f, 0.5f, 1.0f)));
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[1], &sm1, 1));

    // Sphere 2 - Phongish
    ILBMaterialHandle sm2;
    bex::apiCall(ILBCreateMaterial(scene, _T("Phongish"), &sm2));
    bex::apiCall(ILBSetShader(sm2, phongishShader));
    bex::apiCall(ILBSetShaderParamColor(sm2, _T("DiffuseColor"), &ILBLinearRGB(0.0f, 0.0f, 0.0f)));
    bex::apiCall(ILBSetShaderParamColor(sm2, _T("SpecularColor"), &ILBLinearRGB(1.0f, 0.0f, 0.0f)));
    bex::apiCall(ILBSetShaderParamColor(sm2, _T("Shininess"), &ILBLinearRGB(400.0f, 400.0f, 400.0f)));
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[2], &sm2, 1));

    // Sphere 3 - Mirror
    ILBMaterialHandle sm3;
    bex::apiCall(ILBCreateMaterial(scene, _T("PerfectMirror"), &sm3));
    bex::apiCall(ILBSetShader(sm3, mirrorShader));
    bex::apiCall(ILBSetShaderParamColor(sm3, _T("Color"), &ILBLinearRGB(0.9f, 0.9f, 0.9f)));
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[3], &sm3, 1));

    // Sphere 4 - Procedural color
    ILBMaterialHandle sm4;
    bex::apiCall(ILBCreateMaterial(scene, _T("Procedural"), &sm4));
    bex::apiCall(ILBSetShader(sm4, slickShader));
    bex::apiCall(ILBSetShaderParamColor(sm4, _T("Cs"), &ILBLinearRGB(0.2f, 0.9f, 0.2f)));
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[4], &sm4, 1));

    // Sphere 5 - Default shader
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[5], &defaultMat, 1));

    // Sphere 6 - Textured lambert
    ILBMaterialHandle sm6;
    bex::apiCall(ILBCreateMaterial(scene, _T("TexturedLambertWithUVSET"), &sm6));
    bex::apiCall(ILBSetShader(sm6, phongishShader));
    bex::apiCall(ILBSetShaderParamTexture(sm6, _T("DiffuseTexture"), xorTexture));
    bex::apiCall(ILBSetShaderParamInt(sm6, _T("HasDiffuseUV"), 1));
    bex::apiCall(ILBSetShaderParamUV(sm6, _T("DiffuseUV"), "uv2"));
    bex::apiCall(ILBSetShaderParamColor(sm6, _T("DiffuseColor"), &ILBLinearRGB(0.5f, 0.5f, 1.0f)));
    bex::apiCall(ILBSetMaterialOverrides(sphereInstances[6], &sm6, 1));
}

inline ILBCameraHandle setupCamera(ILBSceneHandle scene)
{
    // Setup a camera to render from
    bex::Vec3 camPos(0.0f, 5.0f, 48.0f);
    bex::Vec3 lookAt(0.0f, -3.0f, 0.0f);
    ILBCameraHandle camera;
    bex::apiCall(ILBCreatePerspectiveCamera(scene,
                                            _T("Camera"),
                                            &bex::setCameraMatrix(camPos,
                                                                  normalize(lookAt - camPos),
                                                                  bex::Vec3(0.0f, 0.0f, -1.0f)),
                                            &camera));

    // Set a 45 degrees fov
    bex::apiCall(ILBSetFov(camera, static_cast<float>(M_PI) / 4.0f, 1.0f));

    return camera;
}

} // namespace bex



#endif // SAMPLESCOMMON_H
