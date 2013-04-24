#include "FileListTest.h"

using namespace DAVA;


FileListTest::FileListTest()
    :   TestTemplate<FileListTest>("FileListTest")
{
    FileSystem::Instance()->DeleteDirectory("~doc:/TestData/FileListTest/", true);
    RecursiveCopy("~res:/TestData/FileListTest/", "~doc:/TestData/FileListTest/");

	RegisterFunction(this, &FileListTest::ResTestFunction, String("ResTestFunction"), NULL);
	RegisterFunction(this, &FileListTest::DocTestFunction, String("DocTestFunction"), NULL);
}

void FileListTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void FileListTest::UnloadResources()
{
	RemoveAllControls();
}

void FileListTest::ResTestFunction(PerfFuncData * data)
{
    Logger::Debug("[FileListTest::ResTestFunction]");

    FileList fileList("~res:/TestData/FileListTest/");

    TEST_VERIFY(fileList.GetDirectoryCount() == 3);
    TEST_VERIFY(fileList.GetFileCount() == 0);

    for(int32 ifo = 0; ifo < fileList.GetCount(); ++ifo)
    {
        if(fileList.IsNavigationDirectory(ifo)) continue;

        String filename = fileList.GetFilename(ifo);
        FilePath pathname = fileList.GetPathname(ifo);
        FileList files(pathname);
        TEST_VERIFY(files.GetDirectoryCount() == 0);
        
        if(filename == "Folder1")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/");
            TEST_VERIFY(files.GetFileCount() == 3);

            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;

                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file1");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file2.txt");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if(filename == "Folder2")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/");
            TEST_VERIFY(files.GetFileCount() == 6);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;

                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file1");
                }
                else if(filename == "file1.txt")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file1.txt");
                }
                else if(filename == "file2")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file2");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file2.txt");
                }
                else if(filename == "file3")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file3");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if(filename == "Folder3")
        {
            TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/");
            TEST_VERIFY(files.GetFileCount() == 2);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/file1");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else
        {
            TEST_VERIFY(false);
        }
    }
}

void FileListTest::DocTestFunction(PerfFuncData * data)
{
    Logger::Debug("[FileListTest::DocTestFunction]");
    
    FileList fileList("~doc:/TestData/FileListTest/");
    
    TEST_VERIFY(fileList.GetDirectoryCount() == 3);
    TEST_VERIFY(fileList.GetFileCount() == 0);
    
    for(int32 ifo = 0; ifo < fileList.GetCount(); ++ifo)
    {
        if(fileList.IsNavigationDirectory(ifo)) continue;
        
        String filename = fileList.GetFilename(ifo);
        FilePath pathname = fileList.GetPathname(ifo);
        FileList files(pathname);
        TEST_VERIFY(files.GetDirectoryCount() == 0);
        
        if(filename == "Folder1")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/");
            TEST_VERIFY(files.GetFileCount() == 3);
            
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file1");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file2.txt");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if(filename == "Folder2")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/");
            TEST_VERIFY(files.GetFileCount() == 6);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file1");
                }
                else if(filename == "file1.txt")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file1.txt");
                }
                else if(filename == "file2")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file2");
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file2.txt");
                }
                else if(filename == "file3")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file3");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else if(filename == "Folder3")
        {
            TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/");
            TEST_VERIFY(files.GetFileCount() == 2);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/file1");
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/file3.doc");
                }
                else
                {
                    TEST_VERIFY(false);
                }
            }
        }
        else
        {
            TEST_VERIFY(false);
        }
    }
}

void FileListTest::RecursiveCopy(const DAVA::FilePath &src, const DAVA::FilePath &dst)
{
    DVASSERT(src.IsDirectoryPathname() && dst.IsDirectoryPathname());
    
    FileSystem::Instance()->CreateDirectory(dst, true);
    FileSystem::Instance()->CopyDirectory(src, dst);

    FileList fileList(src);
    for(int32 i = 0; i < fileList.GetCount(); ++i)
    {
        if(fileList.IsDirectory(i) && !fileList.IsNavigationDirectory(i))
        {
            RecursiveCopy(fileList.GetPathname(i), dst + (fileList.GetFilename(i) + "/"));
        }
    }
}
