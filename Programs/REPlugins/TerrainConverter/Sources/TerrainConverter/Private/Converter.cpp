#include "Private/Converter.h"

#include "Converter/ArenaDefCompilator.h"
#include "ModelTemplatePath.h"
#include "DataOutputPath.h"
#include "FilePathHelper.h"
#include "Utils.h"
#include "terrain.h"

#include <REPlatform/Deprecated/EditorConfig.h>

#include <Render/PixelFormatDescriptor.h>

#include <libpng/png.h>
#include <libpng/zconf.h>
#include <string.h>

using namespace DAVA;

Converter::Converter(const DAVA::FilePath& templatesPath_, const DAVA::FilePath& clientDataSource_)
    : templatesPath(templatesPath_)
    , clientDataSource(clientDataSource_)
{
}

void Converter::Do(const DAVA::FilePath& filePath, const DAVA::Vector<DAVA::String>& tags)
{
    FilePath::AddResourcesFolder(templatesPath);
    FilePath::AddResourcesFolder(clientDataSource);

    sceneNodesLinks = new KeyedArchive();
    arenaDefCompilator = ArenaDefCompliator::Create(ModelTemplate::GetArenaDef(), DataOutputPath::GetArenaDef());

    nextid = 0;
    Logger::Info("Terrain Converter started: %s", filePath.GetAbsolutePathname().c_str());

    inputFile = filePath;
    String fullPath = inputFile.GetAbsolutePathname();
    uint32 noff = fullPath.rfind("Data");
    if (noff < fullPath.size())
    {
        projectPath = fullPath.substr(0, noff);
    }

    FilePath lkaPath = FilePathHelper::GetLKAPath(inputFile, tags);
    DAVA::FileSystem* fs = GetEngineContext()->fileSystem;
    fs->CreateDirectory(lkaPath.GetDirectory());

    mapName = inputFile.GetBasename() + "/";

    arenaDefCompilator->Start(inputFile.GetBasename());

    config = new DAVA::EditorConfig();
    config->ParseConfig(projectPath.GetAbsolutePathname() + "EditorConfig.yaml");

    scene = new Scene();
    SceneFileV2::eError mapLoadError = scene->LoadScene(inputFile);
    if (mapLoadError == SceneFileV2::ERROR_NO_ERROR)
    {
        //update for transform matrix calculation
        scene->transformSystem->Process(0.1f);
        DoConvertHM();
        GenerateMaterialKindMap();
        CreateBorderFile();
        DoConvertModels(tags);
        SetupVersionLKA();

        File* f = File::Create(lkaPath, File::CREATE | File::WRITE);
        sceneNodesLinks->Save(f);
        SafeRelease(f);

        Logger::Info("Terrain Converter finished converting models from %s", inputFile.GetAbsolutePathname().c_str());
    }
    else
    {
        Logger::Error("Failed to load input file %s", inputFile.GetAbsolutePathname().c_str());
    }
    mapName[mapName.length() - 1] = '\\';

    Logger::Info("Terrain Converter finished converting %s", mapName.c_str());

    arenaDefCompilator->End();

    SafeDelete(config);
    SafeRelease(sceneNodesLinks);
    arenaDefCompilator.reset();
    FilePath::RemoveResourcesFolder(clientDataSource);
    FilePath::RemoveResourcesFolder(templatesPath);
}

#define CHUNKBUFFER_SIZE 1048576
void Converter::DoConvertModels(const DAVA::Vector<DAVA::String>& tags)
{
    //prepare buffers
    using namespace DAVA;

    File* f = File::Create(FilePath("~res:/modelTemplate\\model.xml"), File::OPEN | File::READ);
    uint32 res = f->Read(modelContentFormatPattern, sizeof(modelContentFormatPattern));
    modelContentFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\speedTree.xml"), File::OPEN | File::READ);
    res = f->Read(speedTreeFormatPattern, sizeof(speedTreeFormatPattern));
    speedTreeFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\spawnpoint.xml"), File::OPEN | File::READ);
    res = f->Read(spawnPointFormatPattern, sizeof(spawnPointFormatPattern));
    spawnPointFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\botSpawn.xml"), File::OPEN | File::READ);
    res = f->Read(botSpawnFormatPattern, sizeof(botSpawnFormatPattern));
    botSpawnFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\patrolNode.xml"), File::OPEN | File::READ);
    res = f->Read(patrolFormatPattern, sizeof(patrolFormatPattern));
    patrolFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\controlpoint.xml"), File::OPEN | File::READ);
    res = f->Read(controlPointFormatPattern, sizeof(controlPointFormatPattern));
    controlPointFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\water.xml"), File::OPEN | File::READ);
    res = f->Read(waterFormatPattern, sizeof(waterFormatPattern));
    waterFormatPattern[res] = 0;
    SafeRelease(f);

    f = File::Create(FilePath("~res:/modelTemplate\\water.vlo"), File::OPEN | File::READ);
    res = f->Read(waterVloFormatPattern, sizeof(waterVloFormatPattern));
    waterVloFormatPattern[res] = 0;
    SafeRelease(f);

    strategyPointPattern = Utils::GetContentOfFile(FilePath("~res:/modelTemplate\\chunk\\StrategicPoint.xml"));

    chunkBuffer = new char[CHUNKBUFFER_SIZE];
    CheckNodes(scene, tags);
    CheckWater(scene, tags);
    CheckUserNodes(scene, tags);
    CheckPath(scene, tags);
    delete chunkBuffer;
}

bool Converter::GetCollisionType(DAVA::Entity* curr, eModelPreset& result)
{
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(curr);
    if (customProperties && customProperties->IsKeyExists("CollisionType"))
    {
        result = (eModelPreset)customProperties->GetInt32("CollisionType", eMP_NO_COLLISION);
        return true;
    }
    else
    {
        int size = curr->GetChildrenCount();
        for (int i = 0; i < size; i++)
        {
            if (GetCollisionType(curr->GetChild(i), result))
            {
                return true;
            }
        }
        return false;
    }
}

void Converter::CheckPath(Entity* curr, const Vector<String>& tags)
{
    if (!CheckTags(curr, tags))
    {
        return;
    }
    bool processed = false;
    PathComponent* path = GetPathComponent(curr);
    if (path)
    {
        if (path->GetPoints().size() > 1)
        {
            KeyedArchive* customProperties = GetCustomPropertiesArchieve(curr);
            int32 team = customProperties->GetInt32("team", 0);

            const String& botname = customProperties->GetString("botname", "");

            const String& vehicle = customProperties->GetString("vehicle", "none");
            const String& script = customProperties->GetString("script", "");
            int32 group = customProperties->GetInt32("group", 0);
            bool isPerformanceTestBot = customProperties->GetBool("performanceTestBot");
            UniqueID guid = GetEntityID(curr);
            _snprintf(resultBuff, sizeof(resultBuff), botSpawnFormatPattern,
                      guid.toString().c_str(),
                      team,
                      vehicle.c_str(),
                      (botname.empty() ? botname.c_str() : Format("\n\t\t\t<botName>\t%s\t</botName>",
                                                                  botname.c_str())
                                                           .c_str()),
                      nextid,
                      (isPerformanceTestBot ? "1" : "0"),
                      script.c_str(),
                      group);

            PathComponent::Waypoint* startWP = path->GetPoints()[0];
            PathComponent::Waypoint* nextWP = NULL;
            Matrix4 pos;
            Vector3 nextPos;
            if (startWP->edges.size() > 0)
            {
                nextWP = startWP->edges[0]->destination;
                Quaternion q = Quaternion::MakeRotation(Vector3(0.0f, 1.0f,
                                                                0.0f),
                                                        (nextWP->position - startWP->position));
                pos = q.GetMatrix();
            }
            pos.SetTranslationVector(startWP->position);
            AddEntityToChunk(resultBuff, pos, 0);
            if (nextWP)
            {
                AddWayPointToChunk(nextWP, 0, curr);
                nextid++;
            }
        }
        processed = true;
    }
    if (!processed)
    {
        int size = curr->GetChildrenCount();
        for (int i = 0; i < size; i++)
        {
            CheckPath(curr->GetChild(i), tags);
        }
    }
} // Converter::CheckPath

