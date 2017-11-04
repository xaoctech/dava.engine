/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
 * Created by Yury Danilov
   =====================================================================================*/
#include "Model.h"
#include "physics2/bsp.hpp"

#include <REPlatform/Deprecated/EditorConfig.h>

#include <Scene3D/Components/SwitchComponent.h>

DAVA::Map<DAVA::String, DAVA::int32> InitMaterialKindMap()
{
    using namespace DAVA;

    //from material_kinds.xml:
    // <id> 112</id><effect_material>metal</effect_material>
    // <id> 111</id><effect_material> stone </effect_material>
    // <id> 110</id><effect_material>snow</effect_material>
    // <id> 108 </id><effect_material> wood </effect_material>
    // <id> 107 </id><effect_material> ground </effect_material>
    // <id> 103 </id><effect_material> sand </effect_material>
    Map<String, int32> map;
    map["metal"] = 112;
    map["stone"] = 111;
    map["snow"] = 110;
    map["wood"] = 108;
    map["ground"] = 107;
    map["sand"] = 103;
    return map;
}

// enum
// {
//  // Nothing will ever collide with this triangle
//  TRIANGLE_NOT_IN_BSP			= 0xff,
//
//  TRIANGLE_CAMERANOCOLLIDE	= 1 << 0,
//  TRIANGLE_TRANSPARENT		= 1 << 1,
//  TRIANGLE_BLENDED			= 1 << 2,
//  TRIANGLE_TERRAIN			= 1 << 3,
//
//  // Player does not collide. Camera still does
//  TRIANGLE_NOCOLLIDE			= 1 << 4,
//
//  TRIANGLE_DOUBLESIDED		= 1 << 5,
//  TRIANGLE_DOOR				= 1 << 6,
//  TRIANGLE_PROJECTILENOCOLLIDE = 1 << 7,
// };

static DAVA::Map<DAVA::String, DAVA::int32> materialKindMap = InitMaterialKindMap();

bool IsFragile(eModelPreset value)
{
    return (value == eMP_FRAGILE_PR || value == eMP_FRAGILE_NPR);
}

bool IsFallingAtom(eModelPreset value)
{
    return (value == eMP_FALLING_ATOM);
}

bool IsTree(eModelPreset value)
{
    return (value == eMP_TREE);
}

DAVA::int32 Model::GetBWCollision(DAVA::int32 primitiveGroup)
{
    DAVA::int32 result = 0;

    if (collision == eMP_NO_COLLISION)
    {
        result = TRIANGLE_NOT_IN_BSP;
    }
    else if (collision == eMP_TREE)
    {
        if (groups[primitiveGroup].identifier.find("trunk") != std::string::npos)
        {
            result = TRIANGLE_PROJECTILENOCOLLIDE;
        }
        else
        {
            result = TRIANGLE_NOCOLLIDE | TRIANGLE_PROJECTILENOCOLLIDE;
        }
    }
    else if (collision == eMP_BUSH)
    {
        result = TRIANGLE_PROJECTILENOCOLLIDE | TRIANGLE_NOCOLLIDE;
    }
    else if (collision == eMP_FRAGILE_PR)
    {
        if (primitiveGroup == 0)
        {
            result = 0;
        }
        else if (primitiveGroup == 1)
        {
            result = (groups[1].matCollision == eMP_BUILDING) ? 0 : TRIANGLE_NOT_IN_BSP;
        }
    }
    else if (collision == eMP_FRAGILE_NPR)
    {
        if (primitiveGroup == 0)
        {
            result = TRIANGLE_PROJECTILENOCOLLIDE;
        }
        else if (primitiveGroup == 1)
        {
            result = (groups[1].matCollision == eMP_BUILDING) ? TRIANGLE_PROJECTILENOCOLLIDE : TRIANGLE_NOT_IN_BSP;
        }
    }
    else if (collision == eMP_FALLING_ATOM)
    {
        result = TRIANGLE_PROJECTILENOCOLLIDE | TRIANGLE_CAMERANOCOLLIDE | TRIANGLE_TRANSPARENT;
    }
    else if (collision == eMP_BUILDING)
    {
        result = 0;
    }
    else if (collision == eMP_INVISIBLE_WALL)
    {
        result = TRIANGLE_CAMERANOCOLLIDE | TRIANGLE_TRANSPARENT | TRIANGLE_PROJECTILENOCOLLIDE;
        return result;
    }

    //	if (groups[primitiveGroup].twoSided)
    result = result | TRIANGLE_DOUBLESIDED;

    return result;
} // Model::GetBWCollision

