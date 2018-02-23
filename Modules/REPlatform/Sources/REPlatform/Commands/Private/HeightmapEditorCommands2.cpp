#include "REPlatform/Commands/HeightmapEditorCommands2.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/Heightmap.h>
#include <Render/Highlevel/Landscape.h>

namespace DAVA
{
ModifyHeightmapCommand::ModifyHeightmapCommand(LandscapeEditorDrawSystem* drawSystem_,
                                               Heightmap* originalHeightmap,
                                               const Rect& updatedRect)
    : RECommand("Height Map Change")
    , drawSystem(drawSystem_)
{
    HeightmapProxy* heightmapProxy = drawSystem->GetHeightmapProxy();
    if (originalHeightmap && heightmapProxy)
    {
        this->updatedRect = updatedRect;
        undoRegion = GetHeightmapRegion(originalHeightmap);
        redoRegion = GetHeightmapRegion(heightmapProxy->GetHeightmap());
    }
}

ModifyHeightmapCommand::~ModifyHeightmapCommand()
{
    SafeDeleteArray(undoRegion);
    SafeDeleteArray(redoRegion);
}

void ModifyHeightmapCommand::Redo()
{
    ApplyHeightmapRegion(redoRegion);
}

void ModifyHeightmapCommand::Undo()
{
    ApplyHeightmapRegion(undoRegion);
}

uint16* ModifyHeightmapCommand::GetHeightmapRegion(Heightmap* heightmap)
{
    int32 size = heightmap->Size();
    int32 width = static_cast<int32>(ceilf(updatedRect.dx));
    int32 height = static_cast<int32>(ceilf(updatedRect.dy));
    int32 xOffset = static_cast<int32>(floorf(updatedRect.x));
    int32 yOffset = static_cast<int32>(floorf(updatedRect.y));

    DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

    uint16* newData = new uint16[width * height];
    uint16* oldData = heightmap->Data();

    for (int32 i = 0; i < height; ++i)
    {
        uint16* src = oldData + (yOffset + i) * size + xOffset;
        uint16* dst = newData + i * width;
        memcpy(dst, src, sizeof(uint16) * width);
    }

    return newData;
}

void ModifyHeightmapCommand::ApplyHeightmapRegion(uint16* region)
{
    int32 size = drawSystem->GetHeightmapProxy()->GetHeightmap()->Size();
    int32 width = static_cast<int32>(ceilf(updatedRect.dx));
    int32 height = static_cast<int32>(ceilf(updatedRect.dy));
    int32 xOffset = static_cast<int32>(floorf(updatedRect.x));
    int32 yOffset = static_cast<int32>(floorf(updatedRect.y));

    DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

    uint16* data = drawSystem->GetHeightmapProxy()->GetHeightmap()->Data();

    for (int32 i = 0; i < height; ++i)
    {
        uint16* src = region + i * width;
        uint16* dst = data + (yOffset + i) * size + xOffset;
        memcpy(dst, src, sizeof(uint16) * width);
    }

    drawSystem->GetHeightmapProxy()->UpdateRect(updatedRect);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ModifyHeightmapCommand)
{
    ReflectionRegistrator<ModifyHeightmapCommand>::Begin()
    .End();
}

ModifyHeightmapCommandV2::ModifyHeightmapCommandV2(LandscapeEditorSystemV2* system_, Landscape* landscape_, const Vector<uint16>& srcData_,
                                                   const Vector<uint16>& dstData_, const Rect2i& updatedRect_)
    : RECommand("Heightmap edit")
    , system(system_)
    , landscape(landscape_)
    , srcData(srcData_)
    , dstData(dstData_)
    , updatedRect(updatedRect_)
{
}

void ModifyHeightmapCommandV2::Redo()
{
    system->UpdateHeightmap(landscape, dstData, updatedRect);
}

void ModifyHeightmapCommandV2::Undo()
{
    system->UpdateHeightmap(landscape, srcData, updatedRect);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ModifyHeightmapCommandV2)
{
    ReflectionRegistrator<ModifyHeightmapCommandV2>::Begin()
    .End();
}

} // namespace DAVA