void Converter::AddWayPointToChunk(PathComponent::Waypoint* wp, uint32 pointNumber, DAVA::Entity* parentEntity)
{
    UniqueID guid = GetEntityID(parentEntity, pointNumber);
    String directive = wp->GetProperties()->GetString("directive", "");
    bool needNewPath = false;
    String::size_type switchPos = directive.find("switch(");
    if (switchPos != std::string::npos)
    {
        String s = Format(", %d", nextid + 1);
        directive = directive.insert(directive.find(")", switchPos), s);
        needNewPath = true;
    }
    _snprintf(resultBuff, sizeof(resultBuff), patrolFormatPattern,
              guid.toString().c_str(),
              nextid,
              pointNumber,
              directive.c_str());
    Matrix4 wt;
    wt.BuildTranslation(wp->position);
    AddEntityToChunk(resultBuff, wt, 0);

    if (wp->edges.size() == 1)
    {
        if (needNewPath)
        {
            nextid++;
            AddWayPointToChunk(wp->edges[0]->destination, 0, parentEntity);
        }
        else
        {
            AddWayPointToChunk(wp->edges[0]->destination, pointNumber + 1, parentEntity);
        }
    }
    else if (wp->edges.size() > 1)
    {
        for (size_t i = 0; i < wp->edges.size(); i++)
        {
            //todo here we need create correct branching
            nextid++;
            AddWayPointToChunk(wp->edges[i]->destination, 0, parentEntity);
        }
    }
}

void Converter::AddEntityToChunk(char* textBuf, const Matrix4& wt, const uint32 nodeId)
{
    Matrix4 t = Model::globalTransformInverse * wt * Model::globalTransform;

    Vector3 offset = Vector3(0, 0, 0) * t;
    int32 chunkX = (int32)floor(offset.x / 100.0f);
    int32 chunkZ = (int32)floor(offset.z / 100.0f);
    String identifier = outsideChunkIdentifier(chunkX, chunkZ);

    t = t * Matrix4::MakeTranslation(Vector3(-chunkX * 100.0f, 0, -chunkZ * 100.0f));

    char* beginTr = strstr(textBuf, "<transform>") + 12;

    _snprintf(transformBuf, sizeof(transformBuf),
              "\t\t\t<row0> %f %f %f </row0>\r\n\t\t\t<row1> %f %f %f </row1>\r\n\t\t\t<row2> %f %f %f </row2>\r\n\t\t\t<row3> %f %f %f </row3>",
              t._00, t._01, t._02,
              t._10, t._11, t._12,
              t._20, t._21, t._22,
              t._30, t._31, t._32);

    File* f = File::Create("out\\spaces\\" + identifier + ".chunk", File::OPEN | File::READ);
    if (f)
    {
        uint32 res = f->Read(chunkBuffer, CHUNKBUFFER_SIZE);
        chunkBuffer[res] = 0;
        SafeRelease(f);

        char* str = strstr(chunkBuffer, "</root>");
        if (str != NULL)
        {
            //str += 6;
            f = File::Create("out\\spaces\\" + identifier + ".chunk", File::CREATE | File::WRITE);
            f->Write(chunkBuffer, (uint32)(str - chunkBuffer));
            f->Write(textBuf, (uint32)(beginTr - textBuf));
            f->Write(transformBuf, strlen(transformBuf));
            f->Write(beginTr, strlen(beginTr));
            f->Write(str, strlen(str));
            SafeRelease(f);
        }
    }
    resultBuff[0] = 0;
    transformBuf[0] = 0;

    if (nodeId != 0)
    {
        //store and increment counter for models in chunk
        //bigger 16bit is chunkID
        uint32 value = (chunkIDFromChunkIndexes(chunkX, chunkZ) & 0xFFFF); // << 16;
        value = value << 16;
        //lower 16biit is model index
        value += chunkIndices[identifier];
        //store value
        sceneNodesLinks->SetUInt32(Format("%u", nodeId), value);

        //here is increment
        chunkIndices[identifier] = chunkIndices[identifier] + 1;
        if (chunkIndices[identifier] > 255)
        {
            Logger::Error("Error, objects counter on chunk %s is %d (bigger than 255!)",
                          identifier.c_str(), chunkIndices[identifier]);
        }
    }
} // Converter::AddEntityToChunk

void Converter::CheckWater(Entity* curr, const Vector<String>& tags)
{
    if (!CheckTags(curr, tags))
    {
        return;
    }
    bool processed = false;
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(curr);
    if (customProperties)
    {
        if (customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            eModelPreset collision = eMP_NO_COLLISION;
            GetCollisionType(curr, collision);
            if (collision == eMP_WATER)
            {
                UniqueID guid = GetEntityID(curr);
                String guidStr = guid.toString();
                guidStr = customProperties->GetString("guid", guidStr);
                AddWater(curr, guidStr);
            }
            processed = true;
        }
    }
    if (!processed)
    {
        int size = curr->GetChildrenCount();
        for (int i = 0; i < size; i++)
        {
            CheckWater(curr->GetChild(i), tags);
        }
    }
}

void Converter::CheckUserNodes(Entity* curr, const Vector<String>& tags)
{
    if (!CheckTags(curr, tags))
    {
        return;
    }
    bool processed = false;
    UserComponent* userComp = (UserComponent*)curr->GetComponent(Component::USER_COMPONENT);
    if (userComp)
    {
        AddPointToChunk(curr);
        processed = true;
    }
    if (!processed)
    {
        int32 size = curr->GetChildrenCount();
        for (int32 i = 0; i < size; i++)
        {
            CheckUserNodes(curr->GetChild(i), tags);
        }
    }
}

bool Converter::CheckTags(Entity* curr, const DAVA::Vector<DAVA::String>& tags)
{
    bool okTags = true; // all nodes should be converted by default
    if (curr && tags.size() > 0)
    {
        KeyedArchive* customProperties = GetCustomPropertiesArchieve(curr);
        if (customProperties)
        {
            String currTags = customProperties->GetString("tags");
            if (currTags.size() > 0)
            {
                // found non-empty tags in node (if we won't find any of tags - node must not be converted)
                okTags = false;
                for (const auto& tag : tags)
                {
                    if (currTags.find(tag.c_str()) != String::npos)
                    {
                        // found tag in node tags - node should be converted
                        Logger::Info("requested tags found in %s (%s)", curr->GetName().c_str(), currTags.c_str());
                        okTags = true;
                        break;
                    }
                }
                if (!okTags)
                {
                    Logger::Info("requested tags not found in %s (%s) - skipped", curr->GetName().c_str(), currTags.c_str());
                }
            }
        }
    }
    return okTags;
}

