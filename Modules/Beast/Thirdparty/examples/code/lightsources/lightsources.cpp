/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Lightsources

The purpose of this sample is to demonstrate how to use the API to:
1. Create the different types of light sources available
2. Render a demonstration scene for each one of them

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

ILBLightHandle generateLight(int lsType, ILBSceneHandle scene, ILBTextureHandle goboTex)
{
    ILBLightHandle lh = 0;
    bex::Vec3 lookAt(0.0f, -2.0f, 0.0f);
    switch (lsType)
    {
    case 0:
    {
        // Directional light
        bex::apiCall(ILBCreateDirectionalLight(scene,
                                               _T("Light"),
                                               &bex::directionalLightOrientation(bex::Vec3(1.0, -1.0f, -1.0f)),
                                               &bex::ColorRGB(1.0f, 1.0f, .8f),
                                               &lh));
        // Give the light a nice soft shadow
        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        bex::apiCall(ILBSetShadowAngle(lh, .1f));
        break;
    }
    case 1:
    {
        // Point light
        bex::apiCall(ILBCreatePointLight(scene,
                                         _T("Light"),
                                         &bex::translation(bex::Vec3(-1.0, 1.0f, 1.0f)),
                                         &bex::ColorRGB(3.0f, 3.0f, 2.0f),
                                         &lh));

        // Give the light a soft shadow
        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        bex::apiCall(ILBSetShadowRadius(lh, 1.0f));

        // Set a quadratic falloff
        bex::apiCall(ILBSetFalloff(lh, ILB_FO_EXPONENT, 2.0f, 20.0f, true));
        break;
    }
    case 2:
    {
        // Spot light
        bex::Vec3 pos = bex::Vec3(-1.0, 1.0f, 1.0f);
        bex::Matrix4x4 spotMatrix = bex::setSpotlightMatrix(pos,
                                                            lookAt - pos);
        bex::apiCall(ILBCreateSpotLight(scene,
                                        _T("Light"),
                                        &spotMatrix,
                                        &bex::ColorRGB(3.0f, 3.0f, 2.0f),
                                        &lh));

        // Give the light a soft shadow
        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        bex::apiCall(ILBSetShadowRadius(lh, 1.0f));
        const float coneAngle = static_cast<float>(M_PI) / 3.0f;

        // Set a cone 60 degrees cone angle and a soft penumbra
        // of .1 radian
        bex::apiCall(ILBSetSpotlightCone(lh, coneAngle, .1f, 2.0f));
        break;
    }
    case 3:
    {
        // Window Light
        bex::Vec3 pos = bex::Vec3(-1.0, 1.0f, 1.0f);
        bex::Matrix4x4 matrix = bex::setAreaLightMatrix(pos,
                                                        lookAt - pos,
                                                        bex::Vec3(0.0f, 1.0f, 0.0f),
                                                        bex::Vec2(2.0f, 5.0f));

        /*bex::Matrix4x4 matrix = bex::setAreaLightMatrix(bex::Vec3(0.0f, 0.0f, 3.0f), 
				bex::Vec3(-1.0f, -1.0f, 0.0f), 
				bex::Vec3(0.0f, 0.0f, 1.0f), 
				bex::Vec2(1.0f, 3.0f));*/

        bex::apiCall(ILBCreateWindowLight(scene,
                                          _T("Light"),
                                          &matrix,
                                          &bex::ColorRGB(3.0f, 3.0f, 2.0f),
                                          &lh));

        // Give the light a soft shadow
        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        bex::apiCall(ILBSetShadowAngle(lh, .1f));
        break;
    }
    case 4:
    {
        // Area Light
        bex::Vec3 pos = bex::Vec3(-1.0, 1.0f, 1.0f);
        bex::Matrix4x4 matrix = bex::setAreaLightMatrix(pos,
                                                        lookAt - pos,
                                                        bex::Vec3(0.0f, 1.0f, 0.0f),
                                                        bex::Vec2(3.0f, 1.0f));
        bex::apiCall(ILBCreateAreaLight(scene,
                                        _T("Light"),
                                        &matrix,
                                        &bex::ColorRGB(.4f, .4f, .3f),
                                        &lh));

        // Give the light a soft shadow
        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        break;
    }
    case 5:
    {
        // Point light with ramp
        // Set a scale of 3 since the ramp coordinate system is
        // between 0 and 1 in light space
        bex::Matrix4x4 transform = bex::scaleTranslation(bex::Vec3(17.0f, 17.0f, 17.0f), bex::Vec3(-1.0, 1.0f, 1.0f));
        bex::apiCall(ILBCreatePointLight(scene,
                                         _T("Light"),
                                         &transform,
                                         &bex::ColorRGB(1.0f, 1.0f, 1.0f),
                                         &lh));

        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        bex::apiCall(ILBSetShadowRadius(lh, 1.0f));
        bex::apiCall(ILBSetFalloff(lh, ILB_FO_EXPONENT, 0.0f, 20.0f, true));

        // Set some random colors in a ramp
        const int rampColors = 10;
        for (int i = 0; i < rampColors; ++i)
        {
            bex::ColorRGBA randCol = bex::randomRGBA(3.0f);
            bex::apiCall(ILBSetLightRampEntry(lh, static_cast<float>(i) / static_cast<float>(rampColors - 1), &randCol.toColorRGB()));
        }
        break;
    }
    case 6:
    {
        // Spot light
        bex::Vec3 pos = bex::Vec3(-1.0, 1.0f, 1.0f);
        bex::Matrix4x4 spotMatrix = bex::setSpotlightMatrix(pos,
                                                            lookAt - pos);
        bex::apiCall(ILBCreateSpotLight(scene,
                                        _T("Light"),
                                        &spotMatrix,
                                        &bex::ColorRGB(3.0f, 3.0f, 2.0f),
                                        &lh));

        // Give the light a soft shadow
        bex::apiCall(ILBSetCastShadows(lh, true));
        bex::apiCall(ILBSetShadowSamples(lh, 32));
        bex::apiCall(ILBSetShadowRadius(lh, 1.0f));
        bex::apiCall(ILBSetLightProjectedTexture(lh, goboTex));
        const float coneAngle = static_cast<float>(M_PI) / 3.0f;

        // Set a cone 60 degrees cone angle and a soft penumbra
        // of .1 radian
        bex::apiCall(ILBSetSpotlightCone(lh, coneAngle, .1f, 2.0f));
        break;
    }
    case 7:
    {
        // Ambient light
        bex::apiCall(ILBCreateAmbientLight(scene,
                                           _T("Light"),
                                           &bex::scaleTranslation(bex::Vec3(4.0, 5.0f, 5.0f), bex::Vec3(4.0f, 0.0f, 0.0f)),
                                           &bex::ColorRGB(0.5f, 0.0f, 0.0f),
                                           &lh));

        bex::apiCall(ILBSetLightVolumeType(lh, ILB_LVT_CUBE));

        break;
    }
    default:
        throw std::runtime_error("Incorrect light source type");
    }
    return lh;
}

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
        bex::apiCall(ILBCreateManager(_T("../../../temp/lightsource"), ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(bmh, _T("../../../bin")));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(bmh));

        // Create ball and a plane meshes
        std::basic_string<TCHAR> sphereMatName(_T("SphereMaterial"));
        std::basic_string<TCHAR> floorMatName(_T("FloorMaterial"));
        ILBMeshHandle sphereMesh = bex::createSphere(bmh, _T("Sphere"), sphereMatName, 30, 15);
        ILBMeshHandle floorMesh = bex::createPlane(bmh, _T("Floor"), floorMatName);

        // Create a gobo texture for the spotlight
        ILBTextureHandle goboTex;
        goboTex = bex::createXorTexture(bmh, _T("xorTex"), bex::ColorRGB(1.0f, 1.0f, 1.0f));

        int lsTypes = 8;

        // Now render a set of scenes with different light sources
        std::vector<ILBSceneHandle> scenes;
        std::vector<ILBCameraHandle> cameras;
        scenes.reserve(lsTypes);

        for (int lightType = 0; lightType < lsTypes; ++lightType)
        {
            // Create a scene
            ILBSceneHandle scene;
            std::basic_stringstream<TCHAR> sceneName;
            sceneName << _T("SphereInstance_") << lightType;
            bex::apiCall(ILBBeginScene(bmh, sceneName.str().c_str(), &scene));

            ILBLightHandle skyLight;
            bex::apiCall(ILBCreateSkyLight(scene,
                                           _T("SkyLight"),
                                           &bex::identity(),
                                           &bex::ColorRGB(0.21f, 0.21f, 0.3f),
                                           &skyLight));

            // Create an instance of the plane that will be a floor
            ILBInstanceHandle floorInstance;
            bex::Matrix4x4 floorTrans = bex::scaleTranslation(bex::Vec3(10.0f, 1.0f, 10.0f),
                                                              bex::Vec3(0.0f, -5.0f, 0.0f));
            bex::apiCall(ILBCreateInstance(scene, floorMesh, _T("FloorInstance"), &floorTrans, &floorInstance));

            float x = 0.0f;
            float z = 0.0f;
            const float sphereRad = 2.0f;

            bex::Matrix4x4 trans = bex::scaleTranslation(bex::Vec3(sphereRad, sphereRad, sphereRad),
                                                         bex::Vec3(x, -3.0f, z));
            ILBInstanceHandle tempInstance;
            std::basic_stringstream<TCHAR> sphereName;
            sphereName << _T("SphereInstance_") << lightType;
            bex::apiCall(ILBCreateInstance(scene, sphereMesh, sphereName.str().c_str(), &trans, &tempInstance));
            bex::apiCall(ILBSetRenderStats(tempInstance, ILB_RS_SHADOW_BIAS, ILB_RSOP_ENABLE));

            // Create the floor material
            ILBMaterialHandle floorMat;
            bex::apiCall(ILBCreateMaterial(scene, floorMatName.c_str(), &floorMat));
            bex::apiCall(ILBSetMaterialColor(floorMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.7f, .7f, .7f, 1.0)));

            // Create the sphere material
            ILBMaterialHandle sphereMat;
            bex::apiCall(ILBCreateMaterial(scene, sphereMatName.c_str(), &sphereMat));
            bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.3f, .3f, .3f, 1.0f)));
            bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_SPECULAR, &bex::ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f)));
            bex::apiCall(ILBSetMaterialScale(sphereMat, ILB_CC_REFLECTION, .1f));
            bex::apiCall(ILBSetMaterialScale(sphereMat, ILB_CC_SHININESS, 15.0f));

            // Create a directional light that casts soft shadows
            ILBLightHandle light = generateLight(lightType, scene, goboTex);

            // Setup a camera to render from
            ILBCameraHandle camera;
            bex::Vec3 camPos = bex::Vec3(3.0f, 3.0f, 20.0f);
            bex::Vec3 lookAt = bex::Vec3(0.0f, -2.0f, 0.0f);
            bex::apiCall(ILBCreatePerspectiveCamera(scene,
                                                    _T("Camera"),
                                                    &bex::setCameraMatrix(camPos,
                                                                          lookAt - camPos,
                                                                          bex::Vec3(0.0f, 1.0f, 0.0f)),
                                                    &camera));
            cameras.push_back(camera);
            // Set a 45 degrees fov
            bex::apiCall(ILBSetFov(camera, static_cast<float>(M_PI) / 4.0f, 1.0f));
            // Finalize the scene
            bex::apiCall(ILBEndScene(scene));
            scenes.push_back(scene);
        }

        for (size_t lightType = 0; lightType < scenes.size(); ++lightType)
        {
            ILBJobHandle job;
            std::basic_stringstream<TCHAR> jobName;
            jobName << _T("TestJob_") << lightType;
            bex::apiCall(ILBCreateJob(bmh, jobName.str().c_str(), scenes[lightType], _T("../../data/simpleFG.xml"), &job));

            // Create pass
            ILBRenderPassHandle fullShadingPass;
            bex::apiCall(ILBCreateFullShadingPass(job, _T("fullShading"), &fullShadingPass));

            // Create Target
            ILBTargetHandle cameraTarget;
            bex::apiCall(ILBCreateCameraTarget(job, _T("cameraTarget"), cameras[lightType], 640, 480, &cameraTarget));

            // Add pass to targets
            bex::apiCall(ILBAddPassToTarget(cameraTarget, fullShadingPass));

            // Finally render the scene
            if (!bex::renderJob(job, tcout))
            {
                return 1;
            }
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
