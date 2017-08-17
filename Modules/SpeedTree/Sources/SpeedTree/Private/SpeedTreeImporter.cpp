#pragma once

#include "SpeedTree/SpeedTreeImporter.h"

#include "Debug/DVAssert.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Scene3D/Converters/SpeedTreeConverter.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Tools/TexturePacker/DefinitionFile.h"
#include "Tools/TexturePacker/TexturePacker.h"
//#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"

namespace DAVA
{
const Array<String, SpeedTreeImporter::ELEMENT_COUNT> SpeedTreeImporter::SPEED_TREE_XML_ELEMENT_KEYS =
{ {
"SpeedTreeRaw", "Bounds", "Min", "Max",
"Materials", "Material", "Textures", "Lods", "Lod", "Geometries", "Geometry", "Vertices", "Indices",
"AnchorX", "AnchorY", "AnchorZ", "OffsetX", "OffsetY", "OffsetZ", "NormalX", "NormalY", "NormalZ",
"TexcoordU", "TexcoordV", "AmbientOcclusion"
} };

const Array<String, SpeedTreeImporter::ATTR_ENUM_SIZE> SpeedTreeImporter::SPEED_TREE_XML_ATTRIBUTES_KEYS =
{ {
"Count", "Max", "Level", "ID", "Diffuse", "Ambient", "Specular", "Opaque", "MaterialID", "Type", "X", "Y", "Z"
} };

const Array<String, SpeedTreeImporter::GEOMETRY_TYPE_COUNT> SpeedTreeImporter::SPEED_TREE_XML_GEOMETRY_TYPE =
{ {
"Frond", "Leaf", "FacingLeaf", "Cap", "Branch"
} };

void UpdateTextureDescriptor(TextureDescriptor* descriptor)
{
    descriptor->drawSettings.mipFilter = rhi::TEXMIPFILTER_LINEAR;
    descriptor->dataSettings.SetGenerateMipmaps(true);
    if (descriptor->pathname.IsEmpty() == false)
    {
        descriptor->Save();
    }
}

SpeedTreeImporter::SpeedTreeImporter(Entity*& _entity, const FilePath& texDir, const FilePath& _xmlPath)
    : entity(_entity)
    , texturesDir(texDir)
    , xmlPath(_xmlPath)
{
    DVASSERT(entity);
    DVASSERT(GetRenderComponent(entity) == nullptr);
    DVASSERT(GetRenderObject(entity) == nullptr);
    DVASSERT(GetLodComponent(entity) == nullptr);
}

SpeedTreeImporter::~SpeedTreeImporter()
{
}

void SpeedTreeImporter::ImportSpeedTreeFromXML(const FilePath& xmlPath, const FilePath& outFile, const FilePath& texturesDir)
{
    DVASSERT(!xmlPath.IsEmpty() && !outFile.IsEmpty() && !texturesDir.IsEmpty() && texturesDir.IsDirectoryPathname());

    if (!outFile.GetDirectory().Exists())
        FileSystem::Instance()->CreateDirectory(outFile.GetDirectory(), true);
    if (!texturesDir.Exists())
        FileSystem::Instance()->CreateDirectory(texturesDir, true);

    Entity* entity = new Entity();
    entity->SetName(FastName(xmlPath.GetBasename()));
    SpeedTreeImporter* importer = new SpeedTreeImporter(entity, texturesDir, xmlPath);
    XMLParser::ParseFile(xmlPath, importer);
    SafeRelease(importer);

    Scene* scene = new Scene();
    scene->AddNode(entity);
    scene->SaveScene(outFile);

    SafeRelease(scene);
    SafeRelease(entity);
}

void SpeedTreeImporter::OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes)
{
    eElementType prevElementType = currentElement;
    currentElement = GetElementType(elementName);

    if (currentElement == ELEMENT_MIN && prevElementType == ELEMENT_BOUNDS)
    {
        bbox.min.x = GetAttributeValue<float32>(attributes, ATTR_X);
        bbox.min.y = GetAttributeValue<float32>(attributes, ATTR_Y);
        bbox.min.z = GetAttributeValue<float32>(attributes, ATTR_Z);
    }
    else if (currentElement == ELEMENT_MAX)
    {
        bbox.max.x = GetAttributeValue<float32>(attributes, ATTR_X);
        bbox.max.y = GetAttributeValue<float32>(attributes, ATTR_Y);
        bbox.max.z = GetAttributeValue<float32>(attributes, ATTR_Z);
    }
    else if (currentElement == ELEMENT_MATERIALS)
    {
        materialsCount = GetAttributeValue<uint32>(attributes, ATTR_COUNT);
    }
    else if (currentElement == ELEMENT_MATERIAL)
    {
        currentMaterialID = GetAttributeValue<uint32>(attributes, ATTR_ID);
    }
    else if (currentElement == ELEMENT_TEXTURES)
    {
        materialsData[currentMaterialID].textures[NMaterialTextureName::TEXTURE_ALBEDO] = FilePath(GetAttributeValue<String>(attributes, ATTR_DIFFUSE));
    }
    else if (currentElement == ELEMENT_LODS)
    {
        lodCount = GetAttributeValue<uint32>(attributes, ATTR_COUNT);
        trunkData.lodGeometryData.resize(lodCount);
        leafsData.lodGeometryData.resize(lodCount);
    }
    else if (currentElement == ELEMENT_LOD)
    {
        currendLodLevel = GetAttributeValue<uint32>(attributes, ATTR_LEVEL);
    }
    else if (currentElement == ELEMENT_GEOMETRY)
    {
        eGeometryType geometryType = GetGeometryType(GetAttributeValue<String>(attributes, ATTR_TYPE));
        DVASSERT(geometryType != GEOMETRY_TYPE_COUNT, Format("Unknown geometry type: %s", GetAttributeValue<String>(attributes, ATTR_TYPE).c_str()).c_str());

        TreePartData& partData = (geometryType == GEOMETRY_TYPE_TRUNK) ? trunkData : leafsData;
        currentLodGeometryData = &partData.lodGeometryData[currendLodLevel];
        currentLodGeometryData->geometryData.emplace_back();

        currentGeometryData = &currentLodGeometryData->geometryData.back();
        currentGeometryData->geometryType = geometryType;
        currentGeometryData->materialID = GetAttributeValue<int32>(attributes, ATTR_MATERIAL_ID);

        partData.materials.insert(currentGeometryData->materialID);
    }
    else if (currentElement == ELEMENT_VERTICES && prevElementType == ELEMENT_GEOMETRY)
    {
        currentGeometryData->vertices.resize(GetAttributeValue<uint32>(attributes, ATTR_COUNT));
        currentLodGeometryData->vertexCount += currentGeometryData->vertices.size();
    }
    else if (currentElement == ELEMENT_INDICES)
    {
        currentGeometryData->indices.resize(GetAttributeValue<uint32>(attributes, ATTR_COUNT));
        currentLodGeometryData->indexCount += currentGeometryData->indices.size();
    }
}