const DAVA::int32 MATKIND_SPT_SOLID = 71;
const DAVA::int32 MATKIND_SPT_LEAVES = 72;

const DAVA::int32 MATKIND_UNDAMAGED = 78;
const DAVA::int32 MATKIND_UNDAMAGED1 = 73;
const DAVA::int32 MATKIND_DESTROYED = 87;

const DAVA::int32 MATKIND_STONE = 111;
const DAVA::int32 MATKIND_WOOD = 108;

DAVA::int32 Model::GetBWMaterialKind(DAVA::int32 primitiveGroup)
{
    if (collision == eMP_NO_COLLISION)
    {
        return MATKIND_UNDAMAGED;
    }
    else if (collision == eMP_BUSH)
    {
        return MATKIND_SPT_LEAVES;
    }
    else if (collision == eMP_TREE)
    {
        if (groups[primitiveGroup].identifier.find("trunk") != std::string::npos)
        {
            return MATKIND_SPT_SOLID;
        }
        else
        {
            return MATKIND_SPT_LEAVES;
        }
    }
    else if (collision == eMP_FRAGILE_PR || collision == eMP_FRAGILE_NPR)
    {
        if (primitiveGroup == 0)
        {
            return MATKIND_UNDAMAGED1;
        }
        else if (primitiveGroup == 1)
        {
            return (groups[1].matCollision == eMP_BUILDING) ? 111 : MATKIND_DESTROYED;
        }
    }
    else if (collision == eMP_FALLING_ATOM)
    {
        return MATKIND_UNDAMAGED1;
    }
    else if (collision == eMP_BUILDING)
    {
        DAVA::int32 res = materialKindMap[materialKind];
        if (res < 100)
        {
            return MATKIND_WOOD;
        }
        return res;
    }
    else if (collision == eMP_INVISIBLE_WALL)
    {
        return MATKIND_UNDAMAGED;
    }
    return MATKIND_UNDAMAGED;
}

DAVA::Matrix4 InverseLambda(const DAVA::Matrix4& p)
{
    DAVA::Matrix4 tmp;
    p.GetInverse(tmp);
    return tmp;
};

const DAVA::Matrix4 Model::globalTransform =
DAVA::Matrix4::MakeRotation(DAVA::Vector3(1.0f, 0.0f, 0.0f), DAVA::DegToRad(90.0f)) * DAVA::Matrix4::MakeScale(DAVA::Vector3(1, 1, -1));
const DAVA::Matrix4 Model::globalTransformInverse = InverseLambda(globalTransform);

Model::Model(const DAVA::String& _fileName, const DAVA::String& _mapName, eModelPreset _collision, DAVA::Entity* model, DAVA::EditorConfig* config)
    : modelName(_fileName)
    , mapName(_mapName)
    , collision(_collision)
    , materialKind("wood")
{
    using namespace DAVA;

    Logger::Info("model %s %d", _fileName.c_str(), collision);

    FilePath fileName(_fileName);
    modelName = fileName.GetBasename();
    //  FilePath::Instance()->SplitPath(fileName, folder, modelName);
    //  modelName = FileSystem::Instance()->ReplaceExtension(modelName, "");

    srand((unsigned int)13);
    Scene* scene = new Scene();
    //	Entity * model = scene->GetRootNode(fileName);
    if (model != 0)
    {
        Entity* clone = model->Clone();
        clone->SetLocalTransform(Matrix4::IDENTITY);
        scene->AddNode(clone);
        scene->Update(0.1f);

        scene->transformSystem->Process(0.1f);
        scene->lodSystem->Process(0.1f);

        ReadCustomProperties(clone, config);
        CreatePolygonGroup();
        Batch(scene);
        createTris();

        Save();
    }
    else
    {
        isWrongModel = true;
        Logger::Warning("Failed to load model file %s", _fileName.c_str());
    }
    SafeRelease(scene);
}

