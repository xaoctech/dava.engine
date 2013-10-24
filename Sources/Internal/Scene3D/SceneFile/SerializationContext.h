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


#ifndef __DAVAENGINE_SERIALIZATIONCONTEXT_H__
#define __DAVAENGINE_SERIALIZATIONCONTEXT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
	class Scene;
	class DataNode;
	class MaterialSystem;
	class Material;
	class InstanceMaterialState;
	class NMaterial;

	class SerializationContext
	{
	private:
		
		FilePath rootNodePathName;
		FilePath scenePath;
		bool debugLogEnabled;
		Scene* scene;
		uint32 lastError;
		uint32 version;
		Map<uint64, DataNode*> dataBlocks;
	
	public:
		
		~SerializationContext();
				
		inline void SetVersion(uint32 curVersion)
		{
			version = curVersion;
		}
		
		inline uint32 GetVersion() const
		{
			return version;
		}
		
		inline void SetDebugLogEnabled(bool state)
		{
			debugLogEnabled = state;
		}
		
		inline bool IsDebugLogEnabled() const
		{
			return debugLogEnabled;
		}
		
		inline void SetScene(Scene* target)
		{
			scene = target;
		}
		
		inline Scene* GetScene()
		{
			return scene;
		}
		
		inline void SetScenePath(const FilePath& path)
		{
			scenePath = path;
		}
		
		inline const FilePath& GetScenePath() const
		{
			return scenePath;
		}
		
		inline void SetRootNodePath(const FilePath& path)
		{
			rootNodePathName = path;
		}
		
		inline const FilePath& GetRootNodePath() const
		{
			return rootNodePathName;
		}
		
		inline void SetDataBlock(uint64 blockId, DataNode* data)
		{
			dataBlocks[blockId] = data;
		}
		
		inline DataNode* GetDataBlock(uint64 blockId)
		{
			Map<uint64, DataNode*>::iterator it = dataBlocks.find(blockId);
			return (it != dataBlocks.end()) ? it->second : NULL;
		}
		
		inline void SetLastError(uint32 error)
		{
			lastError = error;
		}
		
		inline uint32 GetLastError()
		{
			return lastError;
		}
		
		NMaterial* ConvertOldMaterialToNewMaterial(Material* oldMaterial,
											InstanceMaterialState* oldMaterialState);
		NMaterial* GetNewMaterial(const String& name);

		MaterialSystem* GetMaterialSystem();
	};
};

#endif
