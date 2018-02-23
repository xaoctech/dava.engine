#include "ReflectionDeclaration/ReflectionDeclaration.h"
#include "ReflectionDeclaration/Private/AnyCasts.h"

#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"

#include "Engine/Engine.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Components/VisibilityCheckComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/GeoDecalComponent.h"
#include "Scene3D/Components/SlotComponent.h"
#include "Scene3D/Components/TextComponent.h"
#include "Scene3D/Components/SingleComponents/CollisionSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Entity/Component.h"
#include "Entity/ComponentManager.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Particles/ParticleEmitterInstance.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/Highlevel/BillboardRenderObject.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Material/NMaterial.h"
#include "Math/Vector.h"
#include "Math/Rect.h"
#include "Math/AABBox3.h"
#include "Math/Color.h"
#include "Math/Math.h"
#include "UI/Script/UIScriptComponent.h"
#include "UI/Script/UIScriptComponentController.h"
#include "UI/Script/Private/UILuaScriptComponentController.h"
#include "UI/UI3DView.h"
#include "UI/UIButton.h"
#include "UI/UIControl.h"
#include "UI/UIControl.h"
#include "UI/UIJoypad.h"
#include "UI/UIList.h"
#include "UI/UIListCell.h"
#include "UI/UIMovieView.h"
#include "UI/UIParticles.h"
#include "UI/UIScrollBar.h"
#include "UI/UIScrollView.h"
#include "UI/UIScrollViewContainer.h"
#include "UI/UISlider.h"
#include "UI/UISpinner.h"
#include "UI/UIStaticText.h"
#include "UI/UISwitch.h"
#include "UI/UITextField.h"
#include "UI/UIWebView.h"
#include "UI/Components/UIComponent.h"
#include "UI/Events/UIEventBindingComponent.h"
#include "UI/Events/UIMovieEventComponent.h"
#include "UI/Events/UIInputEventComponent.h"
#include "UI/Events/UIShortcutEventComponent.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Focus/UIFocusGroupComponent.h"
#include "UI/Focus/UINavigationComponent.h"
#include "UI/Focus/UITabOrderComponent.h"
#include "UI/Input/UIModalInputComponent.h"
#include "UI/Joypad/UIJoypadComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Layouts/UIAnchorSafeAreaComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UILayoutSourceRectComponent.h"
#include "UI/Layouts/UILayoutIsolationComponent.h"
#include "UI/Render/UIDebugRenderComponent.h"
#include "UI/Render/UIClipContentComponent.h"
#include "UI/Scene3D/UISceneComponent.h"
#include "UI/Scene3D/UIEntityMarkerComponent.h"
#include "UI/Scene3D/UIEntityMarkersContainerComponent.h"
#include "UI/Scroll/UIScrollBarDelegateComponent.h"
#include "UI/Sound/UISoundComponent.h"
#include "UI/Sound/UISoundValueFilterComponent.h"
#include "UI/Update/UIUpdateComponent.h"
#include "UI/Update/UICustomUpdateDeltaComponent.h"
#include "UI/RichContent/UIRichContentComponent.h"
#include "UI/RichContent/UIRichContentObjectComponent.h"
#include "UI/RichContent/UIRichContentAliasesComponent.h"
#include "UI/Scroll/UIScrollComponent.h"
#include "UI/Text/UITextComponent.h"
#include "UI/Flow/UIFlowContext.h"
#include "UI/Flow/UIFlowController.h"
#include "UI/Flow/UIFlowControllerComponent.h"
#include "UI/Flow/UIFlowService.h"
#include "UI/Flow/UIFlowStateComponent.h"
#include "UI/Flow/UIFlowTransitionComponent.h"
#include "UI/Flow/UIFlowViewComponent.h"
#include "UI/Flow/Private/UIFlowLuaController.h"
#include "UI/Flow/Services/UIFlowDataService.h"
#include "UI/Flow/Services/UIFlowEventsService.h"
#include "UI/Flow/Services/UIFlowEngineService.h"
#include "UI/Flow/Services/UIFlowSystemService.h"
#include "Utils/Random.h"