void Model::ReadCustomProperties(DAVA::Entity* model, DAVA::EditorConfig* config)
{
    using namespace DAVA;

    KeyedArchive* p = GetCustomPropertiesArchieve(model);
    if (p)
    {
        health = p->GetInt32("Health", 5);
        fallType = (eFallType)p->GetInt32("FallType", 0);
        density = p->GetFloat("Density", DEFAULT_DENSITY);
        if (p->IsKeyExists("MaterialKind"))
        {
            int32 kind = p->GetInt32("MaterialKind");
            const Vector<String>& names = config->GetComboPropertyValues("MaterialKind");
            if (kind >= 0 && kind < names.size())
            {
                materialKind = names[kind];
            }
        }
    }
}

void ReadBuffer(char* buffer, int bufferSize, DAVA::String fileName)
{
    using namespace DAVA;

    File* f = File::Create(FilePath(fileName), File::OPEN | File::READ);
    uint32 res = f->Read(buffer, bufferSize);
    buffer[res] = 0;
    SafeRelease(f);
}

//  eMP_NO_COLLISION = 0,
//  eMP_TREE,
//  eMP_BUSH,
//  eMP_FRAGILE_PR,
//  eMP_FRAGILE_NPR,
//  eMP_FALLING_ATOM,
//  eMP_BUILDING,

void Model::Save()
{
    using namespace DAVA;

    if (IsFragile(collision))
    {
        groups[0].identifier = "n_wood_0";
        groups[1].identifier = "d_wood_0";
    }

    if (IsFallingAtom(collision))
    {
        groups[0].identifier = "partN_wood_01";
    }

    if (collision == eMP_INVISIBLE_WALL)
    {
        groups[0].identifier = "empty";
    }

    Build();
    BinaryPtr binary = Recollect();

    File* f = File::Create("out\\content\\" + mapName + modelName + ".primitives", File::CREATE | File::WRITE);

    f->Write(&(binary->operator[](0)), binary->size());
    SafeRelease(f);
    delete binary;

    char content[8192];
    char result[8192];
    char primitiveGroup[8192];
    char nodes[8192];

    ReadBuffer(content, sizeof(content), "~res:/modelTemplate\\template.model");

    DVASSERT(!bbox.insideOut(),
             Format("object %s on map %s is probably without geometry", modelName.c_str(), mapName.c_str()).c_str());

    const Vector3& min = bbox.minBounds();
    const Vector3& max = bbox.maxBounds();

    String nodeType;

    if (IsFragile(collision))
    {
        nodeType = "nodefullVisual";
    }
    else
    {
        nodeType = "nodelessVisual";
    }

    _snprintf(result, sizeof(result), content, modelName.c_str(), nodeType.c_str(),
              (mapName + modelName).c_str(), nodeType.c_str(), min.x, min.y, min.z, max.x, max.y, max.z,
              modelName.c_str());
    f = File::Create("out\\content\\" + mapName + modelName + ".model", File::CREATE | File::WRITE);

    f->Write(result, strlen(result));
    SafeRelease(f);

    String buf;

    if (collision == eMP_INVISIBLE_WALL)
    {
        ReadBuffer(primitiveGroup, sizeof(primitiveGroup), "~res:/modelTemplate\\invisible.primitiveGroup");

        _snprintf(result,
                  sizeof(result),
                  primitiveGroup,
                  groups[0].identifier.c_str(),
                  GetBWCollision(0),
                  GetBWMaterialKind(0));

        buf = buf + result;
    }
    else
    {
        ReadBuffer(primitiveGroup, sizeof(primitiveGroup), "~res:/modelTemplate\\template.primitiveGroup");

        for (uint32 i = 0; i < groups.size(); i++)
        {
            _snprintf(result,
                      sizeof(result),
                      primitiveGroup,
                      i,
                      groups[i].identifier.c_str(),
                      GetBWCollision(i),
                      GetBWMaterialKind(i),
                      (groups[i].hasAlpha ? "true" : "false"),
                      (groups[i].twoSided ? "true" : "false"),
                      (mapName + "images/" + groups[i].material).c_str());
            buf = buf + result;
        }
    }

    ReadBuffer(content, sizeof(content), "~res:/modelTemplate\\template.visual");

    if (IsFragile(collision)) //destructible object!
    {
        ReadBuffer(nodes, sizeof(nodes), "~res:/modelTemplate\\nodes.xml");
    }
    else
    {
        nodes[0] = 0;
    }

    _snprintf(result, sizeof(result), content, modelName.c_str(), nodes,
              buf.c_str(), min.x, min.y, min.z, max.x, max.y, max.z, modelName.c_str());

    f = File::Create("out\\content\\" + mapName + modelName + ".visual", File::CREATE | File::WRITE);

    f->Write(result, strlen(result));
    SafeRelease(f);

    PrepareDestructible();
} // Model::Save

