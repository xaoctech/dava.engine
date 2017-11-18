/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Simple

The purpose of this sample is to demonstrate how to use the API to:
1. Create a simple scene with meshes, textures, lights, materials and a camera
2. Render it 

*/

#define _USE_MATH_DEFINES // for M_PI
#include <cmath>

#include <iostream>
#include <sstream>
#include <beastapi/beaststring.h>
#include <beastapi/beastutils.h>
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

// Make sure we have a unicode safe cout define
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

int _tmain(int argc, _TCHAR* argv[])
{
    try
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
        bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_NULL, 0));
#endif

        // Setup our beast manager
        bex::apiCall(ILBCreateManager(_T("../../../temp/simpleCache"), ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(bmh, _T("../../../bin")));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(bmh));

        // Create ball and a plane meshes
        std::basic_string<TCHAR> sphereMatName(_T("SphereMaterial"));
        std::basic_string<TCHAR> floorMatName(_T("FloorMaterial"));
        ILBMeshHandle sphereMesh = bex::createSphere(bmh, _T("Sphere"), sphereMatName, 30, 15);
        ILBMeshHandle floorMesh = bex::createPlane(bmh, _T("Floor"), floorMatName);

        // Create a scene
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginScene(bmh, _T("SimpleScene"), &scene));

        // Create an instance of the plane that will be a floor
        ILBInstanceHandle floorInstance;
        bex::Matrix4x4 floorTrans = bex::scaleTranslation(bex::Vec3(10.0f, 1.0f, 10.0f),
                                                          bex::Vec3(0.0f, -5.0f, 0.0f));
        bex::apiCall(ILBCreateInstance(scene, floorMesh, _T("FloorInstance"), &floorTrans, &floorInstance));
        // Create 5 instances of the sphere on the plane
        const int spheres = 5;
        const float sphereRad = 2.0f;
        const float spherePosRadius = 5.0f;
        for (int i = 0; i < spheres; ++i)
        {
            float angle = static_cast<float>(M_PI) * 2.0f * static_cast<float>(i) / static_cast<float>(spheres);
            float x = cosf(angle) * spherePosRadius;
            float z = sinf(angle) * spherePosRadius;

            bex::Matrix4x4 trans = bex::scaleTranslation(bex::Vec3(sphereRad, sphereRad, sphereRad),
                                                         bex::Vec3(x, -3.0f, z));
            ILBInstanceHandle tempInstance;
            std::basic_stringstream<TCHAR> sphereName;
            sphereName << _T("SphereInstance_") << i;
            bex::apiCall(ILBCreateInstance(scene, sphereMesh, sphereName.str().c_str(), &trans, &tempInstance));
        }

        // Create a texture for the floor
        ILBTextureHandle floorTex = bex::createXorTexture(bmh, _T("xorTexture"), bex::ColorRGB(.9f, .7f, .7f));
        // Create the floor material
        ILBMaterialHandle floorMat;
        bex::apiCall(ILBCreateMaterial(scene, floorMatName.c_str(), &floorMat));
        bex::apiCall(ILBSetMaterialTexture(floorMat, ILB_CC_DIFFUSE, floorTex));

        // Create the sphere material
        ILBMaterialHandle sphereMat;
        bex::apiCall(ILBCreateMaterial(scene, sphereMatName.c_str(), &sphereMat));
        bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.3f, .3f, .3f, 1.0f)));
        bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_SPECULAR, &bex::ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f)));
        bex::apiCall(ILBSetMaterialScale(sphereMat, ILB_CC_REFLECTION, .1f));
        bex::apiCall(ILBSetMaterialScale(sphereMat, ILB_CC_SHININESS, 15.0f));

        // Create a directional light that casts soft shadows
        ILBLightHandle light;
        bex::apiCall(ILBCreateDirectionalLight(scene,
                                               _T("Sun"),
                                               &bex::directionalLightOrientation(bex::Vec3(1.0, -1.0f, -1.0f)),
                                               &bex::ColorRGB(1.0f, 1.0f, .8f),
                                               &light));
        bex::apiCall(ILBSetCastShadows(light, true));
        bex::apiCall(ILBSetShadowSamples(light, 32));
        bex::apiCall(ILBSetShadowAngle(light, .1f));

        ILBLightHandle skyLight;
        bex::apiCall(ILBCreateSkyLight(scene,
                                       _T("SkyLight"),
                                       &bex::identity(),
                                       &bex::ColorRGB(0.21f, 0.21f, 0.3f),
                                       &skyLight));

        // Setup a camera to render from
        ILBCameraHandle camera;
        bex::apiCall(ILBCreatePerspectiveCamera(scene,
                                                _T("Camera"),
                                                &bex::setCameraMatrix(bex::Vec3(.3f, 3.0f, 20.0f),
                                                                      bex::Vec3(.1f, -0.3f, -1.0f),
                                                                      bex::Vec3(0.0f, 1.0f, 0.0f)),
                                                &camera));
        // Set a 45 degrees fov
        bex::apiCall(ILBSetFov(camera, static_cast<float>(M_PI) / 4.0f, 1.0f));
        // Finalize the scene
        bex::apiCall(ILBEndScene(scene));

        ILBJobHandle job;
        bex::apiCall(ILBCreateJob(bmh, _T("TestJob"), scene, _T("../../data/simpleFG.xml"), &job));

        // Create pass
        ILBRenderPassHandle fullShadingPass;
        bex::apiCall(ILBCreateFullShadingPass(job, _T("fullShading"), &fullShadingPass));

        // Create Target
        ILBTargetHandle cameraTarget;
        bex::apiCall(ILBCreateCameraTarget(job, _T("cameraTarget"), camera, 640, 480, &cameraTarget));

        // Add pass to targets
        bex::apiCall(ILBAddPassToTarget(cameraTarget, fullShadingPass));

        // Finally render the scene
        if (!bex::renderJob(job, tcout))
        {
            return 1;
        }
        return 0;
    }
    catch (bex::Exception& ex)
    {
        ILBStringHandle errorString;
        ILBStringHandle extendedError;
        ILBErrorToString(ex.status, &errorString);
        ILBGetExtendErrorInformation(&extendedError);
        tcout << "Beast API error" << std::endl;
        tcout << "Error: " << bex::convertStringHandle(errorString) << std::endl;
        tcout << "Info: " << bex::convertStringHandle(extendedError) << std::endl;
        return 1;
    }
    catch (std::exception& ex)
    {
        tcout << "Standard exception" << std::endl;
        tcout << "Error: " << ex.what() << std::endl;
        ;
        return 1;
    }
}
