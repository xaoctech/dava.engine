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
    
    bool LoadData();
    bool SaveData();
    
    KeyedArchive* GetData();
    
    MongodbObject* GetUpdateObject();
    
    bool SaveToDB(MongodbClient* dbClient);
    
protected:
    void InitWithData();
    
    MongodbObject* updateObject;
    
    KeyedArchive* updateData;
};
    
};

#endif

#endif
