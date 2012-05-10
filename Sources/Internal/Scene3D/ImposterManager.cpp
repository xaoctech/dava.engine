/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "ImposterManager.h"
#include "Scene3D/ImposterNode.h"

namespace DAVA
{

ImposterManager::ImposterManager()
{

}

ImposterManager::~ImposterManager()
{

}

bool ImposterManager::IsEmpty()
{
	return imposters.empty();
}

void ImposterManager::Update(float32 frameTime)
{
	List<ImposterNode*>::iterator end = imposters.end();
	for(List<ImposterNode*>::iterator iter = imposters.begin(); iter != end; ++iter)
	{
		ImposterNode * node = *iter;
		node->UpdateState();
		if(node->IsAskingForRedraw())
		{
			AddToQueue(node);
		}
	}

	ProcessQueue();
}

void ImposterManager::ProcessQueue()
{
	if(!queue.empty())
	{
		ImposterNode * node = queue.front();
		queue.pop_front();

		node->ApproveRedraw();
	}
}

void ImposterManager::Draw()
{
	List<ImposterNode*>::iterator end = imposters.end();
	for(List<ImposterNode*>::iterator iter = imposters.begin(); iter != end; ++iter)
	{
		(*iter)->GeneralDraw();
	}
}

void ImposterManager::Add(ImposterNode * node)
{
	//Add can be called multiple times for one node, because ImposterNode::SetScene can be called multiple times
	List<ImposterNode*>::iterator iter = find(imposters.begin(), imposters.end(), node);
	if(imposters.end() == iter)
	{
		imposters.push_back(node);
	}
}

void ImposterManager::Remove(ImposterNode * node)
{
	List<ImposterNode*>::iterator iter = find(imposters.begin(), imposters.end(), node);
	if(imposters.end() == iter)
	{
		Logger::Error("ImposterManager::Remove node not found");
	}
	else
	{
		imposters.erase(iter);
	}
}

void ImposterManager::AddToQueue(ImposterNode * node)
{
	if(!node->IsQueued())
	{
		queue.push_back(node);
		node->OnAddedToQueue();
	}
}



};