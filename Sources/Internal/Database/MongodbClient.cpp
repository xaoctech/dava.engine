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
#include "Database/MongodbClient.h"

namespace DAVA 
{
    
#pragma mark --MongodbObject
MongodbObject::MongodbObject()
{
    bson_init(&object);
}
    
MongodbObject::~MongodbObject()
{
    bson_destroy(&object);
}
    
void MongodbObject::SetObjectName(const String &objectname)
{
    bson_append_string(&object, String("_id").c_str(), objectname.c_str());
}

void MongodbObject::AddInt(const String fieldname, int32 value)
{
    bson_append_int(&object, fieldname.c_str(), value);
}
    
void MongodbObject::AddData(const String &fieldname, uint8 *data, int32 dataSize)
{
    bson_append_binary(&object, fieldname.c_str(), BSON_BIN_BINARY, (const char *)data, dataSize);
}
    
void MongodbObject::Finish()
{
    bson_finish(&object);
}
    
int32 MongodbObject::GetInt(const String &fieldname)
{
    int32 retValue = 0;
    
    bson_iterator it;
    bson_iterator_init(&it, &object);
    
    bson_iterator foundIt;
    bool found = FindField(&it, &foundIt, fieldname, true);
    if(found)
    {
        retValue = bson_iterator_int(&foundIt);
    }
    
    return retValue;
}

bool MongodbObject::GetData(const String &fieldname, uint8 *outData, int32 dataSize)
{
    bson_iterator it;
    bson_iterator_init(&it, &object);
    
    bson_iterator foundIt;
    bool found = FindField(&it, &foundIt, fieldname, true);
    if(found)
    {
        uint8 *binaryData = (uint8 *)bson_iterator_bin_data(&foundIt);
        Memcpy(outData, binaryData, dataSize);
        found = true;
    }
    
    return found;
}

bool MongodbObject::FindField(bson_iterator *itIn, bson_iterator *itOut, const String &fieldname, bool recursive)
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

    
#pragma mark --MongodbClient
MongodbClient * MongodbClient::Create(const String &ip, int32 port)
{
	MongodbClient * client = new MongodbClient();
	if (client)
	{
        bool ret = client->Connect(ip, port);
        if(!ret)
        {
            Logger::Error("[MongodbClient] can't connect to database");
        }
        
	}
	return client;
}
	
MongodbClient::MongodbClient()
{
#if defined (__DAVAENGINE_WIN32__)
    mongo_init_sockets();
#endif //#if defined (__DAVAENGINE_WIN32__)
    
    Memset(&connection, 0, sizeof(connection));
    
    SetDatabaseName(String("Database"));
    SetCollectionName(String("Collection"));
}

MongodbClient::~MongodbClient()
{
    Disconnect();
    
    for(int32 i = 0; i < objects.size(); ++i)
    {
        SafeRelease(objects[i]);
    }
    objects.clear();
}

bool MongodbClient::Connect(const String &ip, int32 port)
{
    mongo_init(&connection);
    
    int32 status = mongo_connect(&connection, ip.c_str(), port );
    if(MONGO_OK != status)
    {
        LogError(String("Connect"), connection.err);
    }
    
    return (MONGO_OK == status);
}
    
void MongodbClient::Disconnect()
{
    if(connection.connected)
    {
        mongo_disconnect(&connection);
    }
    mongo_destroy(&connection);
}

void MongodbClient::LogError(const String functionName, int32 errorCode)
{
    static String ERROR_MESSAGES[] = 
    {
        String("Connection success!"), //MONGO_CONN_SUCCESS
        String("Could not create a socket."), //MONGO_CONN_NO_SOCKET
        String("An error occured while calling connect()."), //MONGO_CONN_FAIL
        String("An error occured while calling getaddrinfo()."), //MONGO_CONN_ADDR_FAIL
        String("Warning: connected to a non-master node (read-only)."), //MONGO_CONN_NOT_MASTER
        String("Given rs name doesn't match this replica set."), //MONGO_CONN_BAD_SET_NAME
        String("Can't find primary in replica set. Connection closed."), //MONGO_CONN_NO_PRIMARY
        String("An error occurred while reading or writing on the socket."), //MONGO_IO_ERROR
        String("Other socket error."), //MONGO_SOCKET_ERROR
        String("The response is not the expected length."), //MONGO_READ_SIZE_ERROR
        String("The command returned with 'ok' value of 0."), //MONGO_COMMAND_FAILED
        String("Write with given write_concern returned an error."), //MONGO_WRITE_ERROR
        String("The name for the ns (database or collection) is invalid."), //MONGO_NS_INVALID
        String("BSON not valid for the specified op."), //MONGO_BSON_INVALID
        String("BSON object has not been finished."), //MONGO_BSON_NOT_FINISHED
        String("BSON object exceeds max BSON size."), //MONGO_BSON_TOO_LARGE
        String("Supplied write concern object is invalid.") //MONGO_WRITE_CONCERN_INVALID
    };
    
    DVASSERT((MONGO_CONN_SUCCESS <= errorCode) && (errorCode <= MONGO_WRITE_CONCERN_INVALID));
    
    Logger::Error("[MongodbClient] error %d at function %s", errorCode, functionName.c_str());
    Logger::Error("[MongodbClient] %s", ERROR_MESSAGES[errorCode].c_str()); 
}
    
void MongodbClient::SetDatabaseName(const String &newDatabase)
{
    database = newDatabase;
    namespaceName = database + String(".") + collection;
}

void MongodbClient::SetCollectionName(const String &newCollection)
{
    collection = newCollection;
    namespaceName = database + String(".") + collection;
}
    
    
void MongodbClient::DropDatabase()
{
    int32 status = mongo_cmd_drop_db(&connection, database.c_str());
    if(MONGO_OK != status)
    {
        LogError(String("DropDatabase"), connection.err);
    }
}
    
void MongodbClient::DropCollection()
{
    int32 status = mongo_cmd_drop_collection(&connection, database.c_str(), collection.c_str(), NULL);
    if(MONGO_OK != status)
    {
        LogError(String("DropCollection"), connection.err);
    }
}

    
bool MongodbClient::IsConnected()
{
    return (0 != mongo_is_connected(&connection));
}

bool MongodbClient::SaveBinary(const String &key, uint8 *data, int32 dataSize)
{
    int32 status = MONGO_ERROR;
    if(IsConnected())
    {
        MongodbObject * binary = CreateObject();
        DVASSERT(binary);
        
        binary->SetObjectName(key);
        binary->AddInt(String("DataSize").c_str(), dataSize);
        binary->AddData(String("Data").c_str(), data, dataSize);
        binary->Finish();
        
        
        MongodbObject *foundObject = FindObjectbByKey(key);
        if(foundObject)
        {
            status = mongo_update(&connection, namespaceName.c_str(), &foundObject->object, &binary->object, 0, NULL);
            if(MONGO_OK != status)
            {
                LogError(String("SaveBinary, update"), connection.err);
            }
            
            DestroyObject(foundObject);
        }
        else 
        {
            status = mongo_insert(&connection, namespaceName.c_str(), &binary->object, NULL);
            if(MONGO_OK != status)
            {
                LogError(String("SaveBinary, insert"), connection.err);
            }
        }
        
        DestroyObject(binary);
    }
    
    return (MONGO_OK == status);
}



int32 MongodbClient::GetBinarySize(const String &key)
{
    int32 retSize = 0;
    
    MongodbObject *object = FindObjectbByKey(key);
    if(object)
    {
        retSize = object->GetInt(String("DataSize"));
        DestroyObject(object);
    }
    else 
    {
        Logger::Error("[MongodbClient] Can't find binary to get size.");
    }
    
    return retSize;
}
    
bool MongodbClient::GetBinary(const String &key, uint8 *outData, int32 dataSize)
{
    bool found = false;
    
    MongodbObject *object = FindObjectbByKey(key);
    if(object)
    {
        found = object->GetData(String("Data"), outData, dataSize);
        DestroyObject(object);
    }
    else 
    {
        Logger::Error("[MongodbClient] Can't find binary to get data.");
    }
    
    return found;
}

MongodbObject * MongodbClient::FindObjectbByKey(const String &key)
{
    MongodbObject *query = CreateObject();
    DVASSERT(query);
    
    query->SetObjectName(key);
    query->Finish();
    
    MongodbObject *foundObject = CreateObject();
    DVASSERT(foundObject);
    
    int32 status = mongo_find_one(&connection, namespaceName.c_str(), &query->object, 0, &foundObject->object);
    if(MONGO_OK != status)
    {
        DestroyObject(foundObject);
        foundObject = NULL;
    }
        
    DestroyObject(query);
    return foundObject;
}
    
MongodbObject * MongodbClient::CreateObject()
{
    MongodbObject *object = new MongodbObject();
    objects.push_back(object);
    
    return object;
}

void MongodbClient::DestroyObject(DAVA::MongodbObject *object)
{
    Vector<MongodbObject *>::const_iterator endIt = objects.end();
    for(Vector<MongodbObject *>::iterator it= objects.begin(); it != endIt; ++it)
    {
        if(*it == object)
        {
            SafeRelease(object);
            objects.erase(it);
            break;
        }
    }
}
    
//void MongodbClient::Dump()
//{
//    Logger::Debug("***** MONGO DUMP *******");
//
//    bson query;
//    bson_empty(&query);
//    
//    mongo_cursor *cursor = mongo_find(&connection, namespaceName.c_str(), &query, NULL, 0, 0, 0);
//    int32 count = 0;
//    while( mongo_cursor_next( cursor ) == MONGO_OK )
//    {
//        Logger::Debug("BSON[%d]:", count);
//        
//        const bson *currentObject = mongo_cursor_bson(cursor);
//        bson_print(currentObject);
//        
//        ++count;
//    }
//
//    mongo_cursor_destroy(cursor);
//    
//    Logger::Debug("Count: %d", count);
//    
//    
//    Logger::Debug("************************");
//}
    
    
}