#include "Scene/System/DebugDrawSystem.h"
#include "Scene/SceneEditor2.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Application/REGlobal.h"
#include "Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "Deprecated/EditorConfig.h"
#include "Deprecated/SceneValidator.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Classes/Selection/Selection.h"
#include "Render/Highlevel/GeometryOctTree.h"

using namespace DAVA;

DAVA::float32 DebugDrawSystem::HANGING_OBJECTS_HEIGHT = 0.001f;

DebugDrawSystem::DebugDrawSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    SceneEditor2* sc = (SceneEditor2*)GetScene();

    collSystem = sc->collisionSystem;

    DVASSERT(NULL != collSystem);
}

DebugDrawSystem::~DebugDrawSystem()
{
}

void DebugDrawSystem::SetRequestedObjectType(ResourceEditor::eSceneObjectType _objectType)
{
    objectType = _objectType;

    if (ResourceEditor::ESOT_NONE != objectType)
    {
        ProjectManagerData* data = REGlobal::GetDataNode<ProjectManagerData>();
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

void DebugDrawSystem::Draw()
{
    Draw(GetScene());
}

void DebugDrawSystem::Draw(DAVA::Entity* entity)
{
    if (nullptr != entity)
    {
        const SelectableGroup& selection = Selection::GetSelection();
        bool isSelected = selection.ContainsObject(entity);

        DrawObjectBoxesByType(entity);
        DrawUserNode(entity);
        DrawLightNode(entity);
        DrawHangingObjects(entity);
        DrawSwitchesWithDifferentLods(entity);
        DrawWindNode(entity);
        DrawSwitchesWithDifferentLods(entity);
        DrawSoundNode(entity);
        DrawDebugOctTree(entity);

        if (isSelected)
        {
            DrawSelectedSoundNode(entity);
        }

        for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
        {
            Draw(entity->GetChild(i));
        }
    }
}

void DebugDrawSystem::DrawObjectBoxesByType(DAVA::Entity* entity)
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

void DebugDrawSystem::DrawUserNode(DAVA::Entity* entity)
{
    if (NULL != entity->GetComponent(DAVA::Component::USER_COMPONENT))
    {
        SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
        RenderHelper* drawer = editorScene->GetRenderSystem()->GetDebugDrawer();

        AABBox3 worldBox = editorScene->collisionSystem->GetUntransformedBoundingBox(entity);
        DVASSERT(!worldBox.IsEmpty());
        DAVA::float32 delta = worldBox.GetSize().Length() / 4;

        drawer->DrawAABoxTransformed(worldBox, entity->GetWorldTransform(), DAVA::Color(0.5f, 0.5f, 1.0f, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawAABoxTransformed(worldBox, entity->GetWorldTransform(), DAVA::Color(0.2f, 0.2f, 0.8f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);

        const Vector3 center = entity->GetWorldTransform().GetTranslationVector();
        const Vector3 xAxis = MultiplyVectorMat3x3(DAVA::Vector3(delta, 0.f, 0.f), entity->GetWorldTransform());
        const Vector3 yAxis = MultiplyVectorMat3x3(DAVA::Vector3(0.f, delta, 0.f), entity->GetWorldTransform());
        const Vector3 zAxis = MultiplyVectorMat3x3(DAVA::Vector3(0.f, 0.f, delta), entity->GetWorldTransform());

        // axises
        drawer->DrawLine(center, center + xAxis, DAVA::Color(0.7f, 0, 0, 1.0f));
        drawer->DrawLine(center, center + yAxis, DAVA::Color(0, 0.7f, 0, 1.0f));
        drawer->DrawLine(center, center + zAxis, DAVA::Color(0, 0, 0.7f, 1.0f));
    }
}

void DebugDrawSystem::DrawDebugOctTree(DAVA::Entity* entity)
{
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

        octTree->DebugDraw(entity->GetWorldTransform(), 0, drawer);

        for (uint32 triangleIndex : octTree->invalidTriangles)
        {
            int32 i1, i2, i3;
            pg->GetIndex(triangleIndex * 3 + 0, i1);
            pg->GetIndex(triangleIndex * 3 + 1, i2);
            pg->GetIndex(triangleIndex * 3 + 2, i3);

            Vector3 v1, v2, v3;
            pg->GetCoord(i1, v1);
            pg->GetCoord(i2, v2);
            pg->GetCoord(i3, v3);

            v1 = v1 * (*renderObject->GetWorldTransformPtr());
            v2 = v2 * (*renderObject->GetWorldTransformPtr());
            v3 = v3 * (*renderObject->GetWorldTransformPtr());
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, DAVA::Color(0.0f, 0.5f, 1.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v2, v3, DAVA::Color(0.0f, 0.5f, 1.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v1, DAVA::Color(0.0f, 0.5f, 1.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawLightNode(DAVA::Entity* entity)
{
    DAVA::Light* light = GetLight(entity);
    if (NULL != light)
    {
        SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
        RenderHelper* drawer = editorScene->GetRenderSystem()->GetDebugDrawer();

        AABBox3 worldBox;
        AABBox3 localBox = editorScene->collisionSystem->GetUntransformedBoundingBox(entity);
        DVASSERT(!localBox.IsEmpty());
        localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);

        if (light->GetType() == Light::TYPE_DIRECTIONAL)
        {
            DAVA::Vector3 center = worldBox.GetCenter();
            DAVA::Vector3 direction = -light->GetDirection();

            direction.Normalize();
            direction = direction * worldBox.GetSize().x;

            center -= (direction / 2);

            drawer->DrawArrow(center + direction, center, direction.Length() / 2, DAVA::Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
        else if (light->GetType() == Light::TYPE_POINT)
        {
            drawer->DrawIcosahedron(worldBox.GetCenter(), worldBox.GetSize().x / 2, DAVA::Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawIcosahedron(worldBox.GetCenter(), worldBox.GetSize().x / 2, DAVA::Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
        else
        {
            drawer->DrawAABox(worldBox, DAVA::Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawAABox(worldBox, DAVA::Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSoundNode(DAVA::Entity* entity)
{
    SettingsManager* settings = SettingsManager::Instance();

    if (!settings->GetValue(Settings::Scene_Sound_SoundObjectDraw).AsBool())
        return;

    DAVA::SoundComponent* sc = GetSoundComponent(entity);
    if (sc)
    {
        SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());

        AABBox3 worldBox;
        AABBox3 localBox = editorScene->collisionSystem->GetUntransformedBoundingBox(entity);
        if (!localBox.IsEmpty())
        {
            localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);

            Color soundColor = settings->GetValue(Settings::Scene_Sound_SoundObjectBoxColor).AsColor();
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, ClampToUnityRange(soundColor), RenderHelper::DRAW_SOLID_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSelectedSoundNode(DAVA::Entity* entity)
{
    SettingsManager* settings = SettingsManager::Instance();

    if (!settings->GetValue(Settings::Scene_Sound_SoundObjectDraw).AsBool())
        return;

    DAVA::SoundComponent* sc = GetSoundComponent(entity);
    if (sc)
    {
        SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());

        Vector3 position = entity->GetWorldTransform().GetTranslationVector();

        uint32 fontHeight = 0;
        GraphicFont* debugTextFont = sceneEditor->textDrawSystem->GetFont();
        if (debugTextFont)
            fontHeight = debugTextFont->GetFontHeight();

        uint32 eventsCount = sc->GetEventsCount();
        for (uint32 i = 0; i < eventsCount; ++i)
        {
            SoundEvent* sEvent = sc->GetSoundEvent(i);
            float32 distance = sEvent->GetMaxDistance();

            Color soundColor = settings->GetValue(Settings::Scene_Sound_SoundObjectSphereColor).AsColor();

            sceneEditor->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(position, distance,
                                                                              ClampToUnityRange(soundColor), RenderHelper::DRAW_SOLID_DEPTH);

            sceneEditor->textDrawSystem->DrawText(sceneEditor->textDrawSystem->ToPos2d(position) - Vector2(0.f, fontHeight - 2.f) * i,
                                                  sEvent->GetEventName(), Color::White, TextDrawSystem::Align::Center);

            if (sEvent->IsDirectional())
            {
                sceneEditor->GetRenderSystem()->GetDebugDrawer()->DrawArrow(position, sc->GetLocalDirection(i), .25f, DAVA::Color(0.0f, 1.0f, 0.3f, 1.0f), RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
    }
}

void DebugDrawSystem::DrawWindNode(DAVA::Entity* entity)
{
    WindComponent* wind = GetWindComponent(entity);
    if (wind)
    {
        const Matrix4& worldMx = entity->GetWorldTransform();
        Vector3 worldPosition = worldMx.GetTranslationVector();

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(worldPosition, worldPosition + wind->GetDirection() * 3.f, .75f, DAVA::Color(1.0f, 0.5f, 0.2f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawEntityBox(DAVA::Entity* entity, const DAVA::Color& color)
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    AABBox3 localBox = editorScene->collisionSystem->GetUntransformedBoundingBox(entity);
    if (localBox.IsEmpty() == false)
    {
        AABBox3 worldBox;
        localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, color, RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawHangingObjects(DAVA::Entity* entity)
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

DAVA::float32 DebugDrawSystem::GetMinimalZ(const RenderBatchesWithTransforms& batches) const
{
    float32 minZ = AABBOX_INFINITY;
    for (const auto& batch : batches)
    {
        PolygonGroup* polygonGroup = batch.first->GetPolygonGroup();
        for (uint32 v = 0, e = polygonGroup->GetVertexCount(); v < e; ++v)
        {
            Vector3 pos;
            polygonGroup->GetCoord(v, pos);
            minZ = DAVA::Min(minZ, pos.z);
        }
    }
    return minZ;
}

void DebugDrawSystem::GetLowestVertexes(const RenderBatchesWithTransforms& batches, DAVA::Vector<DAVA::Vector3>& vertexes) const
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
            if (scale * (pos.z - minZ) <= HANGING_OBJECTS_HEIGHT)
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
        if ((vertex.z - landscapePoint.z) > DAVA::EPSILON)
        {
            return true;
        }
    }

    return false;
}

Vector3 DebugDrawSystem::GetLandscapePointAtCoordinates(const Vector2& centerXY) const
{
    LandscapeEditorDrawSystem* landSystem = ((SceneEditor2*)GetScene())->landscapeEditorDrawSystem;
    LandscapeProxy* landscape = landSystem->GetLandscapeProxy();

    if (landscape)
    {
        return landscape->PlacePoint(Vector3(centerXY));
    }

    return Vector3();
}

void DebugDrawSystem::DrawSwitchesWithDifferentLods(DAVA::Entity* entity)
{
    if (switchesWithDifferentLodsEnabled && SceneValidator::IsEntityHasDifferentLODsCount(entity))
    {
        SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());

        AABBox3 worldBox;
        AABBox3 localBox = editorScene->collisionSystem->GetUntransformedBoundingBox(entity);
        DVASSERT(!localBox.IsEmpty());
        localBox.GetTransformedBox(entity->GetWorldTransform(), worldBox);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, Color(1.0f, 0.f, 0.f, 1.f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawMeasureLaserComponent(DAVA::Entity* entity)
{
    /*
	TransformComponent * transform = entity->GetComponent<TransformComponent>();
	MeasureLaserComponent * measureLaser = entity->GetComponent<MeasureLaserComponent>();

	if (!measureLaser) return;

	Vector3 laserStart = transform->GetWorldTransform().GetTranslationVector();
	Vector3 laserEnd = MultiplyVectorMat3x3(Vector3(1.0f, 0.0f, 0.0f), transform->GetWorldTransform());
	laserEnd = laserStart + (Normalize(laserEnd) * measureLaser->GetCollisionDistance());

	GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(laserStart, laserEnd, .75f, DAVA::Color(1.0f, 0.5f, 0.2f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);


	if (measureLaser->GetCollisionDistance() < measureLaser->GetMaxLaserDistance())
	{
		if (measureLaser->GetCollision().geometry)
		{
			RayTraceCollision & collision = measureLaser->GetCollision();
			int32 i1, i2, i3;
			collision.geometry->GetIndex(collision.triangleIndex * 3 + 0, i1);
			collision.geometry->GetIndex(collision.triangleIndex * 3 + 1, i2);
			collision.geometry->GetIndex(collision.triangleIndex * 3 + 2, i3);

			Vector3 v1, v2, v3;
			collision.geometry->GetCoord(i1, v1);
			collision.geometry->GetCoord(i2, v2);
			collision.geometry->GetCoord(i3, v3);
			v1 = v1 * (*collision.renderObject->GetWorldTransformPtr());
			v2 = v2 * (*collision.renderObject->GetWorldTransformPtr());
			v3 = v3 * (*collision.renderObject->GetWorldTransformPtr());
			GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, DAVA::Color(0.0f, 0.5f, 1.0f, 1.0f));
			GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v2, v3, DAVA::Color(0.0f, 0.5f, 1.0f, 1.0f));
			GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v1, DAVA::Color(0.0f, 0.5f, 1.0f, 1.0f));
		}
	}
	*/
}