namespace DAVA
{
namespace ReflectionDeclarationDetail
{
float32 GetMinX(AABBox3* box)
{
    return box->min.x;
}

void SetMinX(AABBox3* box, float32 v)
{
    box->min.x = v;
}

float32 GetMinY(AABBox3* box)
{
    return box->min.y;
}

void SetMinY(AABBox3* box, float32 v)
{
    box->min.y = v;
}

float32 GetMinZ(AABBox3* box)
{
    return box->min.z;
}

void SetMinZ(AABBox3* box, float32 v)
{
    box->min.z = v;
}

float32 GetMaxX(AABBox3* box)
{
    return box->max.x;
}

void SetMaxX(AABBox3* box, float32 v)
{
    box->max.x = v;
}

float32 GetMaxY(AABBox3* box)
{
    return box->max.y;
}

void SetMaxY(AABBox3* box, float32 v)
{
    box->max.y = v;
}

float32 GetMaxZ(AABBox3* box)
{
    return box->max.z;
}

void SetMaxZ(AABBox3* box, float32 v)
{
    box->max.z = v;
}
}

void RegisterBaseTypes()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(int8);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(uint8);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(int16);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(uint16);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(int32);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(uint32);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(int64);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(uint64);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(float32);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(float64);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(bool);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(String);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WideString);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(FastName);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Matrix3);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Matrix2);
}

void RegisterVector2()
{
    ReflectionRegistrator<Vector2>::Begin()
    .Field("X", &Vector2::x)[M::SubProperty()]
    .Field("Y", &Vector2::y)[M::SubProperty()]
    .Method("Mul", static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(&DAVA::Mul))
    .Method("CrossProduct", static_cast<float32 (*)(const Vector2&, const Vector2&)>(&DAVA::CrossProduct))
    .Method("DotProduct", static_cast<float32 (*)(const Vector2&, const Vector2&)>(&DAVA::DotProduct))
    .Method("Normalize", static_cast<Vector2 (*)(const Vector2&)>(&DAVA::Normalize))
    .Method("Reflect", static_cast<Vector2 (*)(const Vector2&, const Vector2&)>(&DAVA::Reflect))
    .Method("Rotate", static_cast<Vector2 (*)(const Vector2&, float32)>(&DAVA::Rotate))
    .End();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Vector2);
}

void RegisterVector3()
{
    ReflectionRegistrator<Vector3>::Begin()
    .Field("X", &Vector3::x)[M::SubProperty()]
    .Field("Y", &Vector3::y)[M::SubProperty()]
    .Field("Z", &Vector3::z)[M::SubProperty()]
    .Method("Mul", static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(&DAVA::Mul))
    .Method("CrossProduct", static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(&DAVA::CrossProduct))
    .Method("DotProduct", static_cast<float32 (*)(const Vector3&, const Vector3&)>(&DAVA::DotProduct))
    .Method("Normalize", static_cast<Vector3 (*)(const Vector3&)>(&DAVA::Normalize))
    .Method("Reflect", static_cast<Vector3 (*)(const Vector3&, const Vector3&)>(&DAVA::Reflect))
    .Method("Lerp", static_cast<Vector3 (*)(const Vector3&, const Vector3&, float32)>(&DAVA::Lerp))
    .Method("Distance", static_cast<float32 (*)(const Vector3&, const Vector3&)>(&DAVA::Distance))
    .Method("PerpendicularVector", static_cast<Vector3 (*)(const Vector3&)>(&DAVA::PerpendicularVector))
    .End();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Vector3);
}

void RegisterVector4()
{
    ReflectionRegistrator<Vector4>::Begin()
    .Field("X", &Vector4::x)[M::SubProperty()]
    .Field("Y", &Vector4::y)[M::SubProperty()]
    .Field("Z", &Vector4::z)[M::SubProperty()]
    .Field("W", &Vector4::w)[M::SubProperty()]
    .Method("Mul", static_cast<Vector4 (*)(const Vector4&, const Vector4&)>(&DAVA::Mul))
    .Method("CrossProduct", static_cast<Vector4 (*)(const Vector4&, const Vector4&)>(&DAVA::CrossProduct))
    .Method("DotProduct", static_cast<float32 (*)(const Vector4&, const Vector4&)>(&DAVA::DotProduct))
    .Method("Normalize", static_cast<Vector4 (*)(const Vector4&)>(&DAVA::Normalize))
    .Method("Lerp", static_cast<Vector4 (*)(const Vector4&, const Vector4&, float32)>(&DAVA::Lerp))
    .End();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Vector4);
}

