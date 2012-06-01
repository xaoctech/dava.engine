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
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTR ACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "MongodbTest.h"

MongodbTest::MongodbTest()
: TestTemplate<MongodbTest>(String("MongodbTest"))
{
    client = NULL;
}

void MongodbTest::LoadResources()
{
    client = MongodbClient::Create(String("127.0.0.1"), 27017);
//    client = MongodbClient::Create(String("10.128.128.163"), 27017);
    if(client)
    {
        client->SetDatabaseName(String("PerformanceTest"));
        client->SetCollectionName(String("Mongodb"));

        int32 repeatCount = 10;
        RegisterFunction(this, &MongodbTest::BinaryDataTest, "BinaryDataTest", repeatCount,  NULL);
    }
}

void MongodbTest::UnloadResources()
{
    client->DropCollection();
    client->DropDatabase();
    
    SafeRelease(client);
}


void MongodbTest::BinaryDataTest(PerfFuncData * data)
{
    File * log = GameCore::Instance()->logFile;
    log->WriteLine(Format("%s, BinaryDataTest #%d", screenName.c_str(), runIndex));

    //PrepareData
    int32 testDataSize = 10 + runIndex*10;
    uint8 *testData = new uint8[testDataSize];
    for(int32 i = 0; i < testDataSize; ++i)
    {
        testData[i] = i;
    }
    
    //Test DB
    bool retSave = client->SaveBinary(String("TestData"), testData, testDataSize);
    if(retSave)
    {
        int32 dataSize = client->GetBinarySize(String("TestData"));
        if(dataSize == testDataSize)
        {
            uint8 *retData = new uint8[dataSize];
            client->GetBinary(String("TestData"), retData, dataSize);
            
            bool dataIsEqual = true;
            for(int32 i = 0; (i < dataSize) && (dataIsEqual); ++i)
            {
                dataIsEqual = (testData[i] == retData[i]);
            }
            
            if(dataIsEqual)
            {
                log->WriteLine(String("Test succeed"));
            }
            else 
            {
                log->WriteLine(String("Read wrong data"));
            }
            
            SafeDeleteArray(retData);
        }
        else 
        {
            log->WriteLine(Format("Read wrong size: % instead of %d", dataSize, testDataSize));
        }
    }
    else 
    {
        log->WriteLine(String("Can't save binary data"));
    }
    
    
    SafeDeleteArray(testData);
}




