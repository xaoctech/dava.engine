/*
Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Unwrapping example

The purpose of this sample is to demonstrate how to unwrap a mesh
and read back the resulting uv layer data.

*/
#include <beastapi/beaststring.h>
#include <beastapi/beastutils.h>
#include <beastapi/beastmanager.h>
#include <beastapi/beasttarget.h>
#include <beastapi/beasttargetentity.h>
#include <beastapi/beastuvlayer.h>
#include <assert.h>

#include "utils.h"
#include "primitives.h"

// Make sure we have a unicode safe cout define
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

typedef std::vector<ILBMeshHandle> MeshHandleVector;

class UnwrapMeshJob
{
protected:
    typedef std::pair<ILBTargetHandle, ILBTargetEntityHandle> UnwrappingHandle;

public:
    UnwrapMeshJob(ILBManagerHandle manager, const std::wstring& jobName, const MeshHandleVector& meshes)
        :
        m_sourceMeshes(meshes)
    {
        bex::apiCall(ILBCreateUVJob(manager, jobName.c_str(), &m_job));

        const float texelsPerWorldUnit = 5.0f;

        // Create unwrap target
        std::stringstream str;
        str << "unwrapTarget" << 0;
        const bex::tstring targetName = bex::string2tstring(str.str());
        ILBTargetHandle unwrappingTarget;
        bex::apiCall(ILBCreateUVUnwrapTarget(m_job, targetName.c_str(), texelsPerWorldUnit, &unwrappingTarget));

        // Add all meshes for unwrapping
        for (unsigned int i = 0; i < m_sourceMeshes.size(); i++)
        {
            ILBMeshHandle mesh = m_sourceMeshes[i];

            ILBTargetEntityHandle unwrappingEntity;
            bex::apiCall(ILBAddMeshToTarget(unwrappingTarget, mesh, &unwrappingEntity));

            m_unwrappingHandles.push_back(UnwrappingHandle(unwrappingTarget, unwrappingEntity));
        }
    }