void SpeedTreeImporter::OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName)
{
    currentElement = GetElementType(elementName);

    if (currentElement == ELEMENT_SPEED_TREE_RAW)
    {
        BuildEntity();
    }
    else if (currentElement == ELEMENT_MATERIAL)
    {
        currentMaterialID = -1;
    }
    else if (currentElement == ELEMENT_LOD)
    {
        currendLodLevel = -1;
    }
    else if (currentElement == ELEMENT_GEOMETRY)
    {
        currentLodGeometryData = nullptr;
        currentGeometryData = nullptr;
    }

    currentElement = ELEMENT_COUNT;
}

void SpeedTreeImporter::OnFoundCharacters(const String& chars)
{
    if (currentElement < ELEMENT_INDICES || currentElement == ELEMENT_COUNT)
        return;

    Vector<String> values;
    Split(chars, " \x0A", values);

    DVASSERT(currentGeometryData->indices.size() == values.size() || currentElement != ELEMENT_INDICES);
    DVASSERT(currentGeometryData->vertices.size() == values.size() || currentElement == ELEMENT_INDICES);

    for (size_t i = 0; i < values.size(); ++i)
    {
        if (currentElement == ELEMENT_INDICES)
        {
            currentGeometryData->indices[i] = ParseStringTo<int16>(values[i]);
        }
        else
        {
            VertexData& vxData = currentGeometryData->vertices[i];

            if (currentElement == ELEMENT_VERTICES_ANCHOR_X)
                vxData.anchor.x = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_ANCHOR_Y)
                vxData.anchor.y = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_ANCHOR_Z)
                vxData.anchor.z = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_OFFSET_X)
                vxData.offset.x = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_OFFSET_Y)
                vxData.offset.y = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_OFFSET_Z)
                vxData.offset.z = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_NORMAL_X)
                vxData.normal.x = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_NORMAL_Y)
                vxData.normal.y = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_NORMAL_Z)
                vxData.normal.z = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_TEXCOORDS_U)
                vxData.texCoords.x = ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_TEXCOORDS_V)
                vxData.texCoords.y = 1.f - ParseStringTo<float32>(values[i]);
            else if (currentElement == ELEMENT_VERTICES_AMBIENT)
                vxData.ambientOcclusion = ParseStringTo<float32>(values[i]);
        }
    }
}

