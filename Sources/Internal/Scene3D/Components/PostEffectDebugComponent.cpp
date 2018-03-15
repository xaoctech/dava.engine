#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "Scene3D/Components/PostEffectDebugComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(PostEffectDebugComponent)
{
    ReflectionRegistrator<PostEffectDebugComponent>::Begin()
    .ConstructorByPointer()
    .Field("enableLightMeter", &PostEffectDebugComponent::GetLightMeterEnabled, &PostEffectDebugComponent::SetLightMeterEnabled)[M::DisplayName("Light Meter")]
    .Field("enableLightMeterMask", &PostEffectDebugComponent::GetLightMeterMaskEnabled, &PostEffectDebugComponent::SetLightMeterMaskEnabled)[M::DisplayName("Draw Light Meter Mask")]
    .Field("drawHDRTarget", &PostEffectDebugComponent::GetDrawHDRTarget, &PostEffectDebugComponent::SetDrawHDRTarget)[M::DisplayName("Draw HDR image")]
    .Field("drawLuminance", &PostEffectDebugComponent::GetDrawLuminance, &PostEffectDebugComponent::SetDrawLuminance)[M::DisplayName("Draw Luminance Chain")]
    .Field("drawAdaptation", &PostEffectDebugComponent::GetDrawAdaptataion, &PostEffectDebugComponent::SetDrawAdaptation)[M::DisplayName("Draw Adaptation")]
    .Field("drawHistogram", &PostEffectDebugComponent::GetDrawHistogram, &PostEffectDebugComponent::SetDrawHistogram)[M::DisplayName("Draw Histogram")]
    .Field("drawBloom", &PostEffectDebugComponent::GetDrawBloom, &PostEffectDebugComponent::SetDrawBloom)[M::DisplayName("Draw Bloom")]
    .Field("debugRectOffset", &PostEffectDebugComponent::GetDebugRectOffset, &PostEffectDebugComponent::SetDebugRectOffset)[M::DisplayName("Debug Rect Offset")]
    .Field("debugRectSize", &PostEffectDebugComponent::GetDebugRectSize, &PostEffectDebugComponent::SetDebugRectSize)[M::DisplayName("Debug Rect Size")]
    .Field("taaSampleIndex", &PostEffectDebugComponent::GetTAASampleIndex, &PostEffectDebugComponent::SetTAASampleIndex)[M::DisplayName("TAA Sample Index")]
    .End();
}

Component* PostEffectDebugComponent::Clone(Entity* toEntity)
{
    PostEffectDebugComponent* component = new PostEffectDebugComponent();
    component->SetEntity(toEntity);

    component->drawHDRTarget = drawHDRTarget;
    component->drawLuminance = drawLuminance;
    component->drawAdaptation = drawAdaptation;
    component->debugRectOffset = debugRectOffset;
    component->debugRectSize = debugRectSize;
    component->drawHistogram = drawHistogram;
    component->drawBloom = drawBloom;
    component->enableLightMeter = enableLightMeter;
    component->drawLightMeterMask = drawLightMeterMask;

    return component;
}

void PostEffectDebugComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        archive->SetBool("pe.debug.drawHDRTarget", drawHDRTarget);
        archive->SetBool("pe.debug.drawLuminance", drawLuminance);
        archive->SetBool("pe.debug.drawAdaptation", drawAdaptation);
        archive->SetVector2("pe.debug.debugRectOffset", debugRectOffset);
        archive->SetInt32("pe.debug.debugRectSize", debugRectSize);
        archive->SetBool("pe.debug.drawHistogram", drawHistogram);
        archive->SetBool("pe.debug.drawBloom", drawBloom);
        archive->SetBool("pe.debug.lightMeter", enableLightMeter);
        archive->SetBool("pe.debug.lightMeterMask", drawLightMeterMask);
    }
}

void PostEffectDebugComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        drawHDRTarget = archive->GetBool("pe.debug.drawHDRTarget", false);
        drawLuminance = archive->GetBool("pe.debug.drawLuminance", false);
        drawAdaptation = archive->GetBool("pe.debug.drawAdaptation", false);
        debugRectOffset = archive->GetVector2("pe.debug.debugRectOffset");
        debugRectSize = archive->GetInt32("pe.debug.debugRectSize", 128);
        drawHistogram = archive->GetBool("pe.debug.drawHistogram", false);
        drawBloom = archive->GetBool("pe.debug.drawBloom", false);
        enableLightMeter = archive->GetBool("pe.debug.lightMeter", false);
        drawLightMeterMask = archive->GetBool("pe.debug.lightMeterMask", false);
    }
    Component::Deserialize(archive, serializationContext);
}

void PostEffectDebugComponent::SetDrawHDRTarget(bool draw)
{
    drawHDRTarget = draw;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

bool PostEffectDebugComponent::GetDrawHDRTarget() const
{
    return drawHDRTarget;
}

void PostEffectDebugComponent::SetDrawLuminance(bool draw)
{
    drawLuminance = draw;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

bool PostEffectDebugComponent::GetDrawLuminance() const
{
    return drawLuminance;
}

void PostEffectDebugComponent::SetDebugRectOffset(const Vector2& rect)
{
    debugRectOffset = rect;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

Vector2 PostEffectDebugComponent::GetDebugRectOffset() const
{
    return debugRectOffset;
}

void PostEffectDebugComponent::SetDebugRectSize(const int32& size)
{
    debugRectSize = size;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

int32 PostEffectDebugComponent::GetDebugRectSize()
{
    return debugRectSize;
}

void PostEffectDebugComponent::SetDrawAdaptation(bool draw)
{
    drawAdaptation = draw;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

bool PostEffectDebugComponent::GetDrawAdaptataion() const
{
    return drawAdaptation;
}

void PostEffectDebugComponent::SetDrawHistogram(bool draw)
{
    drawHistogram = draw;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

bool PostEffectDebugComponent::GetDrawHistogram() const
{
    return drawHistogram;
}

void PostEffectDebugComponent::SetDrawBloom(bool draw)
{
    drawBloom = draw;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

bool PostEffectDebugComponent::GetDrawBloom() const
{
    return drawBloom;
}

bool PostEffectDebugComponent::GetLightMeterEnabled() const
{
    return enableLightMeter;
}

void PostEffectDebugComponent::SetLightMeterEnabled(bool value)
{
    enableLightMeter = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

bool PostEffectDebugComponent::GetLightMeterMaskEnabled() const
{
    return drawLightMeterMask;
}

void PostEffectDebugComponent::SetLightMeterMaskEnabled(bool value)
{
    drawLightMeterMask = value;
    GlobalEventSystem::Instance()->Event(this, EventSystem::POSTEFFECT_DEBUG_CHANGED);
}

int32 PostEffectDebugComponent::GetTAASampleIndex() const
{
    return Renderer::GetOptions()->GetOptionValue(RenderOptions::TAA_SAMPLE_INDEX);
}

void PostEffectDebugComponent::SetTAASampleIndex(int32 value)
{
    Renderer::GetOptions()->SetOptionValue(RenderOptions::TAA_SAMPLE_INDEX, value);
}
};
