#include "REPlatform/Scene/Systems/DebugDrawSystem.h"
#include "REPlatform/Scene/Systems/BeastSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

#include "REPlatform/DataNodes/ProjectManagerData.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"
#include "REPlatform/Deprecated/EditorConfig.h"
#include "REPlatform/Deprecated/SceneValidator.h"

#include <TArc/Core/Deprecated.h>

#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/GeoDecalComponent.h>

#define DAVA_ALLOW_OCTREE_DEBUG 0

namespace DAVA
{
const float32 DebugDrawSystem::HANGING_OBJECTS_DEFAULT_HEIGHT = 0.001f;

DebugDrawSystem::DebugDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
    drawComponentFunctionsMap[Component::SOUND_COMPONENT] = MakeFunction(this, &DebugDrawSystem::DrawSoundNode);
    drawComponentFunctionsMap[Component::WIND_COMPONENT] = MakeFunction(this, &DebugDrawSystem::DrawWindNode);
    drawComponentFunctionsMap[Component::GEO_DECAL_COMPONENT] = MakeFunction(this, &DebugDrawSystem::DrawDecals);
    drawComponentFunctionsMap[Component::LIGHT_COMPONENT] = Bind(&DebugDrawSystem::DrawLightNode, this, DAVA::_1, false);
}

DebugDrawSystem::~DebugDrawSystem()
{
}

void DebugDrawSystem::SetRequestedObjectType(ResourceEditor::eSceneObjectType _objectType)
{
    objectType = _objectType;

    if (ResourceEditor::ESOT_NONE != objectType)
    {
        ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
        DVASSERT(data);
        const Vector<Color>& colors = data->GetEditorConfig()->GetColorPropertyValues("CollisionTypeColor");
        if ((uint32)objectType < (uint32)colors.size())
        {
            objectTypeColor = colors[objectType];
        }
        else
        {
            objectTypeColor = Color(1.f, 0, 0, 1.f);
        }
    }
}

ResourceEditor::eSceneObjectType DebugDrawSystem::GetRequestedObjectType() const
{
    return objectType;
}

void DebugDrawSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);

    for (uint32 type = 0; type < Component::COMPONENT_COUNT; ++type)
    {
        for (uint32 index = 0, count = entity->GetComponentCount(type); index < count; ++index)
        {
            Component* component = entity->GetComponent(type, index);
            RegisterComponent(entity, component);
        }
    }
}

void DebugDrawSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(entities, entity);

    for (auto& it : entitiesComponentMap)
    {
        FindAndRemoveExchangingWithLast(it.second, entity);
    }
}

void DebugDrawSystem::RegisterComponent(Entity* entity, Component* component)
{
    Component::eType type = static_cast<Component::eType>(component->GetType());

    auto it = drawComponentFunctionsMap.find(type);

    if (it != drawComponentFunctionsMap.end())
    {
        Vector<Entity*>& array = entitiesComponentMap[type];

        auto it = find_if(array.begin(), array.end(), [entity](const Entity* obj) { return entity == obj; });

        if (it == array.end())
        {
            array.push_back(entity);
        }
    }
}

void DebugDrawSystem::UnregisterComponent(Entity* entity, Component* component)
{
    Component::eType type = static_cast<Component::eType>(component->GetType());

    auto it = entitiesComponentMap.find(type);

    if (it != entitiesComponentMap.end() && entity->GetComponentCount(component->GetType()) == 1)
    {
        FindAndRemoveExchangingWithLast(it->second, entity);
    }
}

void DebugDrawSystem::PrepareForRemove()
{
    entities.clear();
    entitiesComponentMap.clear();
}

void DebugDrawSystem::DrawComponent(Component::eType type, const Function<void(Entity*)>& func)
{
    auto it = entitiesComponentMap.find(type);

    if (it != entitiesComponentMap.end())
    {
        Vector<Entity*>& array = it->second;

        for (Entity* entity : array)
        {
            func(entity);
        }
    }
}

void DebugDrawSystem::Draw()
{
    for (auto& it : drawComponentFunctionsMap)
    {
        DrawComponent(it.first, it.second);
    }

    for (Entity* entity : entities)
    { //drawing methods do not use data from components
        DrawObjectBoxesByType(entity);
        DrawHangingObjects(entity);
        DrawSwitchesWithDifferentLods(entity);
        DrawDebugOctTree(entity);

        //draw selected objects
        const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
        bool isSelected = selection.ContainsObject(entity);

        if (isSelected)
        {
            DrawLightNode(entity, true);
            DrawSelectedSoundNode(entity);
        }
    }
}

