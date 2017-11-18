/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Materials

The purpose of this sample is to demonstrate how to use the API to:
1. Create the different kinds of materials available
2. Render a test scene which shows them in action

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
        bex::apiCall(ILBCreateManager(_T("../../../temp/materials"), ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

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

        // Create a 3 x 3 grid of spheres
        const int sphereSideCount = 3;
        const float sphereRad = 2.0f;
        const float sphereDist = 5.0f;
        std::vector<ILBInstanceHandle> sphereInstances;
        for (int gy = 0; gy < sphereSideCount; ++gy)
        {
            for (int gx = 0; gx < sphereSideCount; ++gx)
            {
                float offset = static_cast<float>(sphereSideCount - 1) * sphereDist / 2.0f;
                float x = static_cast<float>(gx) * sphereDist - offset;
                float z = static_cast<float>(gy) * sphereDist - offset;

                bex::Matrix4x4 trans = bex::scaleTranslation(bex::Vec3(sphereRad, sphereRad, sphereRad),
                                                             bex::Vec3(x, -3.0f, z));
                ILBInstanceHandle tempInstance;
                std::basic_stringstream<TCHAR> sphereName;
                sphereName << _T("SphereInstance_") << gx << _T("_") << gy;
                bex::apiCall(ILBCreateInstance(scene, sphereMesh, sphereName.str().c_str(), &trans, &tempInstance));

                // Store away the generated instance to set render stats
                sphereInstances.push_back(tempInstance);
            }
        }

        // Create the floor material
        ILBMaterialHandle floorMat;
        bex::apiCall(ILBCreateMaterial(scene, floorMatName.c_str(), &floorMat));
        bex::apiCall(ILBSetMaterialColor(floorMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.8f, .8f, .8f, 1.0f)));

        // Create the sphere material
        ILBMaterialHandle sphereMat;
        bex::apiCall(ILBCreateMaterial(scene, sphereMatName.c_str(), &sphereMat));
        bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.3f, .3f, .3f, 1.0f)));
        bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_SPECULAR, &bex::ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f)));
        bex::apiCall(ILBSetMaterialScale(sphereMat, ILB_CC_REFLECTION, .5f));
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
        bex::apiCall(ILBSetShadowAngle(light, .01f));

        ILBLightHandle skyLight;
        bex::apiCall(ILBCreateSkyLight(scene,
                                       _T("SkyLight"),
                                       &bex::identity(),
                                       &bex::ColorRGB(0.21f, 0.21f, 0.3f),
                                       &skyLight));

        // Setup a camera to render from
        bex::Vec3 camPos(10.0f, 20.0f, 10.0f);
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

        // Create a textured diffuse material for sphere 0
        ILBMaterialHandle sm0;
        ILBTextureHandle tex = bex::createXorTexture(bmh, _T("Tex1"), bex::ColorRGB(.6f, .6f, .3f));
        bex::apiCall(ILBCreateMaterial(scene, _T("Textured"), &sm0));
        bex::apiCall(ILBSetMaterialTexture(sm0, ILB_CC_DIFFUSE, tex));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[0], &sm0, 1));

        // Create a textured specular material for sphere 1
        ILBMaterialHandle sm1;
        bex::apiCall(ILBCreateMaterial(scene, _T("TexturedSpec"), &sm1));
        bex::apiCall(ILBSetMaterialTexture(sm1, ILB_CC_DIFFUSE, tex));
        bex::apiCall(ILBSetMaterialColor(sm1, ILB_CC_SPECULAR, &bex::ColorRGBA(1.0f, 1.0f, .7f, 1.0f)));
        bex::apiCall(ILBSetMaterialScale(sm1, ILB_CC_SHININESS, 25.0f));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[1], &sm1, 1));

        // Create a semi transparent material for sphere 2
        ILBMaterialHandle sm2;
        bex::apiCall(ILBCreateMaterial(scene, _T("Transp"), &sm2));
        bex::ColorRGBA col2(.6f, .9f, .9f, 1.0f);
        bex::apiCall(ILBSetMaterialColor(sm2, ILB_CC_DIFFUSE, &col2));
        bex::apiCall(ILBSetMaterialColor(sm2, ILB_CC_TRANSPARENCY, &col2));
        //bex::apiCall(ILBSetAlphaAsTransparency(sm2, true));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[2], &sm2, 1));

        // Create a color textured transparent material for sphere 3
        ILBMaterialHandle sm3;
        ILBTextureHandle tex2 = bex::createXorTexture(bmh, _T("Tex2"), bex::ColorRGB(1.0f, 0.2f, 0.2f));
        bex::apiCall(ILBCreateMaterial(scene, _T("TranspTex"), &sm3));
        bex::apiCall(ILBSetMaterialTexture(sm3, ILB_CC_DIFFUSE, tex2));
        bex::apiCall(ILBSetMaterialTexture(sm3, ILB_CC_TRANSPARENCY, tex2));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[3], &sm3, 1));

        // Create an emissive material for sphere 4
        ILBMaterialHandle sm4;
        bex::apiCall(ILBCreateMaterial(scene, _T("Emissive"), &sm4));
        bex::apiCall(ILBSetMaterialColor(sm4, ILB_CC_DIFFUSE, &bex::ColorRGBA(.1f, .1f, .1f, 1.0f)));
        bex::apiCall(ILBSetMaterialColor(sm4, ILB_CC_EMISSIVE, &bex::ColorRGBA(1.0f, .6f, .3f, 1.0f)));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[4], &sm4, 1));

        // Create a vertex colored material for sphere 5
        ILBMaterialHandle sm5;
        bex::apiCall(ILBCreateMaterial(scene, _T("VertexColor"), &sm5));
        bex::apiCall(ILBSetMaterialUseVertexColors(sm5, ILB_CC_DIFFUSE));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[5], &sm5, 1));

        // Create non reflective material with textured specular on sphere 6
        ILBMaterialHandle sm6;
        bex::apiCall(ILBCreateMaterial(scene, _T("SpecNoReflectivity"), &sm6));
        bex::apiCall(ILBSetMaterialTexture(sm6, ILB_CC_SPECULAR, tex2));
        bex::apiCall(ILBSetMaterialScale(sm6, ILB_CC_SPECULAR, 2.0f));
        bex::apiCall(ILBSetMaterialScale(sm6, ILB_CC_SHININESS, 4.0f));
        bex::apiCall(ILBSetMaterialColor(sm6, ILB_CC_DIFFUSE, &bex::ColorRGBA(.3f, .3f, .3f, 1.0f)));
        bex::apiCall(ILBSetMaterialScale(sm6, ILB_CC_REFLECTION, 0.0f));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[6], &sm6, 1));

        // Create a textured diffuse material for sphere 0
        ILBMaterialHandle sm7;
        bex::apiCall(ILBCreateMaterial(scene, _T("TexturedCheckerAlpha"), &sm7));
        bex::apiCall(ILBSetMaterialColor(sm7, ILB_CC_SPECULAR, &bex::ColorRGBA(1.0f, 1.0f, .7f, 1.0f)));
        bex::apiCall(ILBSetMaterialTexture(sm7, ILB_CC_DIFFUSE, tex));
        bex::apiCall(ILBSetMaterialScale(sm7, ILB_CC_SHININESS, 25.0f));
        bex::apiCall(ILBSetAlphaAsTransparency(sm7, true));
        bex::apiCall(ILBSetMaterialScale(sm7, ILB_CC_REFLECTION, 0.0f));
        bex::apiCall(ILBSetMaterialOverrides(sphereInstances[7], &sm7, 1));

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