void SpeedTreeImporter::BuildEntity()
{
    ScopedPtr<SpeedTreeObject> treeObject(new SpeedTreeObject());
    entity->AddComponent(new LodComponent());
    entity->AddComponent(new RenderComponent(treeObject));
    entity->AddComponent(new SpeedTreeComponent());

    ProcessTrunkGeometry(treeObject);
    ProcessLeafsGeometry(treeObject);
}

void SpeedTreeImporter::ProcessTrunkGeometry(SpeedTreeObject* treeObject)
{
    NMaterial* rootMaterial = new NMaterial();
    rootMaterial->SetFXName(NMaterialName::SPEEDTREE_OPAQUE);
    rootMaterial->SetMaterialName(FastName(Format("%s_trunk", xmlPath.GetBasename().c_str())));
    float32 color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    rootMaterial->AddProperty(NMaterialParamName::PARAM_TREE_LEAF_COLOR_MUL, color, rhi::ShaderProp::TYPE_FLOAT4, 1);
    float32 occlusionMul = 1.0f;
    rootMaterial->AddProperty(FastName("treeLeafOcclusionMul"), &occlusionMul, rhi::ShaderProp::TYPE_FLOAT1, 1);

    Map<int32, NMaterial*> materials;
    for (int32 materialID : trunkData.materials)
    {
        NMaterial* m = new NMaterial();
        m->SetParent(rootMaterial);
        m->SetMaterialName(FastName(Format("Instance%d", materialID)));

        FilePath albedoPath = FindTexture(materialsData[materialID].textures[NMaterialTextureName::TEXTURE_ALBEDO]);
        if (!albedoPath.IsEmpty())
        {
            albedoPath = CopyTextureAndCreateDescriptor(albedoPath);
            ScopedPtr<Texture> albedoTexture(Texture::CreateFromFile(albedoPath));
            DAVA::TextureDescriptor* descriptor = albedoTexture->GetDescriptor();
            UpdateTextureDescriptor(descriptor);
            m->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, albedoTexture);
        }

        materials[materialID] = m;
    }

    //////////////////////////////////////////////////////////////////////////

    float32 treeHeight = (bbox.max.z - bbox.min.z);
    DVASSERT(uint32(trunkData.lodGeometryData.size()) == lodCount);
    for (uint32 lod = 0; lod < lodCount; ++lod)
    {
        const LodGeometryData& lodGeometry = trunkData.lodGeometryData[lod];

        if (lodGeometry.vertexCount == 0 || lodGeometry.indexCount == 0)
            continue;

        Map<int32, GeometryData> combinedData;
        for (const GeometryData& gData : lodGeometry.geometryData)
        {
            GeometryData& cData = combinedData[gData.materialID];
            cData.indices.reserve(cData.indices.size() + gData.indices.size());

            uint16 indexOffset = uint16(cData.vertices.size());
            for (uint16 ind : gData.indices)
                cData.indices.push_back(ind + indexOffset);

            cData.vertices.insert(cData.vertices.end(), gData.vertices.begin(), gData.vertices.end());
        }

        for (auto& it : combinedData)
        {
            int32 vxCount = int32(it.second.vertices.size());
            int32 indCount = int32(it.second.indices.size());

            ScopedPtr<PolygonGroup> dataSource(new PolygonGroup());
            dataSource->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_PIVOT4 | EVF_FLEXIBILITY | EVF_ANGLE_SIN_COS, vxCount, indCount);

            uint32 vi = 0;
            for (const VertexData& vx : it.second.vertices)
            {
                uint32 color = 0xff000000 | (((uint32)(vx.ambientOcclusion * 255.f)) << 16) | (((uint32)(vx.ambientOcclusion * 255.f)) << 8) | ((uint32)(vx.ambientOcclusion * 255.f));

                dataSource->SetCoord(vi, vx.anchor + vx.offset);
                dataSource->SetColor(vi, color);
                dataSource->SetTexcoord(0, vi, vx.texCoords);
                dataSource->SetPivot(vi, Vector4());
                dataSource->SetFlexibility(vi, SpeedTreeConverter::CalculateVertexFlexibility(dataSource, vi, treeHeight));
                dataSource->SetAngle(vi, SpeedTreeConverter::CalculateVertexAngle(dataSource, vi, treeHeight));

                ++vi;
            }

            Memcpy(dataSource->indexArray, it.second.indices.data(), indCount * sizeof(uint16));

            ScopedPtr<RenderBatch> batch(new RenderBatch());
            ScopedPtr<PolygonGroup> sortedDataSource(SpeedTreeObject::CreateSortedPolygonGroup(dataSource));

            batch->SetMaterial(materials[it.first]);
            batch->SetPolygonGroup(sortedDataSource);

            treeObject->AddRenderBatch(batch, lod, -1);
        }
    }

    for (auto& it : materials)
        SafeRelease(it.second);
}

