#include "QualityPreferences.h"

#include <Scene3D/Systems/QualitySettingsSystem.h>
#include <FileSystem/KeyedArchive.h>

namespace QualityPreferences
{
DAVA::String SETTNING_RENDER_FLOW = "Render/Flow";
DAVA::String SETTING_QUALITY_MATERIALS = "Quality/Material/";
DAVA::String SETTING_QUALITY_PARTICLE = "Quality/Particle";
DAVA::String SETTING_QUALITY_OPTIONS = "Quality/Options/";

void LoadFromSettings(Settings& appSettings)
{
    using namespace DAVA;

    QualitySettingsSystem* qs = QualitySettingsSystem::Instance();

    const KeyedArchive* settings = appSettings.GetQualitySettings();
    if (settings != nullptr)
    {
        uint32 flow = settings->GetUInt32(SETTNING_RENDER_FLOW, static_cast<uint32>(Renderer::GetCurrentRenderFlow()));
        if (Renderer::IsRenderFlowSupported(static_cast<RenderFlow>(flow)))
        {
            Renderer::SetRenderFlow(static_cast<RenderFlow>(flow));
        }

        for (uint32 i = 0; i < QualityGroup::Count; ++i)
        {
            QualityGroup group = static_cast<QualityGroup>(i);
            VariantType* value = settings->GetVariant(qs->GetQualityGroupName(group).c_str());
            if ((value != nullptr) && (value->GetType() == VariantType::TYPE_FASTNAME) && qs->HasQualityInGroup(group, value->AsFastName()))
            {
                qs->SetCurrentQualityForGroup(group, value->AsFastName());
            }
        }

        VariantType* value = settings->GetVariant(SETTING_QUALITY_MATERIALS);
        if (value != nullptr && value->GetType() == VariantType::TYPE_KEYED_ARCHIVE)
        {
            KeyedArchive* materialQualities = value->AsKeyedArchive();
            KeyedArchive::UnderlyingMap data = materialQualities->GetArchieveData();
            for (KeyedArchive::UnderlyingMap::value_type& entry : data)
            {
                FastName storedGroupName(entry.first);

                size_t count = qs->GetMaterialQualityGroupCount();
                for (size_t i = 0; i < count; ++i)
                {
                    FastName groupName = qs->GetMaterialQualityGroupName(i);
                    if (groupName == storedGroupName)
                    {
                        VariantType* value = entry.second;
                        if (value->GetType() == VariantType::TYPE_FASTNAME)
                        {
                            if (qs->GetMaterialQuality(groupName, value->AsFastName()) != nullptr)
                            {
                                qs->SetCurMaterialQuality(groupName, value->AsFastName());
                            }
                        }
                    }
                }
            }
        }

        value = settings->GetVariant(SETTING_QUALITY_PARTICLE);
        if (value != nullptr && value->GetType() == VariantType::TYPE_FASTNAME)
        {
            ParticlesQualitySettings& particlesSettings = qs->GetParticlesQualitySettings();
            if (particlesSettings.GetQualityIndex(value->AsFastName()) != -1)
            {
                particlesSettings.SetCurrentQuality(value->AsFastName());
            }
        }

        value = settings->GetVariant(SETTING_QUALITY_OPTIONS);
        if (value != nullptr && value->GetType() == VariantType::TYPE_KEYED_ARCHIVE)
        {
            KeyedArchive* options = value->AsKeyedArchive();
            KeyedArchive::UnderlyingMap data = options->GetArchieveData();
            for (KeyedArchive::UnderlyingMap::value_type& entry : data)
            {
                FastName storedOptionName(entry.first);
                VariantType* value = entry.second;

                if (value->GetType() == VariantType::TYPE_BOOLEAN)
                {
                    for (int32 i = 0; i < qs->GetOptionsCount(); ++i)
                    {
                        if (qs->GetOptionName(i) == storedOptionName)
                        {
                            qs->EnableOption(storedOptionName, value->AsBool());
                            break;
                        }
                    }
                }
            }
        }
    }
}

void SaveToSettings(Settings& appSettings)
{
    using namespace DAVA;

    QualitySettingsSystem* qs = QualitySettingsSystem::Instance();

    ScopedPtr<KeyedArchive> archive(new KeyedArchive());

    archive->SetUInt32(SETTNING_RENDER_FLOW, static_cast<uint32>(Renderer::GetCurrentRenderFlow()));

    for (uint32 i = 0; i < QualityGroup::Count; ++i)
    {
        QualityGroup group = static_cast<QualityGroup>(i);
        archive->SetFastName(qs->GetQualityGroupName(group).c_str(), qs->GetCurrentQualityForGroup(group));
    }

    archive->SetArchive(SETTING_QUALITY_MATERIALS, new KeyedArchive);
    KeyedArchive* materialsArchive = archive->GetArchive(SETTING_QUALITY_MATERIALS);
    for (size_t i = 0; i < qs->GetMaterialQualityGroupCount(); ++i)
    {
        FastName groupName = qs->GetMaterialQualityGroupName(i);
        FastName groupValue = qs->GetCurMaterialQuality(groupName);
        materialsArchive->SetFastName(groupName.c_str(), groupValue);
    }

    const ParticlesQualitySettings& particlesSettings = qs->GetParticlesQualitySettings();
    if (particlesSettings.GetQualitiesCount() > 0)
    {
        archive->SetFastName(SETTING_QUALITY_PARTICLE, particlesSettings.GetCurrentQuality());
    }

    archive->SetArchive(SETTING_QUALITY_OPTIONS, new KeyedArchive);
    KeyedArchive* optionsArchive = archive->GetArchive(SETTING_QUALITY_OPTIONS);
    for (int32 i = 0; i < qs->GetOptionsCount(); ++i)
    {
        FastName optionName = qs->GetOptionName(i);
        bool optionValue = qs->IsOptionEnabled(optionName);
        optionsArchive->SetBool(optionName.c_str(), optionValue);
    }

    appSettings.SetQualitySettings(archive);
}
}
