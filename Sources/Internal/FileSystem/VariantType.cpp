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


namespace DAVA 
{

VariantType::VariantType()
:	type(TYPE_NONE)
,	int32Value(0)
,   pointerValue(NULL)
{
}

VariantType::VariantType(const VariantType &var)
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
	stringValue = value;
}
void VariantType::SetWideString(const WideString & value)
{
    ReleasePointer();
	type = TYPE_WIDE_STRING;
	wideStringValue = value;
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
	return stringValue;
}

const WideString & VariantType::AsWideString() const
{
	DVASSERT(type == TYPE_WIDE_STRING);
	return wideStringValue;
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
			int32 len = (int32)stringValue.length();
			written = fp->Write(&len, 4);
			if (written != 4)return false;
			
			written = fp->Write(stringValue.c_str(), len);
			if (written != len)return false;
		}
		break;	
	case TYPE_WIDE_STRING:
		{
			int32 len = (int32)wideStringValue.length();
			written = fp->Write(&len, 4);
			if (written != 4)return false;
			
			written = fp->Write(wideStringValue.c_str(), len * sizeof(wchar_t));
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
			stringValue = buf;
			delete [] buf;
			if (read != len)return false;
			
		}
        break;	
		case TYPE_WIDE_STRING:
		{ 
			int32 len;
			read = fp->Read(&len, 4);
			if (read != 4)return false;
			
			wideStringValue.resize(len);
			for (int k = 0; k < len; ++k)
			{
				wchar_t c;
				read = fp->Read(&c, sizeof(wchar_t));
				if (read != sizeof(wchar_t))return false;
				wideStringValue[k] = c;
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
            SafeDelete(pData);
            
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
        }
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
        }
    }
    return isEqual;
}

bool VariantType::operator!=(const VariantType& other) const
{
    return (!(*this == other));
}
	
};