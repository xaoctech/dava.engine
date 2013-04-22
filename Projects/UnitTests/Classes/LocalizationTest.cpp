#include "LocalizationTest.h"

static const String files[] = {
	"weird_characters",
	"de",
	"en",
	"es",
	"it",
	"ru"
};

LocalizationTest::LocalizationTest()
:	TestTemplate<LocalizationTest>("LocalizationTest")
{
	currentTest = FIRST_TEST;

	for (int32 i = FIRST_TEST; i < FIRST_TEST + TEST_COUNT; ++i)
	{
		RegisterFunction(this, &LocalizationTest::TestFunction, Format("Localization test of %s", files[i].c_str()), NULL);
	}

	srcDir = FilePath("~res:/TestData/LocalizationTest/");
	cpyDir = FileSystem::Instance()->GetCurrentDocumentsDirectory() + FilePath("LocalizationTest/");

	FileSystem::Instance()->DeleteDirectory(cpyDir);
	FileSystem::Instance()->CreateDirectory(cpyDir);
}

void LocalizationTest::LoadResources()
{
}

void LocalizationTest::UnloadResources()
{
}

void LocalizationTest::Draw(const DAVA::UIGeometricData &geometricData)
{
}

void LocalizationTest::TestFunction(TestTemplate<LocalizationTest>::PerfFuncData *data)
{
	FilePath srcFile = srcDir + FilePath(files[currentTest] + ".yaml");
	FilePath cpyFile = cpyDir + FilePath(files[currentTest] + ".yaml");

	FileSystem::Instance()->CopyFile(srcFile, cpyFile);

	LocalizationSystem* localizationSystem = LocalizationSystem::Instance();

	localizationSystem->SetCurrentLocale(files[currentTest]);
	localizationSystem->InitWithDirectory(cpyDir);

	localizationSystem->SaveLocalizedStrings();

	localizationSystem->Cleanup();

	bool res = CompareFiles(srcFile, cpyFile);

	String s = Format("Localization test %d: %s - %s", currentTest, files[currentTest].c_str(), (res ? "passed" : "fail"));
	Logger::Debug(s.c_str());

	data->testData.message = s;
	TEST_VERIFY(res);

	++currentTest;
}

bool LocalizationTest::CompareFiles(const FilePath& file1, const FilePath& file2)
{
	File* f1 = File::Create(file1, File::OPEN | File::READ);
	File* f2 = File::Create(file2, File::OPEN | File::READ);

	bool res = (f1 && f2);

	if (res)
	{
		int32 size = Max(f1->GetSize(), f2->GetSize());

		char* buf1 = new char[size];
		char* buf2 = new char[size];

		while (res && !f1->IsEof() && !f2->IsEof())
		{
			int32 read1;
			int32 read2;

			read1 = f1->ReadLine(buf1, size);
			read2 = f2->ReadLine(buf2, size);

			res &= (read1 == read2);
			res &= !memcmp(buf1, buf2, read1);
		}

		res &= (f1->IsEof() && f2->IsEof());

		delete[] buf1;
		delete[] buf2;
	}

	SafeRelease(f1);
	SafeRelease(f2);

	return res;
}
