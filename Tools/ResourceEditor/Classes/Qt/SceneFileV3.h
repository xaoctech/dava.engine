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
#ifndef __DAVAENGINE_SCENEFILEV3_H__
#define __DAVAENGINE_SCENEFILEV3_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

/*
class TestBase : public DAVA::BaseObject
{
public:
	int in;

	INTROSPECTION_EXTEND(TestBase, DAVA::BaseObject,
		MEMBER(in, "Test in", 0)
		);
};

class TestBaseChild : public TestBase
{
public:
	int ccc;

	INTROSPECTION_EXTEND(TestBaseChild, TestBase, 
		MEMBER(ccc, "ccc", 0)
		);
};

class TestIntro : public DAVA::BaseObject
{
public:
	TestIntro()
	{
		child1 = new TestBaseChild();
		child1->in = 111;
		((TestBaseChild *) child1)->ccc = 222;

		const DAVA::IntrospectionInfo* i1 = DAVA::GetIntrospection(child1);
		const DAVA::IntrospectionInfo* i2 = DAVA::GetIntrospectionByObject<TestBase>(child1);
		const DAVA::IntrospectionInfo* i3 = DAVA::MetaInfo::Instance<TestBase>()->GetIntrospection();
		const DAVA::IntrospectionInfo* i4 = DAVA::MetaInfo::Instance<TestBase>()->GetIntrospection(child1);

		for(int i = 0; i < 10; ++i)
		{
			TestBaseChild *tt = NULL;

			if(i & 0x1)
			{
				tt = new TestBaseChild();
				tt->ccc = i * 5;
				tt->in = i * 10;
			}

			v.push_back(tt);
		}
	}

	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
	TestBase *child1;
	DAVA::Vector<TestBase *> v;

	INTROSPECTION_EXTEND(TestIntro, DAVA::BaseObject,
		MEMBER(e, "Test e", 0)
		MEMBER(f, "Test f", 0)
		MEMBER(a, "Test a", 0)
		MEMBER(b, "Test b", 0)
		MEMBER(c, "Test c", 0)
		MEMBER(d, "Test d", 0)
		MEMBER(child1, "child1", 0)
		COLLECTION(v, "Test collection v", 0)
		);
};
*/

class SceneFileV3 : public DAVA::BaseObject
{
public:
	//void Test(DAVA::BaseObject *object);

	static DAVA::KeyedArchive* SaveIntospection(void *object, const DAVA::IntrospectionInfo *info);
	static void* LoadIntrospection(DAVA::KeyedArchive *archive);

protected:
	static DAVA::KeyedArchive* SerializeIntrospection(void *object, const DAVA::IntrospectionInfo *info);
	static DAVA::KeyedArchive* SerializeCollection(void *object, const DAVA::IntrospectionCollection *collection);

	static void DeserializeIntrospection(void *object, const DAVA::IntrospectionInfo *info, DAVA::KeyedArchive *archive, DAVA::Map<void **, DAVA::uint64> *structure);
	static void DeserializeCollection(void *object, const DAVA::IntrospectionCollection *collection, DAVA::KeyedArchive *archive, DAVA::Map<void **, DAVA::uint64> *structure);

	static void SearchIntrospection(void *object, const DAVA::IntrospectionInfo *info, DAVA::Map<void *, const DAVA::IntrospectionInfo *> *result, bool skipFirst = false);
	static void DumpKeyedArchive(DAVA::KeyedArchive *archive, int level = 0);
};

#endif // __DAVAENGINE_SCENEFILEV3_H__




