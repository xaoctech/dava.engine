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

#include "HashMapTest.h"

HashMapTest::HashMapTest()
: TestTemplate<HashMapTest>("HashMapTest")
{
    ObjectFactory::Instance()->Dump();
	RegisterFunction(this, &HashMapTest::HashMapInsertRemoveGetTest, "HashMapInsertRemoveGetTest", NULL);
}

void HashMapTest::LoadResources()
{
    GetBackground()->SetColor(Color::White());
}

void HashMapTest::UnloadResources()
{
    
}

void HashMapTest::HashMapInsertRemoveGetTest(PerfFuncData * data)
{
    
//    HashMap<uint32*, uint32*> memoryHashMap(1024);
    uint32 *var1 = new uint32();
    uint32 *var2 = new uint32();
    uint32 *var3 = new uint32();

//    memoryHashMap.Insert(var1, var1);
//    memoryHashMap.Insert(var2, var2);
//    memoryHashMap.Insert(var3, var3);
//    //const void * var1ref = var1;
//    
//    HashMap<uint32*, uint32*>::Iterator find1 = memoryHashMap.Find(var1);
//    TEST_VERIFY(find1 != memoryHashMap.End());
//    TEST_VERIFY(*find1 == var1);
//    
//    HashMap<uint32*, uint32*>::Iterator find2 = memoryHashMap.Find(var2);
//    TEST_VERIFY(find2 != memoryHashMap.End());
//    TEST_VERIFY(*find2 == var2);
//
//    HashMap<uint32*, uint32*>::Iterator find3 = memoryHashMap.Find(var3);
//    TEST_VERIFY(find3 != memoryHashMap.End());
//    TEST_VERIFY(*find3 == var3);
//    
//    HashMap<uint32*, uint32*>::Iterator find4 = memoryHashMap.Find(0);
//    TEST_VERIFY(find4 == memoryHashMap.End()); 
//    
//        
//    const uint32 ITEM_COUNT = 5000;
//    uint32 array[ITEM_COUNT];
//    for (uint32 k = 0; k < ITEM_COUNT - 1; ++k)
//    {
//        memoryHashMap.Insert(&array[k], &array[k + 1]);
//    }
//    
//    for (uint32 k = 0; k < ITEM_COUNT - 1; ++k)
//    {
//        HashMap<uint32*, uint32*>::Iterator findN = memoryHashMap.Find(&array[k]);
//        TEST_VERIFY(findN != memoryHashMap.End());
//        TEST_VERIFY(findN == &array[k + 1]);
//    }    
//    
}