    bool run()
    {
        bex::apiCall(ILBStartJob(m_job, ILB_SR_CLOSE_WHEN_DONE, ILB_RD_AUTODETECT));

        ILBBool isCompleted = false, isRunning = true;

        while (isRunning)
        {
            bex::apiCall(ILBWaitJobDone(m_job, 0xffffffff));

            if (!isCompleted)
            {
                bex::apiCall(ILBIsJobCompleted(m_job, &isCompleted));
                if (isCompleted)
                {
                    tcout << "Job is completed. It might still be running if the user has selected ILB_SR_KEEP_OPEN." << std::endl;
                }
            }

            bex::apiCall(ILBIsJobRunning(m_job, &isRunning));

            if (isRunning)
            {
                ILBBool newProgress, newActivity;
                bex::apiCall(ILBJobHasNewProgress(m_job, &newActivity, &newProgress));
                if (newProgress)
                {
                    ILBStringHandle taskName;
                    int32 progress;
                    bex::apiCall(ILBGetJobProgress(m_job, &taskName, &progress));

                    if (newActivity)
                    {
                        std::basic_string<TCHAR> jobNameString = bex::convertStringHandle(taskName);
                        tcout << jobNameString << std::endl;
                    }

                    tcout << "Progress " << progress << "%" << std::endl;
                    tcout.flush();
                }
            }
        }

        ILBJobStatus status;
        bex::apiCall(ILBGetJobResult(m_job, &status));

        if (status != ILB_JS_SUCCESS)
        {
            switch (status)
            {
            case ILB_JS_CANCELLED:
                tcout << "User canceled rendering" << std::endl;
                break;
            case ILB_JS_INVALID_LICENSE:
                tcout << "Problem with the Beast License!" << std::endl;
                break;
            case ILB_JS_CMDLINE_ERROR:
                tcout << "Error parsing Beast command line!" << std::endl;
                break;
            case ILB_JS_CONFIG_ERROR:
                tcout << "Error parsing Beast config files!" << std::endl;
                break;
            case ILB_JS_CRASH:
                tcout << "Error: Beast crashed!" << std::endl;
                break;
            case ILB_JS_OTHER_ERROR:
                tcout << "Other error running Beast." << std::endl;
                break;
            }

            bex::apiCall(ILBDestroyJob(m_job));

            return false;
        }

        reportResult();

        bex::apiCall(ILBDestroyJob(m_job));

        return true;
    }

private:
    void reportResult()
    {
        for (size_t i = 0; i < m_unwrappingHandles.size(); i++)
        {
            ILBTargetHandle unwrappingTarget = m_unwrappingHandles[i].first;
            ILBTargetEntityHandle unwrappingEntity = m_unwrappingHandles[i].second;

            ILBUVLayerHandle uvLayer;
            bex::apiCall(ILBGetUVLayerFromTarget(unwrappingTarget, unwrappingEntity, &uvLayer));

            int32 uvLayerSize, uvLayerIndexSize;
            bex::apiCall(ILBGetUVLayerSize(uvLayer, &uvLayerSize));
            bex::apiCall(ILBGetUVLayerIndexSize(uvLayer, &uvLayerIndexSize));

            int32 j;
            for (j = 0; j < uvLayerSize; ++j)
            {
                ILBVec2 uv;
                bex::apiCall(ILBReadUVLayerValues(uvLayer, j, 1, &uv));
                tcout << L"UV value " << j << L" = (" << uv.x << L", " << uv.y << L")" << std::endl;
            }

            for (j = 0; j < uvLayerIndexSize; ++j)
            {
                int32 uvIndex = 0;
                bex::apiCall(ILBReadUVLayerIndexValues(uvLayer, j, 1, &uvIndex));
                tcout << L"UV index value " << j << L" = " << uvIndex << std::endl;
            }

            int32 width = 0;
            int32 height = 0;
            bex::apiCall(ILBGetUVLayerResolutionFromTarget(unwrappingTarget, unwrappingEntity, &width, &height));

            tcout << L"Size of packed UV layout = " << width << L" " << height << std::endl;
        }
    }

private:
    ILBJobHandle m_job;
    MeshHandleVector m_sourceMeshes;
    std::vector<UnwrappingHandle> m_unwrappingHandles;
};

int main(char argc, char** argv)
{
    try
    {
        ILBManagerHandle beastManagerHandle;

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

        // Setup our beast manager
        bex::apiCall(ILBCreateManager(L"../../../temp/unwrapping", ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &beastManagerHandle));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(beastManagerHandle, L"../../../bin"));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(beastManagerHandle));

        // Create sphere and Cornell box meshes
        std::basic_string<TCHAR> sphereMaterialName(_T("SphereMaterial"));
        std::basic_string<TCHAR> boxMaterialName(_T("FloorMaterial"));
        ILBMeshHandle sphereMesh = bex::createSphere(beastManagerHandle, _T("Sphere"), sphereMaterialName, 30, 15);
        ILBMeshHandle boxMesh0 = bex::createCornellBox(beastManagerHandle, _T("Box0"), boxMaterialName);
        ILBMeshHandle boxMesh1 = bex::createCornellBox(beastManagerHandle, _T("Box1"), boxMaterialName);

        MeshHandleVector meshes;
        meshes.push_back(sphereMesh);
        meshes.push_back(boxMesh0);
        meshes.push_back(boxMesh1);

        UnwrapMeshJob job(beastManagerHandle, _T("UnwrapJob"), meshes);

        if (!job.run())
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
        tcout << L"Beast API error" << std::endl;
        tcout << L"Error: " << bex::convertStringHandle(errorString) << std::endl;
        tcout << L"Info: " << bex::convertStringHandle(extendedError) << std::endl;

        throw;

        return 1;
    }
    catch (std::exception& ex)
    {
        tcout << L"Standard exception" << std::endl;
        tcout << L"Error: " << ex.what() << std::endl;
        ;

        throw;

        return 1;
    }
}
