/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PROCESSOR_AFFINITY_HPP
#define PROCESSOR_AFFINITY_HPP

#if defined(_WIN32)

#include "cstdmf/stdmf.hpp"

BW_BEGIN_NAMESPACE
/**
 *	These methods aid with setting the processor affinity of the main thread
 *	This has to be done so that the timing can be done as accurately as possible
 */
namespace ProcessorAffinity
{
    void set(uint32 processorHint = 0);
    uint32 get();
    void update();

	void unbindMainThread();
};

BW_END_NAMESPACE

#endif

#endif // PROCESSOR_AFFINITY_HPP
