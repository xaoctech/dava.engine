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

namespace DAVA
{  
    
MongodbUpdateObject::MongodbUpdateObject()
    : MongodbObject()
    , updateObject(NULL)
    , updateData(new KeyedArchive())
{
}
    
MongodbUpdateObject::~MongodbUpdateObject()
{
    SafeRelease(updateObject);
    SafeRelease(updateData);
}
    
void MongodbUpdateObject::LoadData()
{
    SafeRelease(updateData);
    updateData = new KeyedArchive();
    Finish();
    
    MongodbClient::DBObjectToKeyedArchive(this, updateData);
}    
    
void MongodbUpdateObject::SaveData()
{
    SafeRelease(updateObject);
    
    updateObject = new MongodbObject();
    updateObject->SetObjectName(GetObjectName());
    MongodbClient::KeyedArchiveToDBObject(updateData, updateObject);
}    
    
KeyedArchive* MongodbUpdateObject::GetData()
{
    return updateData;
}
  
MongodbObject* MongodbUpdateObject::GetUpdateObject()
{
    return updateObject;
}

bool MongodbUpdateObject::SaveToDB(MongodbClient* dbClient)
{
    if(!updateObject)
    {
        SaveData();
    }
    
    if(dbClient && updateObject)
    {
        updateObject->Finish();
        
        //Logger::Debug("MongodbUpdateObject::SaveToDB old object:");
        //Print();
        
        //Logger::Debug("MongodbUpdateObject::SaveToDB new object:");
        //updateObject->Print();
        
        return dbClient->SaveObject(updateObject);
    }
    return false;
}
    
};

#endif
