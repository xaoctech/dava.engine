#include "FileSystemTest.h"

using namespace DAVA;

FileSystemTest::FileSystemTest()
    :   TestTemplate<FileSystemTest>("FileSystemTest")
{
    FileSystem::Instance()->DeleteDirectory(FilePath("~doc:/TestData/FileSystemTest/"), true);
    bool dataPrepared = RecursiveCopy(FilePath("~res:/TestData/FileSystemTest/"), FilePath("~doc:/TestData/FileSystemTest/"));
    DVASSERT(dataPrepared);

	RegisterFunction(this, &FileSystemTest::ResTestFunction, String("ResTestFunction"), NULL);
	RegisterFunction(this, &FileSystemTest::DocTestFunctionCheckCopy, String("DocTestFunctionCheckCopy"), NULL);
	RegisterFunction(this, &FileSystemTest::DocTestFunction, String("DocTestFunction"), NULL);
}

void FileSystemTest::LoadResources()
{
	GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void FileSystemTest::UnloadResources()
{
	RemoveAllControls();
}

void FileSystemTest::ResTestFunction(PerfFuncData * data)
{
    Logger::Debug("[FileSystemTest::ResTestFunction]");

    FileList fileList(FilePath("~res:/TestData/FileSystemTest/"));

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
            TEST_VERIFY(pathname == FilePath("~res:/TestData/FileSystemTest/Folder1/"));
            TEST_VERIFY(files.GetFileCount() == 3);

            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;

                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);

                File *file = File::Create(pathname, File::OPEN | File::READ);
                TEST_VERIFY(file != NULL);

                if(!file) continue;
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == FilePath("~res:/TestData/FileSystemTest/Folder1/file1"));
                    TEST_VERIFY(file->GetFilename() == FilePath("~res:/TestData/FileSystemTest/Folder1/file1"));
                    TEST_VERIFY(file->GetSize() == 350);
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == FilePath("~res:/TestData/FileSystemTest/Folder1/file2.txt"));
                    TEST_VERIFY(file->GetFilename() == FilePath("~res:/TestData/FileSystemTest/Folder1/file2.txt"));
                    TEST_VERIFY(file->GetSize() == 30240);
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == FilePath("~res:/TestData/FileSystemTest/Folder1/file3.doc"));
                    TEST_VERIFY(file->GetFilename() == FilePath("~res:/TestData/FileSystemTest/Folder1/file3.doc"));
                    TEST_VERIFY(file->GetSize() == 37479);
                }
                else
                {
                    TEST_VERIFY(false);
                }
                
                SafeRelease(file);
            }
        }
    }
}

void FileSystemTest::DocTestFunctionCheckCopy(PerfFuncData * data)
{
    Logger::Debug("[FileSystemTest::DocTestFunctionCheckCopy]");
    FileList fileList(FilePath("~doc:/TestData/FileSystemTest/"));
    
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
            TEST_VERIFY(pathname == FilePath("~doc:/TestData/FileSystemTest/Folder1/"));
            TEST_VERIFY(files.GetFileCount() == 3);
            
            for(int32 ifi = 0; ifi < files.GetCount(); ++ifi)
            {
                if(files.IsNavigationDirectory(ifi)) continue;
                
                String filename = files.GetFilename(ifi);
                FilePath pathname = files.GetPathname(ifi);
                
                File *file = File::Create(pathname, File::OPEN | File::READ);
                TEST_VERIFY(file != NULL);
                
                if(!file) continue;
                
                if(filename == "file1")
                {
                    TEST_VERIFY(pathname == FilePath("~doc:/TestData/FileSystemTest/Folder1/file1"));
                    TEST_VERIFY(file->GetFilename() == FilePath("~doc:/TestData/FileSystemTest/Folder1/file1"));
                    TEST_VERIFY(file->GetSize() == 350);
                }
                else if(filename == "file2.txt")
                {
                    TEST_VERIFY(pathname == FilePath("~doc:/TestData/FileSystemTest/Folder1/file2.txt"));
                    TEST_VERIFY(file->GetFilename() == FilePath("~doc:/TestData/FileSystemTest/Folder1/file2.txt"));
                    TEST_VERIFY(file->GetSize() == 30240);
                }
                else if(filename == "file3.doc")
                {
                    TEST_VERIFY(pathname == FilePath("~doc:/TestData/FileSystemTest/Folder1/file3.doc"));
                    TEST_VERIFY(file->GetFilename() == FilePath("~doc:/TestData/FileSystemTest/Folder1/file3.doc"));
                    TEST_VERIFY(file->GetSize() == 37479);
                }
                else
                {
                    TEST_VERIFY(false);
                }
                
                SafeRelease(file);
            }
        }
    }
}

