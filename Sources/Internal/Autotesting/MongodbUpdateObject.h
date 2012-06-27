//
//  MongodbUpdateObject.h
//  Framework
//
//  Created by Dmitry Shpakov on 6/27/12.
//  Copyright (c) 2012 DAVA Consulting. All rights reserved.
//

#ifndef __DAVAENGINE_MONGODB_UPDATE_OBJECT_H__
#define __DAVAENGINE_MONGODB_UPDATE_OBJECT_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "FileSystem/KeyedArchive.h"
#include "Database/MongodbObject.h"

namespace DAVA
{
    
class MongodbUpdateObject : public MongodbObject
{
public:
    MongodbUpdateObject();
    virtual ~MongodbUpdateObject();
    
    void LoadData();
    void SaveData();
    
    KeyedArchive* GetData();
    
    MongodbObject* GetUpdateObject();
    
    bool SaveToDB(MongodbClient* dbClient);
    
protected:
    void ReadData(KeyedArchive* archive, void* bsonObj);
    void WriteData(MongodbObject* mongoObj, const String & key, VariantType *value);
    
    MongodbObject* updateObject;
    Vector<MongodbObject*> updateObjects;
    
    KeyedArchive* updateData;
};
    
};

#endif

#endif
