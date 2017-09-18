/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: UVSet

The purpose of this sample is to demonstrate how to use the API to:
1. Create a mesh with several UV sets
2. Create a material which uses different uv sets for different channels
3. Show the different channels in a render with the help of a spot light
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
#include <sstream>
#include <fstream>
#include <iostream>
#include "vecmath.h"
#include "utils.h"
#include "primitives.h"
#include "textures.h"
#include "xmlwriter.h"

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
        bex::apiCall(ILBCreateManager(_T("../../../temp/uvset"), ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(bmh, _T("../../../bin")));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(bmh));

        // Create ball and a plane meshes
        std::basic_string<TCHAR> floorMatName(_T("FloorMaterial"));
        ILBMeshHandle floorMesh = bex::createPlaneMultiUV(bmh, _T("Floor"), floorMatName);

        // Create a scene
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginScene(bmh, _T("UvsetScene"), &scene));

        // Create an instance of the plane that will be a floor
        ILBInstanceHandle floorInstance;
        bex::Matrix4x4 floorTrans = bex::scaleTranslation(bex::Vec3(10.0f, 1.0f, 10.0f),
                                                          bex::Vec3(0.0f, -5.0f, 0.0f));
        bex::apiCall(ILBCreateInstance(scene, floorMesh, _T("FloorInstance"), &floorTrans, &floorInstance));

        // Here we create a texture with four colors
        ILBTextureHandle floorTex = bex::createTestColorTexture(bmh, _T("TestColorTexture"));

        // The floor material uses different UV sets (uv3 and uv4) for diffuse and emissive
        // channels. This
        ILBMaterialHandle floorMat;
        bex::apiCall(ILBCreateMaterial(scene, floorMatName.c_str(), &floorMat));
        bex::apiCall(ILBSetMaterialTexture(floorMat, ILB_CC_DIFFUSE, floorTex));
        bex::apiCall(ILBSetChannelUVLayer(floorMat, ILB_CC_DIFFUSE, _T("uv3")));
        bex::apiCall(ILBSetMaterialTexture(floorMat, ILB_CC_EMISSIVE, floorTex));
        bex::apiCall(ILBSetChannelUVLayer(floorMat, ILB_CC_EMISSIVE, _T("uv4")));

        // Create a spotlight to show the diffuse channel in the middle
        ILBLightHandle light;
        bex::Vec3 pos(0.0, 1.0f, 0.0f);
        bex::Vec3 lookAt(0.0f, 0.0f, 0.0f);
        bex::Matrix4x4 spotMatrix = bex::setSpotlightMatrix(pos, lookAt - pos);
        bex::apiCall(ILBCreateSpotLight(scene,
                                        _T("Light"),
                                        &spotMatrix,
                                        &bex::ColorRGB(1.0f, 1.0f, 1.0f),
                                        &light));
        bex::apiCall(ILBSetSpotlightCone(light, static_cast<float>(M_PI) / 3.0f, .1f, 2.0f));

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

        std::string xmlFileName = "../../data/uvset.xml";

        // Create settings xml file
        {
            using namespace bex;
            std::ofstream ofs(xmlFileName.c_str(), std::ios_base::out | std::ios_base::trunc);
            XMLWriter xml(ofs);
            {
                ScopedTag _x(xml, "ILConfig");
                {
                    ScopedTag _x(xml, "AASettings");
                    xml.data("minSampleRate", -1);
                    xml.data("maxSampleRate", 1);
                }
            }
        }

        ILBJobHandle job;
        bex::apiCall(ILBCreateJob(bmh, _T("UVSetJob"), scene, xmlFileName.c_str(), &job));

        // Create pass
        ILBRenderPassHandle fullShadingPass;
        bex::apiCall(ILBCreateFullShadingPass(job, _T("fullShading"), &fullShadingPass));

        // Create Target
        ILBTargetHandle cameraTarget;
        bex::apiCall(ILBCreateCameraTarget(job, _T("cameraTarget"), camera, 512, 512, &cameraTarget));

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
