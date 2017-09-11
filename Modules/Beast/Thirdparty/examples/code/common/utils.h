/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * Utility functions and classes
 */ 
#ifndef UTILS_H
#define UTILS_H

#if defined(WIN32)
#include <tchar.h>
#else
#ifndef _TCHAR
#define _TCHAR char
#define TCHAR _TCHAR
#define _T(x) (x)
#define _tmain main
#endif
#endif

#include <stdexcept>
#include <string>
#include <iostream>
#include <beastapi/beastmanager.h>
#include <beastapi/beaststring.h>
#include <beastapi/beastjob.h>
#include <beastapi/beastmesh.h>
#include <beastapi/beasttexture.h>
//Used for getenv/dupenv
#include <stdlib.h>
#include <sstream>


#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace bex
{
typedef std::basic_string<TCHAR> tstring;
typedef std::basic_ostream<TCHAR> tostream;
typedef std::basic_ofstream<TCHAR> tofstream;

inline tstring string2tstring(const std::string& src)
{
#ifdef UNICODE
    size_t chars = strlen(src.c_str()) + 1;
    tstring result;
    result.resize(chars);

    if (MultiByteToWideChar(CP_ACP, 0, src.c_str(), (int)chars, (LPWSTR)result.c_str(), (int)chars) == 0)
    {
        return L"";
    }
    return result;
#else
    return src;
#endif
}

/**
	 * Class representing wrapping an error code
	 * from the beast api as a c++ exception.
	 */
class Exception : public std::runtime_error
{
public:
    Exception(ILBStatus s)
        : std::runtime_error("Beast exception")
        , status(s)
    {
    }
    ILBStatus status;
};

/**
	 * Function throwing an Exception from
	 * the error code if it's ILB_ST_SUCCESS
	 */
inline void apiCall(ILBStatus s)
{
    if (s != ILB_ST_SUCCESS)
    {
        throw Exception(s);
    }
}

/**
	 * Function returning a string from a beast api
	 * string handle
	 */
inline std::basic_string<TCHAR> convertStringHandle(ILBStringHandle h)
{
    int32 len;
    apiCall(ILBGetLength(h, &len));
    // len -1 since basic_string allocates the null character
    std::basic_string<TCHAR> result(len - 1, '\0');
    apiCall(ILBCopy(h, &result[0], static_cast<int32>(len)));
    apiCall(ILBReleaseString(h));
    return result;
}

/**
	 * Render a scene with progress report
	 * inlined to avoid conflicts if linking two files including primitives.h
	 */
inline bool renderJob(ILBJobHandle job,
                      std::basic_ostream<TCHAR>& logTarget,
                      bool returnWhenComplete = false,
                      bool destroyJob = true,
                      ILBDistributionType distribution = ILB_RD_FORCE_LOCAL)
{
    apiCall(ILBStartJob(job, returnWhenComplete ? ILB_SR_CLOSE_WHEN_DONE : ILB_SR_KEEP_OPEN, distribution));

    ILBBool isRunning = true;

    bool firstJob = true;
    ILBBool completed = false;
    int32 oldProgress = 0;
    while (isRunning)
    {
        apiCall(ILBWaitJobDone(job, 0xffffffff));

        if (!completed)
        {
            apiCall(ILBIsJobCompleted(job, &completed));
            if (completed)
            {
                logTarget << "Job is completed. It might still be running if the user has selected ILB_SR_KEEP_OPEN." << std::endl;
            }
        }

        apiCall(ILBIsJobRunning(job, &isRunning));
        if (isRunning)
        {
            ILBBool newProgress;
            ILBBool newActivity;
            apiCall(ILBJobHasNewProgress(job, &newActivity, &newProgress));
            if (newProgress)
            {
                const int progressBarSize = 20;
                ILBStringHandle taskName;
                int32 progress;
                apiCall(ILBGetJobProgress(job, &taskName, &progress));
                progress /= 100 / progressBarSize;
                std::basic_string<TCHAR> jobNameString = convertStringHandle(taskName);
                if (newActivity)
                {
                    if (!firstJob && oldProgress < progressBarSize)
                    {
                        for (int i = 0; i < progressBarSize - oldProgress; i++)
                        {
                            logTarget << "-";
                        }
                        logTarget << "]" << std::endl;
                    }
                    else
                    {
                        firstJob = false;
                    }
                    logTarget << jobNameString << std::endl;
                    logTarget << "[";
                }
                for (int i = 0; i < progress - oldProgress; i++)
                {
                    logTarget << "-";
                }
                if (progress == progressBarSize && oldProgress < progressBarSize)
                {
                    logTarget << "]" << std::endl;
                }
                logTarget.flush();
                oldProgress = progress;
            }
            logTarget.flush();
        }
    }
    ILBJobStatus status;
    apiCall(ILBGetJobResult(job, &status));

    if (status != ILB_JS_SUCCESS)
    {
        switch (status)
        {
        case ILB_JS_CANCELLED:
            logTarget << "User canceled rendering" << std::endl;
            break;
        case ILB_JS_INVALID_LICENSE:
            logTarget << "Problem with the Beast License!" << std::endl;
            break;
        case ILB_JS_CMDLINE_ERROR:
            logTarget << "Error parsing Beast command line!" << std::endl;
            break;
        case ILB_JS_CONFIG_ERROR:
            logTarget << "Error parsing Beast config files!" << std::endl;
            break;
        case ILB_JS_CRASH:
            logTarget << "Error: Beast crashed!" << std::endl;
            break;
        case ILB_JS_OTHER_ERROR:
            logTarget << "Other error running Beast." << std::endl;
            break;
        }
        return false;
    }

    if (destroyJob)
    {
        apiCall(ILBDestroyJob(job));
    }

    return true;
}

/**
	 * Check if a mesh exists in the cache and return it if it does
	 * inlined to avoid conflicts if linking two files including primitives.h
	 * @param bmh the beastmanager which cache should be used
	 * @param name the name of the mesh to check for
	 * @param target the mesh handle to store the mesh in if found
	 * @param logTarget the stream to write the results of the search to
	 */
inline bool findCachedMesh(ILBManagerHandle bmh, const std::basic_string<TCHAR>& name, ILBMeshHandle& target, std::basic_ostream<TCHAR>& logTarget)
{
    ILBStatus findRes = ILBFindMesh(bmh, name.c_str(), &target);
    if (findRes == ILB_ST_SUCCESS)
    {
        logTarget << "Found the mesh: " << name << ", in the cache!" << std::endl;
        return true;
    }
    else if (findRes == ILB_ST_UNKNOWN_OBJECT)
    {
        logTarget << "Didn't find the mesh: " << name << ", creating it!" << std::endl;
        return false;
    }
    throw Exception(findRes);
}

/**
	 * Check if a texture exists in the cache and return it if it does
	 * inlined to avoid conflicts if linking two files including primitives.h
	 * @param bmh the beastmanager which cache should be used
	 * @param name the name of the texture to check for
	 * @param target the texture handle to store the texture in if found
	 * @param logTarget the stream to write the results of the search to
	 */
inline bool findCachedTexture(ILBManagerHandle bmh, const std::basic_string<TCHAR>& name, ILBTextureHandle& target, std::basic_ostream<TCHAR>& logTarget)
{
    ILBStatus findRes = ILBFindTexture(bmh, name.c_str(), &target);
    if (findRes == ILB_ST_SUCCESS)
    {
        logTarget << "Found the texture: " << name << ", in the cache!" << std::endl;
        return true;
    }
    else if (findRes == ILB_ST_UNKNOWN_OBJECT)
    {
        logTarget << "Didn't find the texture: " << name << ", creating it!" << std::endl;
        return false;
    }
    throw Exception(findRes);
}

#ifdef WIN32
inline std::basic_string<TCHAR> getLicenseKey()
{
    std::basic_stringstream<TCHAR> result;
    char* tempVal;
    size_t length;
    errno_t error = _dupenv_s(&tempVal, &length, "BEAST_LICENSE");
    if (error == 0 && tempVal != NULL)
    {
        result << tempVal;
        free(tempVal);
    }
    return result.str();
}
#else
inline std::basic_string<TCHAR> getLicenseKey()
{
    std::basic_stringstream<TCHAR> result;
    char* temp = getenv("BEAST_LICENSE");
    if (temp != NULL)
    {
        result << temp;
    }
    return result.str();
}
#endif

} // namespace bex



#endif // UTILS_H
