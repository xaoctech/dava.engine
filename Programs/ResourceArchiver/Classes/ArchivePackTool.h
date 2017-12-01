#pragma once

#include "CommandLineTool.h"

#include <Base/BaseTypes.h>
#include <DavaTools/AssetCache/AssetCacheClient.h>
#include <Compression/Compressor.h>

class ArchivePackTool : public CommandLineTool
{
public:
    ArchivePackTool();

private:
    enum class Source
    {
        UseListFiles,
        UseSrc,
        UseMetaDB,
        Unknown
    };

    bool ConvertOptionsToParamsInternal() override;
    int ProcessInternal() override;

    DAVA::String compressionStr;
    DAVA::Compressor::Type compressionType;
    bool addHidden = false;
    bool useCache = false;
    DAVA::AssetCacheClient::ConnectionParams assetCacheParams;
    DAVA::String logFileName;
    DAVA::String srcDir;
    DAVA::List<DAVA::String> listFiles;
    DAVA::Vector<DAVA::String> srcFiles;
    DAVA::String packFileName;
    DAVA::String baseDir;
    DAVA::String metaDbPath;
    Source source = Source::Unknown;
};
