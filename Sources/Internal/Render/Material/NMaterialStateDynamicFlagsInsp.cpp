#include "Render/Material/NMaterial.h"
#include "Render/Material/FXCache.h"
#include "Render/Material/NMaterialStateDynamicFlagsInsp.h"

#include "Base/GlobalEnum.h"

enum eNormalBlendMode
{
    NormalBlendUDN = 0,
    NormalBlendPartial_Derivative = 1,
    NormalBlendWhiteout = 2,
    NormalBlendOverlay = 3,
    NormalBlendLinear = 4,
    NormalBlendReorientedNormal = 5
};

enum eAlbedoTintBlendMode
{
    AlbedoBlendNone = 0,
    AlbedoBlendMultiply = 1,
    AlbedoBlendSoftLight = 2
};

enum eLandscapeHeightBlendMode
{
    LandscapeBlendNone = 0,
    LandscapeBlendDTE,
    LandscapeBlendLinear
};

ENUM_DECLARE(eNormalBlendMode)
{
    ENUM_ADD(NormalBlendUDN);
    ENUM_ADD(NormalBlendPartial_Derivative);
    ENUM_ADD(NormalBlendWhiteout);
    ENUM_ADD(NormalBlendOverlay);
    ENUM_ADD(NormalBlendLinear);
    ENUM_ADD(NormalBlendReorientedNormal);
}

ENUM_DECLARE(eAlbedoTintBlendMode)
{
    ENUM_ADD(AlbedoBlendNone);
    ENUM_ADD(AlbedoBlendMultiply);
    ENUM_ADD(AlbedoBlendSoftLight);
}

ENUM_DECLARE(eLandscapeHeightBlendMode)
{
    ENUM_ADD(LandscapeBlendNone);
    ENUM_ADD(LandscapeBlendDTE);
    ENUM_ADD(LandscapeBlendLinear);
}