void SpeedTreeImporter::ProcessLeafsGeometry(SpeedTreeObject* treeObject)
{
    AtlasData atlasData = PackLeafsTextures(NMaterialTextureName::TEXTURE_ALBEDO);
    ScopedPtr<Texture> albedoTexture(Texture::CreateFromFile(atlasData.packedTexturePath));
    TextureDescriptor* descriptor = albedoTexture->GetDescriptor();
    UpdateTextureDescriptor(descriptor);

    ScopedPtr<NMaterial> rootMaterial(new NMaterial());
    rootMaterial->SetFXName(NMaterialName::SPEEDTREE_ALPHABLEND);
    rootMaterial->SetMaterialName(FastName(Format("%s_leafs", xmlPath.GetBasename().c_str())));
    rootMaterial->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, albedoTexture);
    rootMaterial->AddFlag(NMaterialFlagName::FLAG_ALPHASTEPVALUE, 1);
    float32 color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    rootMaterial->AddProperty(NMaterialParamName::PARAM_TREE_LEAF_COLOR_MUL, color, rhi::ShaderProp::TYPE_FLOAT4, 1);
    float32 occlusionMul = 1.0f;
    rootMaterial->AddProperty(FastName("treeLeafOcclusionMul"), &occlusionMul, rhi::ShaderProp::TYPE_FLOAT1, 1);

    float32 treeHeight = (bbox.max.z - bbox.min.z);
    DVASSERT(uint32(leafsData.lodGeometryData.size()) == lodCount);
    for (uint32 lod = 0; lod < lodCount; ++lod)
    {
        const LodGeometryData& lodGeometry = leafsData.lodGeometryData[lod];

        if (lodGeometry.vertexCount == 0 || lodGeometry.indexCount == 0)
            continue;

        int32 vxCount = int32(lodGeometry.vertexCount);
        int32 indCount = int32(lodGeometry.indexCount);

        ScopedPtr<PolygonGroup> dataSource(new PolygonGroup());
        dataSource->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_PIVOT4 | EVF_FLEXIBILITY | EVF_ANGLE_SIN_COS, vxCount, indCount);

        int32 vi = 0;
        int32 ii = 0;
        int16 indOffset = 0;
        for (const GeometryData& gData : lodGeometry.geometryData)
        {
            const Vector2& texCoordOffset = atlasData.textureMapping[gData.materialID].offset;
            const Vector2& texCoordScale = atlasData.textureMapping[gData.materialID].scale;
            for (const VertexData& vx : gData.vertices)
            {
                dataSource->SetCoord(vi, vx.anchor + vx.offset);

                uint32 color = 0xff000000 | (((uint32)(vx.ambientOcclusion * 255.f)) << 16) | (((uint32)(vx.ambientOcclusion * 255.f)) << 8) | ((uint32)(vx.ambientOcclusion * 255.f));
                dataSource->SetColor(vi, color);

                Vector2 texCoords = vx.texCoords * texCoordScale + texCoordOffset;
                dataSource->SetTexcoord(0, vi, texCoords);

                Vector4 pivot = (gData.geometryType == GEOMETRY_TYPE_FACING_LEAF) ? Vector4(vx.anchor, 1.f) : Vector4();
                dataSource->SetPivot(vi, pivot);

                dataSource->SetFlexibility(vi, SpeedTreeConverter::CalculateVertexFlexibility(dataSource, vi, treeHeight));
                dataSource->SetAngle(vi, SpeedTreeConverter::CalculateVertexAngle(dataSource, vi, treeHeight));

                ++vi;
            }

            for (const uint16& ind : gData.indices)
            {
                dataSource->SetIndex(ii, ind + indOffset);
                ++ii;
            }

            indOffset += uint16(gData.vertices.size());
        }

        DVASSERT(vi == vxCount);
        DVASSERT(ii == indCount);

        ScopedPtr<NMaterial> m(new NMaterial());
        m->SetMaterialName(FastName(Format("Instance%d", lod)));
        m->SetParent(rootMaterial);

        ScopedPtr<PolygonGroup> sortedDataSource(SpeedTreeObject::CreateSortedPolygonGroup(dataSource));
        ScopedPtr<RenderBatch> batch(new RenderBatch());
        batch->SetMaterial(m);
        batch->SetPolygonGroup(sortedDataSource);

        treeObject->AddRenderBatch(batch, lod, -1);
    }
}