DAVA::String Model::GetPhysicsParams(DAVA::int32 fallType_)
{
    static char* fallParams[4] =
    //  {"800 10 0.26 20000 4000 20 0.15",
    //  "%.1f %.1f 0.03 300000 10000 0 0.1",
    //  "%.1f %.1f 0.03 300000 10000 0 0.1",
    //  "800 10 0.26 20000 4000 20 0.15"};
    {
      "800 10 0.26 20000 4000 20 0.15",
      "400 10 0.03 300000 10000 0 0.1",
      "400 2 0.03 300000 10000 0 0.02",
      "800 10 0.26 20000 4000 20 0.15"
    };
    // масса, длина, угол пружины, жесткость, сопротивление пружины, сопротивление воздуха, углубление в землю

    //  if (fallType == eFT_STOLB || fallType == eFT_PALMA)
    //  {
    //      float32 h = bbox.height();
    //
    //      float32 mass1 = 400.0f; //kg
    //      float32 h1 = 10.0f; //meters
    //
    //      // mass = h ^ 3 * mass1 / h1^3
    //
    //      float k = h / h1;
    //      float32 mass = k * k * k * mass1;
    //
    //      char result[128];
    //      _snprintf( result, sizeof(result), fallParams[fallType], mass, h);
    //      return String(result);
    //  }
    //  else
    return DAVA::String(fallParams[fallType_]);
}

void Model::PrepareDestructible()
{
    using namespace DAVA;

    if (collision == eMP_NO_COLLISION || collision == eMP_BUILDING)
    {
        return;
    }

    char content[8192];
    char result[8192];

    String param = GetPhysicsParams(fallType);
    String path = "out\\";
    if (collision == eMP_TREE || collision == eMP_BUSH)
    {
        path = "out\\tree\\";
        ReadBuffer(content, sizeof(content), "~res:/modelTemplate\\destructibleTree.xml");
        _snprintf(result, sizeof(result), content,
                  (mapName + modelName + ".model").c_str(), health, density, param.c_str());
    }
    else if (collision == eMP_FALLING_ATOM)
    {
        path = "out\\falling\\";
        ReadBuffer(content, sizeof(content), "~res:/modelTemplate\\destructibleFalling.xml");
        _snprintf(result, sizeof(result), content, (mapName + modelName + ".model").c_str(), health, param.c_str());
    }
    else if (collision == eMP_FRAGILE_PR || collision == eMP_FRAGILE_NPR)
    {
        path = "out\\destructible\\";
        ReadBuffer(content, sizeof(content), "~res:/modelTemplate\\destructible.xml");
        _snprintf(result, sizeof(result), content, (mapName + modelName + ".model").c_str(), health,
                  materialKind.c_str());
    }

    File* f = File::Create(path + modelName + ".xml", File::CREATE | File::WRITE);
    f->Write(result, strlen(result));
    SafeRelease(f);
}

