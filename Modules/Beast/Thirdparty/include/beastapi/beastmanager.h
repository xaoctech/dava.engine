/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
  * The beast manager is the core object for all interaction with the Beast API
  */ 
#ifndef BEASTMANAGER_H
#define BEASTMANAGER_H
#include "beastapitypes.h"


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Revision number for released headers.
 * Will increase with every public release with
 * interface changes
 */
#define ILB_BEAST_INTERFACE_VERSION 5

/**
 * Sets the scope for the cache.
 */
typedef enum {
    /**
	 * Makes the cache global. A different new beast manager
	 * using the same cache directory will be able
	 * to find cached resources
	 */
    ILB_CS_GLOBAL,
    /**
	 * Makes the cache local. A different new beast manager
	 * using the same cache directory will not be able
	 * to find cached resources
	 */
    ILB_CS_LOCAL
} ILBCacheScope;

/** 
 * Sets the character type for Beast. 
 * Should generally not be called explicitly but automatically
 * called from ILBCreateManager or ILBSetLogTarget.
 * @param encoding the encoding for input and output strings.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetStringEncodingImp(ILBStringEncoding encoding);

/// @cond IGNORE_IN_DOCUMENTATION
ILB_DLL_FUNCTION ILBStatus ILBCreateManagerImp(uint32 interfaceVersion,
                                               ILBConstString cacheDirectory,
                                               ILBCacheScope cacheScope,
                                               ILBConstString licenseKey,
                                               ILBManagerHandle* beastManager);
/// @endcond

/** 
 * Creates a Beast Manager
 * @param cacheDirectory sets the directory where the Beast Manager stores cached
 * and temporary files.
 * @param cacheScope sets whether the cache is local to this beast manager or it can be
 * reopened by another Beast Manager in the same directory.
 * @param licenseKey the license key provided to you as part of your evaluation/purchase.
 * @param beastManager a pointer to a Beast manager object that will receive the newly allocated handle
 * @return The result of the operation.
 */
static inline ILBStatus ILBCreateManager(ILBConstString cacheDirectory,
                                         ILBCacheScope cacheScope,
                                         ILBConstString licenseKey,
                                         ILBManagerHandle* beastManager)
{
    ILBStatus st = ILBSetStringEncodingImp(ILB_STRING_ENCODING);
    if (st != ILB_ST_SUCCESS)
    {
        return st;
    }
    return ILBCreateManagerImp(ILB_BEAST_INTERFACE_VERSION, cacheDirectory, cacheScope, licenseKey, beastManager);
}

/** 
 * Destroys a Beast Manager\n
 * Will invalidate all resources and handles associated it as well
 * @param beastManager the Beast Manager to destroy
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBDestroyManager(ILBManagerHandle beastManager);

/** 
 * Clears the cache. Will only work if there are no scenes present.
 * @param beastManager the beastManager to clear the cache for
 * @return The result of the operation. 
 */
ILB_DLL_FUNCTION ILBStatus ILBClearCache(ILBManagerHandle beastManager);

/**
 * Sets where the Beast binaries are located. The default search order is:\n
 * 1. The bin directory of where the environment variable BEAST_ROOT points.
 * I.E. BEAST_ROOT\\bin.
 * 2. The directory the beast dll is located in.
 * When calling this, all other search paths are disregarded.
 * @param beastManager the BeastManager to set the root
 * @param beastPath the path to the Beast binaries
 * @return ILB_ST_SUCCESS if everything went ok. ILB_ST_FILE_IO_ERROR if
 * the specified directory doesn't contain a valid set of Beast binaries.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetBeastPath(ILBManagerHandle beastManager,
                                           ILBConstString beastPath);

/**
 * Enum selecting a certain log target
 */
typedef enum {
    /**
	 * Error messages
	 */
    ILB_LT_ERROR,
    /**
	 * Information messages and render progress messages
	 */
    ILB_LT_INFO
} ILBLogType;

/**
 * Enum selecting where to route messages
 */
typedef enum {
    /**
	 * Discards messages
	 */
    ILB_LS_NULL,
    /**
	 * Routes messages to stdout
	 */
    ILB_LS_STDOUT,
    /**
	 * Routes messages to stderr
	 */
    ILB_LS_STDERR,

    /**
	 * Routes messages to a user specified file
	 */
    ILB_LS_FILE,

    /**
	 * Routes messages to the debug output in visual
	 * studio when a debugger is connected
	 */
    ILB_LS_DEBUG_OUTPUT
} ILBLogSink;

/// @cond IGNORE_IN_DOCUMENTATION
ILB_DLL_FUNCTION ILBStatus ILBSetLogTargetImp(ILBLogType type, ILBLogSink sink, ILBConstString filename);
/// @endcond

/** 
 * Sets where log messages should be routed. Note this function is 
 * global rather than connected since some log
 * messages happens before a beast manager may be present or known.
 * Note, this method is not thread safe! Don't call it while other threads 
 * are using Beast
 * @param type the message type to route to this target
 * @param sink where to route the messages
 * @param filename the file to write the log info to. Only used if sink is ILB_LS_FILE
 * @return ILB_ST_SUCCESS if routing was successful.
 */
static inline ILBStatus ILBSetLogTarget(ILBLogType type, ILBLogSink sink, ILBConstString filename)
{
    ILBStatus st = ILBSetStringEncodingImp(ILB_STRING_ENCODING);
    if (st != ILB_ST_SUCCESS)
    {
        return st;
    }
    return ILBSetLogTargetImp(type, sink, filename);
}

/** 
 * Sets where log messages should be routed. Note this function is 
 * global rather than connected since some log
 * messages happens before a beast manager may be present or known.
 * Note, this method is not thread safe! Don't call it while other threads 
 * are using Beast
 * @param type the message type to route to this target
 * @param handle where to route the messages, or ILB_INVALID_FILE_HANDLE to disable logging
 * @return ILB_ST_SUCCESS if routing was successful.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLogTargetFileHandle(ILBLogType type, ILBFileHandle handle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BEASTMANAGER_H
