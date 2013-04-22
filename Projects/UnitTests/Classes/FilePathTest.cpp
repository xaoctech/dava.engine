#include "FilePathTest.h"

using namespace DAVA;


FilePathTest::FilePathTest()
    :   TestTemplate<FilePathTest>("FilePathTest")
{
	RegisterFunction(this, &FilePathTest::MacTestFunction, String("FilePathTest Mac OS"), NULL);
	RegisterFunction(this, &FilePathTest::WinTestFunction, String("FilePathTest Win32"), NULL);
}

void FilePathTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void FilePathTest::UnloadResources()
{
	RemoveAllControls();
}

void FilePathTest::MacTestFunction(PerfFuncData * data)
{
    String oldProjectPathname = FilePath::GetProjectPathname();
    
    Logger::Debug("[FilePathTest] MacOS");

    FilePath filepath1("~res:/Gfx/UI/Screen/texture.tex");

    FilePath::SetProjectPathname("/TestProject");
    
    FilePath filepath0("~res:/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath0.IsInitalized());
    TEST_VERIFY(!filepath0.IsDirectoryPathname());
    
    TEST_VERIFY(filepath0.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath0.GetBasename() == "texture");
    TEST_VERIFY(filepath0.GetExtension() == ".tex");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));

    filepath0.ReplaceFilename("file.psd");
    TEST_VERIFY(filepath0.GetFilename() == "file.psd");
    TEST_VERIFY(filepath0.GetBasename() == "file");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));

    filepath0.ReplaceBasename("image");
    TEST_VERIFY(filepath0.GetFilename() == "image.psd");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));

    filepath0.ReplaceExtension(".doc");
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));
    
    TEST_VERIFY(filepath0.ResolvePathname() == "/TestProject/Gfx/UI/Screen/image.doc")

    
    filepath0.ReplaceDirectory(String("/Mac/Users"));
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("/Mac/Users/"));

    TEST_VERIFY(filepath0.ResolvePathname() == "/Mac/Users/image.doc")

    
    FilePath filepath2(filepath0);
    TEST_VERIFY(filepath0 == filepath2);
    
    FilePath filepath3;
    TEST_VERIFY(!filepath3.IsInitalized());
    TEST_VERIFY(!filepath3.IsDirectoryPathname());

    filepath3 = filepath0;
    TEST_VERIFY(filepath0 == filepath3);
    TEST_VERIFY(filepath2 == filepath3);

    
    FilePath filepath4("~res:/Gfx/UI/", "Screen/texture.tex");
    TEST_VERIFY(filepath4.IsInitalized());
    TEST_VERIFY(!filepath4.IsDirectoryPathname());
    
    TEST_VERIFY(filepath4.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath4.GetBasename() == "texture");
    TEST_VERIFY(filepath4.GetExtension() == ".tex");
    TEST_VERIFY(filepath4.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));

    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/Gfx/UI/Screen/")) == "texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/Gfx/UI/")) == "Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/Gfx")) == "UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/")) == "Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:")) == "Gfx/UI/Screen/texture.tex");

    
    FilePath filepath5("~res:/Gfx/UI/", "../Screen/texture.tex");
    TEST_VERIFY(filepath5.IsInitalized());
    TEST_VERIFY(!filepath5.IsDirectoryPathname());
    
    TEST_VERIFY(filepath5.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath5.GetBasename() == "texture");
    TEST_VERIFY(filepath5.GetExtension() == ".tex");
    TEST_VERIFY(filepath5.GetDirectory() == FilePath("~res:/Gfx/Screen/"));
    
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/Gfx/Screen/")) == "texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/Gfx/")) == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/Gfx")) == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/")) == "Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:")) == "Gfx/Screen/texture.tex");
    
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

    FilePath filepath11("texture.tex");
    TEST_VERIFY(filepath11.IsInitalized());
    TEST_VERIFY(!filepath11.IsDirectoryPathname());
    
    TEST_VERIFY(filepath11.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath11.GetBasename() == "texture");
    TEST_VERIFY(filepath11.GetExtension() == ".tex");
    TEST_VERIFY(filepath11.GetDirectory() == FilePath());

    filepath11.ReplaceBasename("image");
    TEST_VERIFY(filepath11.GetFilename() == "image.tex");

    filepath11.ReplaceExtension(".psd");
    TEST_VERIFY(filepath11.GetFilename() == "image.psd");

    filepath11.ReplaceFilename("music.mp3");
    TEST_VERIFY(filepath11.GetFilename() == "music.mp3");

    filepath11.ReplaceDirectory(String("/Users/Test"));
    TEST_VERIFY(filepath11.GetDirectory() == FilePath("/Users/Test/"));

    
    FilePath filepath12("/Users/Test");
    filepath12.MakeDirectoryPathname();
    DVASSERT(filepath12.IsDirectoryPathname());
    
    TEST_VERIFY(filepath12.GetFilename() == "");
    TEST_VERIFY(filepath12.GetBasename() == "");
    TEST_VERIFY(filepath12.GetExtension() == "");
    TEST_VERIFY(filepath12.GetDirectory() == FilePath("/Users/Test/"));

    
    FilePath filepath13("/Users/Test/file");
    TEST_VERIFY(filepath13.GetFilename() == "file");
    TEST_VERIFY(filepath13.GetBasename() == "file");
    TEST_VERIFY(filepath13.GetExtension() == "");
    TEST_VERIFY(filepath13.GetDirectory() == FilePath("/Users/Test/"));

    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "/Mac/Users/image.doc");
    TEST_VERIFY(filepath0.ResolvePathname() == "/Mac/Users/image.doc");

    TEST_VERIFY(filepath1.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath1.ResolvePathname() == "/TestProject/Data/Gfx/UI/Screen/texture.tex");

    TEST_VERIFY(filepath2.GetAbsolutePathname() == "/Mac/Users/image.doc");
    TEST_VERIFY(filepath2.ResolvePathname() == "/Mac/Users/image.doc");

    TEST_VERIFY(filepath3.GetAbsolutePathname() == "/Mac/Users/image.doc");
    TEST_VERIFY(filepath3.ResolvePathname() == "/Mac/Users/image.doc");

    TEST_VERIFY(filepath4.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.ResolvePathname() == "/TestProject/Data/Gfx/UI/Screen/texture.tex");

    TEST_VERIFY(filepath5.GetAbsolutePathname() == "~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5.ResolvePathname() == "/TestProject/Data/Gfx/Screen/texture.tex");

    TEST_VERIFY(filepath6.GetAbsolutePathname() == "~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath6.ResolvePathname() == "/TestProject/Data/Gfx/Screen/texture.tex");

    TEST_VERIFY(filepath7.GetAbsolutePathname() == "~res:/Gfx/Screen/");
    TEST_VERIFY(filepath7.ResolvePathname() == "/TestProject/Data/Gfx/Screen/");

    TEST_VERIFY(filepath8.GetAbsolutePathname() == "~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath8.ResolvePathname() == "/TestProject/Data/Gfx/Screen/texture.tex");

    TEST_VERIFY(filepath9.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath9.ResolvePathname() == "/TestProject/Data/Gfx/UI/Screen/texture.tex");

    TEST_VERIFY(filepath10.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath10.ResolvePathname() == "/TestProject/Data/Gfx/UI/Screen/texture.tex");

    TEST_VERIFY(filepath11.GetAbsolutePathname() == "/Users/Test/music.mp3");
    TEST_VERIFY(filepath11.ResolvePathname() == "/Users/Test/music.mp3");

    TEST_VERIFY(filepath12.GetAbsolutePathname() == "/Users/Test/");
    TEST_VERIFY(filepath12.ResolvePathname() == "/Users/Test/");

    TEST_VERIFY(filepath13.GetAbsolutePathname() == "/Users/Test/file");
    TEST_VERIFY(filepath13.ResolvePathname() == "/Users/Test/file");

    
    Logger::Debug("[FilePathTest] Mac OS Done");
    
    FilePath::SetProjectPathname(oldProjectPathname);
}

void FilePathTest::WinTestFunction(PerfFuncData * data)
{
    String oldProjectPathname = FilePath::GetProjectPathname();
    
    Logger::Debug("[FilePathTest] Win32");
    
    FilePath filepath1("~res:/Gfx/UI/Screen/texture.tex");
    
    FilePath::SetProjectPathname("c:/TestProject");
    
    FilePath filepath0("~res:/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath0.IsInitalized());
    TEST_VERIFY(!filepath0.IsDirectoryPathname());
    
    TEST_VERIFY(filepath0.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath0.GetBasename() == "texture");
    TEST_VERIFY(filepath0.GetExtension() == ".tex");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));
    
    filepath0.ReplaceFilename("file.psd");
    TEST_VERIFY(filepath0.GetFilename() == "file.psd");
    TEST_VERIFY(filepath0.GetBasename() == "file");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));
    
    filepath0.ReplaceBasename("image");
    TEST_VERIFY(filepath0.GetFilename() == "image.psd");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".psd");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));
    
    filepath0.ReplaceExtension(".doc");
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));
    
    TEST_VERIFY(filepath0.ResolvePathname() == "c:/TestProject/Gfx/UI/Screen/image.doc")
    
    
    filepath0.ReplaceDirectory(String("c:/Mac/Users"));
    TEST_VERIFY(filepath0.GetFilename() == "image.doc");
    TEST_VERIFY(filepath0.GetBasename() == "image");
    TEST_VERIFY(filepath0.GetExtension() == ".doc");
    TEST_VERIFY(filepath0.GetDirectory() == FilePath("c:/Mac/Users/"));
    
    TEST_VERIFY(filepath0.ResolvePathname() == "c:/Mac/Users/image.doc")
    
    
    FilePath filepath2(filepath0);
    TEST_VERIFY(filepath0 == filepath2);
    
    FilePath filepath3;
    TEST_VERIFY(!filepath3.IsInitalized());
    TEST_VERIFY(!filepath3.IsDirectoryPathname());
    
    filepath3 = filepath0;
    TEST_VERIFY(filepath0 == filepath3);
    TEST_VERIFY(filepath2 == filepath3);
    
    
    FilePath filepath4("~res:/Gfx/UI/", "Screen/texture.tex");
    TEST_VERIFY(filepath4.IsInitalized());
    TEST_VERIFY(!filepath4.IsDirectoryPathname());
    
    TEST_VERIFY(filepath4.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath4.GetBasename() == "texture");
    TEST_VERIFY(filepath4.GetExtension() == ".tex");
    TEST_VERIFY(filepath4.GetDirectory() == FilePath("~res:/Gfx/UI/Screen/"));
    
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/Gfx/UI/Screen/")) == "texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/Gfx/UI/")) == "Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/Gfx")) == "UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:/")) == "Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.GetRelativePathname(String("~res:")) == "Gfx/UI/Screen/texture.tex");
    
    
    FilePath filepath5("~res:/Gfx/UI/", "../Screen/texture.tex");
    TEST_VERIFY(filepath5.IsInitalized());
    TEST_VERIFY(!filepath5.IsDirectoryPathname());
    
    TEST_VERIFY(filepath5.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath5.GetBasename() == "texture");
    TEST_VERIFY(filepath5.GetExtension() == ".tex");
    TEST_VERIFY(filepath5.GetDirectory() == FilePath("~res:/Gfx/Screen/"));
    
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/Gfx/Screen/")) == "texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/Gfx/")) == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/Gfx")) == "Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:/")) == "Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5.GetRelativePathname(String("~res:")) == "Gfx/Screen/texture.tex");
    
    FilePath filepath6("~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5 == filepath6);
    
    FilePath filepath7("~res:/Gfx/Screen/");
    TEST_VERIFY(filepath7.IsInitalized());
    TEST_VERIFY(filepath7.IsDirectoryPathname());
    
    FilePath filepath8 = filepath7 + FilePath("texture.tex");
    TEST_VERIFY(filepath8 == filepath6);
    
    
    FilePath::SetProjectPathname("c:/TestProject/Data");
    FilePath filepath9("~res:/Gfx/UI/Screen/texture.tex");
    FilePath filepath10("~res:/Gfx/", "../../Data/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath9 == filepath10);
    TEST_VERIFY(filepath8 != filepath10);
    
    FilePath filepath11("texture.tex");
    TEST_VERIFY(filepath11.IsInitalized());
    TEST_VERIFY(!filepath11.IsDirectoryPathname());
    
    TEST_VERIFY(filepath11.GetFilename() == "texture.tex");
    TEST_VERIFY(filepath11.GetBasename() == "texture");
    TEST_VERIFY(filepath11.GetExtension() == ".tex");
    TEST_VERIFY(filepath11.GetDirectory() == FilePath());
    
    filepath11.ReplaceBasename("image");
    TEST_VERIFY(filepath11.GetFilename() == "image.tex");
    
    filepath11.ReplaceExtension(".psd");
    TEST_VERIFY(filepath11.GetFilename() == "image.psd");
    
    filepath11.ReplaceFilename("music.mp3");
    TEST_VERIFY(filepath11.GetFilename() == "music.mp3");
    
    filepath11.ReplaceDirectory(String("c:/Users/Test"));
    TEST_VERIFY(filepath11.GetDirectory() == FilePath("c:/Users/Test/"));
    
    FilePath filepath12("c:/Users/Test");
    filepath12.MakeDirectoryPathname();
    DVASSERT(filepath12.IsDirectoryPathname());
    
    TEST_VERIFY(filepath12.GetFilename() == "");
    TEST_VERIFY(filepath12.GetBasename() == "");
    TEST_VERIFY(filepath12.GetExtension() == "");
    TEST_VERIFY(filepath12.GetDirectory() == FilePath("c:/Users/Test/"));

    
    FilePath filepath13("c:/Users/Test/file");
    TEST_VERIFY(filepath13.GetFilename() == "file");
    TEST_VERIFY(filepath13.GetBasename() == "file");
    TEST_VERIFY(filepath13.GetExtension() == "");
    TEST_VERIFY(filepath13.GetDirectory() == FilePath("c:/Users/Test/"));

    
    
    
    TEST_VERIFY(filepath0.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath0.ResolvePathname() == "c:/Mac/Users/image.doc");
    
    TEST_VERIFY(filepath1.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath1.ResolvePathname() == "c:/TestProject/Data/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath2.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath2.ResolvePathname() == "c:/Mac/Users/image.doc");
    
    TEST_VERIFY(filepath3.GetAbsolutePathname() == "c:/Mac/Users/image.doc");
    TEST_VERIFY(filepath3.ResolvePathname() == "c:/Mac/Users/image.doc");
    
    TEST_VERIFY(filepath4.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath4.ResolvePathname() == "c:/TestProject/Data/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath5.GetAbsolutePathname() == "~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath5.ResolvePathname() == "c:/TestProject/Data/Gfx/Screen/texture.tex");
    
    TEST_VERIFY(filepath6.GetAbsolutePathname() == "~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath6.ResolvePathname() == "c:/TestProject/Data/Gfx/Screen/texture.tex");
    
    TEST_VERIFY(filepath7.GetAbsolutePathname() == "~res:/Gfx/Screen/");
    TEST_VERIFY(filepath7.ResolvePathname() == "c:/TestProject/Data/Gfx/Screen/");
    
    TEST_VERIFY(filepath8.GetAbsolutePathname() == "~res:/Gfx/Screen/texture.tex");
    TEST_VERIFY(filepath8.ResolvePathname() == "c:/TestProject/Data/Gfx/Screen/texture.tex");
    
    TEST_VERIFY(filepath9.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath9.ResolvePathname() == "c:/TestProject/Data/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath10.GetAbsolutePathname() == "~res:/Gfx/UI/Screen/texture.tex");
    TEST_VERIFY(filepath10.ResolvePathname() == "c:/TestProject/Data/Gfx/UI/Screen/texture.tex");
    
    TEST_VERIFY(filepath11.GetAbsolutePathname() == "c:/Users/Test/music.mp3");
    TEST_VERIFY(filepath11.ResolvePathname() == "c:/Users/Test/music.mp3");
    
    TEST_VERIFY(filepath12.GetAbsolutePathname() == "c:/Users/Test/");
    TEST_VERIFY(filepath12.ResolvePathname() == "c:/Users/Test/");

    TEST_VERIFY(filepath13.GetAbsolutePathname() == "c:/Users/Test/file");
    TEST_VERIFY(filepath13.ResolvePathname() == "c:/Users/Test/file");

    
    Logger::Debug("[FilePathTest] Win32 Done");
    
    FilePath::SetProjectPathname(oldProjectPathname);
}