void RegisterMatrix4()
{
    ReflectionRegistrator<Matrix4>::Begin()
    .Method("MakeTranslation", &Matrix4::MakeTranslation)
    .Method("MakeRotation", &Matrix4::MakeRotation)
    .Method("MakeScale", &Matrix4::MakeScale)
    .Method("GetTranslationVector", &Matrix4::GetTranslationVector)
    .Method("GetScaleVector", &Matrix4::GetScaleVector)
    .End();
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Matrix4);
}

void RegisterRect()
{
    ReflectionRegistrator<Rect>::Begin()
    .Field("X", &Rect::x)[M::SubProperty()]
    .Field("Y", &Rect::y)[M::SubProperty()]
    .Field("Width", &Rect::dx)[M::SubProperty()]
    .Field("Height", &Rect::dy)[M::SubProperty()]
    .End();
}

void RegisterAABBox3()
{
    ReflectionRegistrator<AABBox3>::Begin()
    .Field("MinX", &ReflectionDeclarationDetail::GetMinX, &ReflectionDeclarationDetail::SetMinX)[M::SubProperty()]
    .Field("MinY", &ReflectionDeclarationDetail::GetMinY, &ReflectionDeclarationDetail::SetMinY)[M::SubProperty()]
    .Field("MinZ", &ReflectionDeclarationDetail::GetMinZ, &ReflectionDeclarationDetail::SetMinZ)[M::SubProperty()]
    .Field("MaxX", &ReflectionDeclarationDetail::GetMaxX, &ReflectionDeclarationDetail::SetMaxX)[M::SubProperty()]
    .Field("MaxY", &ReflectionDeclarationDetail::GetMaxY, &ReflectionDeclarationDetail::SetMaxY)[M::SubProperty()]
    .Field("MaxZ", &ReflectionDeclarationDetail::GetMaxZ, &ReflectionDeclarationDetail::SetMaxZ)[M::SubProperty()]
    .End();
}

void RegisterColor()
{
    ReflectionRegistrator<Color>::Begin()
    .Field("R", &Color::r)
    .Field("G", &Color::g)
    .Field("B", &Color::b)
    .Field("A", &Color::a)
    .End();
}

void RegisterIntegerMath()
{
    ReflectionRegistrator<IntegerMath>::Begin()
    .Method("Inc", [](int32 a) { return ++a; })
    .Method("Dec", [](int32 a) { return --a; })
    .Method("Sum", [](int32 a, int32 b) { return a + b; })
    .Method("Sub", [](int32 a, int32 b) { return a - b; })
    .Method("Mul", [](int32 a, int32 b) { return a * b; })
    .Method("Div", [](int32 a, int32 b) { return a / b; })
    .End();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(IntegerMath);
}

void RegisterFloatMath()
{
    ReflectionRegistrator<FloatMath>::Begin()
    .Method("Sum", [](float32 a, float32 b) { return a + b; })
    .Method("Sub", [](float32 a, float32 b) { return a - b; })
    .Method("Mul", [](float32 a, float32 b) { return a * b; })
    .Method("Div", [](float32 a, float32 b) { return a / b; })
    .End();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(FloatMath);
}
template <class T>
bool CompareFuncEqual(const T& a, const T& b)
{
    return a == b;
}
template <class T>
bool CompareFuncNEqual(const T& a, const T& b)
{
    return a != b;
}
template <class T>
bool CompareFuncLess(const T& a, const T& b)
{
    return a < b;
}
template <class T>
bool CompareFuncLEqual(const T& a, const T& b)
{
    return a <= b;
}
template <class T>
bool CompareFuncGreater(const T& a, const T& b)
{
    return a > b;
}
template <class T>
bool CompareFuncGEqual(const T& a, const T& b)
{
    return a >= b;
}
class ConditionsInt
{
};
class ConditionsFloat
{
};

