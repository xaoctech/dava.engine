/*==================================================================================
 Copyright (c) 2008, DAVA, INC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/


#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
	CustomPropertiesComponent::CustomPropertiesComponent()
	{
		properties = new KeyedArchive();
	}
	
	CustomPropertiesComponent::CustomPropertiesComponent(const KeyedArchive& srcProperties)
	{
		properties = new KeyedArchive(srcProperties);
	}
	
	CustomPropertiesComponent::~CustomPropertiesComponent()
	{
		SafeRelease(properties);
	}
		
	Component* CustomPropertiesComponent::Clone(Entity * toEntity)
	{
		CustomPropertiesComponent* newProperties = new CustomPropertiesComponent(*properties);
		newProperties->SetEntity(toEntity);
		
		return newProperties;
	}
	
	void CustomPropertiesComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
	{
		Component::Serialize(archive, sceneFile);
		
		if(NULL != archive && properties->Count() > 0)
		{
			String savedPath = "";
			if(properties->IsKeyExists("editor.referenceToOwner"))
			{
				savedPath = properties->GetString("editor.referenceToOwner");
				String newPath = FilePath(savedPath).GetRelativePathname(sceneFile->GetScenePath());
				properties->SetString("editor.referenceToOwner", newPath);
			}
			
			archive->SetByteArrayFromArchive("cpc.properties", properties);
			
			if(savedPath.length())
			{
				properties->SetString("editor.referenceToOwner", savedPath);
			}
		}
	}
	
	void CustomPropertiesComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
	{
		properties->DeleteAllKeys();
		
		if(NULL != archive && archive->IsKeyExists("cpc.properties"))
		{
			SafeRelease(properties);
			properties = archive->GetArchiveFromByteArray("cpc.properties");
			
			if(properties && properties->IsKeyExists("editor.referenceToOwner"))
			{
				FilePath newPath(sceneFile->GetScenePath());
				newPath += properties->GetString("editor.referenceToOwner");
				
				//TODO: why we use absolute pathname instead of relative?
				properties->SetString("editor.referenceToOwner", newPath.GetAbsolutePathname());
			}
		}
		
		Component::Deserialize(archive, sceneFile);

	}
	
	bool CustomPropertiesComponent::GetBool(const String & key, bool defaultValue)
	{
		return properties->GetBool(key, defaultValue);
	}
	
	int32 CustomPropertiesComponent::GetInt32(const String & key, int32 defaultValue)
	{
		return properties->GetInt32(key, defaultValue);
	}
	
	uint32 CustomPropertiesComponent::GetUInt32(const String & key, uint32 defaultValue)
	{
		return properties->GetUInt32(key, defaultValue);
	}
	
	float32 CustomPropertiesComponent::GetFloat(const String & key, float32 defaultValue)
	{
		return properties->GetFloat(key, defaultValue);
	}
	
	String CustomPropertiesComponent::GetString(const String & key, const String & defaultValue)
	{
		return properties->GetString(key, defaultValue);
	}
	
	WideString CustomPropertiesComponent::GetWideString(const String & key, const WideString & defaultValue)
	{
		return properties->GetWideString(key, defaultValue);
	}
	
	const uint8* CustomPropertiesComponent::GetByteArray(const String & key, const uint8 *defaultValue)
	{
		return properties->GetByteArray(key, defaultValue);
	}
	
	int32 CustomPropertiesComponent::GetByteArraySize(const String & key, int32 defaultValue)
	{
		return properties->GetByteArraySize(key, defaultValue);
	}
	
	int64 CustomPropertiesComponent::GetInt64(const String & key, int64 defaultValue)
	{
		return properties->GetInt64(key, defaultValue);
	}
	
	uint64 CustomPropertiesComponent::GetUInt64(const String & key, uint64 defaultValue)
	{
		return properties->GetUInt64(key, defaultValue);
	}
	
	Vector2 CustomPropertiesComponent::GetVector2(const String & key, const Vector2 & defaultValue)
	{
		return properties->GetVector2(key, defaultValue);
	}
	
	Vector3 CustomPropertiesComponent::GetVector3(const String & key, const Vector3 & defaultValue)
	{
		return properties->GetVector3(key, defaultValue);
	}
	
	Vector4 CustomPropertiesComponent::GetVector4(const String & key, const Vector4 & defaultValue)
	{
		return properties->GetVector4(key, defaultValue);
	}
	
	Matrix2 CustomPropertiesComponent::GetMatrix2(const String & key, const Matrix2 & defaultValue)
	{
		return properties->GetMatrix2(key, defaultValue);
	}
	
	Matrix3 CustomPropertiesComponent::GetMatrix3(const String & key, const Matrix3 & defaultValue)
	{
		return properties->GetMatrix3(key, defaultValue);
	}
	
	Matrix4 CustomPropertiesComponent::GetMatrix4(const String & key, const Matrix4 & defaultValue)
	{
		return properties->GetMatrix4(key, defaultValue);
	}
	
	template <class T>
	T CustomPropertiesComponent::GetByteArrayAsType(const String & key, const T & defaultValue)
	{
		return properties->GetByteArrayAsType<T>(key, defaultValue);
	}
	
	VariantType* CustomPropertiesComponent::GetVariant(const String & key)
	{
		return properties->GetVariant(key);
	}
	
	void CustomPropertiesComponent::SetBool(const String & key, bool value)
	{
		properties->SetBool(key, value);
	}
	
	void CustomPropertiesComponent::SetInt32(const String & key, int32 value)
	{
		properties->SetInt32(key, value);
	}
	
	void CustomPropertiesComponent::SetUInt32(const String & key, uint32 value)
	{
		properties->SetUInt32(key, value);
	}
	
	void CustomPropertiesComponent::SetFloat(const String & key, float32 value)
	{
		properties->SetFloat(key, value);
	}
	
	void CustomPropertiesComponent::SetString(const String & key, const String & value)
	{
		properties->SetString(key, value);
	}
	
	void CustomPropertiesComponent::SetWideString(const String & key, const WideString & value)
	{
		properties->SetWideString(key, value);
	}
	
	void CustomPropertiesComponent::SetByteArray(const String & key, const uint8 * value, int32 arraySize)
	{
		properties->SetByteArray(key, value, arraySize);
	}
	
	void CustomPropertiesComponent::SetVariant(const String & key, const VariantType &value)
	{
		properties->SetVariant(key, value);
	}
	
	void CustomPropertiesComponent::SetInt64(const String & key, int64 &value)
	{
		properties->SetInt64(key, value);
	}
	
	void CustomPropertiesComponent::SetUInt64(const String & key, uint64 &value)
	{
		properties->SetUInt64(key, value);
	}
	
	void CustomPropertiesComponent::SetVector2(const String & key, Vector2 &value)
	{
		properties->SetVector2(key, value);
	}
	
	void CustomPropertiesComponent::SetVector3(const String & key, Vector3 &value)
	{
		properties->SetVector3(key, value);
	}
	
	void CustomPropertiesComponent::SetVector4(const String & key, Vector4 &value)
	{
		properties->SetVector4(key, value);
	}
	
	void CustomPropertiesComponent::SetMatrix2(const String & key, Matrix2 &value)
	{
		properties->SetMatrix2(key, value);
	}
	
	void CustomPropertiesComponent::SetMatrix3(const String & key, Matrix3 &value)
	{
		properties->SetMatrix3(key, value);
	}
	
	void CustomPropertiesComponent::SetMatrix4(const String & key, Matrix4 &value)
	{
		properties->SetMatrix4(key, value);
	}
	
	template<class T>
	void CustomPropertiesComponent::SetByteArrayAsType(const String & key, const T & value)
	{
		properties->SetByteArrayAsType<T>(key, value);
	}
	
	bool CustomPropertiesComponent::IsKeyExists(const String & key)
	{
		return properties->IsKeyExists(key);
	}
	
	void CustomPropertiesComponent::DeleteKey(const String & key)
	{
		properties->DeleteKey(key);
	}
	
	void CustomPropertiesComponent::DeleteAllKeys()
	{
		properties->DeleteAllKeys();
	}
	
	uint32 CustomPropertiesComponent::Count(const String & key)
	{
		return properties->Count();
	}
	
	const Map<String, VariantType*> & CustomPropertiesComponent::GetArchieveData()
	{
		return properties->GetArchieveData();
	}
	
	void CustomPropertiesComponent::LoadFromArchive(const KeyedArchive& srcProperties, SceneFileV2 *sceneFile)
	{
		SafeRelease(properties);
		properties = new KeyedArchive(srcProperties);
		
		if(properties && properties->IsKeyExists("editor.referenceToOwner"))
		{
			FilePath newPath(sceneFile->GetScenePath());
			newPath += properties->GetString("editor.referenceToOwner");
			
			//TODO: why we use absolute pathname instead of relative?
			properties->SetString("editor.referenceToOwner", newPath.GetAbsolutePathname());
		}
	}
};