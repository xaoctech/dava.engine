#ifndef __DAVAENGINE_DLC_SYSTEM_H__
#define __DAVAENGINE_DLC_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "DLC/FileDownloader.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class DLCSystemDelegate
{
public:
    enum DLCStatusCode
    {
        DOWNLOAD_SUCCESS ,
        DOWNLOAD_ERROR,
        DOWNLOAD_SERVER_ERROR,
        DOWNLOAD_FILESYSTEM_ERROR
    };
    

public:
    virtual ~DLCSystemDelegate() = 0;

    // File with contents of all DLCs has getted 
    virtual void InitCompleted(DLCStatusCode withStatus){};
    // Single DLC file has downloaded or end with error
    virtual void DLCCompleted(DLCStatusCode withStatus, uint16 index){};
    // All DLC files has downloaded
    virtual void AllDLCCompleted(){};
};

class DLCSource;
    
class DLCSystem: public Singleton<DLCSystem>, public FileDownloaderDelegate
{
public:
    enum DLCState
    {
        DLC_NOT_DOWNLOADED = 0,
        DLC_DOWNLOADED,
        DLC_DOWNLOAD_NOT_FINISHED,
        DLC_FILES_TREE_CORRUPT,
        DLC_OLD_VERSION
    };
    
public:
    DLCSystem();
    virtual ~DLCSystem();

    // Get Index file of all DLCs
    virtual void InitSystem(const String& serverURL, const FilePath & _contentPath = FilePath("~doc:/downloads/"));

    virtual uint16 GetDLCCount() const;
    // Return -1 if not found
    virtual uint16 GetDLCIndexByName(const String& name) const;
    // Return NULL if out of bounds
    virtual const DLCSource * GetDLCSource(const uint16 index);

    // Add DLC to queue to download
    virtual void DownloadDLC( const uint16 index, const uint16 _reconnectCount = 1);
    virtual void DownloadAllDLC();
    
    // Add & remove delegates
    virtual void AddDelegate(DLCSystemDelegate* delegate);
    virtual void RemoveDelegate(DLCSystemDelegate* delegate);

    // Pause & resume download
    virtual void Pause();
    virtual void Resume();
    // Stop & delete DLCSystem
    virtual bool Stop();

    // FileDownloaderDelegate methods
    virtual void DownloadGetPacket(uint64 size);
    virtual void DownloadComplete(FileDownloaderDelegate::DownloadStatusCode status);

protected:
    enum CollbackType
    {
        INIT_COMPLETE = 0,
        DOWNLOAD_COMPLETE = 1,
        ALL_DOWNLOAD_COMPLETE = 2
    };
    
protected:
    virtual void SendStatusToDelegates(const CollbackType type, const DLCSystemDelegate::DLCStatusCode code = DLCSystemDelegate::DOWNLOAD_SUCCESS, const uint16 index = 0) const;
    
    virtual void DownloadNextDLC();
    
    void LoadOldDlcs();
    void SaveOldDlcs() const;
    
    void CheckFileStructureDLC(const FilePath & path, DLCSource * _dlc);
    
private:
    String indexURL;
    FilePath indexFilePath;
    FilePath contentPath;
    Vector<DLCSystemDelegate*> delegates;
    Vector<DLCSource*> dlcs;
    Vector<DLCSource*> oldDlcs;
    Vector<DLCSource*> queueDLC;
    
    FileDownloader * currDownloader;
    bool isInited;
    bool isStoped;
};

// 
class DLCSource: public BaseObject
{
public:
    DLCSource(YamlNode* node);
    DLCSource(File * file);
    void Save(File * file) const;
    
    static void Deleter(BaseObject * obj);
    
public:
    // Yaml fields
    String name;
    String pathOnServer;
    //
    float32 curVersion;
    float32 lastVersion;
    //
    uint64 size;
    uint64 curSize;
    //
    String description;

    // Not Yaml fields
    uint16 index;
    DLCSystem::DLCState state;
    uint16 reconnectCount;
    FilePath fullPath;
};
    
    
}

#endif//__DAVAENGINE_DLC_SYSTEM_H__












