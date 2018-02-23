#include "Render/3D/Geometry.h"
#include "FileSystem/File.h"

namespace DAVA
{
Geometry::Geometry()
{
}

Geometry::~Geometry()
{
    for (PolygonGroup* pg : geometries)
    {
        SafeRelease(pg);
    }
    geometries.clear();
}

void Geometry::AddPolygonGroup(PolygonGroup* pgroup)
{
    geometries.emplace_back(SafeRetain(pgroup));
}

uint32 Geometry::GetPolygonGroupCount() const
{
    return uint32(geometries.size());
}

PolygonGroup* Geometry::GetPolygonGroup(uint32 index) const
{
    DVASSERT(index < uint32(geometries.size()));
    return geometries[index];
}

void Geometry::Load(const FilePath& filepath)
{
    File* file = File::Create(filepath, File::OPEN | File::READ);

    uint8 size = 0;
    file->Read(&size, sizeof(uint8));

    geometries.resize(size);

    for (uint8 geoIndex = 0; geoIndex < size; ++geoIndex)
    {
        geometries[geoIndex] = new PolygonGroup();
        KeyedArchive* ka = new KeyedArchive();
        ka->Load(file);
        /*
            TODO: Add ability to download data according to material requirements
         */
        geometries[geoIndex]->LoadPolygonData(ka, nullptr, 0, false);
        SafeRelease(ka);
    }
    SafeRelease(file);
}

void Geometry::Save(const FilePath& filepath)
{
    File* file = File::Create(filepath, File::CREATE | File::WRITE);

    DVASSERT(geometries.size() < 256);

    uint8 size = uint8(geometries.size());
    file->Write(&size, sizeof(uint8));

    for (uint8 geoIndex = 0; geoIndex < size; ++geoIndex)
    {
        KeyedArchive* ka = new KeyedArchive();
        geometries[geoIndex]->Save(ka, nullptr);
        ka->Save(file);
        SafeRelease(ka);
    }

    SafeRelease(file);
}

void Geometry::Reload()
{
}
};
