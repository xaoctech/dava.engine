/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Final bake with physical materials

The purpose of this sample is to demonstrate how to:
1. How to setup a physical scene for baking
2. How to retrieve the finished light map data
*/

#include "samplescommon.h"

#include <beastapi/beaststring.h>
#include <beastapi/beastutils.h>
#include <beastapi/beastmanager.h>
#include <beastapi/beastscene.h>
#include <beastapi/beastinstance.h>
#include <beastapi/beasttarget.h>
#include <beastapi/beastrenderpass.h>

// Make sure we have a unicode safe cout define
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

int main(char argc, char** argv)
{
    try
    {
        // Setup the beast manager
        ILBManagerHandle bmh = bex::setupBeastManager(_T("baking-physical"));

        // Create a scene
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginPhysicalScene(bmh, "PhysicalFinalScene", &scene));

        bex::InstanceMap instances;
        bex::setupTestScene(bmh, scene, instances);
        bex::setupCamera(scene);

        // Finalize the scene
        bex::apiCall(ILBEndScene(scene));

        ILBJobHandle job;
        bex::apiCall(ILBCreateJob(bmh, _T("TestJob"), scene, _T(""), &job));

        // Set render quality for the job
        bex::apiCall(ILBSetJobRenderQuality(job, 0.5f));

        // Create passes
        ILBRenderPassHandle pass;
        bex::apiCall(ILBCreateIlluminationPass(job, _T("Illumination"), ILB_IM_FULL, &pass));

        // Create targets
        ILBTargetHandle textureTarget;
        bex::apiCall(ILBCreateTextureTarget(job, _T("TextureTarget"), 512, 256, &textureTarget));

        const bool enableWorldSpaceFilter = true;
        if (enableWorldSpaceFilter)
        {
            bex::apiCall(ILBEnableWorldSpaceFilter(textureTarget, ILB_WSFC_DEFAULT));
        }

        // Add bake instances to targets
        ILBTargetEntityHandle textureTargetEntity;
        bex::apiCall(ILBAddBakeInstance(textureTarget, instances[_T("FloorInstance")], &textureTargetEntity));

        // Add passes to targets
        bex::apiCall(ILBAddPassToTarget(textureTarget, pass));

        // Start the job
        const bool showProgressWindow = true;
        bex::apiCall(ILBStartJob(job, showProgressWindow ? ILB_SR_KEEP_OPEN : ILB_SR_NO_DISPLAY, ILB_RD_AUTODETECT));

        // Render and display the result
        int returnCode = 0;
        if (!bex::displayJobResult(job, tcout, textureTarget, pass))
        {
            returnCode = 1;
        }

        // Destroy the job
        bex::apiCall(ILBDestroyJob(job));

        return returnCode;
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