SpeedTreeImporter::AtlasData SpeedTreeImporter::PackLeafsTextures(const FastName& textureName)
{
    AtlasData packData;

    TexturePacker packer;
    packer.SetUseOnlySquareTextures();
    packer.SetMaxTextureSize(16u * 1024u);
    packer.SetTexturesMargin(0);
    packer.SetAlgorithms({
    PackingAlgorithm::ALG_MAXRECTS_BEST_AREA_FIT,
    PackingAlgorithm::ALG_MAXRECTS_BEST_LONG_SIDE_FIT,
    PackingAlgorithm::ALG_MAXRECTS_BEST_SHORT_SIDE_FIT,
    PackingAlgorithm::ALG_MAXRECTS_BOTTOM_LEFT,
    PackingAlgorithm::ALG_MAXRRECT_BEST_CONTACT_POINT
    });

    FilePath tempProcessDir = texturesDir + "$process/";
    FileSystem::Instance()->CreateDirectory(tempProcessDir, true);

    Set<FilePath> texturesToPack;
    Map<int32, String> materialIDToTextureBaseName;
    for (int32 materialID : leafsData.materials)
    {
        FilePath texturePath = FindTexture(materialsData[materialID].textures[textureName]);

        if (texturePath.IsEmpty())
            continue;

        texturesToPack.insert(texturePath);
        materialIDToTextureBaseName[materialID] = texturePath.GetBasename();
    }

    DefinitionFile::Collection definitionFileList;
    for (const FilePath& texturePath : texturesToPack)
    {
        definitionFileList.emplace_back(new DefinitionFile());
        definitionFileList.back()->LoadPNG(texturePath, tempProcessDir);
    }

    if (!definitionFileList.empty())
    {
        String atlasBaseName = Format("%s_leafs_%s", xmlPath.GetBasename().c_str(), textureName.c_str());
        packer.PackToMultipleTextures(texturesDir, atlasBaseName.c_str(), definitionFileList, { GPU_ORIGIN });
        packData.packedTexturePath = texturesDir + atlasBaseName + "0.tex";

        Map<String, TextureMapping> packedTextures;
        for (const FilePath& texturePath : texturesToPack)
        {
            String baseName = texturePath.GetBasename();
            FilePath defFilePath = texturesDir + baseName + ".txt";
            packedTextures[baseName] = ParseOffsetScale(defFilePath);
            FileSystem::Instance()->DeleteFile(defFilePath);
        }

        for (int32 materialID : leafsData.materials)
            packData.textureMapping[materialID] = packedTextures[materialIDToTextureBaseName[materialID]];
    }

    FileSystem::Instance()->DeleteDirectory(tempProcessDir);

    return packData;
}

