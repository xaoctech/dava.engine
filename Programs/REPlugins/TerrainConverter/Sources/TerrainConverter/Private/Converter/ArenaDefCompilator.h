#ifndef __TERRAIN_CONVERTER_ERENA_DEF_COMPILATOR_H__
#define __TERRAIN_CONVERTER_ERENA_DEF_COMPILATOR_H__

#include <DAVAEngine.h>

#include "Converter/Visitor/ConverterVisitorBase.h"
#include "Converter/StrategyPointData.h"
#include "Converter/BoundingBoxData.h"

class ArenaDefCompliator
: public ConverterVisitorBase
{
public:
    static std::unique_ptr<ArenaDefCompliator> Create(const DAVA::FilePath& arenaDefPatternPath,
                                                      const DAVA::FilePath& saveTo);

    ArenaDefCompliator(const DAVA::String& arenaPattern, const DAVA::FilePath& saveTo);
    ~ArenaDefCompliator() override = default;

    void Start(const DAVA::String& mapName) override;

    void Visit(StrategyPointData& data) override;
    void Visit(BoundingBoxData& data) override;

    void End() override;

private:
    void Reset();
    void Dump();

    void Verify() const;

    DAVA::FilePath arenaOutputFilepath;
    DAVA::String arenaPattern;

    DAVA::String mapName;
    DAVA::Vector<StrategyPointData> points;
    BoundingBoxData boundingBox;
};

#endif // __TERRAIN_CONVERTER_ERENA_DEF_COMPILATOR_H__