/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Occlusion

The purpose of this sample is to demonstrate how to use the API to:
 - How to set up and bake ambient occlusion to a texture

 */

#define _USE_MATH_DEFINES // for M_PI
#include <math.h>

#include "primitives.h"
#include "textures.h"
#include "xmlwriter.h"

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
#include <sstream>
#include <fstream>
#include <iostream>

const unsigned int SPHERES = 50;
const float SPHERE_RADIUS = 1.0f;

int main(char argc, char** argv)
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
        bex::apiCall(ILBCreateManager("../../../temp/occlusion", ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(bmh, "../../../bin"));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(bmh));

        // Create ball and a plane meshes
        std::string sphereMatName = "SphereMaterial";
        std::string floorMatName = "FloorMaterial";
        ILBMeshHandle sphereMesh = bex::createSphere(bmh, "Sphere", sphereMatName, 32, 32);
        ILBMeshHandle floorMesh = bex::createPlane(bmh, "Floor", floorMatName);

        // Create a scene
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginScene(bmh, "OcclusionScene", &scene));

        // Create an instance of the plane that will be a floor
        ILBInstanceHandle floorInstance;
        bex::Matrix4x4 floorTrans = bex::scaleTranslation(bex::Vec3(10.0f, 1.0f, 10.0f),
                                                          bex::Vec3(0.0f, -5.0f, 0.0f));
        bex::apiCall(ILBCreateInstance(scene, floorMesh, "FloorInstance2", &floorTrans, &floorInstance));

        // Create a directional light that casts soft shadows
        ILBLightHandle light;
        bex::apiCall(ILBCreateDirectionalLight(scene,
                                               "Sun",
                                               &bex::directionalLightOrientation(bex::Vec3(1.0, -1.0f, -1.0f)),
                                               &bex::ColorRGB(1.0f, 1.0f, .8f),
                                               &light));
        bex::apiCall(ILBSetCastShadows(light, true));
        bex::apiCall(ILBSetShadowSamples(light, 32));
        bex::apiCall(ILBSetShadowAngle(light, .1f));

        // Create the spheres on the plane
        const float spherePosRadius = 10.0f - 2.0f * SPHERE_RADIUS;
        for (int i = 0; i < SPHERES; ++i)
        {
            float x = (bex::frand() - 0.5f) * spherePosRadius * 2.0f;
            float z = (bex::frand() - 0.5f) * spherePosRadius * 2.0f;

            bex::Matrix4x4 trans = bex::scaleTranslation(bex::Vec3(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS),
                                                         bex::Vec3(x, SPHERE_RADIUS - 5.0f, z));
            ILBInstanceHandle tempInstance;
            std::stringstream sphereName;
            sphereName << "SphereInstance_" << i;
            bex::apiCall(ILBCreateInstance(scene, sphereMesh, sphereName.str().c_str(), &trans, &tempInstance));
            bex::apiCall(ILBSetRenderStats(tempInstance, ILB_RS_SHADOW_BIAS, ILB_RSOP_ENABLE));
        }

        // Create the floor material
        ILBMaterialHandle floorMat;
        bex::apiCall(ILBCreateMaterial(scene, floorMatName.c_str(), &floorMat));
        bex::apiCall(ILBSetMaterialColor(floorMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.7f, .7f, .7f, 1.0f)));

        // Create the sphere material
        ILBMaterialHandle sphereMat;
        bex::apiCall(ILBCreateMaterial(scene, sphereMatName.c_str(), &sphereMat));
        bex::apiCall(ILBSetMaterialColor(sphereMat, ILB_CC_DIFFUSE, &bex::ColorRGBA(.9f, .9f, .9f, 1.0f)));

        // Setup a camera to render from
        ILBCameraHandle camera;
        bex::apiCall(ILBCreatePerspectiveCamera(scene, _T("Camera"), &bex::translation(bex::Vec3(0.0f, 0.0f, 10.0)), &camera));

        // Finalize the scene
        bex::apiCall(ILBEndScene(scene));

        std::string xmlFileName = "../../data/occlusion.xml";

        // Create settings xml file
        {
            using namespace bex;
            std::ofstream ofs(xmlFileName.c_str(), std::ios_base::out | std::ios_base::trunc);
            XMLWriter xml(ofs);

            {
                ScopedTag _x(xml, "ILConfig");
                {
                    ScopedTag _x(xml, "AASettings");
                    xml.data("minSampleRate", 0);
                    xml.data("maxSampleRate", 2);
                }
                {
                    ScopedTag _x(xml, "RenderSettings");
                    xml.data("bias", 0.00001f);
                }
            }
        }

        ILBJobHandle job;
        bex::apiCall(ILBCreateJob(bmh, _T("TestJob"), scene, xmlFileName.c_str(), &job));

        // Create pass
        ILBRenderPassHandle occlusionPass;
        bex::apiCall(ILBCreateAmbientOcclusionPass(job, _T("occlusion"), 0, 180, &occlusionPass));
        bex::apiCall(ILBSetAOAdaptive(occlusionPass, 1, 1));
        bex::apiCall(ILBSetAONumRays(occlusionPass, 256, 1024));
        bex::apiCall(ILBSetAOContrast(occlusionPass, 1.1f, 1.f));
        bex::apiCall(ILBSetAOUniformSampling(occlusionPass));
        bex::apiCall(ILBSetAOSelfOcclusion(occlusionPass, ILB_SO_SET_ENVIRONMENT));

        // Create Targets
        ILBTargetHandle textureTarget;
        bex::apiCall(ILBCreateTextureTarget(job, _T("textureTarget"), 512, 512, &textureTarget));
        ILBTargetEntityHandle entity;
        bex::apiCall(ILBAddBakeInstance(textureTarget, floorInstance, &entity));

        // Add pass to targets
        bex::apiCall(ILBAddPassToTarget(textureTarget, occlusionPass));

        // Finally render the scene
        if (!bex::renderJob(job, std::cout))
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
        std::cout << "Beast API error" << std::endl;
        std::cout << "Error: " << bex::convertStringHandle(errorString) << std::endl;
        std::cout << "Info: " << bex::convertStringHandle(extendedError) << std::endl;
        return 1;
    }
    catch (std::exception& ex)
    {
        std::cout << "Standard exception" << std::endl;
        std::cout << "Error: " << ex.what() << std::endl;
        ;
        return 1;
    }
}
