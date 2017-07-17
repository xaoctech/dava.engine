#pragma once

namespace DAVA
{
class Scene;
namespace TArc
{
class DataContext;
} // namespace TArc
} // namespace DAVA

namespace EditorPhysicsDetail
{
DAVA::Scene* ExtractScene(DAVA::TArc::DataContext* context);

} // namespace EditorPhysics
