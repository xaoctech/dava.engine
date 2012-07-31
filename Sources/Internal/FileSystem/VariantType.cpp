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

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "FileSystem/VariantType.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Math/MathConstants.h"
#include "Math/Math2D.h"
#include "Math/Vector.h"
#include "Math/Matrix2.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Utils.h"

namespace DAVA 
{
    const VariantType::PairTypeName VariantType::variantNamesMap[] =
{
    VariantType::PairTypeName(VariantType::TYPE_NONE,          TYPENAME_UNKNOWN),
    VariantType::PairTypeName(VariantType::TYPE_BOOLEAN,       TYPENAME_BOOLEAN),
    VariantType::PairTypeName(VariantType::TYPE_INT32,         TYPENAME_INT32),
    VariantType::PairTypeName(VariantType::TYPE_FLOAT,         TYPENAME_FLOAT),
    VariantType::PairTypeName(VariantType::TYPE_STRING,        TYPENAME_STRING),
    VariantType::PairTypeName(VariantType::TYPE_WIDE_STRING,   TYPENAME_WIDESTRING),
    VariantType::PairTypeName(VariantType::TYPE_BYTE_ARRAY,    TYPENAME_BYTE_ARRAY),
    VariantType::PairTypeName(VariantType::TYPE_UINT32,        TYPENAME_UINT32),
    VariantType::PairTypeName(VariantType::TYPE_KEYED_ARCHIVE, TYPENAME_KEYED_ARCHIVE),
    VariantType::PairTypeName(VariantType::TYPE_INT64,         TYPENAME_INT64),
    VariantType::PairTypeName(VariantType::TYPE_UINT64,        TYPENAME_UINT64),
    VariantType::PairTypeName(VariantType::TYPE_VECTOR2,       TYPENAME_VECTOR2),
    VariantType::PairTypeName(VariantType::TYPE_VECTOR3,       TYPENAME_VECTOR3),
    VariantType::PairTypeName(VariantType::TYPE_VECTOR4,       TYPENAME_VECTOR4),
    VariantType::PairTypeName(VariantType::TYPE_MATRIX2,       TYPENAME_MATRIX2),
    VariantType::PairTypeName(VariantType::TYPE_MATRIX3,       TYPENAME_MATRIX3),
    VariantType::PairTypeName(VariantType::TYPE_MATRIX4,       TYPENAME_MATRIX4)
};

VariantType::VariantType()
:	type(TYPE_NONE)
,	int32Value(0)
//,   pointerValue(NULL)
{
}

VariantType::VariantType(const VariantType &var) : pointerValue(NULL)
{
    type = var.type;
	switch(type)
	{
		case TYPE_BOOLEAN:
		{
            boolValue = var.boolValue;
		}
            break;
		case TYPE_INT32:
		{
            int32Value = var.int32Value;
		}
            break;	
		case TYPE_UINT32:
		{
            uint32Value = var.uint32Value;
		}
            break;	
		case TYPE_FLOAT:
		{
            floatValue = var.floatValue;
		}
            break;	
		case TYPE_STRING:
		{
            stringValue = var.stringValue;
		}
            break;	
		case TYPE_WIDE_STRING:
		{ 
            wideStringValue = var.wideStringValue;
		}
            break;
		case TYPE_BYTE_ARRAY:
		{
            pointerValue = (void*)new Vector<uint8>(*((Vector<uint8>*)var.pointerValue));
		}
            break;
        case TYPE_KEYED_ARCHIVE:
		{
            pointerValue = new KeyedArchive(*((KeyedArchive*)var.pointerValue));
		}
            break;
		case TYPE_INT64:
		{
            int64Value = new int64(var.AsInt64());
		}
            break;
        case TYPE_UINT64:
		{
            uint64Value = new uint64(var.AsUInt64());
		}
            break;
        case TYPE_VECTOR2:
		{
            vector2Value = new Vector2(var.AsVector2());
		}
            break;
        case TYPE_VECTOR3:
		{
            vector3Value = new Vector3(var.AsVector3());
		}
            break;
        case TYPE_VECTOR4:
		{
            vector4Value = new Vector4(var.AsVector4());
		}
            break;
        case TYPE_MATRIX2:
		{
            matrix2Value = new Matrix2(var.AsMatrix2());
		}
            break;
        case TYPE_MATRIX3:
		{
            matrix3Value = new Matrix3(var.AsMatrix3());
		}
            break;
        case TYPE_MATRIX4:
		{
            matrix4Value = new Matrix4(var.AsMatrix4());
		}
            break;
		default:
		{
                //DVASSERT(0 && "Something went wrong with VariantType");
		}
	}
}

VariantType::~VariantType()
{
    ReleasePointer();
}

void VariantType::SetBool(bool value)
{
    ReleasePointer();
	type = TYPE_BOOLEAN;
	boolValue = value;
}

void VariantType::SetInt32(int32 value)
{
    ReleasePointer();
	type = TYPE_INT32;
	int32Value = value;
}

void VariantType::SetUInt32(uint32 value)
{
    ReleasePointer();
    type = TYPE_UINT32;
    uint32Value = value;
}

void VariantType::SetFloat(float32 value)
{
    ReleasePointer();
	type = TYPE_FLOAT;
	floatValue = value;
}

void VariantType::SetString(const String & value)
{
    ReleasePointer();
	type = TYPE_STRING;
	stringValue = new String(value);
}
void VariantType::SetWideString(const WideString & value)
{
    ReleasePointer();
	type = TYPE_WIDE_STRING;
	wideStringValue = new WideString(value);
}

void VariantType::SetByteArray(const uint8 *array, int32 arraySizeInBytes)
{
    ReleasePointer();
	type = TYPE_BYTE_ARRAY;
    pointerValue = (void*)new Vector<uint8>;
    ((Vector<uint8>*)pointerValue)->resize(arraySizeInBytes);
    memcpy(&((Vector<uint8>*)pointerValue)->front(), array, arraySizeInBytes);
}

void VariantType::SetKeyedArchive(KeyedArchive *archive)
{
    ReleasePointer();
    type = TYPE_KEYED_ARCHIVE;
    pointerValue = new KeyedArchive(*archive);
}
    
void VariantType::SetInt64(const int64 & value)
{
    ReleasePointer();
    type = TYPE_INT64;
    int64Value = new int64(value);
}

void VariantType::SetUInt64(const uint64 & value)
{
    ReleasePointer();
    type = TYPE_UINT64;
    uint64Value = new uint64(value);
}

void VariantType::SetVector2(const Vector2 & value)
{
    ReleasePointer();
    type = TYPE_VECTOR2;
    vector2Value = new Vector2(value);
}

void VariantType::SetVector3(const Vector3 & value)
{
    ReleasePointer();
    type = TYPE_VECTOR3;
    vector3Value = new Vector3(value);
}

void VariantType::SetVector4(const Vector4 & value)
{
    ReleasePointer();
    type = TYPE_VECTOR4;
    vector4Value = new Vector4(value);
}

void VariantType::SetMatrix2(const Matrix2 & value)
{
    ReleasePointer();
    type = TYPE_MATRIX2;
    matrix2Value = new Matrix2(value);
}
void VariantType::SetMatrix3(const Matrix3 & value)
{
    ReleasePointer();
    type = TYPE_MATRIX3;
    matrix3Value = new Matrix3(value);
}
void VariantType::SetMatrix4(const Matrix4 & value)
{
    ReleasePointer();
    type = TYPE_MATRIX4;
    matrix4Value = new Matrix4(value);
}
    
bool VariantType::AsBool() const
{
	DVASSERT(type == TYPE_BOOLEAN);
	return boolValue;
}

int32  VariantType::AsInt32() const
{
	DVASSERT(type == TYPE_INT32);
	return int32Value;
}
    
uint32  VariantType::AsUInt32() const
{
    DVASSERT(type == TYPE_UINT32);
    return uint32Value;
}

float32  VariantType::AsFloat() const
{
	DVASSERT(type == TYPE_FLOAT);
	return floatValue;
}

const String &  VariantType::AsString() const
{
	DVASSERT(type == TYPE_STRING);
	return *stringValue;
}

const WideString & VariantType::AsWideString() const
{
	DVASSERT(type == TYPE_WIDE_STRING);
	return *wideStringValue;
}
	
const uint8 *VariantType::AsByteArray() const
{
	DVASSERT(type == TYPE_BYTE_ARRAY);
	return &((Vector<uint8>*)pointerValue)->front();
}
	
int32 VariantType::AsByteArraySize() const
{
	DVASSERT(type == TYPE_BYTE_ARRAY);
	return (int32)((Vector<uint8>*)pointerValue)->size();
}

KeyedArchive *VariantType::AsKeyedArchive() const
{
    DVASSERT(type == TYPE_KEYED_ARCHIVE);
    return (KeyedArchive*)pointerValue;
}
    
int64 VariantType::AsInt64() const
{
    DVASSERT(type == TYPE_INT64);
    return *int64Value;
}

uint64 VariantType::AsUInt64() const
{
    DVASSERT(type == TYPE_UINT64);
    return *uint64Value;
}
    
const Vector2 & VariantType::AsVector2() const
{
    DVASSERT(type == TYPE_VECTOR2);
    return *vector2Value;
}

const Vector3 & VariantType::AsVector3() const
{
    DVASSERT(type == TYPE_VECTOR3);
    return *vector3Value;
}

const Vector4 & VariantType::AsVector4() const
{
    DVASSERT(type == TYPE_VECTOR4);
    return *vector4Value;
}

const Matrix2 & VariantType::AsMatrix2() const
{
    DVASSERT(type == TYPE_MATRIX2);
    return *matrix2Value;
}
    
const Matrix3 & VariantType::AsMatrix3() const
{
    DVASSERT(type == TYPE_MATRIX3);
    return *matrix3Value;
}
    
const Matrix4 & VariantType::AsMatrix4() const
{
    DVASSERT(type == TYPE_MATRIX4);
    return *matrix4Value;
}
    
bool VariantType::Write(File * fp) const
{
	DVASSERT(type != TYPE_NONE)
	int32 written = fp->Write(&type, 1);
	if (written != 1)return false;
	
	switch(type)
	{
	case TYPE_BOOLEAN:
		{
			written = fp->Write(&boolValue, 1);
			if (written != 1)return false;
		}
		break;
	case TYPE_INT32:
		{
			written = fp->Write(&int32Value, 4);
			if (written != 4)return false;
		}
		break;	
    case TYPE_UINT32:
    {
        written = fp->Write(&uint32Value, 4);
        if (written != 4)return false;
    }
        break;	
	case TYPE_FLOAT:
		{
			written = fp->Write(&floatValue, 4);
			if (written != 4)return false;
		}
		break;	
	case TYPE_STRING:
		{
			int32 len = (int32)stringValue->length();
			written = fp->Write(&len, 4);
			if (written != 4)return false;
			
			written = fp->Write(stringValue->c_str(), len);
			if (written != len)return false;
		}
		break;	
	case TYPE_WIDE_STRING:
		{
			int32 len = (int32)wideStringValue->length();
			written = fp->Write(&len, 4);
			if (written != 4)return false;
			
			written = fp->Write(wideStringValue->c_str(), len * sizeof(wchar_t));
			if (written != len * (int)sizeof(wchar_t))return false;
		}
		break;
	case TYPE_BYTE_ARRAY:
		{
			int32 len = (int32)((Vector<uint8>*)pointerValue)->size();
			written = fp->Write(&len, 4);
			if (written != 4)return false;
			
			written = fp->Write(&((Vector<uint8>*)pointerValue)->front(), len);
			if (written != len)return false;
		}
		break;	
    case TYPE_KEYED_ARCHIVE:
		{
            DynamicMemoryFile *pF = DynamicMemoryFile::Create(File::WRITE|File::APPEND);
            ((KeyedArchive *)pointerValue)->Save(pF);
			int32 len = pF->GetSize();
			written = fp->Write(&len, 4);
			if (written != 4)return false;
			
			written = fp->Write(pF->GetData(), len);
            SafeRelease(pF);
			if (written != len)return false;
		}
        break;
    case TYPE_INT64:
		{
            written = fp->Write(int64Value, sizeof(int64));
            if (written != sizeof(int64))return false;
		}
            break;
    case TYPE_UINT64:
		{
            written = fp->Write(uint64Value, sizeof(uint64));
            if (written != sizeof(uint64))return false;
		}
            break;
    case TYPE_VECTOR2:
		{
            written = fp->Write(vector2Value, sizeof(Vector2));
            if (written != sizeof(Vector2))return false;
		}
            break;
    case TYPE_VECTOR3:
		{
            written = fp->Write(vector3Value, sizeof(Vector3));
            if (written != sizeof(Vector3))return false;
		}
            break;
    case TYPE_VECTOR4:
		{
            written = fp->Write(vector4Value, sizeof(Vector4));
            if (written != sizeof(Vector4))return false;
		}
            break;
    case TYPE_MATRIX2:
		{
            written = fp->Write(matrix2Value, sizeof(Matrix2));
            if (written != sizeof(Matrix2))return false;
		}
            break;
    case TYPE_MATRIX3:
		{
            written = fp->Write(matrix3Value, sizeof(Matrix3));
            if (written != sizeof(Matrix3))return false;
		}
            break;
    case TYPE_MATRIX4:
		{
            written = fp->Write(matrix4Value, sizeof(Matrix4));
            if (written != sizeof(Matrix4))return false;
		}
            break;
            
	}
	return true;
}

bool VariantType::Read(File * fp)
{
	int32 read = fp->Read(&type, 1);
	if (read == 0)return false;
	
	switch(type)
	{
		case TYPE_BOOLEAN:
		{
			read = fp->Read(&boolValue, 1);
			if (read != 1)return false;
		}
		break;
		case TYPE_INT32:
		{
			read = fp->Read(&int32Value, 4);
			if (read != 4)return false;
		}
        break;	
		case TYPE_UINT32:
		{
			read = fp->Read(&uint32Value, 4);
			if (read != 4)return false;
		}
        break;	
		case TYPE_FLOAT:
		{
			read = fp->Read(&floatValue, 4);
			if (read != 4)return false;
		}
        break;	
		case TYPE_STRING:
		{
			int32 len;
			read = fp->Read(&len, 4);
			if (read != 4)return false;
			
			char *buf = new char[len + 1];
			read = fp->Read(buf, len);
			buf[len] = 0;
			stringValue = new String(buf);
			delete [] buf;
			if (read != len)return false;
			
		}
        break;	
		case TYPE_WIDE_STRING:
		{ 
			int32 len;
			read = fp->Read(&len, 4);
			if (read != 4)return false;
			
            wideStringValue = new WideString();
			wideStringValue->resize(len);
			for (int k = 0; k < len; ++k)
			{
				wchar_t c;
				read = fp->Read(&c, sizeof(wchar_t));
				if (read != sizeof(wchar_t))return false;
				(*wideStringValue)[k] = c;
			}
		}
        break;
		case TYPE_BYTE_ARRAY:
		{
            ReleasePointer();
			int32 len;
			read = fp->Read(&len, 4);
			if (read != 4)return false;
			
            pointerValue = (void*)new Vector<uint8>;
            ((Vector<uint8>*)pointerValue)->resize(len);
			read = fp->Read(&((Vector<uint8>*)pointerValue)->front(), len);
			if (read != len)return false;
		}
        break;	
		case TYPE_KEYED_ARCHIVE:
		{
            ReleasePointer();
			int32 len;
			read = fp->Read(&len, 4);
			if (read != 4)return false;
			
            uint8 *pData = new uint8[len];
			read = fp->Read(pData, len);
			if (read != len)return false;
            DynamicMemoryFile *pF = DynamicMemoryFile::Create(pData, len, File::READ);
            pointerValue = new KeyedArchive();
            ((KeyedArchive*)pointerValue)->Load(pF);
            SafeRelease(pF);
            SafeDeleteArray(pData);
		}
        break;
        case TYPE_INT64:
		{
            int64Value = new int64;
            read = fp->Read(int64Value, sizeof(int64));
            if (read != sizeof(int64))return false;
		}
            break;
        case TYPE_UINT64:
		{
            uint64Value = new uint64;
            read = fp->Read(uint64Value, sizeof(uint64));
            if (read != sizeof(uint64))return false;
		}
            break;
        case TYPE_VECTOR2:
		{
            vector2Value = new Vector2;
            read = fp->Read(vector2Value, sizeof(Vector2));
            if (read != sizeof(Vector2))return false;
		}
            break;
        case TYPE_VECTOR3:
		{
            vector3Value = new Vector3;
            read = fp->Read(vector3Value, sizeof(Vector3));
            if (read != sizeof(Vector3))return false;
		}
            break;
        case TYPE_VECTOR4:
		{
            vector4Value = new Vector4;
            read = fp->Read(vector4Value, sizeof(Vector4));
            if (read != sizeof(Vector4))return false;
		}
            break;
        case TYPE_MATRIX2:
		{
            matrix2Value = new Matrix2;
            read = fp->Read(matrix2Value, sizeof(Matrix2));
            if (read != sizeof(Matrix2))return false;
		}
            break;
        case TYPE_MATRIX3:
		{
            matrix3Value = new Matrix3;
            read = fp->Read(matrix3Value, sizeof(Matrix3));
            if (read != sizeof(Matrix3))return false;
		}
            break;
        case TYPE_MATRIX4:
		{
            matrix4Value = new Matrix4;
            read = fp->Read(matrix4Value, sizeof(Matrix4));
            if (read != sizeof(Matrix4))return false;
		}
            break;

		default:
		{
			//DVASSERT(0 && "Something went wrong with VariantType");
		}
	}
	return true;
}
	
	
void VariantType::ReleasePointer()
{
    if (pointerValue) 
    {
        switch (type) 
        {
            case TYPE_BYTE_ARRAY:
            {
                delete (Vector<uint8> *)pointerValue;
            }
                break;
            case TYPE_KEYED_ARCHIVE:
            {
                ((KeyedArchive *)pointerValue)->Release();
            }
                break;
            case TYPE_INT64:
            {
                delete int64Value;
            }
                break;
            case TYPE_UINT64:
            {
                delete uint64Value;
            }
                break;
            case TYPE_VECTOR2:
            {
                delete vector2Value;
            }
                break;
            case TYPE_VECTOR3:
            {
                delete vector3Value;
            }
                break;
            case TYPE_VECTOR4:
            {
                delete vector4Value;
            }
                break;
            case TYPE_MATRIX2:
            {
                delete matrix2Value;
            }
                break;
            case TYPE_MATRIX3:
            {
                delete matrix3Value;
            }
                break;
            case TYPE_MATRIX4:
            {
                delete matrix4Value;
            }
                break;

            case TYPE_STRING:
            {
                delete stringValue;
            }
                break;

            case TYPE_WIDE_STRING:
            {
                delete wideStringValue;
            }
                break;
        }
        
        // It is enough to set only pointerValue to NULL - all other pointers are in union, so
        // actually points to the same address and thus will be set to NULL too.
        pointerValue = NULL;
    }
}

bool VariantType::operator==(const VariantType& other) const
{
    bool isEqual = false;
    if(type == other.type)
    {
        switch(type)
        {
            case TYPE_BOOLEAN:
                isEqual = (AsBool() == other.AsBool());
                break;
		    case TYPE_INT32:
                isEqual = (AsInt32() == other.AsInt32());
                break;
		    case TYPE_FLOAT:
                isEqual = (AsFloat() == other.AsFloat());
                break;
		    case TYPE_STRING:
                isEqual = (AsString() == other.AsString());
                break;
            case TYPE_WIDE_STRING:
                isEqual = (AsWideString() == other.AsWideString());
                break;
		    case TYPE_BYTE_ARRAY:
            {
                int32 byteArraySize = AsByteArraySize();
                if(byteArraySize == other.AsByteArraySize())
                {
                    isEqual = true;
                    const uint8* byteArray = AsByteArray();
                    const uint8* otherByteArray = other.AsByteArray();
                    if(byteArray != otherByteArray)
                    {
                        for(int32 i = 0; i < byteArraySize; ++i)
                        {
                            if(byteArray[i]!=otherByteArray[i])
                            {
                                isEqual = false;
                                break;
                            }
                        }
                    }
                }
            }
                break;
            case TYPE_UINT32:
                isEqual = (AsUInt32() == other.AsUInt32());
                break;
            case TYPE_KEYED_ARCHIVE:
                {
                    KeyedArchive* keyedArchive = AsKeyedArchive();
                    KeyedArchive* otherKeyedArchive = other.AsKeyedArchive();
                    if(keyedArchive && otherKeyedArchive)
                    {
                        isEqual = true;
                        if(keyedArchive != otherKeyedArchive)
                        {                                
                            const Map<String, VariantType*> data = keyedArchive->GetArchieveData();
                            const Map<String, VariantType*> otherData = otherKeyedArchive->GetArchieveData();
                            for(Map<String, VariantType*>::const_iterator it = data.begin(); it != data.end(); ++it)
                            {
                                Map<String, VariantType*>::const_iterator findIt = otherData.find(it->first);
                                if(findIt != otherData.end())
                                {
                                    if(it->second != findIt->second)
                                    {
                                        if((*it->second) != (*findIt->second))
                                        {
                                            isEqual = false;
                                            break;
                                        }
                                    }
                                }
                                else
                                {
                                    isEqual = false;
                                    break;
                                }
                            }
                        }
                    }
                }
                break;
            case TYPE_INT64:
                isEqual = (AsInt64() == other.AsInt64());
                break;
            case TYPE_UINT64:
                isEqual = (AsUInt64()== other.AsUInt64());
                break;
            case TYPE_VECTOR2:
            {
                isEqual = ( AsVector2() == other.AsVector2());
            }
                break;
            case TYPE_VECTOR3:
            {
                isEqual = ( AsVector3() == other.AsVector3());
            }
                break;
            case TYPE_VECTOR4:
            {
                isEqual = ( AsVector4() == other.AsVector4());
            }
                break;
            case TYPE_MATRIX2:
            {
                isEqual = ( AsMatrix2() == other.AsMatrix2());
            }
                break;
            case TYPE_MATRIX3:
            {
                isEqual = ( AsMatrix3() == other.AsMatrix3());
            }
                break;
            case TYPE_MATRIX4:
            {
                isEqual = ( AsMatrix4() == other.AsMatrix4());
            }
                break;
                
        }
    }
    return isEqual;
}

bool VariantType::operator!=(const VariantType& other) const
{
    return (!(*this == other));
}
	
};