void Converter::CheckNodes(Entity* curr, const Vector<String>& tags)
{
    if (!CheckTags(curr, tags))
    {
        return;
    }
    bool processed = false;
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(curr);
    if (customProperties)
    {
        if (customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            const String& val = customProperties->GetString("editor.referenceToOwner", "");
            eModelPreset collision = eMP_NO_COLLISION;
            GetCollisionType(curr, collision);

            if (collision == eMP_SPEED_TREE)
            {
                if (savedTrees.find(val) == savedTrees.end())
                {
                    savedTrees[val] = new ServerSPT(val, mapName, curr);
                }
                if (!savedTrees[val]->IsWrong())
                {
                    AddTreeToChunk(curr, savedTrees[val]);
                }
            }
            else if (collision != eMP_NO_COLLISION && collision != eMP_WATER)
            {
                if (savedModels.find(val) == savedModels.end())
                {
                    const AABBox3& wtBox = curr->GetWTMaximumBoundingBoxSlow();
                    float32 dx = wtBox.GetSize().x;
                    float32 dy = wtBox.GetSize().y;
                    float32 dz = wtBox.GetSize().z;
                    if (dx > 100.0f || dy > 100.0f || dz > 100.0f)
                    {
                        if (dx > 200.0f || dy > 200.0f)
                        {
                            // error only in case of really big objects
                            Logger::Error("bbox is too big %s %s  min=(%f, %f, %f) max=(%f, %f, %f)",
                                          mapName.c_str(),
                                          val.c_str(), wtBox.min.x, wtBox.min.y, wtBox.min.z, wtBox.max.x, wtBox.max.y,
                                          wtBox.max.z);
                        }
                        else
                        {
                            Logger::Warning("bbox is too big %s %s min=(%f, %f, %f) max=(%f, %f, %f)",
                                            mapName.c_str(),
                                            val.c_str(), wtBox.min.x, wtBox.min.y, wtBox.min.z, wtBox.max.x, wtBox.max.y,
                                            wtBox.max.z);
                        }
                    }
                    savedModels[val] = new Model(val, mapName, collision, curr, config);
                }
                if (!savedModels[val]->IsWrong())
                {
                    AddModelToChunk(curr, savedModels[val]);
                }
            }
            processed = true;
        }
    }
    if (!processed)
    {
        int size = curr->GetChildrenCount();
        for (int i = 0; i < size; i++)
        {
            CheckNodes(curr->GetChild(i), tags);
        }
    }
} // Converter::CheckNodes

void Converter::AddModelToChunk(Entity* node, Model* model)
{
    Logger::Info("AddModelToChunk model %s", model->GetModelName().c_str());
    _snprintf(resultBuff, sizeof(resultBuff), modelContentFormatPattern, "",
              (mapName + model->GetModelName()).c_str(), "<destructibleState> undamaged </destructibleState>");
    AddEntityToChunk(resultBuff, node->GetWorldTransform(), node->GetID());
    resultBuff[0] = 0;
}

const String SPAWN_PREFERRED_VEHICLE_TYPES[eSPVT_COUNT] =
{ "&lt;any&gt;", "lightTank", "mediumTank", "heavyTank", "AT-SPG" };

void Converter::AddTreeToChunk(Entity* node, ServerSPT* tree)
{
    Logger::Info("AddModelToChunk model %s", tree->GetModelName().c_str());
    String fn = mapName + "speedTree/" + tree->GetModelName();
    _snprintf(resultBuff, sizeof(resultBuff), speedTreeFormatPattern, fn.c_str());
    AddEntityToChunk(resultBuff, node->GetWorldTransform(), node->GetID());
    resultBuff[0] = 0;
}

void Converter::AddPointToChunk(Entity* node)
{
    Logger::Info("AddPointToChunk %s", node->GetName().c_str());
    resultBuff[0] = 0;
    String val = "";
    int32 team = 0;
    int32 preferredVehicleType = eSPVT_ANY;
    float32 pointsPerSecond = 1.0f;
    float32 maxPointsPerSecond = 3.0f;
    float32 radius = 25.0f;
    int32 group = 0;

    String vehicle = "none";
    String botname = "";
    String script = "";
    bool isPerformanceTestBot = false;
    int32 pathIDbotspawn = 0;

    int32 pathIDpatrol = 1;
    int32 pointNumber = 1;
    String directive = "";

    KeyedArchive* customProperties = GetCustomPropertiesArchieve(node);
    if (customProperties)
    {
        val = customProperties->GetString("type", "");
        team = customProperties->GetInt32("team", 0);
        preferredVehicleType = customProperties->GetInt32("SpawnPreferredVehicleType", eSPVT_ANY);

        pointsPerSecond = customProperties->GetFloat("pointsPerSecond", 1.0f);
        maxPointsPerSecond = customProperties->GetFloat("maxPointsPerSecond", 3.0f);
        radius = customProperties->GetFloat("radius", 25.0f);

        botname = customProperties->GetString("botname", "");
        vehicle = customProperties->GetString("vehicle", "none");
        isPerformanceTestBot = customProperties->GetBool("performanceTestBot");
        pathIDbotspawn = customProperties->GetInt32("pathID", 0);
        script = customProperties->GetString("script", "");
        group = customProperties->GetInt32("group", 0);

        pathIDpatrol = customProperties->GetInt32("pathID", 1);
        pointNumber = customProperties->GetInt32("pointNumber", 1);

        directive = customProperties->GetString("directive", "");
    }

    UniqueID guid = GetEntityID(node);
    if (val.find("spawnpoint") != std::string::npos)
    {
        String preferredVehicleTypeStr;

        if (0 <= preferredVehicleType && preferredVehicleType < eSPVT_COUNT)
        {
            preferredVehicleTypeStr = SPAWN_PREFERRED_VEHICLE_TYPES[preferredVehicleType];
        }
        else
        {
            Logger::Warning("Converter::AddPointToChunk unknown preferred vehicle type %d for spawnpoint",
                            preferredVehicleType);
            preferredVehicleTypeStr = SPAWN_PREFERRED_VEHICLE_TYPES[eSPVT_ANY];
        }
        _snprintf(resultBuff, sizeof(resultBuff), spawnPointFormatPattern,
                  guid.toString().c_str(), team, preferredVehicleTypeStr.c_str(), group, pointNumber);
    }
    else if (val.find("controlpoint") != std::string::npos)
    {
        _snprintf(resultBuff, sizeof(resultBuff), controlPointFormatPattern,
                  radius,
                  pointsPerSecond,
                  maxPointsPerSecond);
    }
    else if (val.find("botspawn") != std::string::npos)
    {
        _snprintf(resultBuff, sizeof(resultBuff), botSpawnFormatPattern,
                  guid.toString().c_str(),
                  team,
                  vehicle.c_str(),
                  (botname.empty() ? botname.c_str() : Format("\n\t\t\t<botName>\t%s\t</botName>",
                                                              botname.c_str())
                                                       .c_str()),
                  pathIDbotspawn,
                  (isPerformanceTestBot ? "1" : "0"),
                  script.c_str(),
                  group);
    }
    else if (val.find("patrol") != std::string::npos)
    {
        _snprintf(resultBuff, sizeof(resultBuff), patrolFormatPattern,
                  guid.toString().c_str(),
                  pathIDpatrol,
                  pointNumber,
                  directive.c_str());
    }
    else if (val.find("strategicpoint") != std::string::npos)
    {
        StrategyPointData strategyPointData = StrategyPointData::Create(customProperties);

        String currentStrategyPoint = strategyPointPattern;
        strategyPointData.FillPattern(currentStrategyPoint);

        _snprintf(resultBuff, sizeof(resultBuff), currentStrategyPoint.c_str());

        arenaDefCompilator->Visit(strategyPointData);
    }

    if (resultBuff[0] != 0)
    {
        AddEntityToChunk(resultBuff, node->GetWorldTransform(), 0);
    }
    resultBuff[0] = 0;
} // Converter::AddPointToChunk