void Model::Build(void)
{
    using namespace DAVA;

    //vertices
    DataSection* vert = new DataSection("vertices");
    int32 dataSize = sizeof(VertexXYZNUV) * vertices.size();
    vert->SetBinarySize(sizeof(VertexHeader) + dataSize);
    void* buff = vert->GetBuffer();

    VertexHeader* vh = (VertexHeader*)buff;
    vh->nVertices_ = vertices.size();
    _snprintf(vh->vertexFormat_, sizeof(vh->vertexFormat_), "xyznuv");
    ::memcpy((void*)(vh + 1), &vertices[0], dataSize);
    children_.push_back(vert);

    //indices
    DataSection* prim = new DataSection("indices");
    dataSize = sizeof(IndexValue) * primitive.size();
    prim->SetBinarySize(sizeof(IndexHeader) + dataSize + sizeof(PrimitiveGroup) * groups.size());
    buff = prim->GetBuffer();

    IndexHeader* ih = (IndexHeader*)buff;
    ih->nIndices_ = primitive.size();
    _snprintf(ih->indexFormat_, sizeof(ih->indexFormat_), "list");
    ih->nTriangleGroups_ = groups.size();
    ::memcpy((void*)(ih + 1), &primitive[0], dataSize);

    PrimitiveGroup* group = (PrimitiveGroup*)((uint8*)buff + sizeof(IndexHeader) + dataSize);
    for (uint32 i = 0; i < groups.size(); i++)
    {
        *group = groups[i].group;
        group++;
    }
    children_.push_back(prim);

    //bsp2
    DataSection* bsp2 = new DataSection("bsp2");
    bool result = pBSP->save(bsp2);
    children_.push_back(bsp2);

    //bsp2_materials
    DataSection* bspMat = new DataSection("bsp2_materials");
    String val = "<temp_bsp_materials.xml>";
    for (uint32 i = 0; i < groups.size(); i++)
    {
        val += "<id>" + groups[i].identifier + "</id>";
    }
    val += "</temp_bsp_materials.xml>";
    bspMat->SetBinarySize(val.size() + 1);
    buff = bspMat->GetBuffer();
    _snprintf((char*)buff, val.size() + 1, val.c_str());
    children_.push_back(bspMat);
} // Model::Build

void Model::CreatePolygonGroup()
{
    MaterialPrimitiveGroup group;

    group.twoSided = true;
    group.hasAlpha = true;
    group.matCollision = eMP_NO_COLLISION;

    group.group.startIndex_ = primitive.size();
    group.group.nPrimitives_ = 0;
    group.group.startVertex_ = vertices.size();
    group.group.nVertices_ = 0;
    groups.push_back(group);
}

void Model::AddGeometry(DAVA::Entity* entity, DAVA::RenderObject* renderObject, DAVA::int32 requestedLodIndex, DAVA::int32 requestedSwitchIndex)
{
    using namespace DAVA;

    Matrix4 createdWith = entity->GetWorldTransform() * globalTransform;
    Matrix4 uniformMatrixNormal;
    createdWith.GetInverse(uniformMatrixNormal);
    uniformMatrixNormal.Transpose();
    Matrix3 matrixNormal = uniformMatrixNormal;

    VertexXYZNUV v;
    int32 index;

    MaterialPrimitiveGroup* group = &groups[groups.size() - 1];

    uint32 usedBatchIndex = 0;
    uint32 meshesSize = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < meshesSize; ++k)
    {
        int32 lodIndex = -1, switchIndex = -1;
        RenderBatch* batch = renderObject->GetRenderBatch(k, lodIndex, switchIndex);

        bool isProperLodLayer = ((requestedLodIndex == lodIndex) || (requestedLodIndex == -1)); // -1 means that we
        // don't interested in
        // lod layer
        bool isProperSwitchIndex = ((switchIndex == requestedSwitchIndex) || (requestedSwitchIndex == -1)); // -1 means
        // that we
        // don't
        // interested
        // in switch
        // index
        if (!isProperLodLayer || !isProperSwitchIndex)
        {
            continue; //select only batches for proper lod and switch indexes
        }

        if (usedBatchIndex > 0 && collision == eMP_TREE)
        {
            CreatePolygonGroup();
            group = &groups[groups.size() - 1];
        }
        ++usedBatchIndex;

        PolygonGroup* pg = batch->GetPolygonGroup();
        NMaterial* mat = batch->GetMaterial();

        group->twoSided = false;
        group->hasAlpha = false;

        FilePath albedoPath;
        Texture* effectiveAlbedo = mat->GetEffectiveTexture(NMaterialTextureName::TEXTURE_ALBEDO);
        if (effectiveAlbedo)
        {
            albedoPath = effectiveAlbedo->texDescriptor->pathname;
        }

        TextureDescriptor* descriptor = TextureDescriptor::CreateFromFile(albedoPath);
        if (descriptor)
        {
            FilePath sourceImagePath = descriptor->GetSourceTexturePathname();

            group->identifier = sourceImagePath.GetBasename() + std::to_string((long long)groups.size());
            group->material = FilePath::CreateWithNewExtension(sourceImagePath, ".dds").GetFilename();
#if defined(CREATE_IMAGES_DDS)
            FilePath outImagePathname = "out\\content\\" + mapName + "images\\" + sourceImagePath.GetBasename()
            + ".dds";
            if (sourceImagePath.IsEqualToExtension(".dds"))
            {
                FileSystem::Instance()->CopyFile(sourceImagePath, outImagePathname);
            }
            else
            {
                Vector<Image*> images;
                ImageSystem::Load(sourceImagePath, images);
                ImageSystem::Save(outImagePathname, images);

                for (auto im : images)
                {
                    SafeRelease(im);
                }
            }
#endif
            SafeDelete(descriptor);
        }
        int32 vertexCount = pg->GetVertexCount();
        int32 indexCount = pg->GetPrimitiveCount() * 3;

        for (int32 i = 0; i < vertexCount; i++)
        {
            pg->GetCoord(i, v.pos_);
            pg->GetTexcoord(0, i, v.uv_);

            //transform into world coordinates!
            v.pos_ = v.pos_ * createdWith;

            if (pg->normalArray)
            {
                pg->GetNormal(i, v.normal_);

                //normals we need rotate but not translate!
                v.normal_ = v.normal_ * matrixNormal;

                v.normal_.Normalize();
            }
            else
            {
                v.normal_ = Vector3(0, 0, 1);
            }

            vertices.push_back(v);
            bbox.addBounds(v.pos_);
        }

        for (int32 i = 0; i < indexCount; i++)
        {
            pg->GetIndex(i, index);
            primitive.push_back((IndexValue)(index + indexOffset));
        }

        group->group.nPrimitives_ += indexCount / 3;
        group->group.nVertices_ += vertexCount;

        indexOffset += vertexCount;
    }
} // Model::AddGeometry