SpeedTreeImporter::TextureMapping SpeedTreeImporter::ParseOffsetScale(const FilePath& filePath)
{
    ScopedPtr<File> file(File::Create(filePath, DAVA::File::OPEN | DAVA::File::READ));

    DVASSERT(file);
    if (!file)
        return TextureMapping();

    DAVA::char8 buf[512] = {};
    file->ReadLine(buf, sizeof(buf)); //textures count

    DAVA::uint32 readSize = file->ReadLine(buf, sizeof(buf)); //texture name
    DAVA::FilePath atlasPath = filePath.GetDirectory() + DAVA::String(buf, readSize);
    atlasPath.ReplaceExtension(".png");

    file->ReadLine(buf, sizeof(buf)); //image size
    file->ReadLine(buf, sizeof(buf)); //frames count
    file->ReadLine(buf, sizeof(buf)); //frame rect

    int32 x, y, dx, dy, unused0, unused1, unused2;
    sscanf(buf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &unused0, &unused1, &unused2);

    ImageInfo atlasInfo = ImageSystem::GetImageInfo(atlasPath);
    Vector2 atlasSize(float32(atlasInfo.width), float32(atlasInfo.height));

    TextureMapping mapping;
    mapping.offset = Vector2(float32(x), float32(y)) / atlasSize;
    mapping.scale = Vector2(float32(dx), float32(dy)) / atlasSize;
    return mapping;
}

FilePath SpeedTreeImporter::FindTexture(const FilePath& path)
{
    if (path.IsEmpty())
        return path;

    if (!FileSystem::Instance()->Exists(path))
    {
        FilePath texturePath = path;
        texturePath.ReplaceDirectory(xmlPath.GetDirectory());

        if (FileSystem::Instance()->Exists(texturePath))
            return texturePath;
    }

    Logger::Error("[SpeedTreeImporter] Can't find file '%s'", path.GetFilename().c_str());
    return FilePath();
}

FilePath SpeedTreeImporter::CopyTextureAndCreateDescriptor(const FilePath& texturePath)
{
    DVASSERT(FileSystem::Instance()->Exists(texturePath));

    FilePath copyPath = texturesDir + texturePath.GetFilename();
    FileSystem::Instance()->CopyFile(texturePath, copyPath, true);
    //    TextureDescriptorUtils::CreateOrUpdateDescriptor(copyPath);

    return copyPath;
}

SpeedTreeImporter::eElementType SpeedTreeImporter::GetElementType(const String& elemKey)
{
    for (int i = 0; i < ELEMENT_COUNT; i++)
        if (SPEED_TREE_XML_ELEMENT_KEYS[i] == elemKey)
            return (eElementType)i;

    return ELEMENT_COUNT;
}

SpeedTreeImporter::eAttributeType SpeedTreeImporter::GetAttributeType(const String& attrKey)
{
    for (int i = 0; i < ATTR_ENUM_SIZE; i++)
        if (SPEED_TREE_XML_ATTRIBUTES_KEYS[i] == attrKey)
            return (eAttributeType)i;

    return ATTR_ENUM_SIZE;
}

SpeedTreeImporter::eGeometryType SpeedTreeImporter::GetGeometryType(const String& geomType)
{
    for (int i = 0; i < GEOMETRY_TYPE_COUNT; i++)
        if (SPEED_TREE_XML_GEOMETRY_TYPE[i] == geomType)
            return (eGeometryType)i;

    return GEOMETRY_TYPE_COUNT;
};
}
