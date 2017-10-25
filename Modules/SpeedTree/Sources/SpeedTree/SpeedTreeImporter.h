#pragma once 

#ifdef __DAVAENGINE_SPEEDTREE__

#include "DAVAEngine.h"

namespace DAVA
{
class SpeedTreeImporter : public BaseObject, public XMLParserDelegate
{
public:
    static void ImportSpeedTreeFromXML(const FilePath& xmlPath, const FilePath& outFile, const FilePath& texturesDir);

private:
    enum eElementType
    {
        ELEMENT_SPEED_TREE_RAW = 0,
        ELEMENT_BOUNDS,
        ELEMENT_MIN,
        ELEMENT_MAX,

        ELEMENT_MATERIALS,
        ELEMENT_MATERIAL,
        ELEMENT_TEXTURES,

        ELEMENT_LODS,
        ELEMENT_LOD,
        ELEMENT_GEOMETRIES,
        ELEMENT_GEOMETRY,
        ELEMENT_VERTICES,
        ELEMENT_INDICES,

        ELEMENT_VERTICES_ANCHOR_X,
        ELEMENT_VERTICES_ANCHOR_Y,
        ELEMENT_VERTICES_ANCHOR_Z,
        ELEMENT_VERTICES_OFFSET_X,
        ELEMENT_VERTICES_OFFSET_Y,
        ELEMENT_VERTICES_OFFSET_Z,
        ELEMENT_VERTICES_NORMAL_X,
        ELEMENT_VERTICES_NORMAL_Y,
        ELEMENT_VERTICES_NORMAL_Z,
        ELEMENT_VERTICES_TEXCOORDS_U,
        ELEMENT_VERTICES_TEXCOORDS_V,
        ELEMENT_VERTICES_AMBIENT,

        ELEMENT_COUNT
    };

    enum eAttributeType
    {
        ATTR_COUNT = 0,
        ATTR_MAX,
        ATTR_LEVEL,
        ATTR_ID,
        ATTR_DIFFUSE,
        ATTR_AMBIENT,
        ATTR_SPECULAR,
        ATTR_OPAQUE,
        ATTR_MATERIAL_ID,
        ATTR_TYPE,
        ATTR_X,
        ATTR_Y,
        ATTR_Z,

        ATTR_ENUM_SIZE
    };

    enum eGeometryType
    {
        GEOMETRY_TYPE_BRANCH = 0,
        GEOMETRY_TYPE_LEAF,
        GEOMETRY_TYPE_FACING_LEAF,
        GEOMETRY_TYPE_CAP,
        GEOMETRY_TYPE_TRUNK,

        GEOMETRY_TYPE_COUNT
    };

    static const Array<String, GEOMETRY_TYPE_COUNT> SPEED_TREE_XML_GEOMETRY_TYPE;
    static const Array<String, ELEMENT_COUNT> SPEED_TREE_XML_ELEMENT_KEYS;
    static const Array<String, ATTR_ENUM_SIZE> SPEED_TREE_XML_ATTRIBUTES_KEYS;

    struct MaterialData
    {
        Map<FastName, FilePath> textures;
    };

    struct VertexData
    {
        Vector3 anchor;
        Vector3 offset;
        Vector2 texCoords;
        Vector3 normal;
        float32 ambientOcclusion;
    };

    struct GeometryData
    {
        Vector<VertexData> vertices;
        Vector<int16> indices;
        eGeometryType geometryType = GEOMETRY_TYPE_COUNT;
        int32 materialID = 0;
    };

    struct LodGeometryData
    {
        Vector<GeometryData> geometryData;
        size_t vertexCount = 0;
        size_t indexCount = 0;
    };

    struct TreePartData
    {
        Vector<LodGeometryData> lodGeometryData;
        Set<int32> materials;
    };

private:
    SpeedTreeImporter(Entity*& _entity, const FilePath& texturesDir, const FilePath& xmlPath);
    ~SpeedTreeImporter();

    struct TextureMapping
    {
        Vector2 offset;
        Vector2 scale = Vector2(1.f, 1.f);
    };

    struct AtlasData
    {
        Map<int32, TextureMapping> textureMapping; //tex-coord offset and scale by material id
        FilePath packedTexturePath;
    };

    void OnElementStarted(const String& elementName, const String& namespaceURI, const String& qualifedName, const Map<String, String>& attributes) override;
    void OnElementEnded(const String& elementName, const String& namespaceURI, const String& qualifedName) override;
    void OnFoundCharacters(const String& chars) override;

    void BuildEntity();
    void ProcessTrunkGeometry(SpeedTreeObject* treeObject);
    void ProcessLeafsGeometry(SpeedTreeObject* treeObject);

    AtlasData PackLeafsTextures(const FastName& textureName);
    TextureMapping ParseOffsetScale(const FilePath& file);

    FilePath FindTexture(const FilePath& path);
    FilePath CopyTextureAndCreateDescriptor(const FilePath& texturePath);

    static const String& GetAttributeKey(eAttributeType attr)
    {
        return SPEED_TREE_XML_ATTRIBUTES_KEYS[attr];
    }
    static const String& GetElementKey(eElementType element)
    {
        return SPEED_TREE_XML_ELEMENT_KEYS[element];
    }
    static const String& GetGeometryTypeString(eGeometryType type)
    {
        return SPEED_TREE_XML_GEOMETRY_TYPE[type];
    }
    static eElementType GetElementType(const String& elemKey);
    static eAttributeType GetAttributeType(const String& attrKey);
    static eGeometryType GetGeometryType(const String& geomType);

    template <class T>
    T GetAttributeValue(const Map<String, String>& attributesMap, eAttributeType attr);

private:
    Entity* entity = nullptr;
    FilePath texturesDir;
    FilePath xmlPath;

    TreePartData trunkData;
    TreePartData leafsData;
    Map<int32, MaterialData> materialsData;
    uint32 lodCount = 0;
    uint32 materialsCount = 0;
    AABBox3 bbox;

    eElementType currentElement = eElementType::ELEMENT_COUNT;
    int32 currentMaterialID = -1;
    int32 currendLodLevel = -1;
    LodGeometryData* currentLodGeometryData = nullptr;
    GeometryData* currentGeometryData = nullptr;
};

template <class T>
inline T SpeedTreeImporter::GetAttributeValue(const Map<String, String>& attributesMap, eAttributeType attr)
{
    String attrStr;
    GetAttribute(attributesMap, GetAttributeKey(attr), attrStr);
    return ParseStringTo<T>(attrStr);
}
};

#endif //__DAVAENGINE_SPEEDTREE__
