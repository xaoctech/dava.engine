#include "SaveImageTest.h"
#include "GameCore.h"
#include "Render/ImageSystem.h"

using namespace DAVA;

SaveImageTest::SaveImageTest()
    : TestTemplate<SaveImageTest>("SaveImageTest")
{
    FilePath testFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "SaveImageTest/Dest/";
    FileSystem::Instance()->CreateDirectory(testFolder, true);
    RegisterFunction(this, &SaveImageTest::Test, "ImagesTest", NULL);
}

void SaveImageTest::Test(PerfFuncData * data)
{
    ReadWriteTest(data, "~res:/TestData/SaveImageTest/Source/t1.png",FilePath::FilepathInDocuments("SaveImageTest/Dest/t1.png"), true);
    ReadWriteTest(data, "~res:/TestData/SaveImageTest/Source/t2.jpeg",FilePath::FilepathInDocuments("SaveImageTest/Dest/t2.jpeg"), true);
    ReadWriteTest(data, "~res:/TestData/SaveImageTest/Source/t3.jpeg",FilePath::FilepathInDocuments("SaveImageTest/Dest/t3.jpeg"), true);

    
    ReadWriteTest(data, "~res:/TestData/SaveImageTest/Source/t4.pvr", "", false);
    ReadWriteTest(data, "~res:/TestData/SaveImageTest/Source/t5.dds", "", false);
    ReadWriteTest(data, "~res:/TestData/SaveImageTest/Source/t4.pvr",FilePath::FilepathInDocuments("SaveImageTest/Dest/t5.dds"), true, FORMAT_DXT3);
}


void SaveImageTest::ReadWriteTest(PerfFuncData * data, const FilePath& sourcePath, const FilePath& destPath, bool writing, PixelFormat format)
{
    Vector<Image*> images;
    eErrorCode retCode = ImageSystem::Instance()->Load(sourcePath, images);
    TEST_VERIFY(retCode == SUCCESS);
    if(writing)
    {
        retCode = ImageSystem::Instance()->Save(destPath, images, format);
        TEST_VERIFY( retCode == SUCCESS);
    }
    
    for_each(images.begin(), images.end(), SafeRelease<Image>);
}