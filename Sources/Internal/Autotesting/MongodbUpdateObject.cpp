//
//  MongodbUpdateObject.cpp
//  Framework
//
//  Created by Dmitry Shpakov on 6/27/12.
//  Copyright (c) 2012 DAVA Consulting. All rights reserved.
//

#include "Autotesting/MongodbUpdateObject.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Database/MongodbClient.h"
#include "Utils/StringFormat.h"
#include "mongodb/bson.h"

namespace DAVA
{

MongodbUpdateObject::MongodbUpdateObject()
    : MongodbObject()
    , updateObject(NULL)
    , updateData(NULL)
{
    
}
    
MongodbUpdateObject::~MongodbUpdateObject()
{
    SafeRelease(updateObject);
    SafeRelease(updateData);
    
    for(int32 i = 0; i < updateObjects.size(); ++i)
    {
        SafeRelease(updateObjects[i]);
    }
}
    
void MongodbUpdateObject::LoadData()
{
    SafeRelease(updateData);
    updateData = new KeyedArchive();
    ReadData(updateData, (bson*)InternalObject());
}    
    
void MongodbUpdateObject::SaveData()
{
    // write data from updateData into updateObject
    SafeRelease(updateObject);
    updateObject = new MongodbObject();
    
    Map<String, VariantType*> archiveData = updateData->GetArchieveData();
    for(Map<String, VariantType*>::iterator it = archiveData.begin(); it != archiveData.end(); ++it)
    {
        WriteData(updateObject, it->first, it->second);
    }
}    
    
KeyedArchive* MongodbUpdateObject::GetData()
{
    return updateData;
}
  
MongodbObject* MongodbUpdateObject::GetUpdateObject()
{
    return updateObject;
}
    
void MongodbUpdateObject::ReadData(KeyedArchive* archive, void* bsonObj)
{
    if((!archive) || (!bsonObj)) return;
    
    bson_iterator it;
    bson_iterator_init(&it, (bson*)bsonObj);
    
    while (bson_iterator_next(&it))
    {
        String key = String(bson_iterator_key(&it));
        bson_type type = bson_iterator_type(&it);
        
        switch (type)
        {
            case BSON_STRING:
                archive->SetString(key, String(bson_iterator_string(&it)));
                break;
                
            case BSON_INT:
                archive->SetInt32(key, bson_iterator_int(&it));
                break;
                
            case BSON_LONG:
                archive->SetInt32(key, bson_iterator_long(&it));
                break;
                
            case BSON_DOUBLE:
                archive->SetFloat(key, bson_iterator_double(&it));
                break;
                
            case BSON_OBJECT:
            {
                bson sub;
                
                bson_iterator_subobject(&it, &sub);
                
                KeyedArchive* subArchive = new KeyedArchive();
                ReadData(subArchive, &sub);
                archive->SetArchive(key, subArchive);
                SafeRelease(subArchive);
                
                //bson_append_bson(object, key, &sub);
                break;
            }
                
            case BSON_OID:
                //TODO: add 12-bytes array
                //bson_append_oid(object, key, bson_iterator_oid(&it));
                break;
                
                
            default:
                DVASSERT(false);
                Logger::Error("[MongodbUpdateObject::ReadData] Not implemented type: %d", type);
                break;
        }
    }    
    
}
    
void MongodbUpdateObject::WriteData(MongodbObject* mongoObj, const String & key, VariantType *value)
{
    if(mongoObj && value)
    {
        
    }
}

bool MongodbUpdateObject::SaveToDB(MongodbClient* dbClient)
{
    if(dbClient && updateObject)
    {
        updateObject->Finish();
        return dbClient->SaveObject(updateObject, this);
    }
    return false;
}
    
};

#endif
