#ifndef __DAVAENGINE_DYNAMIC_BINDINGS_H__
#define __DAVAENGINE_DYNAMIC_BINDINGS_H__

#include "Math/Color.h"
#include "Math/Matrix4.h"
#include "Math/Vector.h"
#include "Base/FastName.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
class DynamicBindings
{
public:
    enum eUniformSemantic
    {
        UNKNOWN_SEMANTIC = 0,

        PARAM_WORLD,
        PARAM_INV_WORLD,
        PARAM_WORLD_INV_TRANSPOSE,
        PARAM_VIEW,
        PARAM_INV_VIEW,
        PARAM_PROJ,
        PARAM_INV_PROJ,

        PARAM_WORLD_VIEW,
        PARAM_INV_WORLD_VIEW,
        PARAM_WORLD_VIEW_INV_TRANSPOSE, //NORMAL, // NORMAL MATRIX

        PARAM_VIEW_PROJ,
        PARAM_INV_VIEW_PROJ,

        PARAM_WORLD_VIEW_PROJ,
        PARAM_INV_WORLD_VIEW_PROJ,

        PARAM_GLOBAL_TIME,
        PARAM_WORLD_SCALE,

        PARAM_CAMERA_POS,
        PARAM_CAMERA_DIR,
        PARAM_CAMERA_UP,

        PARAM_LIGHT0_POSITION,
        PARAM_LIGHT0_COLOR,

        PARAM_STATIC_SHADOW_AO_UV,
        PARAM_STATIC_SHADOW_AO_SIZE,

        PARAM_LOCAL_BOUNDING_BOX,
        PARAM_WORLD_VIEW_OBJECT_CENTER,
        PARAM_BOUNDING_BOX_SIZE,

        PARAM_JOINT_POSITIONS,
        PARAM_JOINT_QUATERNIONS,
        PARAM_JOINTS_COUNT, //it will not be bound into shader, but will be used to bind joints

        PARAM_VIEWPORT_SIZE,
        PARAM_RCP_VIEWPORT_SIZE, // = 1/PARAM_VIEWPORT_SIZE
        PARAM_VIEWPORT_OFFSET,

        PARAM_LANDSCAPE_HEIGHTMAP_SIZE,
        PARAM_LANDSCAPE_HEIGHTMAP_SIZE_POW2,

        PARAM_SHADOW_COLOR,
        PARAM_WATER_CLEAR_COLOR,

        PARAM_SHADOW_VIEW,
        PARAM_SHADOW_PARAMETERS,
        PARAM_Z_NEAR_FAR,

        /*
         * Shadowmapping-v2 bindings
         */
        PARAM_SHADOW_PROJECTION_SCALE,
        PARAM_SHADOW_PROJECTION_OFFSET,
        PARAM_SHADOW_LIGHTING_PARAMETERS,
        PARAM_SHADOW_PARAMS,

        PARAM_POINT_LIGHTS,
        PARAM_POINT_SHADOW_MAP_FACE_SIZE,

        LOCAL_PROBE_CAPTURE_POSITION_IN_WORLDSPACE,
        LOCAL_PROBE_CAPTURE_WORLD_TO_LOCAL_MATRIX,

        PARAM_WIND,
        PARAM_FLEXIBILITY,

        PARAM_PROJECTION_FLIPPED,
        PARAM_RENDER_TARGET_SIZE,

        PARAM_FOG_VALUES /* distance scale, turbidity, anisotropy, unused */,

        PARAM_GLOBAL_DIFFUSE_SPHERICAL_HARMONICS,

        PARAM_GLOBAL_LUMINANCE_SCALE,

        PARAM_VT_PAGE_INFO,
        PARAM_VT_POS,
        PARAM_VT_BASIS,

        PARAM_DISTANT_DEPTH_VALUE,

        PARAM_TESSELLATION_HEIGHT,

        PARAM_CAMERA_DYNAMIC_RANGE,
        PARAM_CAMERA_TARGET_LUMINANCE,

        AUTOBIND_UNIFORMS_END,

        DYNAMIC_PARAMETERS_COUNT = AUTOBIND_UNIFORMS_END,
    };

    enum
    {
        UPDATE_SEMANTIC_ALWAYS = 0,
    };

