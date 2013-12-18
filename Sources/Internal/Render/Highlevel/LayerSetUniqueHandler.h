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


#ifndef __DAVAENGINE_LAYERSET_UNIQUE_HANDLER_H__
#define __DAVAENGINE_LAYERSET_UNIQUE_HANDLER_H__

#include "Base/FastNameMap.h"

namespace DAVA
{
	class RenderBatch;
	class RenderLayerBatchArray;
	
	struct LayerData
	{
		FastNameSet nameSet;
		RenderLayerBatchArray** layer;
		
		LayerData()
		{
			layer = NULL;
		}
		
		LayerData(const LayerData& data)
		{
			nameSet.Combine(data.nameSet);
			layer = data.layer;
		}
		
		LayerData& operator=(const LayerData& data)
		{
			nameSet.clear();
			nameSet.Combine(data.nameSet);
			layer = data.layer;

			return *this;
		}
	};
	
	class LayerSetUniqueHandler
	{
	public:
		
		void Assign(LayerData* to, const LayerData* from)
		{
			Release(to);
			
			to->nameSet.Combine(from->nameSet);
			if(to->nameSet.size() > 0)
			{
				to->layer = new RenderLayerBatchArray*[to->nameSet.size()];
				memcpy(to->layer, from->layer, to->nameSet.size() * sizeof(RenderLayerBatchArray*));
			}
		}
		
		void Release(LayerData* data)
		{
			//do nothing here
		}
		
		void Clear(LayerData* data)
		{
			if(data->layer)
			{
				SafeDeleteArray(data->layer);
			}
			
			data->nameSet.clear();

		}
		
		bool Equals(const LayerData* a, const LayerData* b)
		{
			return a->nameSet.Equals(b->nameSet);
		}
	};
};

#endif