void DebugDrawSystem::DrawObjectBoxesByType(Entity* entity)
{
    bool drawBox = false;

    KeyedArchive* customProperties = GetCustomPropertiesArchieve(entity);
    if (customProperties)
    {
        if (customProperties->IsKeyExists("CollisionType"))
        {
            drawBox = customProperties->GetInt32("CollisionType", 0) == objectType;
        }
        else if (objectType == ResourceEditor::ESOT_UNDEFINED_COLLISION && entity->GetParent() == GetScene())
        {
            const bool skip =
            GetLight(entity) == NULL &&
            GetCamera(entity) == NULL &&
            GetLandscape(entity) == NULL;

            drawBox = skip;
        }
    }

    if (drawBox)
    {
        DrawEntityBox(entity, objectTypeColor);
    }
}

void DebugDrawSystem::DrawDebugOctTree(Entity* entity)
{
#if (DAVA_ALLOW_OCTREE_DEBUG)
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    RenderHelper* drawer = editorScene->GetRenderSystem()->GetDebugDrawer();

    RenderObject* renderObject = GetRenderObject(entity);
    if (renderObject == nullptr)
        return;

    for (uint32 k = 0; k < renderObject->GetActiveRenderBatchCount(); ++k)
    {
        RenderBatch* renderBatch = renderObject->GetActiveRenderBatch(k);

        PolygonGroup* pg = renderBatch->GetPolygonGroup();
        if (pg == nullptr)
            continue;

        GeometryOctTree* octTree = pg->GetGeometryOctTree();
        if (octTree == nullptr)
            continue;

        const Matrix4& wt = entity->GetWorldTransform();
        if (renderBatch->debugDrawOctree)
            octTree->DebugDraw(wt, 0, drawer);

        for (const GeometryOctTree::Triangle& tri : octTree->GetDebugTriangles())
        {
            Vector3 v1 = tri.v1 * entity->GetWorldTransform();
            Vector3 v2 = tri.v2 * entity->GetWorldTransform();
            Vector3 v3 = tri.v3 * entity->GetWorldTransform();
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, Color(1.0f, 0.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v2, v3, Color(1.0f, 0.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v1, Color(1.0f, 0.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
        }

        for (const AABBox3& box : octTree->GetDebugBoxes())
        {
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(box, wt, Color(0.0f, 1.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
        }
    }
#endif
}

void DebugDrawSystem::DrawLightNode(Entity* entity, bool isSelected)
{
    Light* light = GetLight(entity);
    if (NULL != light)
    {
        Scene* scene = GetScene();
        RenderHelper* drawer = scene->GetRenderSystem()->GetDebugDrawer();

        AABBox3 worldBox;
        AABBox3 localBox = scene->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
        DVASSERT(!localBox.IsEmpty());
        localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);

        if (light->GetType() == Light::TYPE_DIRECTIONAL)
        {
            Vector3 center = worldBox.GetCenter();
            Vector3 direction = -light->GetDirection();

            direction.Normalize();
            direction = direction * worldBox.GetSize().x;

            center -= (direction / 2);

            drawer->DrawArrow(center + direction, center, direction.Length() / 2, Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
        else if (light->GetType() == Light::TYPE_POINT)
        {
            Vector3 worldCenter = worldBox.GetCenter();
            drawer->DrawIcosahedron(worldCenter, worldBox.GetSize().x / 2, Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawIcosahedron(worldCenter, worldBox.GetSize().x / 2, Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
            KeyedArchive* properties = GetCustomPropertiesArchieve(entity);
            VariantType* value = properties->GetVariant("editor.staticlight.falloffcutoff");
            if (value != nullptr && value->GetType() == VariantType::TYPE_FLOAT && isSelected)
            {
                float32 distance = value->AsFloat();
                if (distance < BeastSystem::DEFAULT_FALLOFFCUTOFF_VALUE)
                {
                    uint32 segmentCount = 32;
                    drawer->DrawCircle(worldCenter, Vector3(1.0f, 0.0f, 0.0f), distance, segmentCount, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                    drawer->DrawCircle(worldCenter, Vector3(0.0f, 1.0f, 0.0f), distance, segmentCount, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                    drawer->DrawCircle(worldCenter, Vector3(0.0f, 0.0f, 1.0f), distance, segmentCount, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                }
            }
        }
        else
        {
            drawer->DrawAABox(worldBox, Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawAABox(worldBox, Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSoundNode(Entity* entity)
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    if (settings->drawSoundObjects == false)
        return;

    SoundComponent* sc = GetSoundComponent(entity);
    if (sc)
    {
        AABBox3 worldBox;
        AABBox3 localBox = GetScene()->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
        if (!localBox.IsEmpty())
        {
            localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);

            Color soundColor = settings->soundObjectBoxColor;
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, ClampToUnityRange(soundColor), RenderHelper::DRAW_SOLID_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSelectedSoundNode(Entity* entity)
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();
    if (settings->drawSoundObjects == false)
        return;

    SoundComponent* sc = GetSoundComponent(entity);
    if (sc)
    {
        Scene* scene = GetScene();
        TextDrawSystem* textDrawSystem = scene->GetSystem<TextDrawSystem>();
        RenderSystem* renderSystem = scene->GetRenderSystem();
        Vector3 position = entity->GetWorldTransform().GetTranslationVector();

        uint32 fontHeight = 0;
        GraphicFont* debugTextFont = textDrawSystem->GetFont();
        if (debugTextFont)
            fontHeight = debugTextFont->GetFontHeight();

        uint32 eventsCount = sc->GetEventsCount();
        for (uint32 i = 0; i < eventsCount; ++i)
        {
            SoundEvent* sEvent = sc->GetSoundEvent(i);
            float32 distance = sEvent->GetMaxDistance();

            Color soundColor = settings->soundObjectSphereColor;
            renderSystem->GetDebugDrawer()->DrawIcosahedron(position, distance, ClampToUnityRange(soundColor), RenderHelper::DRAW_SOLID_DEPTH);

            textDrawSystem->DrawText(textDrawSystem->ToPos2d(position) - Vector2(0.f, fontHeight - 2.f) * i,
                                     sEvent->GetEventName(), Color::White, TextDrawSystem::Align::Center);

            if (sEvent->IsDirectional())
            {
                renderSystem->GetDebugDrawer()->DrawArrow(position, sc->GetLocalDirection(i), .25f,
                                                          Color(0.0f, 1.0f, 0.3f, 1.0f), RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
    }
}

void DebugDrawSystem::DrawWindNode(Entity* entity)
{
    WindComponent* wind = GetWindComponent(entity);
    if (wind)
    {
        const Matrix4& worldMx = entity->GetWorldTransform();
        Vector3 worldPosition = worldMx.GetTranslationVector();

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(worldPosition, worldPosition + wind->GetDirection() * 3.f, .75f,
                                                                   Color(1.0f, 0.5f, 0.2f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawEntityBox(Entity* entity, const Color& color)
{
    AABBox3 localBox = GetScene()->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
    if (localBox.IsEmpty() == false)
    {
        AABBox3 worldBox;
        localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, color, RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawHangingObjects(Entity* entity)
{
    if (hangingObjectsModeEnabled && (entity->GetParent() == GetScene()) && IsObjectHanging(entity))
    {
        DrawEntityBox(entity, Color(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

void DebugDrawSystem::CollectRenderBatchesRecursively(Entity* entity, RenderBatchesWithTransforms& batches) const
{
    auto ro = GetRenderObject(entity);
    if (ro != nullptr)
    {
        auto roType = ro->GetType();
        if ((roType == RenderObject::TYPE_MESH) || (roType == RenderObject::TYPE_RENDEROBJECT) || (roType == RenderObject::TYPE_SPEED_TREE))
        {
            const Matrix4& wt = entity->GetWorldTransform();
            for (uint32 i = 0, e = ro->GetActiveRenderBatchCount(); i < e; ++i)
            {
                RenderBatch* batch = ro->GetActiveRenderBatch(i);
                if (batch != nullptr)
                {
                    PolygonGroup* pg = batch->GetPolygonGroup();
                    if (pg != nullptr)
                    {
                        batches.emplace_back(batch, wt);
                    }
                }
            }
        }
    }

    for (int32 i = 0, e = entity->GetChildrenCount(); i < e; ++i)
    {
        CollectRenderBatchesRecursively(entity->GetChild(i), batches);
    }
}

float32 DebugDrawSystem::GetMinimalZ(const RenderBatchesWithTransforms& batches) const
{
    float32 minZ = AABBOX_INFINITY;
    for (const auto& batch : batches)
    {
        PolygonGroup* polygonGroup = batch.first->GetPolygonGroup();
        for (uint32 v = 0, e = polygonGroup->GetVertexCount(); v < e; ++v)
        {
            Vector3 pos;
            polygonGroup->GetCoord(v, pos);
            minZ = Min(minZ, pos.z);
        }
    }
    return minZ;
}

void DebugDrawSystem::GetLowestVertexes(const RenderBatchesWithTransforms& batches, Vector<Vector3>& vertexes) const
{
    const float32 minZ = GetMinimalZ(batches);
    for (const auto& batch : batches)
    {
        float32 scale = std::sqrt(batch.second._20 * batch.second._20 + batch.second._21 * batch.second._21 + batch.second._22 * batch.second._22);
        PolygonGroup* polygonGroup = batch.first->GetPolygonGroup();
        for (uint32 v = 0, e = polygonGroup->GetVertexCount(); v < e; ++v)
        {
            Vector3 pos;
            polygonGroup->GetCoord(v, pos);
            if (scale * (pos.z - minZ) <= hangingObjectsHeight)
            {
                vertexes.push_back(pos * batch.second);
            }
        }
    }
}

bool DebugDrawSystem::IsObjectHanging(Entity* entity) const
{
    Vector<Vector3> lowestVertexes;
    RenderBatchesWithTransforms batches;
    CollectRenderBatchesRecursively(entity, batches);
    GetLowestVertexes(batches, lowestVertexes);

    for (const auto& vertex : lowestVertexes)
    {
        Vector3 landscapePoint = GetLandscapePointAtCoordinates(Vector2(vertex.x, vertex.y));
        if ((vertex.z - landscapePoint.z) > EPSILON)
        {
            return true;
        }
    }

    return false;
}

Vector3 DebugDrawSystem::GetLandscapePointAtCoordinates(const Vector2& centerXY) const
{
    LandscapeEditorDrawSystem* landSystem = GetScene()->GetSystem<LandscapeEditorDrawSystem>();
    LandscapeProxy* landscape = landSystem->GetLandscapeProxy();

    if (landscape)
    {
        return landscape->PlacePoint(Vector3(centerXY));
    }

    return Vector3();
}

void DebugDrawSystem::DrawSwitchesWithDifferentLods(Entity* entity)
{
    if (switchesWithDifferentLodsEnabled && SceneValidator::IsEntityHasDifferentLODsCount(entity))
    {
        Scene* scene = GetScene();
        AABBox3 worldBox;
        AABBox3 localBox = scene->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
        DVASSERT(!localBox.IsEmpty());
        localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);
        scene->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, Color(1.0f, 0.f, 0.f, 1.f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawDecals(Entity* entity)
{
    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    uint32 componentsCount = entity->GetComponentCount(Component::eType::GEO_DECAL_COMPONENT);
    for (uint32 i = 0; i < componentsCount; ++i)
    {
        Component* component = entity->GetComponent(Component::eType::GEO_DECAL_COMPONENT, i);
        DVASSERT(component != nullptr);

        GeoDecalComponent* decal = static_cast<GeoDecalComponent*>(component);
        Matrix4 transform = entity->GetWorldTransform();

        RenderHelper::eDrawType dt = RenderHelper::eDrawType::DRAW_WIRE_DEPTH;
        Color baseColor(1.0f, 0.5f, 0.25f, 1.0f);
        Color accentColor(1.0f, 1.0f, 0.5f, 1.0f);

        AABBox3 box = decal->GetBoundingBox();
        Vector3 boxCenter = box.GetCenter();
        Vector3 boxHalfSize = 0.5f * box.GetSize();

        Vector3 farPoint = Vector3(boxCenter.x, boxCenter.y, box.min.z) * transform;
        Vector3 nearPoint = Vector3(boxCenter.x, boxCenter.y, box.max.z) * transform;

        Vector3 direction = farPoint - nearPoint;
        direction.Normalize();

        drawer->DrawAABoxTransformed(box, transform, baseColor, dt);

        if (decal->GetConfig().mapping == GeoDecalManager::Mapping::CYLINDRICAL)
        {
            Vector3 side = Vector3(boxCenter.x - boxHalfSize.x, 0.0f, box.max.z) * transform;

            float radius = (side - nearPoint).Length();
            drawer->DrawCircle(nearPoint, direction, radius, 32, accentColor, dt);
            drawer->DrawCircle(farPoint, -direction, radius, 32, accentColor, dt);
            drawer->DrawLine(nearPoint, side, accentColor);
        }
        else if (decal->GetConfig().mapping == GeoDecalManager::Mapping::SPHERICAL)
        {
            // no extra debug visualization
        }
        else /* planar assumed */
        {
            drawer->DrawArrow(nearPoint - direction, nearPoint, 0.25f * direction.Length(), accentColor, dt);
        }
    }
}
} // namespace DAVA