    enum eTextureSemantic
    {
        DYNAMIC_TEXTURE_GLOBAL_REFLECTION = 0,
        DYNAMIC_TEXTURE_LOCAL_REFLECTION,
        DYNAMIC_TEXTURE_SHADOW_AO,
        DYNAMIC_TEXTURE_SRC_0,
        DYNAMIC_TEXTURE_SRC_1,
        DYNAMIC_TEXTURE_SRC_2,
        DYNAMIC_TEXTURE_SRC_3,
        DYNAMIC_TEXTURE_COUNT,
    };

public:
    DynamicBindings();

    static DynamicBindings::eUniformSemantic GetUniformSemanticByName(const FastName& name);

    void SetDynamicParam(eUniformSemantic shaderSemantic, const void* value, pointer_size updateSemantic);
    const void* GetDynamicParam(eUniformSemantic shaderSemantic);
    pointer_size GetDynamicParamUpdateSemantic(eUniformSemantic shaderSemantic);
    uint32 GetDynamicParamArraySize(eUniformSemantic shaderSemantic, uint32 defaultValue = 1);
    inline const Matrix4& GetDynamicParamMatrix(eUniformSemantic shaderSemantic);

    static const FastName& GetDynamicTextureName(eTextureSemantic textureSemantic);
    static bool IsDynamicTexture(const FastName& name);
    void SetDynamicTexture(eTextureSemantic textureSemantic, rhi::HTexture texture_);
    rhi::HTexture GetDynamicTexture(eTextureSemantic textureSemantic) const;
    void ClearDynamicTextures();

private:
    // dynamic variables
    struct AutobindVariableData
    {
        pointer_size updateSemantic; // Use lower 1 bit, for indication of update
        const void* value;
    };
    AutobindVariableData dynamicParameters[DYNAMIC_PARAMETERS_COUNT];
    uint64 dynamicParamersRequireUpdate;

    Matrix4 worldViewMatrix;
    Matrix4 viewProjMatrix;
    Matrix4 worldViewProjMatrix;
    Matrix4 invWorldViewMatrix;
    Matrix4 normalMatrix;
    Matrix4 invWorldMatrix;
    Matrix4 worldInvTransposeMatrix;
    Matrix4 shadowWorldViewProjMatrix;
    Vector3 worldScale;
    Vector3 boundingBoxSize;
    Vector3 worldViewObjectCenter;
    float32 projectionFlipped;
    float32 globalLuminanceScale = 1.0f;
    Vector4 defaultFogValues = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
    Vector4 defaultShadowAOParam = Vector4(0.f, 0.f, 1.f, 1.f);

    void ComputeWorldViewMatrixIfRequired();
    void ComputeWorldScaleIfRequired();
    void ComputeViewProjMatrixIfRequired();
    void ComputeWorldViewProjMatrixIfRequired();
    void ComputeInvWorldViewMatrixIfRequired();
    void ComputeWorldViewInvTransposeMatrixIfRequired();
    void ComputeLocalBoundingBoxSizeIfRequired();
    void ComputeWorldViewObjectCenterIfRequired();

    void ComputeInvWorldMatrixIfRequired();
    void ComputeWorldInvTransposeMatrixIfRequired();

    // dynamic textures
    rhi::HTexture dynamicTextureArray[DYNAMIC_TEXTURE_COUNT];
};

inline const Matrix4& DynamicBindings::GetDynamicParamMatrix(DynamicBindings::eUniformSemantic shaderSemantic)
{
    return *static_cast<const Matrix4*>(GetDynamicParam(shaderSemantic));
}

inline void DynamicBindings::SetDynamicTexture(eTextureSemantic textureSemantic, rhi::HTexture texture_)
{
    DVASSERT(texture_ != rhi::InvalidHandle);
    dynamicTextureArray[textureSemantic] = texture_;
}

inline rhi::HTexture DynamicBindings::GetDynamicTexture(eTextureSemantic textureSemantic) const
{
    return dynamicTextureArray[textureSemantic];
}

inline void DynamicBindings::ClearDynamicTextures()
{
    for (rhi::HTexture& h : dynamicTextureArray)
        h = rhi::HTexture();
}
}
#endif
