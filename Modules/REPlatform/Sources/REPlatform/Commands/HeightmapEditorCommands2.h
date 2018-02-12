#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Math/Rect.h>
#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Heightmap;
class Entity;
class LandscapeEditorDrawSystem;
class ModifyHeightmapCommand : public RECommand
{
public:
    ModifyHeightmapCommand(LandscapeEditorDrawSystem* drawSystem, Heightmap* originalHeightmap, const Rect& updatedRect);
    ~ModifyHeightmapCommand() override;

private:
    void Redo() override;
    void Undo() override;

    uint16* GetHeightmapRegion(Heightmap* heightmap);
    void ApplyHeightmapRegion(uint16* region);

private:
    LandscapeEditorDrawSystem* drawSystem = nullptr;

    uint16* undoRegion = nullptr;
    uint16* redoRegion = nullptr;
    Rect updatedRect;

    DAVA_VIRTUAL_REFLECTION(ModifyHeightmapCommand, RECommand);
};

class LandscapeEditorSystemV2;
class Landscape;

class ModifyHeightmapCommandV2 : public RECommand
{
public:
    ModifyHeightmapCommandV2(LandscapeEditorSystemV2* system, Landscape* landscape, const Vector<uint16>& srcData, const Vector<uint16>& dstData, const Rect2i& updatedRect);

    void Redo() override;
    void Undo() override;

private:
    LandscapeEditorSystemV2* system = nullptr;
    Landscape* landscape = nullptr;
    Vector<uint16> srcData;
    Vector<uint16> dstData;
    Rect2i updatedRect;

    DAVA_VIRTUAL_REFLECTION(ModifyHeightmapCommandV2, RECommand);
};
} // namespace DAVA