void RegisterConditions()
{
    ReflectionRegistrator<ConditionsInt>::Begin()
    .Method("==", &CompareFuncEqual<int32>)
    .Method("!=", &CompareFuncNEqual<int32>)
    .Method("<", &CompareFuncLess<int32>)
    .Method("<=", &CompareFuncLEqual<int32>)
    .Method(">", &CompareFuncGreater<int32>)
    .Method(">=", &CompareFuncGEqual<int32>)
    .End();
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ConditionsInt);

    ReflectionRegistrator<ConditionsFloat>::Begin()
    .Method("==", &CompareFuncEqual<float32>)
    .Method("!=", &CompareFuncNEqual<float32>)
    .Method("<", &CompareFuncLess<float32>)
    .Method("<=", &CompareFuncLEqual<float32>)
    .Method(">", &CompareFuncGreater<float32>)
    .Method(">=", &CompareFuncGEqual<float32>)
    .End();
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ConditionsFloat);
}

void RegisterRandom()
{
    ReflectionRegistrator<Random>::Begin()
    .Method("Random", [](uint32 n) { return Random::Instance()->Rand(n); })
    .Method("RandomFloat", []() { return Random::Instance()->RandFloat(); })
    .Method("RandomVector2", []() { return Vector2(static_cast<float32>(Random::Instance()->RandFloat()),
                                                   static_cast<float32>(Random::Instance()->RandFloat())); })
    .Method("RandomVector3", []() { return Vector3(static_cast<float32>(Random::Instance()->RandFloat()),
                                                   static_cast<float32>(Random::Instance()->RandFloat()),
                                                   static_cast<float32>(Random::Instance()->RandFloat())); })
    .Method("RandomVector4", []() { return Vector4(static_cast<float32>(Random::Instance()->RandFloat()),
                                                   static_cast<float32>(Random::Instance()->RandFloat()),
                                                   static_cast<float32>(Random::Instance()->RandFloat()),
                                                   static_cast<float32>(Random::Instance()->RandFloat())); })
    .End();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Random);
}

void RegisterPermanentNames()
{
    // Engine classes
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Engine);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EngineContext);

    // Common classes
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BaseObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(KeyedArchive);

    // 3D classes
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Component);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RotationControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapToLandscapeControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WASDControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EdgeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PathComponent);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(PathComponent::Waypoint, "Waypoint");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(PathComponent::Edge, "Edge");
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaypointComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ActionComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(AnimationComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BulletComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CameraComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CustomPropertiesComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DebugRenderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LightComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleEffectComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleEmitterInstance);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleLayer);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleForce);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleForceSimplified);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(QualitySettingsComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SkeletonComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MotionComponent);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(SkeletonComponent::Joint, "Joint");
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SoundComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SoundComponentElement);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedTreeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionDataComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionDebugDrawComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SwitchComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TransformComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UpdatableComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UserComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisibilityCheckComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaveComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WindComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LodComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SlotComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TextComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ActionComponent::Action);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PolygonGroup);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderBatchWithOptions);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LandscapeSubdivision);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LandscapeSubdivision::SubdivisionMetrics);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderBatch);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VegetationRenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BillboardRenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Heightmap);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Landscape);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Light);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedTreeObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Entity);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GeoDecalComponent);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(PartilceEmitterLoadProxy, "ParticleEmitter3D");

    // Systems
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SceneSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TransformSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderUpdateSystem);

    // Components related stuff
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CollisionInfo);

    // UI controls
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UI3DView);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIButton);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIControl);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIJoypad);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIList);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIListCell);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIMovieView);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIParticles);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScrollBar);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScrollView);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScrollViewContainer);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISlider);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISpinner);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIStaticText);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UISwitch);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UITextField);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIWebView);

    // UI components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIComponent);

