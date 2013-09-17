/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Job/JobManager.h"
#include "Job/JobQueue.h"
#include "Job/Job.h"
#include "Debug/DVAssert.h"
#include "Base/ScopedPtr.h"
#include "Platform/Thread.h"
#include "Job/JobWaiter.h"

namespace DAVA
{

JobManager::JobManager()
{
	mainQueue = new JobQueue();
}

JobManager::~JobManager()
{
	SafeDelete(mainQueue);
}

void JobManager::Update()
{
	UpdateMainQueue();
}

void JobManager::UpdateMainQueue()
{
	mainQueue->Update();
}

void JobManager::CreateJob(eThreadType threadType, const Message & message)
{
	const Thread::ThreadId & creatorThreadId = Thread::GetCurrentThreadId();
	ScopedPtr<Job> job(new Job(message, creatorThreadId));
	
	OnJobCreated(job);

	if(THREAD_MAIN == threadType)
	{	
		if(Thread::IsMainThread())
		{
			mainQueue->PerformJob(job);
		}
		else
		{
			mainQueue->AddJob(job);
		}
	}
	else
	{
		DVASSERT(0);
	}
}



void JobManager::OnJobCreated(Job * job)
{
	jobsDoneMutex.Lock();

	jobsPerCreatorThread[job->creatorThreadId]++;

	jobsDoneMutex.Unlock();
}

void JobManager::OnJobCompleted(Job * job)
{
	jobsDoneMutex.Lock();

	uint32 & jobsCount = jobsPerCreatorThread[job->creatorThreadId];

	DVASSERT(jobsCount> 0);

	jobsCount--;
	if(0 == jobsCount)
	{
		CheckAndCallWaiterForThreadId(job->creatorThreadId, true);
	}

	jobsDoneMutex.Unlock();
}

JobManager::eWaiterRegistrationResult JobManager::RegisterWaiterForCreatorThread(ThreadIdJobWaiter * waiter)
{
	jobsDoneMutex.Lock();

	JobManager::eWaiterRegistrationResult result = JobManager::WAITER_WILL_WAIT;
	waitersPerCreatorThread[waiter->GetThreadId()] = waiter;

	//check if all desired jobs are already done
	uint32 & jobsCount = jobsPerCreatorThread[waiter->GetThreadId()];
	if(0 == jobsCount)
	{
		CheckAndCallWaiterForThreadId(waiter->GetThreadId(), false);
		result = JobManager::WAITER_RETURN_IMMEDIATELY;
	}

	jobsDoneMutex.Unlock();

	return result;
}


void JobManager::CheckAndCallWaiterForThreadId(const Thread::ThreadId & threadId, bool sendSignal)
{
	Map<Thread::ThreadId,  ThreadIdJobWaiter *>::iterator it = waitersPerCreatorThread.find(threadId);
	if(waitersPerCreatorThread.end() != it)
	{
		if(sendSignal)
		{
			Thread::Broadcast(((*it).second)->GetConditionalVariable());
		}
		waitersPerCreatorThread.erase(it);
	}
}


}