#include "FilePathTest.h"

using namespace DAVA;


FilePathTest::FilePathTest()
    :   TestTemplate<FilePathTest>("FilePathTest")
{
	RegisterFunction(this, &FilePathTest::TestFunction, Format("FilePathTest"), NULL);
}

void FilePathTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void FilePathTest::UnloadResources()
{
	RemoveAllControls();
}

void FilePathTest::TestFunction(PerfFuncData * data)
{
    String oldProjectPathname = FilePath::GetProjectPathname();
    
    Logger::Debug("[FilePathTest]");

    FilePath filepath1("~res:/Gfx/UI/Screen/texture.tex");

    FilePath::SetProjectPathname("/TestProject");
    
    FilePath filepath("~res:/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath.IsInitalized());
    TEST_VERIFY(!filepath.IsDirectoryPathname());
    
    TEST_VERIFY(filepath.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath.GetBasename() == "texture");
    TEST_VERIFY(filepath.GetExtension() == ".tex");
    TEST_VERIFY(filepath.GetDirectory() == "~res:/Gfx/UI/Screen/");

    filepath.ReplaceFilename("file.psd");
    TEST_VERIFY(filepath.GetFilename() == "file.psd");
    TEST_VERIFY(filepath.GetBasename() == "file");
    TEST_VERIFY(filepath.GetExtension() == ".psd");
    TEST_VERIFY(filepath.GetDirectory() == "~res:/Gfx/UI/Screen/");

    filepath.ReplaceBasename("image");
    TEST_VERIFY(filepath.GetFilename() == "image.psd");
    TEST_VERIFY(filepath.GetBasename() == "image");
    TEST_VERIFY(filepath.GetExtension() == ".psd");
    TEST_VERIFY(filepath.GetDirectory() == "~res:/Gfx/UI/Screen/");

    filepath.ReplaceExtension(".doc");
    TEST_VERIFY(filepath.GetFilename() == "image.doc");
    TEST_VERIFY(filepath.GetBasename() == "image");
    TEST_VERIFY(filepath.GetExtension() == ".doc");
    TEST_VERIFY(filepath.GetDirectory() == "~res:/Gfx/UI/Screen/");
    
    TEST_VERIFY(filepath.ResolvePathname() == "/TestProject/Gfx/UI/Screen/image.doc")

    
    filepath.ReplaceDirectory("/Mac/Users");
    TEST_VERIFY(filepath.GetFilename() == "image.doc");
    TEST_VERIFY(filepath.GetBasename() == "image");
    TEST_VERIFY(filepath.GetExtension() == ".doc");
    TEST_VERIFY(filepath.GetDirectory() == "/Mac/Users/");

    TEST_VERIFY(filepath.ResolvePathname() == "/Mac/Users/image.doc")

    
    FilePath filepath2(filepath);
    TEST_VERIFY(filepath == filepath2);
    
    FilePath filepath3;
    TEST_VERIFY(!filepath3.IsInitalized());
    TEST_VERIFY(!filepath3.IsDirectoryPathname());

    filepath3 = filepath;
    TEST_VERIFY(filepath == filepath3);
    TEST_VERIFY(filepath2 == filepath3);

    
    FilePath filepath4("~res:/Gfx/UI/", "Screen/texture.tex");
    TEST_VERIFY(filepath4.IsInitalized());
    TEST_VERIFY(!filepath4.IsDirectoryPathname());
    
    TEST_VERIFY(filepath4.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath4.GetBasename() == "texture");
    TEST_VERIFY(filepath4.GetExtension() == ".tex");
    TEST_VERIFY(filepath4.GetDirectory() == "~res:/Gfx/UI/Screen/");

    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/UI/Screen/") == "texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx/UI/") == "Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/Gfx") == "UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:/") == "Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname("~res:") == "Gfx/UI/Screen/texture.tex");

    
    FilePath filepath5("~res:/Gfx/UI/", "../Screen/texture.tex");
    TEST_VERIFY(filepath5.IsInitalized());
    TEST_VERIFY(!filepath5.IsDirectoryPathname());
    
    TEST_VERIFY(filepath5.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath5.GetBasename() == "texture");
    TEST_VERIFY(filepath5.GetExtension() == ".tex");
    TEST_VERIFY(filepath5.GetDirectory() == "~res:/Gfx/Screen/");
    
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/Screen/") == "texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx/") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/Gfx") == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:/") == "Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname("~res:") == "Gfx/Screen/texture.tex");
    
    FilePath filepath6("~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5 == filepath6);

    FilePath filepath7("~res:/Gfx/Screen/");
    TEST_VERIFY(filepath7.IsInitalized());
    TEST_VERIFY(filepath7.IsDirectoryPathname());

    FilePath filepath8 = filepath7 + FilePath("texture.tex");
    TEST_VERIFY(filepath8 == filepath6);

    
    FilePath::SetProjectPathname("/TestProject/Data");
    FilePath filepath9("~res:/Gfx/UI/Screen/texture.tex");
    FilePath filepath10("~res:/Gfx/", "../../Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath9 == filepath10);
    TEST_VERIFY(filepath8 != filepath10);

    
    Logger::Info("[00] = %s (%s)", filepath.GetAbsolutePathname().c_str(), filepath.ResolvePathname().c_str());
    Logger::Info("[01] = %s (%s)", filepath1.GetAbsolutePathname().c_str(), filepath1.ResolvePathname().c_str());
    Logger::Info("[02] = %s (%s)", filepath2.GetAbsolutePathname().c_str(), filepath2.ResolvePathname().c_str());
    Logger::Info("[03] = %s (%s)", filepath3.GetAbsolutePathname().c_str(), filepath3.ResolvePathname().c_str());
    Logger::Info("[04] = %s (%s)", filepath4.GetAbsolutePathname().c_str(), filepath4.ResolvePathname().c_str());
    Logger::Info("[05] = %s (%s)", filepath5.GetAbsolutePathname().c_str(), filepath5.ResolvePathname().c_str());
    Logger::Info("[06] = %s (%s)", filepath6.GetAbsolutePathname().c_str(), filepath6.ResolvePathname().c_str());
    Logger::Info("[07] = %s (%s)", filepath7.GetAbsolutePathname().c_str(), filepath7.ResolvePathname().c_str());
    Logger::Info("[08] = %s (%s)", filepath8.GetAbsolutePathname().c_str(), filepath8.ResolvePathname().c_str());
    Logger::Info("[09] = %s (%s)", filepath9.GetAbsolutePathname().c_str(), filepath9.ResolvePathname().c_str());
    Logger::Info("[10] = %s (%s)", filepath10.GetAbsolutePathname().c_str(), filepath10.ResolvePathname().c_str());

    Logger::Debug("[FilePathTest] Done");
    
    FilePath::SetProjectPathname(oldProjectPathname);
}

