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


#ifndef __DAVAENGINE_CUSTOM_PROPERTIES_COMPONENT_H__
#define __DAVAENGINE_CUSTOM_PROPERTIES_COMPONENT_H__

#include "Entity/Component.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
	class CustomPropertiesComponent : public Component
	{
	public:
		CustomPropertiesComponent();
		virtual ~CustomPropertiesComponent();
		
		IMPLEMENT_COMPONENT_TYPE(CUSTOM_PROPERTIES_COMPONENT);
				
		virtual Component * Clone(Entity * toEntity);
		virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		
		bool GetBool(const String & key, bool defaultValue = false);
		int32 GetInt32(const String & key, int32 defaultValue = 0);
		uint32 GetUInt32(const String & key, uint32 defaultValue = 0);
		float32 GetFloat(const String & key, float32 defaultValue = 0.0f);
		String GetString(const String & key, const String & defaultValue = "");
		WideString GetWideString(const String & key, const WideString & defaultValue = L"");
		const uint8 *GetByteArray(const String & key, const uint8 *defaultValue = NULL);
		int32 GetByteArraySize(const String & key, int32 defaultValue = 0);
		int64 GetInt64(const String & key, int64 defaultValue = 0);
		uint64 GetUInt64(const String & key, uint64 defaultValue = 0);
		Vector2 GetVector2(const String & key, const Vector2 & defaultValue = Vector2());
		Vector3 GetVector3(const String & key, const Vector3 & defaultValue = Vector3());
		Vector4 GetVector4(const String & key, const Vector4 & defaultValue = Vector4());
		Matrix2 GetMatrix2(const String & key, const Matrix2 & defaultValue = Matrix2());
		Matrix3 GetMatrix3(const String & key, const Matrix3 & defaultValue = Matrix3());
		Matrix4 GetMatrix4(const String & key, const Matrix4 & defaultValue = Matrix4());
		template <class T>
		T GetByteArrayAsType(const String & key, const T & defaultValue = T());
		VariantType *GetVariant(const String & key);

		void SetBool(const String & key, bool value);
		void SetInt32(const String & key, int32 value);
		void SetUInt32(const String & key, uint32 value);
		void SetFloat(const String & key, float32 value);
		void SetString(const String & key, const String & value);
		void SetWideString(const String & key, const WideString & value);
		void SetByteArray(const String & key, const uint8 * value, int32 arraySize);
		void SetVariant(const String & key, const VariantType &value);
		void SetInt64(const String & key, int64 &value);
		void SetUInt64(const String & key, uint64 &value);
		void SetVector2(const String & key, Vector2 &value);
		void SetVector3(const String & key, Vector3 &value);
		void SetVector4(const String & key, Vector4 &value);
		void SetMatrix2(const String & key, Matrix2 &value);
		void SetMatrix3(const String & key, Matrix3 &value);
		void SetMatrix4(const String & key, Matrix4 &value);
		template<class T>
		void SetByteArrayAsType(const String & key, const T & value);

		const Map<String, VariantType*> & GetArchieveData();
		bool IsKeyExists(const String & key);
		void DeleteKey(const String & key);
		void DeleteAllKeys();
		uint32 Count(const String & key = "");
		
		//this method helps to load data for older scene file version
		void LoadFromArchive(const KeyedArchive& srcProperties, SceneFileV2 *sceneFile);
		
	public:
		
		INTROSPECTION_EXTEND(CustomPropertiesComponent, Component,
							 MEMBER(properties, "Custom properties", I_SAVE | I_VIEW | I_EDIT)
		);
	private:
		
		CustomPropertiesComponent(const KeyedArchive& srcProperties);
		
	private:
		
		KeyedArchive* properties;
	};
};

#endif /* defined(__DAVAENGINE_CUSTOM_PROPERTIES_COMPONENT_H__) */