void Model::RecursiveAddGeometry(DAVA::Entity* curr, DAVA::int32 switchIndex)
{
    using namespace DAVA;

    RenderObject* renderObject = GetRenderObject(curr);
    if (renderObject && renderObject->GetType() == RenderObject::TYPE_MESH)
    {
        int32 maxLodIndex = FindMaxLodIndex(renderObject, switchIndex);
        AddGeometry(curr, renderObject, maxLodIndex, switchIndex);
    }
    int size = curr->GetChildrenCount();
    for (int i = 0; i < size; i++)
    {
        RecursiveAddGeometry(curr->GetChild(i), switchIndex);
    }
}

void Model::Batch(DAVA::Entity* curr)
{
    using namespace DAVA;

    if (GetAnimationComponent(curr))
    {
        return;
    }
    RenderObject* renderObject = GetRenderObject(curr);
    SwitchComponent* sw = GetSwitchComponent(curr);
    if (sw)
    {
        KeyedArchive* customProperties = GetCustomPropertiesArchieve(curr);
        health = (customProperties) ? customProperties->GetInt32("Health", 5) : 5;

        RecursiveAddGeometry(curr, 0);

        CreatePolygonGroup();
        MaterialPrimitiveGroup* group = &groups[groups.size() - 1];
        group->matCollision = (customProperties) ? (eModelPreset)customProperties->GetInt32("CollisionTypeCrashed",
                                                                                            eMP_NO_COLLISION) :
                                                   eMP_NO_COLLISION;

        RecursiveAddGeometry(curr, 1);
    }
    else if (renderObject && renderObject->GetType() == RenderObject::TYPE_MESH)
    {
        LodComponent* lodComponent = GetLodComponent(curr);

        if (lodComponent)
        {
            int32 maxLodIndex = renderObject->GetMaxLodIndex();
            if (maxLodIndex >= 0)
            {
                AddGeometry(curr, renderObject, maxLodIndex); //switch not requested
            }
        }
        else
        {
            AddGeometry(curr, renderObject); //lod and switch not requested
        }
    }
    if (!sw)
    {
        int size = curr->GetChildrenCount();
        for (int i = 0; i < size; i++)
        {
            Batch(curr->GetChild(i));
        }
    }
}

