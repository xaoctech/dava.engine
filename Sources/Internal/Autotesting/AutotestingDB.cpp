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
#include "Autotesting/AutotestingDB.h"

#ifdef __DAVAENGINE_AUTOTESTING__

namespace DAVA
{
/*	
bool AutotestingDB::ConnectToDB()
{
	DVASSERT(NULL == dbClient);

	dbClient = MongodbClient::Create(AUTOTESTING_DB_HOST, AUTOTESTING_DB_PORT);
	if(dbClient)
	{
		dbClient->SetDatabaseName(AUTOTESTING_DB_NAME);
		dbClient->SetCollectionName(projectName);
	}

	return (NULL != dbClient);
}

KeyedArchive *AutotestingDB::GetArchive(const String &name)
{
	Logger::Debug("AutotestingDB::GetArchive name=%s", name.c_str());

	MongodbUpdateObject* dbUpdateObject = new MongodbUpdateObject();
	KeyedArchive* dbUpdateData;
		
	bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);
		if(!isFound)
		{
			dbUpdateObject->SetObjectName(testsName);
			Logger::Debug("AutotestingSystem::InsertTestArchive new MongodbUpdateObject %s", testsName.c_str());
			dbUpdateObject->LoadData();
			dbUpdateData = dbUpdateObject->GetData();

			dbUpdateData->SetString("Platform", AUTOTESTING_PLATFORM_NAME);
			dbUpdateData->SetString("Date", Format("%u", testsDate));
			dbUpdateData->SetString("Group", groupName);
			dbUpdateData->SetString("Device", device);

			//SaveToDB(dbUpdateObject);
		}
		else
		{
			dbUpdateObject->LoadData();
			dbUpdateData = dbUpdateObject->GetData();
		}

		return dbUpdateData;
}

bool AutotestingDB::SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName)
{
	bool ret = false;
	
	KeyedArchive* dbUpdateData = FindOrInsertRunArchive(dbUpdateObject, docName);
		
	dbUpdateData->SetArchive(archiveName.c_str(), archive);
	dbUpdateData->SetString("Map", docName.c_str());

	ret = SaveToDB(dbUpdateObject);
	SafeRelease(dbUpdateObject);
	return ret;
}
*/
}

#endif //__DAVAENGINE_AUTOTESTING__