namespace DAVA
{
namespace NMaterialStateDynamicFlagsInspDetails
{
Array<FastName, 3> enumProps =
{ {
NMaterialFlagName::FLAG_NORMAL_BLEND_MODE,
NMaterialFlagName::FLAG_ALBEDO_TINT_BLEND_MODE,
NMaterialFlagName::FLAG_BLEND_LANDSCAPE_HEIGHT
} };
}
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicFlagsInsp implementation

InspInfoDynamic::DynamicData NMaterialStateDynamicFlagsInsp::Prepare(void* object, int filter) const
{
    InspInfoDynamic::DynamicData ddata;
    ddata.object = object;

    return ddata;
}

Vector<FastName> NMaterialStateDynamicFlagsInsp::MembersList(const DynamicData& ddata) const
{
    static Vector<FastName> ret;

    if (ret.empty())
    {
        ret.reserve(38);

        ret.emplace_back(NMaterialFlagName::FLAG_VERTEXFOG);
        ret.emplace_back(NMaterialFlagName::FLAG_FOG_LINEAR);
        ret.emplace_back(NMaterialFlagName::FLAG_FOG_HALFSPACE);
        ret.emplace_back(NMaterialFlagName::FLAG_FOG_HALFSPACE_LINEAR);
        ret.emplace_back(NMaterialFlagName::FLAG_FOG_ATMOSPHERE);

        ret.emplace_back(NMaterialFlagName::FLAG_FLATCOLOR);
        ret.emplace_back(NMaterialFlagName::FLAG_FLATALBEDO);
        ret.emplace_back(NMaterialFlagName::FLAG_TEXTURESHIFT);
        ret.emplace_back(NMaterialFlagName::FLAG_TEXTURE0_ANIMATION_SHIFT);

        ret.emplace_back(NMaterialFlagName::FLAG_WAVE_ANIMATION);
        ret.emplace_back(NMaterialFlagName::FLAG_FAST_NORMALIZATION);

        ret.emplace_back(NMaterialFlagName::FLAG_SPECULAR);
        ret.emplace_back(NMaterialFlagName::FLAG_SEPARATE_NORMALMAPS);
        ret.emplace_back(NMaterialFlagName::FLAG_TANGENT_SPACE_WATER_REFLECTIONS);
        ret.emplace_back(NMaterialFlagName::FLAG_DEBUG_UNITY_Z_NORMAL);
        ret.emplace_back(NMaterialFlagName::FLAG_DEBUG_Z_NORMAL_SCALE);
        ret.emplace_back(NMaterialFlagName::FLAG_DEBUG_NORMAL_ROTATION);
        ret.emplace_back(NMaterialFlagName::FLAG_TEST_OCCLUSION);
        ret.emplace_back(NMaterialFlagName::FLAG_TILED_DECAL_MASK);
        ret.emplace_back(NMaterialFlagName::FLAG_TILED_DECAL_ROTATION);
        ret.emplace_back(NMaterialFlagName::FLAG_ALPHATESTVALUE);
        ret.emplace_back(NMaterialFlagName::FLAG_ALPHASTEPVALUE);
        ret.emplace_back(NMaterialFlagName::FLAG_FORCED_SHADOW_DIRECTION);

        ret.emplace_back(NMaterialFlagName::FLAG_ILLUMINATION_USED);
        ret.emplace_back(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
        ret.emplace_back(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);

        ret.emplace_back(NMaterialFlagName::FLAG_TRANSMITTANCE);
        ret.emplace_back(NMaterialFlagName::FLAG_HARD_SKINNING);
        ret.emplace_back(NMaterialFlagName::FLAG_SOFT_SKINNING);
        ret.emplace_back(NMaterialFlagName::FLAG_USE_BAKED_LIGHTING);
        ret.emplace_back(NMaterialFlagName::FLAG_ALBEDO_ALPHA_MASK);

        ret.emplace_back(NMaterialFlagName::FLAG_VERTEX_BLEND_TEXTURES);
        ret.emplace_back(NMaterialFlagName::FLAG_VERTEX_BLEND_4_TEXTURES);

        ret.emplace_back(NMaterialFlagName::FLAG_ALBEDO_TINT_BLEND_MODE);

        ret.emplace_back(NMaterialFlagName::FLAG_NORMAL_BLEND_MODE);

        ret.emplace_back(NMaterialFlagName::FLAG_USE_DETAIL_NORMAL_AO);

        ret.emplace_back(NMaterialFlagName::FLAG_BLEND_LANDSCAPE_HEIGHT);
        ret.emplace_back(NMaterialFlagName::FLAG_RGBM_INPUT);
        ret.emplace_back(NMaterialFlagName::FLAG_FULL_NORMAL);
    }

    return ret;
}

InspDesc NMaterialStateDynamicFlagsInsp::MemberDesc(const DynamicData& ddata, const FastName& member) const
{
    InspDesc descr(member.c_str());
    if (member == NMaterialFlagName::FLAG_NORMAL_BLEND_MODE)
    {
        descr.enumMap = GlobalEnumMap<eNormalBlendMode>::Instance();
    }
    else if (member == NMaterialFlagName::FLAG_ALBEDO_TINT_BLEND_MODE)
    {
        descr.enumMap = GlobalEnumMap<eAlbedoTintBlendMode>::Instance();
    }
    else if (member == NMaterialFlagName::FLAG_BLEND_LANDSCAPE_HEIGHT)
    {
        descr.enumMap = GlobalEnumMap<eLandscapeHeightBlendMode>::Instance();
    }
    return descr;
}

VariantType NMaterialStateDynamicFlagsInsp::MemberValueGet(const DynamicData& ddata, const FastName& member) const
{
    NMaterial* material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    VariantType ret;
    auto it = std::find(NMaterialStateDynamicFlagsInspDetails::enumProps.begin(), NMaterialStateDynamicFlagsInspDetails::enumProps.end(), member);
    if (it != NMaterialStateDynamicFlagsInspDetails::enumProps.end())
    {
        ret.SetInt32(material->GetEffectiveFlagValue(member));
    }
    else
    {
        ret.SetBool(0 != material->GetEffectiveFlagValue(member));
    }
    return ret;
}

void NMaterialStateDynamicFlagsInsp::MemberValueSet(const DynamicData& ddata, const FastName& member, const VariantType& value)
{
    NMaterial* material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    // empty value is thread as flag remove
    if (value.GetType() == VariantType::TYPE_NONE)
    {
        if (material->HasLocalFlag(member))
        {
            material->RemoveFlag(member);
        }
    }
    else
    {
        int32 newValue = 0;
        if ((value.GetType() == VariantType::TYPE_BOOLEAN) && value.AsBool())
        {
            newValue = 1;
        }
        auto it = std::find(NMaterialStateDynamicFlagsInspDetails::enumProps.begin(), NMaterialStateDynamicFlagsInspDetails::enumProps.end(), member);
        if (it != NMaterialStateDynamicFlagsInspDetails::enumProps.end() && value.GetType() == VariantType::TYPE_INT32)
        {
            newValue = value.AsInt32();
        }

        if (material->HasLocalFlag(member))
        {
            material->SetFlag(member, newValue);
        }
        else
        {
            material->AddFlag(member, newValue);
        }
    }
}

DAVA::VariantType NMaterialStateDynamicFlagsInsp::MemberAliasGet(const DynamicData& ddata, const FastName& member) const
{
    InspDesc descr = MemberDesc(ddata, member);
    DAVA::VariantType value = MemberValueGet(ddata, member);

    if (descr.enumMap == nullptr || value.GetType() != DAVA::VariantType::TYPE_INT32)
    {
        return DAVA::VariantType();
    }

    return DAVA::VariantType(DAVA::String(descr.enumMap->ToString(value.AsInt32())));
}

int NMaterialStateDynamicFlagsInsp::MemberFlags(const DynamicData& ddata, const FastName& member) const
{
    NMaterial* material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    return I_VIEW | (material->HasLocalFlag(member) ? I_EDIT : 0);
}
};