void Model::createTris(void)
{
    using namespace DAVA;

    Logger::Info("Creating Triangles for bsp...");
    //clockwise -> counterclockwise triangles conversion
    uint32 size = primitive.size();
    IndexValue v0, v2;
    for (uint32 i = 0; i < size; i += 3)
    {
        v0 = primitive[i];
        v2 = primitive[i + 2];
        primitive[i] = v2;
        primitive[i + 2] = v0;
    }

    degenerateCount = 0;

    WorldTriangle::Flags flags;
    for (uint32 j = 0; j < groups.size(); j++)
    {
        MaterialPrimitiveGroup& group = groups[j];

        flags = WorldTriangle::packFlags(GetBWCollision(j), GetBWMaterialKind(j));
        //      WorldTriangle::Flags flags = WorldTriangle::packFlags(
        //          TRIANGLE_DOUBLESIDED | (solid ? TRIANGLE_PROJECTILENOCOLLIDE : TRIANGLE_NOCOLLIDE |
        // TRIANGLE_PROJECTILENOCOLLIDE),
        //          (solid ? MATKIND_SPT_SOLID : MATKIND_SPT_LEAVES ));

        for (int32 i = group.group.startIndex_; i < group.group.startIndex_ + group.group.nPrimitives_ * 3; i += 3)
        {
            VertexXYZNUV& v0 = vertices[primitive[i]];
            VertexXYZNUV& v1 = vertices[primitive[i + 1]];
            VertexXYZNUV& v2 = vertices[primitive[i + 2]];

            WorldTriangle triangle(v0.pos_, v1.pos_, v2.pos_, flags);

            if (triangle.normal().SquareLength() < 0.000001f)
            {
                degenerateCount++;
            }
            else
            {
                tris.push_back(triangle);
            }
        }
    }
    pBSP = new BSPTree(tris);
}

BinaryPtr Model::Recollect(void)
{
    const DAVA::uint32 BIN_SECTION_MAGIC = 0x42a14e65;
    using namespace DAVA;

    uint32 bsz = 4; // for magic

    // collect our children
    std::vector<BinaryPtr> childBlocks;
    for (uint32 i = 0; i < children_.size(); i++)
    {
        childBlocks.push_back(children_[i]->asBinary());
        bsz += (childBlocks.back()->size() + 3) & (~3L);
    }

    // build the directory
    String stream;
    for (uint32 i = 0; i < children_.size(); i++)
    {
        const String& sectName = children_[i]->sectionName();
        int lens[6] = { childBlocks[i]->size(), 0, 0, 0, 0, sectName.length() };
        stream.append((char*)lens, sizeof(lens));
        stream.append(sectName.data(), lens[5]);
        stream.append((4 - lens[5]) & 3, '\0');
    }
    int streamLen = stream.length();
    stream.append((char*)&streamLen, sizeof(int));
    bsz += stream.length();

    // now make the mega-block and return
    BinaryPtr pBinary = new BinaryBlock();
    pBinary->resize(bsz);

    char* dataPtr = (char*)pBinary->data();
    *((uint32*&)dataPtr)++ = BIN_SECTION_MAGIC;
    for (uint32 i = 0; i < children_.size(); i++)
    {
        uint32 len = childBlocks[i]->size();
        memcpy(dataPtr, childBlocks[i]->data(), len);
        dataPtr += (len + 3) & (~3L);
    }
    memcpy(dataPtr, stream.data(), stream.length());

    //free memory
    for (uint32 i = 0; i < children_.size(); i++)
    {
        delete children_[i];
    }
    children_.clear();

    // and that's it
    return pBinary;
}

DAVA::int32 Model::FindMaxLodIndex(DAVA::RenderObject* object, DAVA::int32 switchIndex)
{
    using namespace DAVA;

    if (!object)
    {
        return LodComponent::INVALID_LOD_LAYER;
    }

    int32 maxLodIndex = LodComponent::INVALID_LOD_LAYER;

    uint32 batchCount = object->GetRenderBatchCount();
    for (uint32 i = 0; i < batchCount; ++i)
    {
        int32 lod = -1, sw = -1;
        object->GetRenderBatch(i, lod, sw);

        if ((sw == switchIndex) && (lod > maxLodIndex))
        {
            maxLodIndex = lod;
        }
    }

    return maxLodIndex;
}

DAVA::uint32 Model::GetSwitchCount(DAVA::RenderObject* object)
{
    using namespace DAVA;

    if (!object)
    {
        return 0;
    }

    int32 maxSwitchIndex = -1;

    uint32 batchCount = object->GetRenderBatchCount();
    for (uint32 i = 0; i < batchCount; ++i)
    {
        int32 lod = -1, sw = -1;
        object->GetRenderBatch(i, lod, sw);

        if (sw > maxSwitchIndex)
        {
            maxSwitchIndex = sw;
        }
    }

    return (maxSwitchIndex + 1);
}
