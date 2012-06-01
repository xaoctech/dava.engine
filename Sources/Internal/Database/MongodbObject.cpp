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
#include "Database/MongodbObject.h"
#include "mongodb/bson.h"

namespace DAVA 
{

class MongodbObjectInternalData: public BaseObject
{
public:

	MongodbObjectInternalData()
	{
		object = new bson();
		DVASSERT(object);

		bson_init(object);
	}

	virtual ~MongodbObjectInternalData()
	{
		bson_destroy(object);
		SafeDelete(object);
	}

	bool FindField(bson_iterator *itIn, bson_iterator *itOut, const String &fieldname, bool recursive)
	{
		bool found = false;
		while(!found && bson_iterator_next(itIn))
		{
			String itKey = String(bson_iterator_key(itIn));
			if(fieldname == itKey)
			{
				*itOut = *itIn;
				found = true;
			}
			else if(recursive && (BSON_OBJECT == bson_iterator_type(itIn)))
			{
				bson_iterator subIt;
				bson_iterator_subiterator(itIn, &subIt);

				found = FindField(&subIt, itOut, fieldname, recursive);
			}
		}

		return found;
	}

public:
	bson *object;
};

MongodbObject::MongodbObject()
{
	objectData = new MongodbObjectInternalData();
	DVASSERT(objectData);
}
    
MongodbObject::~MongodbObject()
{
	SafeRelease(objectData);
}
    
void MongodbObject::SetObjectName(const String &objectname)
{
    bson_append_string(objectData->object, String("_id").c_str(), objectname.c_str());
}

void MongodbObject::AddInt(const String fieldname, int32 value)
{
    bson_append_int(objectData->object, fieldname.c_str(), value);
}
    
void MongodbObject::AddData(const String &fieldname, uint8 *data, int32 dataSize)
{
    bson_append_binary(objectData->object, fieldname.c_str(), BSON_BIN_BINARY, (const char *)data, dataSize);
}
    
void MongodbObject::Finish()
{
    bson_finish(objectData->object);
}
    
int32 MongodbObject::GetInt(const String &fieldname)
{
    int32 retValue = 0;
    
    bson_iterator it;
    bson_iterator_init(&it, objectData->object);
    
    bson_iterator foundIt;
    bool found = objectData->FindField(&it, &foundIt, fieldname, true);
    if(found)
    {
        retValue = bson_iterator_int(&foundIt);
    }
    
    return retValue;
}

bool MongodbObject::GetData(const String &fieldname, uint8 *outData, int32 dataSize)
{
    bson_iterator it;
    bson_iterator_init(&it, objectData->object);
    
    bson_iterator foundIt;
    bool found = objectData->FindField(&it, &foundIt, fieldname, true);
    if(found)
    {
        uint8 *binaryData = (uint8 *)bson_iterator_bin_data(&foundIt);
        Memcpy(outData, binaryData, dataSize);
        found = true;
    }
    
    return found;
}

void * MongodbObject::InternalObject()
{
	return objectData->object;
}

}