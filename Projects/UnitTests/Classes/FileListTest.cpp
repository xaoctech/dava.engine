#include "FileListTest.h"

using namespace DAVA;


FileListTest::FileListTest()
    :   TestTemplate<FileListTest>("FileListTest")
{
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

    FileList fileList(FilePath("~res:/TestData/FileListTest/"));

    DVASSERT(fileList.GetDirectoryCount() == 3);
    DVASSERT(fileList.GetFileCount() == 0);

    for(int32 ifo = 0; ifo < fileList.GetCount(); ++ifo)
    {
        if(fileList.IsNavigationDirectory(ifo)) continue;

        String filename = fileList.GetFilename(ifo);
        FilePath pathname = fileList.GetPathname(ifo);
        FileList files(pathname);
        DVASSERT(files.GetDirectoryCount() == 0);
        
        if(filename == "Folder1")
        {
            DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder1/"));
            DVASSERT(files.GetFileCount() == 3);

            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;

                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder1/file1"));
                }
                else if(filename == "file2.txt")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder1/file2.txt"));
                }
                else if(filename == "file3.doc")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder1/file3.doc"));
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else if(filename == "Folder2")
        {
            DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/"));
            DVASSERT(files.GetFileCount() == 6);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;

                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/file1"));
                }
                else if(filename == "file1.txt")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/file1.txt"));
                }
                else if(filename == "file2")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/file2"));
                }
                else if(filename == "file2.txt")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/file2.txt"));
                }
                else if(filename == "file3")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/file3"));
                }
                else if(filename == "file3.doc")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder2/file3.doc"));
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else if(filename == "Folder3")
        {
            DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder3/"));
            DVASSERT(files.GetFileCount() == 2);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder3/file1"));
                }
                else if(filename == "file3.doc")
                {
                    DVASSERT(pathname == FilePath("~res:/TestData/FileListTest/Folder3/file3.doc"));
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void FileListTest::DocTestFunction(PerfFuncData * data)
{
    FileSystem::Instance()->DeleteDirectory(FilePath("~doc:/TestData/FileListTest/"), true);
    RecursiveCopy(FilePath("~res:/TestData/FileListTest/"), FilePath("~doc:/TestData/FileListTest/"));

    
    Logger::Debug("[FileListTest::DocTestFunction]");
    
    FileList fileList(FilePath("~doc:/TestData/FileListTest/"));
    
    DVASSERT(fileList.GetDirectoryCount() == 3);
    DVASSERT(fileList.GetFileCount() == 0);
    
    for(int32 ifo = 0; ifo < fileList.GetCount(); ++ifo)
    {
        if(fileList.IsNavigationDirectory(ifo)) continue;
        
        String filename = fileList.GetFilename(ifo);
        FilePath pathname = fileList.GetPathname(ifo);
        FileList files(pathname);
        DVASSERT(files.GetDirectoryCount() == 0);
        
        if(filename == "Folder1")
        {
            DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder1/"));
            DVASSERT(files.GetFileCount() == 3);
            
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder1/file1"));
                }
                else if(filename == "file2.txt")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder1/file2.txt"));
                }
                else if(filename == "file3.doc")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder1/file3.doc"));
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else if(filename == "Folder2")
        {
            DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/"));
            DVASSERT(files.GetFileCount() == 6);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/file1"));
                }
                else if(filename == "file1.txt")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/file1.txt"));
                }
                else if(filename == "file2")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/file2"));
                }
                else if(filename == "file2.txt")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/file2.txt"));
                }
                else if(filename == "file3")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/file3"));
                }
                else if(filename == "file3.doc")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder2/file3.doc"));
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else if(filename == "Folder3")
        {
            DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder3/"));
            DVASSERT(files.GetFileCount() == 2);
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                if(filename == "file1")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder3/file1"));
                }
                else if(filename == "file3.doc")
                {
                    DVASSERT(pathname == FilePath("~doc:/TestData/FileListTest/Folder3/file3.doc"));
                }
                else
                {
                    DVASSERT(false);
                }
            }
        }
        else
        {
            DVASSERT(false);
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
            RecursiveCopy(fileList.GetPathname(i), dst + FilePath(fileList.GetFilename(i) + "/"));
        }
    }
}