// clang-format off
#define DECL_UI_COMPONENT(type, string) \
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(type, string); \
    GetEngineContext()->componentManager->RegisterComponent<type>()
    // clang-format on

    DECL_UI_COMPONENT(UILinearLayoutComponent, "LinearLayout");
    DECL_UI_COMPONENT(UIFlowLayoutComponent, "FlowLayout");
    DECL_UI_COMPONENT(UIFlowLayoutHintComponent, "FlowLayoutHint");
    DECL_UI_COMPONENT(UIIgnoreLayoutComponent, "IgnoreLayout");
    DECL_UI_COMPONENT(UISizePolicyComponent, "SizePolicy");
    DECL_UI_COMPONENT(UIAnchorComponent, "Anchor");
    DECL_UI_COMPONENT(UIAnchorSafeAreaComponent, "UIAnchorSafeAreaComponent");
    DECL_UI_COMPONENT(UILayoutSourceRectComponent, "UILayoutSourceRectComponent");
    DECL_UI_COMPONENT(UILayoutIsolationComponent, "UILayoutIsolationComponent");
    DECL_UI_COMPONENT(UIControlBackground, "Background");
    DECL_UI_COMPONENT(UIModalInputComponent, "ModalInput");
    DECL_UI_COMPONENT(UIFocusComponent, "Focus");
    DECL_UI_COMPONENT(UIFocusGroupComponent, "FocusGroup");
    DECL_UI_COMPONENT(UINavigationComponent, "Navigation");
    DECL_UI_COMPONENT(UITabOrderComponent, "TabOrder");
    DECL_UI_COMPONENT(UIEventBindingComponent, "UIEventBindingComponent");
    DECL_UI_COMPONENT(UIInputEventComponent, "UIInputEventComponent");
    DECL_UI_COMPONENT(UIMovieEventComponent, "UIMovieEventComponent");
    DECL_UI_COMPONENT(UIShortcutEventComponent, "UIShortcutEventComponent");
    DECL_UI_COMPONENT(UIScrollBarDelegateComponent, "ScrollBarDelegate");
    DECL_UI_COMPONENT(UIScrollComponent, "ScrollComponent");
    DECL_UI_COMPONENT(UISoundComponent, "Sound");
    DECL_UI_COMPONENT(UISoundValueFilterComponent, "SoundValueFilter");
    DECL_UI_COMPONENT(UIUpdateComponent, "Update");
    DECL_UI_COMPONENT(UICustomUpdateDeltaComponent, "CustomDeltaUpdate");
    DECL_UI_COMPONENT(UIRichContentComponent, "RichContent");
    DECL_UI_COMPONENT(UIRichContentAliasesComponent, "RichContentAliases");
    DECL_UI_COMPONENT(UIRichContentObjectComponent, "RichContentObject");
    DECL_UI_COMPONENT(UIDebugRenderComponent, "DebugRender");
    DECL_UI_COMPONENT(UIClipContentComponent, "ClipContent");
    DECL_UI_COMPONENT(UISceneComponent, "SceneComponent");
    DECL_UI_COMPONENT(UIEntityMarkerComponent, "UIEntityMarkerComponent");
    DECL_UI_COMPONENT(UIEntityMarkersContainerComponent, "UIEntityMarkersContainerComponent");
    DECL_UI_COMPONENT(UITextComponent, "UITextComponent");
    DECL_UI_COMPONENT(UIScriptComponent, "UIScriptComponent");

    DECL_UI_COMPONENT(UIFlowControllerComponent, "UIFlowControllerComponent");
    DECL_UI_COMPONENT(UIFlowStateComponent, "UIFlowStateComponent");
    DECL_UI_COMPONENT(UIFlowTransitionComponent, "UIFlowTransitionComponent");
    DECL_UI_COMPONENT(UIFlowViewComponent, "UIFlowViewComponent");
    DECL_UI_COMPONENT(UIJoypadComponent, "UIJoypadComponent");

#undef DECL_UI_COMPONENT

    // Script types
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIScriptComponentController);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UILuaScriptComponentController);
    // Flow base types
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowContext);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowController);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowLuaController);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowService);
    // Flow services
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowDataService);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowEngineService);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowEventsService);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UIFlowSystemService);
}

void RegisterReflectionForBaseTypes()
{
    RegisterAnyCasts();

    RegisterBaseTypes();
    RegisterVector2();
    RegisterVector3();
    RegisterVector4();
    RegisterMatrix4();
    RegisterRect();
    RegisterAABBox3();
    RegisterColor();

    RegisterIntegerMath();
    RegisterFloatMath();
    RegisterConditions();
    RegisterRandom();

    RegisterPermanentNames();
}
} // namespace DAVA
