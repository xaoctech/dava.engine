/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Materials live

The purpose of this sample is to demonstrate how to use the API to:
1. How to setup a physical scene
2. How to progressively retrieve the current image
3. Show that the scene is live

*/

#include "samplescommon.h"

#include <beastapi/beaststring.h>
#include <beastapi/beastutils.h>
#include <beastapi/beastmanager.h>
#include <beastapi/beastscene.h>
#include <beastapi/beastcamera.h>
#include <beastapi/beasttarget.h>

// Make sure we have a unicode safe cout define
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

class CameraAnimator : public bex::SceneUpdater
{
public:
    CameraAnimator(ILBCameraHandle camera)
        :
        m_camera(camera)
        ,
        m_timer(0.0f)
        ,
        m_rotation(0.0f)
    {
    }

    virtual void update(float deltaTime)
    {
        const float timerInterval = 8.0f;
        const float rotationStep = 0.4f;
        const float radius = 48.0f;

        m_timer += deltaTime;

        if (m_timer >= timerInterval)
        {
            m_timer -= timerInterval;
            m_rotation += rotationStep;

            bex::Vec3 camPos(radius * sin(m_rotation), 5.0f, radius * cos(m_rotation));
            bex::Vec3 lookAt(0.0f, -3.0f, 0.0f);

            bex::Matrix4x4 transform = bex::setCameraMatrix(camPos,
                                                            normalize(lookAt - camPos),
                                                            bex::Vec3(0.0f, 1.0f, 0.0f));

            ILBSetCameraTransform(m_camera, &transform);
        }
    }

private:
    ILBCameraHandle m_camera;
    float m_timer;
    float m_rotation;
};

int _tmain(int argc, _TCHAR* argv[])
{
    try
    {
        // Setup the beast manager
        ILBManagerHandle bmh = bex::setupBeastManager(_T("materials-live-physical"));

        // Create an empty scene with a camera in it
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginPhysicalScene(bmh, _T("SimpleScene"), &scene));
        ILBCameraHandle camera = bex::setupCamera(scene);
        bex::apiCall(ILBEndScene(scene));

        // Create a live Ernst job
        ILBJobHandle job;
        bex::apiCall(ILBCreateLiveErnstJob(bmh, _T("TestJob"), scene, &job));

        // Create camera Target
        ILBTargetHandle cameraTarget;
        bex::apiCall(ILBCreateCameraTarget(job, _T("cameraTarget"), camera, 800, 600, &cameraTarget));

        // Start the job
        bex::apiCall(ILBStartJob(job, ILB_SR_NO_DISPLAY, ILB_RD_AUTODETECT));

        // Create the entire scene with meshes, textures, materials, light sources and instances live
        bex::InstanceMap instances;
        bex::setupTestScene(bmh, scene, instances);

        // We use an animator that moves the camera to show how it could work in a game editor
        CameraAnimator cameraAnimator(camera);

        // Render and display the result
        int returnCode = 0;
        if (!bex::displayLiveJob(job, tcout, cameraTarget, &cameraAnimator))
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
