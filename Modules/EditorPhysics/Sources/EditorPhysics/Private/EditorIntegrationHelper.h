#pragma once

namespace DAVA
{
class Scene;
class DataContext;
} // namespace DAVA

namespace EditorPhysicsDetail
{
DAVA::Scene* ExtractScene(DAVA::DataContext* context);
} // namespace EditorPhysics
