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
    InnerTest("~res:/TestData/SaveImageTest/Source/t1.png",FilePath::FilepathInDocuments("SaveImageTest/Dest/t1.png"),data);
    InnerTest("~res:/TestData/SaveImageTest/Source/t2.jpeg",FilePath::FilepathInDocuments("SaveImageTest/Dest/t2.jpeg"),data);
    InnerTest("~res:/TestData/SaveImageTest/Source/t3.jpeg",FilePath::FilepathInDocuments("SaveImageTest/Dest/t3.jpeg"),data);

    Vector<Image*> images;
    eErrorCode retCode = ImageSystem::Instance()->Load("~res:/TestData/SaveImageTest/Source/t4.pvr", images);
    TEST_VERIFY(retCode == SUCCESS);
    for_each(images.begin(), images.end(), SafeRelease<Image>);
    
    images.clear();
    retCode = ImageSystem::Instance()->Load("~res:/TestData/SaveImageTest/Source/t5.dds", images);
    TEST_VERIFY(retCode == SUCCESS);
    for_each(images.begin(), images.end(), SafeRelease<Image>);
    
    images.clear();
    retCode = ImageSystem::Instance()->Load("~res:/TestData/SaveImageTest/Source/t1.png", images);
    TEST_VERIFY(retCode == SUCCESS);
    retCode = ImageSystem::Instance()->Save(FilePath::FilepathInDocuments("SaveImageTest/Dest/t5.dds"), images, FORMAT_DXT3);
    TEST_VERIFY( retCode == SUCCESS);
    for_each(images.begin(), images.end(), SafeRelease<Image>);
}


void SaveImageTest::InnerTest(const FilePath& sourcePath, const FilePath& destPath, PerfFuncData * data)
{
    Vector<Image*> images;
    eErrorCode retCode = ImageSystem::Instance()->Load(sourcePath, images);
    DVASSERT(retCode == SUCCESS);
    retCode = ImageSystem::Instance()->Save(destPath, images, images[0]->format);
    TEST_VERIFY( retCode == SUCCESS);
    
    for_each(images.begin(), images.end(), SafeRelease<Image>);
}

Image* SaveImageTest::GetImage() const
{
    uint32 size = 512;
    Image* img = Image::Create(size, size, FORMAT_RGBA8888);
    uint8* _date = img->data;
    for (uint32 i1 = 0; i1 < size; ++i1)
    {
        for (uint32 i2 = 0; i2 < size; ++i2)
        {
            *_date++ = 0xFF;    // R
            *_date++ = 0x0;    // G
            *_date++ = 0x0;    // B
            *_date++ = 0xFF;    // A
        }
    }
    return img;
}