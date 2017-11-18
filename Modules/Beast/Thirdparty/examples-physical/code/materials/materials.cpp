/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Materials

The purpose of this sample is to demonstrate how to use the API to:
1. How to setup a physical scene
2. How to retrieve the finished image

*/

#include "samplescommon.h"

#include <beastapi/beaststring.h>
#include <beastapi/beastutils.h>
#include <beastapi/beastmanager.h>
#include <beastapi/beastscene.h>
#include <beastapi/beastcamera.h>
#include <beastapi/beasttarget.h>
#include <beastapi/beastrenderpass.h>

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
        // Setup the beast manager
        ILBManagerHandle bmh = bex::setupBeastManager(_T("materials-physical"));

        // Create a scene
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginPhysicalScene(bmh, _T("SimpleScene"), &scene));

        bex::InstanceMap instances;
        bex::setupTestScene(bmh, scene, instances);
        ILBCameraHandle camera = bex::setupCamera(scene);

        // Finalize the scene
        bex::apiCall(ILBEndScene(scene));

        // Create a job
        ILBJobHandle job;
        bex::apiCall(ILBCreateJob(bmh, _T("TestJob"), scene, _T(""), &job));

        // Set render quality for the job
        bex::apiCall(ILBSetJobRenderQuality(job, 0.4f));

        // Create pass
        ILBRenderPassHandle fullShadingPass;
        bex::apiCall(ILBCreateFullShadingPass(job, _T("fullShading"), &fullShadingPass));

        // Create a camera Target
        ILBTargetHandle cameraTarget;
        bex::apiCall(ILBCreateCameraTarget(job, _T("cameraTarget"), camera, 800, 600, &cameraTarget));

        // Add pass to targets
        bex::apiCall(ILBAddPassToTarget(cameraTarget, fullShadingPass));

        // Start the job
        const bool showProgressWindow = true;
        bex::apiCall(ILBStartJob(job, showProgressWindow ? ILB_SR_KEEP_OPEN : ILB_SR_NO_DISPLAY, ILB_RD_AUTODETECT));

        // Render and display the result
        int returnCode = 0;
        if (!bex::displayJobResult(job, tcout, cameraTarget, fullShadingPass))
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