void Converter::CreateSettingsFile(int32 minx, int32 miny, int32 maxx, int32 maxy)
{
    char content[8192];
    char result[8192];

    File* f = File::Create(FilePath("~res:/modelTemplate\\space.settings"), File::OPEN | File::READ);
    uint32 res = f->Read(content, sizeof(content));
    content[res] = 0;
    SafeRelease(f);

    _snprintf(result, sizeof(result), content, -6, 5, -6, 5);
    f = File::Create("out\\spaces\\space.settings", File::CREATE | File::WRITE);
    f->Write(result, strlen(result));
    SafeRelease(f);
}

void Converter::CreateBorderFile(void)
{
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(land);
    float32 topX(-250.0f), topY(-250.0f), botX(250.0f), botY(250.0f);

    if (customProperties)
    {
        topX = customProperties->GetFloat("borderTopX", topX);
        topY = customProperties->GetFloat("borderTopY", topY);
        botX = customProperties->GetFloat("borderBotX", botX);
        botY = customProperties->GetFloat("borderBotY", botY);
    }

    char content[8192];
    char result[8192];

    File* f = File::Create(FilePath("~res:/modelTemplate\\boundingBox.xml"), File::OPEN | File::READ);
    uint32 res = f->Read(content, sizeof(content));
    content[res] = 0;
    SafeRelease(f);

    //	_snprintf( result, sizeof(result), content, minx, maxx, miny, maxy);
    _snprintf(result, sizeof(result), content, topX, topY, botX, botY);
    f = File::Create("out\\boundingBox.xml", File::CREATE | File::WRITE);
    f->Write(result, strlen(result));
    SafeRelease(f);

    AABBox2 bbox(Vector2(topX, topY), Vector2(botX, botY));
    arenaDefCompilator->Visit(BoundingBoxData(bbox));
}

bool Converter::PrepareIndexTexture(void)
{
    Vector<Color> customColors = config->GetColorPropertyValues("LandscapeCustomColors");
    defaultColorIndex = 1;
    uint8* data = 0;
    texDataWidth = 10;
    texDataHeight = 10;

    KeyedArchive* customProperties = GetCustomPropertiesArchieve(land);
    if (customProperties && customProperties->IsKeyExists("customColorTexture"))
    {
        FilePath fullPath = projectPath + customProperties->GetString("customColorTexture", "");
        Vector<Image*> texs;
        ImageSystem::Load(fullPath, texs);
        if (texs.size() > 0)
        {
            Image* tex = texs[0];
            if (tex)
            {
                //need fix after framework patch
                tex->FlipVertical();

                data = tex->GetData();
                texDataWidth = tex->GetWidth();
                texDataHeight = tex->GetHeight();
                if (texDataWidth != TILEMASK_SZ)
                {
                    Logger::Error("Custom texture size do not match %d (%d)", TILEMASK_SZ, texDataWidth);
                    return false;
                }
            }
        }
        else
        {
            Logger::Error("Cant find custom texture (for level passability) %s",
                          fullPath.GetAbsolutePathname().c_str());
        }
    }

    int32 size = texDataWidth * texDataHeight;
    texData = new uint8[size];
    memset(texData, defaultColorIndex, size);
    if (data)
    {
        int32 offs = 0;
        for (int i = 0; i < size * 4; i += 4)
        {
            Color c((float32)data[i] / 255.f,
                    (float32)data[i + 1] / 255.f,
                    (float32)data[i + 2] / 255.f,
                    (float32)data[i + 3] / 255.f);

            for (uint8 j = 0; j < (uint8)customColors.size(); ++j)
            {
                if (c == customColors[j])
                {
                    texData[offs] = j;
                    break;
                }
            }
            offs++;
        }
    }

    static const String materialsMap[8] =
    {
      "snow", "maps/landscape/00_AllTiles/Snow",
      "ground", "maps/landscape/00_AllTiles/Ground_N_1",
      "sand", "maps/landscape/00_AllTiles/sand_grass",
      "stone", "maps/landscape/00_AllTiles/Road"
    };

    KeyedArchive* materialOptions = new KeyedArchive();
    if (materialOptions->LoadFromYamlFile(FilePathHelper::MapEffectsPath(inputFile)))
    {
        const KeyedArchive::UnderlyingMap mapData = materialOptions->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = mapData.begin(); it != mapData.end(); ++it)
        {
            const String& typeName = it->second->AsString();
            for (int i = 0; i < 4; i++)
            {
                if (materialsMap[i << 1] == typeName)
                {
                    textureNames.push_back(materialsMap[(i << 1) + 1]);
                    break;
                }
            }
        }
    }
    else
    {
        textureNames.push_back("maps/landscape/00_AllTiles/Ground_N_1");
        textureNames.push_back("maps/landscape/00_AllTiles/Ground_N_2");
        textureNames.push_back("maps/landscape/00_AllTiles/Ground_N_3");
        textureNames.push_back("maps/landscape/00_AllTiles/Ground_N_4");
    }
    SafeRelease(materialOptions);

    dominantSize = DOM_RES * DOM_RES;
    dominant = new uint8[dominantSize];

    blends[0] = new uint8[dominantSize];
    blends[1] = new uint8[dominantSize];
    blends[2] = new uint8[dominantSize];
    blends[3] = new uint8[dominantSize];
    return true;
} // Converter::PrepareIndexTexture

uint8 GetDominantIndex(uint8* image, UINT offset)
{
    uint8 maxVal = 0;
    int32 maxInd = 0;
    for (uint8 i = 0; i < BLENDS_SZ; i++)
    {
        uint8 v = image[offset + i];
        if (maxVal < v)
        {
            maxVal = v;
            maxInd = i;
        }
    }
    return maxInd;
}

uint8 downValues[4];

uint8 Downscale4Pixels(uint8* image, UINT strip, UINT offset)
{
    memset(downValues, 0, 4);

    downValues[GetDominantIndex(image, offset)]++;
    downValues[GetDominantIndex(image, offset + 4)]++;
    downValues[GetDominantIndex(image, offset + strip)]++;
    downValues[GetDominantIndex(image, offset + strip + 4)]++;

    uint8 maxVal = 0;
    int32 maxInd = 0;
    for (uint8 i = 0; i < 4; i++)
    {
        if (maxVal < downValues[i])
        {
            maxVal = downValues[i];
            maxInd = i;
        }
    }
    return maxInd;
}

struct DominantBlitzHeader
{
    static const uint32 VERSION_ZIP = 1;
    uint32 magic_;
    uint32 version_;
    uint32 width_;
    uint32 dataSize_;
    uint32 pad_[2];
    static const uint32 MAGIC = 0x006d6b6b; //"mkm"

    DominantBlitzHeader()
    {
        magic_ = MAGIC;
        version_ = VERSION_ZIP;
        width_ = 0;
        dataSize_ = 0;
        pad_[0] = 0;
        pad_[1] = 0;
    }

    bool check()
    {
        return (magic_ == MAGIC && version_ == VERSION_ZIP);
    }

    void Save(File* f)
    {
        f->Write(&magic_, 4);
        f->Write(&version_, 4);
        f->Write(&width_, 4);
        f->Write(&dataSize_, 4);
        f->Write(&pad_, 8);
    }

    void Load(File* f)
    {
        f->Read(&magic_, 4);
        f->Read(&version_, 4);
        f->Read(&width_, 4);
        f->Read(&dataSize_, 4);
        f->Read(&pad_, 8);
    }
};