void FileSystemTest::DocTestFunction(PerfFuncData * data)
{
    Logger::Debug("[FileSystemTest::DocTestFunction]");

    FilePath savedCurrentWorkingDirectory = FileSystem::Instance()->GetCurrentWorkingDirectory();
    TEST_VERIFY(FileSystem::Instance()->SetCurrentWorkingDirectory(FilePath("~doc:/TestData/FileSystemTest/")));
    TEST_VERIFY(FileSystem::Instance()->SetCurrentWorkingDirectory(savedCurrentWorkingDirectory));
    
    FilePath savedCurrentDocDirectory = FileSystem::Instance()->GetCurrentDocumentsDirectory();
    FileSystem::Instance()->SetCurrentDocumentsDirectory(FilePath("~doc:/TestData/FileSystemTest/"));
    TEST_VERIFY(FileSystem::Instance()->GetCurrentDocumentsDirectory() == FilePath("~doc:/"));
    
    FileSystem::Instance()->SetCurrentDocumentsDirectory(savedCurrentDocDirectory);
    TEST_VERIFY(FileSystem::Instance()->GetCurrentDocumentsDirectory() == savedCurrentDocDirectory);
    

    TEST_VERIFY(!FileSystem::Instance()->IsDirectory(FilePath("~doc:/TestData/FileSystemTest/Folder1/file1")));
    TEST_VERIFY(FileSystem::Instance()->IsFile(FilePath("~doc:/TestData/FileSystemTest/Folder1/file1")));
    
    TEST_VERIFY(FileSystem::Instance()->IsDirectory(FilePath("~doc:/TestData/FileSystemTest/")));
    TEST_VERIFY(!FileSystem::Instance()->IsFile(FilePath("~doc:/TestData/FileSystemTest/")));
    

    TEST_VERIFY(    FileSystem::Instance()->FilepathInDocuments((const char *)"Test/test.file")
                ==  FileSystem::Instance()->FilepathInDocuments(String("Test/test.file")));
    
    
    FileSystem::eCreateDirectoryResult created = FileSystem::Instance()->CreateDirectory(FilePath("~doc:/TestData/FileSystemTest/1/2/3"), false);
    TEST_VERIFY(created == FileSystem::DIRECTORY_CANT_CREATE);
    created = FileSystem::Instance()->CreateDirectory(FilePath("~doc:/TestData/FileSystemTest/1/2/3"), true);
    TEST_VERIFY(created == FileSystem::DIRECTORY_CREATED);
    created = FileSystem::Instance()->CreateDirectory(FilePath("~doc:/TestData/FileSystemTest/1/2/3"), false);
    TEST_VERIFY(created == FileSystem::DIRECTORY_EXISTS);

    
    
    bool moved = FileSystem::Instance()->MoveFile(FilePath("~doc:/TestData/FileSystemTest/Folder1/file1"), FilePath("~doc:/TestData/FileSystemTest/Folder1/file_new"));
    TEST_VERIFY(moved);
    
    moved = FileSystem::Instance()->MoveFile(FilePath("~doc:/TestData/FileSystemTest/Folder1/file2.txt"), FilePath("~doc:/TestData/FileSystemTest/Folder1/file_new"));
    TEST_VERIFY(!moved);
    
    moved = FileSystem::Instance()->MoveFile(FilePath("~doc:/TestData/FileSystemTest/Folder1/file2.txt"), FilePath("~doc:/TestData/FileSystemTest/Folder1/file_new"), true);
    TEST_VERIFY(moved);

    FileSystem::Instance()->DeleteFile(FilePath("~doc:/TestData/FileSystemTest/Folder1/file1_new"));
    File *f = File::Create(FilePath("~doc:/TestData/FileSystemTest/Folder1/file1_new"), File::OPEN | File::READ);
    TEST_VERIFY(!f);
    SafeRelease(f);
    
   
    uint32 count = FileSystem::Instance()->DeleteDirectoryFiles(FilePath("~doc:/TestData/FileSystemTest/Folder1/"));
    TEST_VERIFY(count == 2);

    FileList fileList(FilePath("~doc:/TestData/FileSystemTest/Folder1/"));
    TEST_VERIFY(fileList.GetFileCount() == 0);
}








bool FileSystemTest::RecursiveCopy(const DAVA::FilePath &src, const DAVA::FilePath &dst)
{
    DVASSERT(src.IsDirectoryPathname() && dst.IsDirectoryPathname());
    
    FileSystem::eCreateDirectoryResult created = FileSystem::Instance()->CreateDirectory(dst, true);
    DVASSERT(created != FileSystem::DIRECTORY_CANT_CREATE);
    
    bool copied = FileSystem::Instance()->CopyDirectory(src, dst);
    DVASSERT(copied);

    bool retCode = true;
    
    FileList fileList(src);
    for(int32 i = 0; i < fileList.GetCount(); ++i)
    {
        if(fileList.IsDirectory(i) && !fileList.IsNavigationDirectory(i))
        {
            retCode &= RecursiveCopy(fileList.GetPathname(i), dst + FilePath(fileList.GetFilename(i) + "/"));
        }
    }
    
    return retCode;
}