void Converter::GenerateMaterialKindMap(void)
{
    if (tileMask)
    {
        uint8* tileMaskData = tileMask->GetData();
        UINT width = tileMask->GetWidth();
        UINT bufferSz = ((width >> 1) * (width >> 1)) >> 2;
        uint8* matKind = new uint8[bufferSz];
        memset(matKind, 0, bufferSz);
        uint8 val;
        UINT offset = 0;
        UINT matOff = 0;
        for (UINT y = 0; y < width; y += 2)
        {
            for (UINT x = 0; x < width; x += 8)
            {
                val = 0;
                for (int i = 0; i < 4; i++)
                {
                    val = val + ((Downscale4Pixels(tileMaskData, width * 4, offset + i * 8)) << (i * 2));
                }
                matKind[matOff++] = val;
                offset += 32;
            }
            offset += width * 4;
        }

        File* f = File::Create(FilePathHelper::GetMKMPath(inputFile), File::CREATE | File::WRITE);
        if (f)
        {
            DominantBlitzHeader header;
            header.width_ = (width >> 1);
            header.dataSize_ = bufferSz;
            header.Save(f);
            f->Write(matKind, bufferSz);
            SafeRelease(f);
        }
    }
}

void Converter::DoConvertHM()
{
    heightMap = NULL;
    landscapeObject = NULL;
    tileMask = NULL;
    FileSystem* fs = GetEngineContext()->fileSystem;
    fs->DeleteDirectory("out\\");
    fs->CreateDirectory("out\\");
    fs->CreateDirectory("out\\content\\");
    fs->CreateDirectory("out\\content\\" + mapName);
#if defined(CREATE_IMAGES_DDS)
    fs->CreateDirectory("out\\content\\" + mapName + "images\\");
#endif
    fs->CreateDirectory("out\\content\\" + mapName + "speedTree\\");
    fs->CreateDirectory("out\\destructible\\");
    fs->CreateDirectory("out\\tree\\");
    fs->CreateDirectory("out\\falling\\");
    fs->CreateDirectory("out\\spaces\\");
    Terrain* terrain = NULL;
    land = scene->FindByName("Landscape");
    if (land)
    {
        landscapeObject =
        (Landscape*)((RenderComponent*)land->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject();
        heightMap = landscapeObject->GetHeightmap();

        terrain = new Terrain(landscapeObject);

        NMaterial* landscapeMaterial = landscapeObject->GetMaterial();
        DVASSERT(landscapeMaterial != nullptr);

        Texture* tx = landscapeMaterial->GetEffectiveTexture(FastName("tileMask"));
        TextureDescriptor* desc = TextureDescriptor::CreateFromFile(tx->GetPathname());

        Vector<Image*> texs;
        ImageSystem::Load(desc->GetSourceTexturePathname(), texs);
        if (texs.size() > 0)
        {
            tileMask = texs[0];

            //need fix after framework patch
            tileMask->FlipVertical();

            if (tileMask->GetWidth() != TILEMASK_SZ)
            {
                Logger::Error("TileMask texture size do not match %d (%d)", TILEMASK_SZ, tileMask->GetWidth());
                tileMask = 0;
            }
        }
    }
    if (heightMap && tileMask && terrain)
    {
        uint8* tileMaskData = tileMask->GetData();
        if (PrepareIndexTexture())
        {
            AABBox3 box = landscapeObject->GetBoundingBox();
            float realSize = (box.max.x - box.min.x) * 0.5f;

            int32 sizeInChunks = (int32)ceil(realSize * 0.01f);
            float32 newRealSize = sizeInChunks * 100.0f;

            CreateSettingsFile(-sizeInChunks, -sizeInChunks, sizeInChunks - 1, sizeInChunks - 1);

            int16 chunkSize = 64;
            char content[1024];

            int16 nW = chunkSize + 5;
            int16 nH = chunkSize + 5;
            int16 offset;
            float32 min;
            float32 max;
            image32.resize(nW * nH);

            Vector3 point;
            Vector3 resultV;
            float32 ww = (float32)(sizeInChunks * chunkSize);
            float kf = 100.0f / (float32)chunkSize;
            int32 level;
#ifdef __DAVAENGINE_DEBUG__
            const PixelFormatDescriptor& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(FORMAT_A16);
            int32 bytesInPixel = 2;
            Image* spaceImage = NULL;
            spaceImage = Image::Create(chunkSize * 12 + 5, chunkSize * 12 + 5, formatDescriptor.formatID);
            uint8* spaceImageData = spaceImage->GetData();
#endif

            for (int16 yc = -6; yc < 6; yc++)
            {
                for (int16 xc = -6; xc < 6; xc++)
                {
                    min = (float32)INT_MAX;
                    max = (float32)-INT_MAX;
                    offset = 0;

#ifdef __DAVAENGINE_DEBUG__
                    Image* chunkImage = NULL;
                    chunkImage = Image::Create(nW, nH, formatDescriptor.formatID);
                    uint8* chunkImageData = chunkImage->GetData();
#endif
                    //heights
                    for (int16 y = chunkSize * yc - 2; y < chunkSize * (yc + 1) + 3; ++y)
                    {
                        for (int16 x = chunkSize * xc - 2; x < chunkSize * (xc + 1) + 3; ++x)
                        {
                            point.x = (float32)x * kf;
                            point.y = (float32)y * kf;
                            point.z = 0.0f;
                            resultV.z = 0.0f;

                            //landscapeObject->PlacePoint(point, resultV);
                            terrain->PlaceLandscapePoint(point, resultV, NULL);

                            level = Quantise(resultV.z);

#ifdef __DAVAENGINE_DEBUG__
                            int32 pngIndex = offset * bytesInPixel;
                            int32 pngGlobalIndex = (y + 2 + chunkSize * 6) * (chunkSize * 12 + 5) * bytesInPixel
                            + (x + 2 + chunkSize * 6) * bytesInPixel;

                            for (int32 c = 0; c < bytesInPixel; ++c)
                            {
                                int32 shift = 8 * (bytesInPixel - 1 - c);
                                chunkImageData[pngIndex + c] = (level >> shift) & 0xFF;
                                spaceImageData[pngGlobalIndex + c] = (level >> shift) & 0xFF;
                            }
#endif

                            image32[offset++] = level;

                            if (resultV.z > max)
                            {
                                max = resultV.z;
                            }
                            if (resultV.z < min)
                            {
                                min = resultV.z;
                            }
                        }
                    }

                    PNGImageData pngData;
                    pngData.data_ = (uint8*)&(image32[0]);
                    pngData.width_ = nW;
                    pngData.height_ = nH;
                    pngData.bpp_ = 32;
                    pngData.stride_ = nW * sizeof(int32);
                    pngData.upsideDown_ = false;
                    BinaryPtr imgData = compressPNG(pngData);

                    // Create header at front of buffer
                    HeightMapHeader hmh;
                    ::ZeroMemory(&hmh, sizeof(hmh));
                    hmh.magic_ = HeightMapHeader::MAGIC;
                    hmh.compression_ = 0;
                    hmh.width_ = nW;
                    hmh.height_ = nH;
                    hmh.minHeight_ = min;
                    hmh.maxHeight_ = max;
                    hmh.version_ = HeightMapHeader::VERSION_ABS_QFLOAT;

                    // Now add the header to the front:
                    std::vector<uint8> result(imgData->size() + sizeof(hmh) + sizeof(uint32));
                    ::memcpy(&result[0], (void*)&(hmh), sizeof(hmh));

                    *((uint32*)&result[sizeof(hmh)]) = QUANTISED_PNG_VERSION;
                    ::memcpy(&result[sizeof(hmh) + sizeof(uint32)], (void*)&(imgData->operator[](0)), imgData->size());

#ifdef __DAVAENGINE_DEBUG__
                    String chunkId = outsideChunkIdentifier(xc, yc);
                    chunkImage->Save("out\\spaces\\" + chunkId + ".png");
                    SafeRelease(chunkImage);
#endif

                    delete imgData;
                    //dominant texture
                    float32 xx, yy;
                    memset(dominant, defaultColorIndex, dominantSize);
                    offset = 0;

                    for (uint8 i = 0; i < BLENDS_SZ; i++)
                    {
                        memset(blends[i], (i == 0 ? 255 : 0), dominantSize);
                    }

                    float32 kk = (float32)texDataWidth / (realSize * 2.0f);
                    for (int32 y = 0; y < DOM_RES; ++y)
                    {
                        yy = yc * 100.0f + y * 100.0f / 128.0f;
                        if (yy > -realSize && yy < realSize)
                        {
                            int32 py = (int32)((yy + realSize) * kk + 0.5f);
                            py = Min(texDataHeight - 1, py);
                            for (int32 x = 0; x < DOM_RES; ++x)
                            {
                                xx = xc * 100.0f + x * 100.0f / 128.0f;
                                if (xx > -realSize && xx < realSize)
                                {
                                    int32 px = (int32)((xx + realSize) * kk + 0.5f);
                                    px = Min(texDataWidth - 1, px);
                                    int32 off = py * texDataWidth + px;
                                    uint8 val = (texData[off] + 1) << 4;
                                    uint8 maxVal = 0;
                                    int32 maxInd = 0;
                                    for (uint8 i = 0; i < BLENDS_SZ; i++)
                                    {
                                        uint8 v = tileMaskData[off * 4 + i];
                                        blends[i][offset] = v;
                                        if (maxVal < v)
                                        {
                                            maxVal = v;
                                            maxInd = i;
                                        }
                                    }
                                    dominant[offset] = (val + maxInd);
                                }
                                offset++;
                            }
                        }
                    }
                    SaveDominant();
                    for (int i = 0; i < BLENDS_SZ; i++)
                    {
                        SaveBlend(i);
                    }

                    String identifier = outsideChunkIdentifier(xc, yc);
                    compressResult("out\\spaces\\" + identifier + ".cdata", (BinaryPtr)&result);

                    File* f = File::Create("out\\spaces\\" + identifier + ".chunk", File::CREATE | File::WRITE);
                    _snprintf(content, sizeof(content),
                              "<root>\n<terrain>\n<editorOnly>\n<hidden>	false	</hidden>\n<frozen>	false	</frozen>\n</editorOnly>\n<visibilityMask>4294967295</visibilityMask>\n<metaData>	</metaData>\n<resource>	%s/terrain2	</resource>\n</terrain>\n</root>\n",
                              (identifier + ".cdata").c_str());
                    int32 res = f->Write(content, strlen(content));
                    SafeRelease(f);
                }
            }

#ifdef __DAVAENGINE_DEBUG__
            spaceImage->Save("out\\spaces\\space.png");
            SafeRelease(spaceImage);
#endif

            SafeRelease(heightMap);
            SafeDelete(texData);
            SafeDelete(terrain);
        }
        else
        {
            Logger::Error("Error at PrepareIndexTexture()");
        }
    }
    else
    {
        Logger::Error("Can't load tilemask texture or heightmap texture");
    }
} // Converter::DoConvertHM

void Converter::archiveFile(String fileName, FilePath filePath, zipFile& zf)
{
    int32 bufferSize = 8192;
    char* buffer = new char[bufferSize];
    int32 readed = -1;
    File* fin = File::Create(filePath, File::OPEN | File::READ);

    zip_fileinfo zi;
    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;

    int32 err = zipOpenNewFileInZip(zf, fileName.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 6);
    if (err == ZIP_OK)
    {
        while (readed != 0)
        {
            readed = fin->Read(buffer, bufferSize);
            if (readed > 0)
            {
                err = zipWriteInFileInZip(zf, buffer, readed);
            }
        }
        err = zipCloseFileInZip(zf);
    }
    SafeRelease(fin);
    delete buffer;
}

void Converter::browseDir(String dirName, zipFile& archive)
{
    FileList* list = new FileList(dirName);
    int32 listSize = list->GetCount();
    for (int32 i = 0; i < listSize; ++i)
    {
        if (!list->IsNavigationDirectory(i) && !list->IsDirectory(i))
        {
            String path = dirName + list->GetFilename(i);
            path.erase(0, 9);
            archiveFile(path, list->GetPathname(i), archive);
        }
        if (!list->IsNavigationDirectory(i) && list->IsDirectory(i))
        {
            browseDir(dirName + list->GetFilename(i) + "\\", archive);
        }
    }

    list->Release();
}

void Converter::chunkIndexesFromChunkID(int chunkID, int& chunkX, int& chunkZ)
{
    chunkX = (chunkID >> 8) - 127;
    chunkZ = (chunkID & 0xFF) - 127;
}

uint32 Converter::chunkIDFromChunkIndexes(int chunkX, int chunkZ)
{
    return ((chunkX + 127) << 8) | (chunkZ + 127);
}

String Converter::outsideChunkIdentifier(int32 gridX, int32 gridZ)
{
    char chunkIdentifierCStr[32];
    String gridChunkIdentifier;

    uint16 gridxs = uint16(gridX), gridzs = uint16(gridZ);
    _snprintf(chunkIdentifierCStr, sizeof(chunkIdentifierCStr), "%04x%04xo", int(gridxs), int(gridzs));
    gridChunkIdentifier += chunkIdentifierCStr;
    return gridChunkIdentifier;
}

#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

void addFile2Zip(zipFile& zf, zip_fileinfo& zi, char* fileName, BinaryPtr ptr)
{
    int32 err = zipOpenNewFileInZip(zf, fileName, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 6);
    if (err == ZIP_OK)
    {
        err = zipWriteInFileInZip(zf, &(ptr->front()), ptr->size());
        if (err < 0)
        {
            printf("error in writing in the zipfile\n");
        }
    }
    err = zipCloseFileInZip(zf);
    if (err != ZIP_OK)
    {
        printf("error in closing in the zipfile\n");
    }
}

void Converter::compressResult(String archiveName, BinaryPtr hMap)
{
    zipFile zf;
    int errclose;
    //	zf = zipOpen(archiveName.c_str(), APPEND_STATUS_ADDINZIP);
    zf = zipOpen(archiveName.c_str(), 0);
    zip_fileinfo zi;
    zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;

    addFile2Zip(zf, zi, "terrain2\\heights", hMap);
    addFile2Zip(zf, zi, "terrain2\\dominantTextures", (BinaryPtr)&dominantBlock);
    addFile2Zip(zf, zi, "terrain2\\layer 1", (BinaryPtr)&blendBlock[0]);
    addFile2Zip(zf, zi, "terrain2\\layer 2", (BinaryPtr)&blendBlock[1]);
    addFile2Zip(zf, zi, "terrain2\\layer 3", (BinaryPtr)&blendBlock[2]);

    browseDir("~res:/template\\", zf);

    errclose = zipClose(zf, NULL);
    if (errclose != ZIP_OK)
    {
        printf("error in closing\n");
    }
}

namespace ConverterDetail
{
/**
*	This is used when reading from an in-memory PNG buffer to make sure that
*	we don't overrun the end.
*/
struct PNGReadBuffer
{
    uint8* begin_;
    uint8* current_;
    uint8* end_;
};

/**
*	This is a libpng callback for writing out some compressed data.
*
*  @param pngPtr		Information about the PNG being compressed.
*  @param bytes		A pointer to some compressed data.
*  @param size			The size of the compressed data.
*/
void PNGAPI pngWriteBuffer(
png_structp pngPtr,
png_bytep bytes,
png_size_t size)
{
    //	std::vector<BinaryPtr> *buffers = (std::vector<BinaryPtr> *)pngPtr->io_ptr;
    std::vector<BinaryPtr>* buffers = (std::vector<BinaryPtr>*)png_get_io_ptr(pngPtr);
    BinaryPtr buffer = new BinaryBlock();
    buffer->resize(size);

    uint8* fr = &(buffer->operator[](0));
    ::memcpy((void*)fr, bytes, size);
    buffers->push_back(buffer);
}

/**
*	This is a libpng callback to signify the end of writing compressed
*  data.  We do nothing, but we still need this function to use the
*	user call-backs in libpng.
*/
void PNGAPI pngWriteFlush(png_structp /*pngPtr*/)
{
}

/**
*	This concatinates a collection of BinaryBlocks into one big
*	BinaryBlock.  We do this because when compressing to a PNG the
*	call-backs give back multiple small buffers.
*
*  @param buffers		A list of buffers to concatinate.
*	@param resCntStr	A string for resource tracking.
*  @returns			The concatinated buffers.
*/
BinaryPtr concatenate(
std::vector<BinaryPtr> const& buffers)
{
    uint32 totalSize = 0;
    for (size_t i = 0; i < buffers.size(); ++i)
    {
        totalSize += buffers[i]->size();
    }

    if (totalSize == 0)
    {
        return NULL;
    }

    BinaryPtr result = new BinaryBlock();
    result->resize(totalSize);
    uint8* data = (uint8*)&(result->operator[](0));
    uint8* p = data;
    for (size_t i = 0; i < buffers.size(); ++i)
    {
        ::memcpy(p, (void*)&(buffers[i]->operator[](0)), buffers[i]->size());
        p += buffers[i]->size();
    }
    return result;
}

/**
*	This returns true if x is a power of two.
*
*  @param x			The number to test.
*  @returns			True if the number is a power of two, false
*						otherwise.  Zero is not a power of two.
*/
bool isPower2(uint32 x)
{
    while (x != 0)
    {
        if (((x & 1) != 0) && ((x & ~(uint32)1) == 0))
        {
            return true;
        }
        else if ((x & 1) != 0)
        {
            return false;
        }
        x = x >> 1;
    }
    return false;
}

/**
*  Error callback for png_error().  We have to throw an exception here because
*  you aren't allowed to return from a libpng error function (i.e. expect a
*  segfault very shortly if you attempt to continue execution after this).
*/
void onPNGError(png_structp png_ptr, png_const_charp err_msg)
{
    //	Logger::Error( "Got libpng error: %s\n", err_msg );
    throw int(0);
}

/**
*  Warning callback for png_warning().
*/
void onPNGWarning(png_structp png_ptr, png_const_charp err_msg)
{
    //	WARNING_MSG( "Got libpng warning: %s\n", err_msg );
    throw int(1);
}
} // anonymous namespace

PNGImageData::PNGImageData()
    : data_(NULL)
    , width_(0)
    , height_(0)
    , bpp_(0)
    , stride_(0)
    , upsideDown_(false)
{
}

BinaryPtr Converter::compressImage(uint8* image)
{
    PNGImageData pngData;
    pngData.data_ = image;
    pngData.width_ = DOM_RES;
    pngData.height_ = DOM_RES;
    pngData.bpp_ = 8;
    pngData.stride_ = DOM_RES;
    pngData.upsideDown_ = false;
    return compressPNG(pngData);
}

BinaryPtr Converter::compressPNG(PNGImageData const& data)
{
    if (data.data_ == NULL || data.width_ == 0 || data.height_ == 0)
    {
        return NULL;
    }

    DVASSERT(ConverterDetail::isPower2(data.bpp_) && data.bpp_ <= 32);

    // Create the write and info structs:
    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (pngPtr == NULL)
    {
        return NULL;
    }
    png_infop pngInfo = png_create_info_struct(pngPtr);
    if (pngInfo == NULL)
    {
        png_destroy_write_struct(&pngPtr, NULL);
        return NULL;
    }

    // Setup writing callbacks:
    std::vector<BinaryPtr> outputBuffers;
    png_set_write_fn(pngPtr, &outputBuffers, ConverterDetail::pngWriteBuffer, ConverterDetail::pngWriteFlush);

    // Set error and warning callbacks
    png_voidp errorPtr = png_get_error_ptr(pngPtr);
    png_set_error_fn(pngPtr, errorPtr, ConverterDetail::onPNGError, ConverterDetail::onPNGWarning);

    // Let libpng know about the image format:
    int colourType = PNG_COLOR_TYPE_GRAY;
    int bpp = data.bpp_;
    if (bpp == 32)
    {
        colourType = PNG_COLOR_TYPE_RGBA;
        bpp = 8; // per channel
    }
    png_set_IHDR
    (
    pngPtr,
    pngInfo,
    data.width_, data.height_, bpp,
    colourType,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
    );
    png_set_pHYs
    (
    pngPtr,
    pngInfo,
    data.width_,
    data.height_,
    PNG_RESOLUTION_UNKNOWN
    );

    // Set a compression level of three.  This gives reasonable compression
    // levels with good compression and decompression times.
    png_set_compression_level(pngPtr, 3);

    // Create the row pointers:
    png_byte** rowPointers = new png_byte*[data.height_];
    for (uint32 y = 0; y < data.height_; ++y)
    {
        if (data.upsideDown_)
        {
            rowPointers[y] = data.data_ + data.stride_ * (data.height_ - y - 1);
        }
        else
        {
            rowPointers[y] = data.data_ + data.stride_ * y;
        }
    }

    // Write the image:
    png_write_info(pngPtr, pngInfo);
    png_write_image(pngPtr, rowPointers);
    png_write_end(pngPtr, pngInfo);

    // Generate the result:
    BinaryPtr result = ConverterDetail::concatenate(outputBuffers);

    // Cleanup:
    png_destroy_write_struct(&pngPtr, &pngInfo);
    delete[] rowPointers;
    rowPointers = NULL;

    return result;
}

struct DominantTextureMapHeader
{
    static const uint32 VERSION_ZIP = 1;
    DAVA::uint32 magic_;
    DAVA::uint32 version_;
    DAVA::uint32 numTextures_;
    DAVA::uint32 textureNameSize_;
    DAVA::uint32 width_;
    DAVA::uint32 height_;
    DAVA::uint32 pad_[2];
    static const uint32 MAGIC = 0x0074616d; //"mat"
};

struct BlendHeader
{
    enum BHMVersions
    {
        VERSION_RAW_BLENDS = 1,
        VERSION_PNG_BLENDS = 2
    };

    DAVA::uint32 magic_; // Should be "bld\0"
    DAVA::uint32 width_;
    DAVA::uint32 height_;
    DAVA::uint32 bpp_; // Reserved for future use.
    DAVA::Vector4 uProjection_;
    DAVA::Vector4 vProjection_;
    DAVA::uint32 version_; // format version
    DAVA::uint32 pad_[3];
    // static const uint32 MAGIC = '\0dlb';
    static const uint32 MAGIC = 0x00646c62;
};

bool Converter::SaveDominant(void)
{
    // calculate data size from the num of textures and image size.
    uint32 numTextures = textureNames.size();
    uint32 textureNameSize = 0;
    for (uint32 i = 0; i < numTextures; ++i)
    {
        uint32 nameLength = textureNames[i].length();
        if (nameLength > textureNameSize)
        {
            textureNameSize = nameLength;
        }
    }
    uint32 textureDataSize = textureNameSize * numTextures;

    // create header
    std::vector<uint8> hdrData(sizeof(DominantTextureMapHeader) + textureDataSize, 0);
    DominantTextureMapHeader* pHeader = reinterpret_cast<DominantTextureMapHeader*>(&hdrData.front());

    pHeader->magic_ = DominantTextureMapHeader::MAGIC;
    pHeader->version_ = DominantTextureMapHeader::VERSION_ZIP;

    // save texture names
    pHeader->numTextures_ = numTextures;
    pHeader->textureNameSize_ = textureNameSize;
    char* p = (char*)(pHeader + 1);
    for (uint32 i = 0; i < numTextures; ++i)
    {
        strncpy(p, textureNames[i].c_str(), textureNameSize);
        p += textureNameSize;
    }

    // save image and compress it
    pHeader->width_ = DOM_RES;
    pHeader->height_ = DOM_RES;

    //  std::vector<uint8> imgData( DOM_RES * DOM_RES );
    //  memcpy(&imgData.front() dominant
    //  image().copyTo(  );

    CompressDominant();

    // Write the header and image data out to the data section
    dominantBlock.resize(hdrData.size() + imageBlock.size());
    memcpy(&dominantBlock.front(), &hdrData.front(), hdrData.size());
    memcpy(&dominantBlock.front() + hdrData.size(), &imageBlock.front(), imageBlock.size());
    return true;
}

void Converter::CompressDominant(int level)
{
    // Sanity check on the compression level:
    if (level < RAW_COMPRESSION)
    {
        level = RAW_COMPRESSION;
    }
    if (level > BEST_COMPRESSION)
    {
        level = BEST_COMPRESSION;
    }

    // We don't know how well the data compresses.  ZLib can give us an upper
    // bound though.  We compress into a buffer of this size and then copy to
    // a smaller buffer below:
    uLongf compressedSz = compressBound((uint32)dominantSize);
    uint8* largeBuffer = new uint8[compressedSz];
    int result =
    compress2
    (
    largeBuffer,
    &compressedSz,
    (const Byte*)dominant,
    (uint32)dominantSize,
    level
    );
    //	MF_ASSERT(result == Z_OK);
    // compressedSz has been set to the size actually needed.

    // Now copy to a smaller buffer.  The format of this buffer is:
    //	COMPRESSED_MAGIC1		(uint32)
    //	COMPRESSED_MAGIC2		(uint32)
    //	decompressed size		(uint32)
    //	zipped data
    uint32 realBufSz = compressedSz + COMPRESSED_HEADER_SZ * sizeof(uint32);
    imageBlock.resize(realBufSz);

    uint32* magicAndSz = (uint32*)&imageBlock[0];
    *magicAndSz++ = COMPRESSED_MAGIC1;
    *magicAndSz++ = COMPRESSED_MAGIC2;
    *magicAndSz++ = (uint32)dominantSize;
    uint8* compData = (uint8*)magicAndSz;
    ::memcpy(compData, largeBuffer, compressedSz);
    delete[] largeBuffer;
}

bool Converter::SaveBlend(int32 index)
{
    // Compress the blends to PNG format:
    BinaryPtr compData = compressImage(blends[index]);

    size_t dataSz = sizeof(BlendHeader) + compData->size()
    + sizeof(uint32) + sizeof(char) * (textureNames[index].length() + 1);

    blendBlock[index].resize(dataSz, 0);

    uint8* addr = &(blendBlock[index].front());

    BlendHeader* bh = (BlendHeader*)addr;
    bh->magic_ = BlendHeader::MAGIC;
    bh->width_ = DOM_RES;
    bh->height_ = DOM_RES;
    bh->bpp_ = 8;
    bh->uProjection_ = Vector4(1.f / 10.0f, 0.f, 0.f, 0.f);
    bh->vProjection_ = Vector4(0.f, 0.f, 1.f / 10.0f, 0.f);
    bh->version_ = BlendHeader::VERSION_PNG_BLENDS;
    addr += sizeof(BlendHeader);

    uint32* texLen = (uint32*)addr;
    *texLen = textureNames[index].length();
    addr += sizeof(uint32);

    char* tex = (char*)addr;
    ::strncpy(tex, textureNames[index].c_str(), *texLen);
    addr += textureNames[index].length();

    uint8* pHeight = (uint8*)(addr);
    ::memcpy(pHeight, &compData->front(), compData->size());

    delete compData;
    return true;
}

void Converter::AddWater(Entity* node, const String& guidStr)
{
    Logger::Info("AddWater %s", node->GetName().c_str());

    AABBox3 wtBox = node->GetWTMaximumBoundingBoxSlow();
    Vector3 pos = wtBox.GetCenter();
    Vector3 size = wtBox.GetSize();
    size.z = 0.0f;

    //write vlo file
    _snprintf(resultBuff, sizeof(resultBuff), waterVloFormatPattern,
              guidStr.c_str(),
              pos.x, pos.z, pos.y,
              size.x, size.z, size.y,
              guidStr.c_str());

    File* f = File::Create("out\\spaces\\" + guidStr + ".vlo", File::CREATE | File::WRITE);
    f->Write(resultBuff, strlen(resultBuff));
    SafeRelease(f);

    //write to all needed chunks
    _snprintf(resultBuff, sizeof(resultBuff), waterFormatPattern,
              guidStr.c_str(),
              pos.x, pos.z, pos.y,
              size.x, size.z, size.y);

    AABBox3 box(Vector3(0, 0, -1000), Vector3(100, 100, 1000));

    for (int16 yc = -6; yc < 6; yc++)
    {
        for (int16 xc = -6; xc < 6; xc++)
        {
            Matrix4 translation = Matrix4::MakeTranslation(Vector3(xc * 100.0f, yc * 100.0f, 0));
            AABBox3 chunkBox;
            box.GetTransformedBox(translation, chunkBox);
            if (chunkBox.IntersectsWithBox(wtBox))
            {
                String identifier = outsideChunkIdentifier(xc, yc);
                File* f = File::Create("out\\spaces\\" + identifier + ".chunk", File::OPEN | File::READ);
                if (f)
                {
                    uint32 res = f->Read(chunkBuffer, CHUNKBUFFER_SIZE);
                    chunkBuffer[res] = 0;
                    SafeRelease(f);

                    char* str = strstr(chunkBuffer, "</root>");
                    if (str != NULL)
                    {
                        f = File::Create("out\\spaces\\" + identifier + ".chunk", File::CREATE | File::WRITE);
                        f->Write(chunkBuffer, (uint32)(str - chunkBuffer));
                        f->Write(resultBuff, strlen(resultBuff));
                        f->Write(str, strlen(str));
                        SafeRelease(f);
                    }
                }
            }
        }
    }
    resultBuff[0] = 0;
} // Converter::AddWater

void Converter::SetupVersionLKA()
{
    sceneNodesLinks->SetInt32("version", 1);
}

UniqueID Converter::GetEntityID(Entity* node)
{
    std::hash<std::string> hash_str;

    uint32 a = 0, b = 0, c = 0, d = 0;
    a = hash_str(scene->GetName().c_str());
    b = node->GetID();

    return UniqueID(a, b, c, d);
}

UniqueID Converter::GetEntityID(Entity* node, uint32 index)
{
    std::hash<std::string> hash_fn;

    uint32 a = 0, b = 0, c = 0, d = 0;
    a = hash_fn(scene->GetName().c_str());
    b = node->GetID();
    c = index;

    return UniqueID(a, b, c, d);
}